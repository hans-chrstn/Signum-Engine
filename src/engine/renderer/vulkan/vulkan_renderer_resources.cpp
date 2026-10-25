#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/vulkan/command/vulkan_image_barrier.hpp"
#include "engine/renderer/vulkan/memory/vulkan_upload.hpp"
#include "engine/renderer/vulkan/texture/vulkan_texture_address_mode.hpp"
#include "engine/renderer/vulkan/texture/vulkan_texture_filter.hpp"
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
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    auto VulkanRenderer::createMeshBufferResources(const MeshUploadData &mesh)
        -> MeshBufferResources {
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

        return new_mesh_buffers;
    }

    auto VulkanRenderer::registerMesh(const MeshUploadData &mesh)
        -> GpuResourceHandle {
        if (m_MeshVertexStride == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::registerMesh requires a configured vertex "
                "layout");
        }

        if (mesh.vertex_stride != m_MeshVertexStride) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::registerMesh requires a matching vertex "
                "stride");
        }

        MeshBufferResources resources = createMeshBufferResources(mesh);

        GpuResourceHandle handle = m_MeshRegistry.insert(std::move(resources));

        return handle;
    }

    auto VulkanRenderer::unregisterMesh(GpuResourceHandle handle) -> void {
        MeshBufferResources *resources = m_MeshRegistry.find(handle);

        if (resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::unregisterMesh requires a registered mesh "
                "handle");
        }

        if (m_ActiveMeshHandle.has_value() && *m_ActiveMeshHandle == handle) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::unregisterMesh cannot remove the active mesh");
        }

        const bool is_mesh_referenced = std::ranges::any_of(
            m_DrawInstances,
            [handle](const MeshDrawInstance &instance) -> bool {
                return instance.mesh_handle == handle;
            });

        if (is_mesh_referenced) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::unregisterMesh cannot remove a referenced "
                "mesh");
        }

        m_GraphicsResourceRetirementQueue.retire(
            m_GraphicsSubmissionState.lastSubmitted(), *resources);

        static_cast<void>(m_MeshRegistry.remove(handle));
    }

    auto VulkanRenderer::setDrawInstances(
        std::span<const MeshDrawInstance> instances) -> void {

        std::vector<MeshDrawInstance> draw_instances;
        draw_instances.reserve(instances.size());

        for (const MeshDrawInstance &instance : instances) {
            const MeshBufferResources *resources =
                m_MeshRegistry.find(instance.mesh_handle);

            if (resources == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer::setDrawInstances requires registered mesh "
                    "handles");
            }

            if (instance.material_handle.has_value()) {
                const MaterialInstanceResources *material_resources =
                    m_MaterialInstanceRegistry.find(*instance.material_handle);

                if (material_resources == nullptr) {
                    Core::Assertion::failAssertion(
                        Core::Assertion::AssertionType::Precondition,
                        Core::Error::Subsystem::Vulkan,
                        "VulkanRenderer::setDrawInstances requires registered "
                        "material instance handles");
                }
            }

            draw_instances.push_back(instance);
        }

        m_DrawInstances.swap(draw_instances);
    }

    auto VulkanRenderer::uploadMesh(const MeshUploadData &mesh) -> void {

        MeshBufferResources new_mesh_buffers = createMeshBufferResources(mesh);

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

        for (MeshDrawInstance &instance : m_DrawInstances) {
            if (instance.mesh_handle == previous_handle) {
                instance.mesh_handle = replacement_handle;
            }
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
        sampler_create_info.magFilter =
            toVkFilter(texture.sampler.magnification_filter);
        sampler_create_info.minFilter =
            toVkFilter(texture.sampler.minification_filter);
        sampler_create_info.addressModeU =
            toVkSamplerAddressMode(texture.sampler.address_mode_u);
        sampler_create_info.addressModeV =
            toVkSamplerAddressMode(texture.sampler.address_mode_v);
        sampler_create_info.addressModeW =
            toVkSamplerAddressMode(texture.sampler.address_mode_w);
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

    auto VulkanRenderer::registerTexture(const TextureUploadData &texture)
        -> GpuResourceHandle {
        TextureResources resources = uploadTexture(texture);

        GpuResourceHandle handle =
            m_TextureRegistry.insert(std::move(resources));

        return handle;
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

        const bool has_active_material_descriptors =
            findActiveMaterialDescriptorResources() != nullptr;

        if (has_active_material_descriptors &&
            !m_ActiveMaterialDescriptorHandle.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active material descriptor handle is missing "
                "during texture replacement");
        }

        const std::optional<GpuResourceHandle> previous_descriptor_handle =
            m_ActiveMaterialDescriptorHandle;

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

        const bool old_texture_still_referenced =
            std::ranges::count_if(
                m_MaterialBindings.bindings,
                [old_texture_handle](
                    const MaterialResourceBinding &candidate) -> bool {
                    return candidate.resource_handle == old_texture_handle;
                }) > 1;

        const bool old_texture_has_other_dependents =
            previous_descriptor_handle.has_value()
                ? m_ResourceDependencies.hasDependentsOtherThan({
                      .dependent = *previous_descriptor_handle,
                      .dependency = old_texture_handle,
                  })
                : m_ResourceDependencies.hasDependents(old_texture_handle);

        const bool retire_old_texture =
            !old_texture_still_referenced && !old_texture_has_other_dependents;

        TextureResources new_texture_resources = uploadTexture(texture);

        GpuResourceHandle new_texture_handle =
            m_TextureRegistry.insert(std::move(new_texture_resources));

        const MaterialResourceBinding resource_binding{
            .set = 0U,
            .binding = 0U,
            .array_element = 0U,
            .type = Shader::ShaderResourceType::CombinedImageSampler,
            .resource_handle = new_texture_handle,
        };

        std::optional<TextureBindingResources> new_texture_binding;
        std::optional<GpuResourceHandle> replacement_descriptor_handle;

        try {
            new_texture_binding.emplace(createTextureBindingResources(
                resource_binding, descriptor_layout));

            if (has_active_material_descriptors) {
                std::vector<MaterialResourceBinding> new_material_bindings =
                    m_MaterialBindings.bindings;

                const auto replacement_binding_it = std::ranges::find_if(
                    new_material_bindings,
                    [](const MaterialResourceBinding &binding) -> bool {
                        return binding.set == 0U && binding.binding == 0U &&
                               binding.array_element == 0U &&
                               binding.type == Shader::ShaderResourceType::
                                                   CombinedImageSampler;
                    });

                if (replacement_binding_it == new_material_bindings.end()) {
                    Core::Assertion::failAssertion(
                        Core::Assertion::AssertionType::Invariant,
                        Core::Error::Subsystem::Vulkan,
                        "VulkanRenderer replacement material texture binding "
                        "was not found");
                }

                *replacement_binding_it = resource_binding;

                replacement_descriptor_handle.emplace(
                    registerMaterialDescriptorResources(new_material_bindings));
            }
        } catch (...) {
            new_texture_binding.reset();

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
            retireTextureReplacementResources(*old_texture_resources,
                                              active_texture_resources,
                                              retire_old_texture);
        } catch (...) {
            if (replacement_descriptor_handle.has_value()) {
                rollbackRegisteredMaterialDescriptorResources(
                    *replacement_descriptor_handle);
            }

            new_texture_binding.reset();

            static_cast<void>(m_TextureRegistry.remove(new_texture_handle));

            throw;
        }

        if (previous_descriptor_handle.has_value()) {
            static_cast<void>(
                m_ResourceDependencies.removeRelationshipsInvolving(
                    *previous_descriptor_handle));

            static_cast<void>(m_MaterialDescriptorRegistry.remove(
                *previous_descriptor_handle));
        }

        if (retire_old_texture) {
            static_cast<void>(m_TextureRegistry.remove(old_texture_handle));
        }

        m_TextureBindingResources.emplace(std::move(*new_texture_binding));

        if (replacement_descriptor_handle.has_value()) {
            m_ActiveMaterialDescriptorHandle.emplace(
                *replacement_descriptor_handle);
        }

        *texture_binding_it = resource_binding;
    }

    auto VulkanRenderer::registerTextureReplacementMaterialDescriptors(
        const MaterialResourceBinding &replacement_binding,
        bool has_active_material_descriptors)
        -> std::optional<GpuResourceHandle> {

        if (!has_active_material_descriptors) {
            return std::nullopt;
        }

        std::vector<MaterialResourceBinding> bindings =
            m_MaterialBindings.bindings;

        const auto binding_it = std::ranges::find_if(
            bindings, [](const MaterialResourceBinding &candidate) -> bool {
                return candidate.set == 0U && candidate.binding == 0U &&
                       candidate.array_element == 0U &&
                       candidate.type ==
                           Shader::ShaderResourceType::CombinedImageSampler;
            });

        if (binding_it == bindings.end()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer replacement material texture binding "
                "was not found");
        }

        *binding_it = replacement_binding;

        return registerMaterialDescriptorResources(bindings);
    }

    auto VulkanRenderer::retireTextureReplacementResources(
        TextureResources &previous_texture,
        TextureBindingResources &previous_binding, bool retire_texture)
        -> void {

        const std::uint64_t submission_id =
            m_GraphicsSubmissionState.lastSubmitted();

        MaterialDescriptorResources *previous_descriptors =
            findActiveMaterialDescriptorResources();

        if (retire_texture && previous_descriptors != nullptr) {
            m_GraphicsResourceRetirementQueue.retire(
                submission_id, previous_texture, previous_binding,
                *previous_descriptors);
        } else if (retire_texture) {
            m_GraphicsResourceRetirementQueue.retire(
                submission_id, previous_texture, previous_binding);
        } else if (previous_descriptors != nullptr) {
            m_GraphicsResourceRetirementQueue.retire(
                submission_id, previous_binding, *previous_descriptors);
        } else {
            m_GraphicsResourceRetirementQueue.retire(submission_id,
                                                     previous_binding);
        }
    }

    auto VulkanRenderer::setDrawInstanceModelMatrix(
        std::size_t instance_index, const Core::Math::Matrix4f &model_matrix)
        -> void {
        if (instance_index >= m_DrawInstances.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::setDrawInstanceModelMatrix requires a valid "
                "draw instance index");
        }

        m_DrawInstances[instance_index].model_matrix = model_matrix;
    }
} // namespace SNE::Engine::Renderer::Vulkan
