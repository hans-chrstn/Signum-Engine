#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/vulkan/command/vulkan_image_barrier.hpp"
#include "engine/renderer/vulkan/memory/vulkan_upload.hpp"
#include "engine/renderer/vulkan/texture/vulkan_texture_format.hpp"
#include "vulkan_renderer.hpp"
#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    auto VulkanRenderer::uploadMesh(const MeshUploadData &mesh) -> void {
        const std::optional<std::uint32_t> mesh_indices_size =
            Core::Numeric::tryConvertToUint32(mesh.indices.size());

        if (!mesh_indices_size.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan mesh upload index count exceeds uint32_t");
        }

        if (mesh.vertex_bytes.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan mesh upload requires nonempty vertex data");
        }

        if (mesh.indices.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan mesh upload requires nonempty index data");
        }

        if (mesh.vertex_stride == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan mesh upload requires a nonzero vertex stride");
        }

        const std::size_t byte_count = mesh.vertex_bytes.size();

        if ((byte_count % mesh.vertex_stride) != 0) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan mesh upload requires vertex data aligned to its "
                "stride");
        }

        const std::size_t vertex_count = byte_count / mesh.vertex_stride;

        for (const std::uint16_t index : mesh.indices) {
            if (static_cast<std::size_t>(index) >= vertex_count) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Vulkan mesh upload index references a vertex outside the "
                    "mesh");
            }
        }

        const VulkanBufferCreateInfo buffer_create_info{
            .size = static_cast<VkDeviceSize>(byte_count),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            .memory_usage = GpuMemoryUsage::Device,
        };

        VulkanBuffer new_vertex_buffer =
            VulkanBuffer(m_MemoryAllocator, buffer_create_info);

        uploadBufferData(m_MemoryAllocator, m_ImmediateSubmission,
                         new_vertex_buffer, mesh.vertex_bytes);

        const VulkanBufferCreateInfo index_buffer_create_info{
            .size = static_cast<VkDeviceSize>(mesh.indices.size_bytes()),
            .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT |
                     VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            .memory_usage = GpuMemoryUsage::Device,
        };

        const auto index_bytes = std::as_bytes(mesh.indices);

        VulkanBuffer new_index_buffer =
            VulkanBuffer(m_MemoryAllocator, index_buffer_create_info);
        uploadBufferData(m_MemoryAllocator, m_ImmediateSubmission,
                         new_index_buffer, index_bytes);

        MeshBufferResources new_mesh_buffers{
            .m_VertexBuffer = std::move(new_vertex_buffer),
            .m_IndexBuffer = std::move(new_index_buffer),
            .m_IndexCount = mesh_indices_size.value(),
        };

        if (!m_ActiveMeshHandle.has_value()) {
            m_ActiveMeshHandle =
                m_MeshRegistry.insert(std::move(new_mesh_buffers));

            return;
        }

        const GpuResourceHandle previous_handle = m_ActiveMeshHandle.value();

        const GpuResourceHandle replacement_handle =
            m_MeshRegistry.insert(std::move(new_mesh_buffers));

        MeshBufferResources *previous_mesh =
            m_MeshRegistry.find(previous_handle);

        if (previous_mesh == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer previous mesh handle does not resolve to a "
                "registered mesh");
        }

        try {
            m_GraphicsResourceRetirementQueue.retire(
                m_GraphicsSubmissionState.lastSubmitted(), *previous_mesh);
        } catch (...) {
            static_cast<void>(m_MeshRegistry.remove(replacement_handle));
            throw;
        }
        static_cast<void>(m_MeshRegistry.remove(previous_handle));

        m_ActiveMeshHandle.emplace(replacement_handle);
    }

    auto VulkanRenderer::replaceMesh(const MeshUploadData &mesh) -> void {
        if (!m_ActiveMeshHandle.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::replaceMesh requires an existing mesh");
        }

        if (mesh.vertex_stride != m_MeshVertexStride) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::replaceMesh requires a matching vertex "
                "stride");
        }

        uploadMesh(mesh);
    }

    auto VulkanRenderer::uploadTexture(const TextureUploadData &texture)
        -> TextureResources {
        if (texture.width == 0U || texture.height == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan texture upload requires nonzero width and height");
        }

        const auto maximum_mip_levels = static_cast<std::uint32_t>(
            std::bit_width(std::max(texture.width, texture.height)));

        if (texture.mip_levels == 0U ||
            texture.mip_levels > maximum_mip_levels) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan texture upload requires a valid mip-level count");
        }

        const std::size_t max = std::numeric_limits<std::size_t>::max();
        constexpr std::size_t kRgba8BytesPerPixel = 4U;

        std::size_t total_byte_count{};

        for (std::uint32_t level{}; level < texture.mip_levels; ++level) {
            const auto mip_width =
                static_cast<std::size_t>(std::max(1U, texture.width >> level));

            const auto mip_height =
                static_cast<std::size_t>(std::max(1U, texture.height >> level));

            if (mip_width > max / mip_height) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Vulkan texture upload mip pixel count exceeds size_t");
            }

            const std::size_t mip_pixel_count = mip_width * mip_height;

            if (mip_pixel_count > max / kRgba8BytesPerPixel) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Vulkan texture upload mip byte count exceeds size_t");
            }

            const std::size_t mip_byte_count =
                mip_pixel_count * kRgba8BytesPerPixel;

            if (mip_byte_count > max - total_byte_count) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Vulkan texture upload total mip byte count exceeds "
                    "size_t");
            }

            total_byte_count += mip_byte_count;
        }

        if (texture.pixels.size() != total_byte_count) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan texture upload pixel data size does not match mip "
                "chain");
        }

        const VkFormat native_format = toVkFormat(texture.format);

        VulkanImageCreateInfo image_create_info{};
        image_create_info.extent = {
            .width = texture.width,
            .height = texture.height,
            .depth = 1U,
        };

        image_create_info.image_type = VK_IMAGE_TYPE_2D;

        image_create_info.format = native_format;
        image_create_info.mip_levels = texture.mip_levels;

        image_create_info.usage =
            static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_TRANSFER_DST_BIT) |
            static_cast<VkImageUsageFlags>(VK_IMAGE_USAGE_SAMPLED_BIT);

        VulkanImage image{m_MemoryAllocator, image_create_info};

        uploadImageData(m_MemoryAllocator, m_ImmediateSubmission, image,
                        texture.pixels);

        VkImageMemoryBarrier2 memory_barrier{};
        memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        memory_barrier.image = image.nativeHandle();
        memory_barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        memory_barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        memory_barrier.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT;
        memory_barrier.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT;
        memory_barrier.dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT;
        memory_barrier.dstAccessMask = VK_ACCESS_2_SHADER_SAMPLED_READ_BIT;
        memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memory_barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = texture.mip_levels,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        m_ImmediateSubmission.execute(
            [&](VkCommandBuffer command_buffer) -> void {
                recordImageBarrier(command_buffer, memory_barrier);
            });

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = image.nativeHandle();
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = native_format;
        image_view_create_info.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = texture.mip_levels,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        VulkanImageView image_view{m_Device.nativeHandle(),
                                   image_view_create_info};

        VkSamplerCreateInfo sampler_create_info{};
        sampler_create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_create_info.magFilter = VK_FILTER_NEAREST;
        sampler_create_info.minFilter = VK_FILTER_NEAREST;
        sampler_create_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        sampler_create_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        sampler_create_info.minLod = 0.0F;
        sampler_create_info.maxLod =
            static_cast<float>(texture.mip_levels - 1U);

        VulkanSampler sampler{m_Device.nativeHandle(), sampler_create_info};

        TextureResources texture_resources{
            .m_Image = std::move(image),
            .m_ImageView = std::move(image_view),
            .m_Sampler = std::move(sampler),
        };

        return texture_resources;
    }

    auto VulkanRenderer::replaceTexture(const TextureUploadData &texture)
        -> void {
        if (!m_TextureBindingResources.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::replaceTexture requires an existing texture "
                "binding");
        }

        if (!m_TextureDescriptorSetLayout.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::replaceTexture requires an existing "
                "descriptor-set layout");
        }

        const VulkanDescriptorSetLayout &descriptor_layout =
            m_TextureDescriptorSetLayout.value();

        TextureBindingResources &active_texture_resources =
            m_TextureBindingResources.value();

        auto texture_binding_it = std::ranges::find_if(
            m_MaterialBindings.bindings,
            [](const MaterialResourceBinding &binding) -> bool {
                return binding.set == 0U && binding.binding == 0U &&
                       binding.array_element == 0U &&
                       binding.type ==
                           Shader::ShaderResourceType::CombinedImageSampler;
            });

        if (texture_binding_it == m_MaterialBindings.bindings.end()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active material texture binding was not found");
        }

        const GpuResourceHandle old_texture_handle =
            texture_binding_it->resource_handle;

        if (old_texture_handle != active_texture_resources.m_TextureHandle) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active texture handle does not match its "
                "descriptor binding");
        }

        TextureResources new_texture_resources = uploadTexture(texture);

        GpuResourceHandle new_texture_handle =
            m_TextureRegistry.insert(std::move(new_texture_resources));

        std::optional<TextureBindingResources> new_texture_binding;

        const MaterialResourceBinding resource_binding{
            .set = 0U,
            .binding = 0U,
            .array_element = 0U,
            .type = Shader::ShaderResourceType::CombinedImageSampler,
            .resource_handle = new_texture_handle,
        };

        try {
            new_texture_binding.emplace(createTextureBindingResources(
                resource_binding, descriptor_layout));

        } catch (...) {
            static_cast<void>(m_TextureRegistry.remove(new_texture_handle));
            throw;
        }

        TextureResources *old_texture_resources =
            m_TextureRegistry.find(old_texture_handle);

        if (old_texture_resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active texture handle does not resolve to a "
                "registered texture");
        }

        try {
            m_GraphicsResourceRetirementQueue.retire(
                m_GraphicsSubmissionState.lastSubmitted(),
                *old_texture_resources, active_texture_resources);
        } catch (...) {
            new_texture_binding.reset();
            static_cast<void>(m_TextureRegistry.remove(new_texture_handle));
            throw;
        }

        static_cast<void>(m_TextureRegistry.remove(old_texture_handle));

        m_TextureBindingResources.emplace(
            std::move(new_texture_binding.value()));

        *texture_binding_it = resource_binding;
    }
} // namespace SNE::Engine::Renderer::Vulkan
