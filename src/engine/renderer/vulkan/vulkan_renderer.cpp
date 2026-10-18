#include "vulkan_renderer.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/gpu_memory_usage.hpp"
#include "engine/renderer/material/material_resource_binding_compatibility.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage_input.hpp"
#include "engine/renderer/vulkan/command/vulkan_image_barrier.hpp"
#include "engine/renderer/vulkan/common/vulkan_api_version.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_updates.hpp"
#include "engine/renderer/vulkan/device/vulkan_depth_format.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_requests.hpp"
#include "engine/renderer/vulkan/geometry/vulkan_vertex_input.hpp"
#include "engine/renderer/vulkan/memory/vulkan_buffer.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image.hpp"
#include "engine/renderer/vulkan/memory/vulkan_image_view.hpp"
#include "engine/renderer/vulkan/memory/vulkan_sampler.hpp"
#include "engine/renderer/vulkan/memory/vulkan_upload.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_reflection.hpp"
#include "engine/renderer/vulkan/presentation/fail_closed_replacement.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain_result_policy.hpp"
#include "engine/renderer/vulkan/synchronization/vulkan_semaphore.hpp"
#include "engine/renderer/vulkan/texture/vulkan_texture_format.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Error = SNE::Engine::Core::Error;
namespace Math = SNE::Engine::Core::Math;

namespace {
    constexpr std::size_t kMatrixSize = 64U;
    constexpr std::size_t kMatrixOffset = 0U;

    static_assert(sizeof(Math::Matrix4f) == kMatrixSize);
    static_assert(offsetof(Math::Matrix4f, elements) == kMatrixOffset);
    static_assert(std::is_trivially_copyable_v<Math::Matrix4f>);

    [[nodiscard]] auto selectRequiredPhysicalDevice(VkInstance instance,
                                                    VkSurfaceKHR surface)
        -> Vulkan::SelectedPhysicalDevice {
        const std::vector<VkPhysicalDevice> available_devices =
            Vulkan::enumeratePhysicalDevices(instance);

        const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices =
            Vulkan::inspectPhysicalDevices(available_devices, surface);

        std::optional<Vulkan::SelectedPhysicalDevice> selected_device =
            Vulkan::selectPhysicalDevice(discovered_devices);

        if (!selected_device.has_value()) {
            throw Error::EngineError(
                Error::Code::VulkanSuitablePhysicalDeviceUnavailable,
                "No suitable Vulkan physical device is available",
                "Obtain Vulkan Suitable Physical Device");
        }

        return selected_device.value();
    }

    constexpr std::size_t kFramesInFlight = 2;
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    VulkanRenderer::VulkanRenderer(
        const std::string &application_name, const Platform::Window &window,
        PresentationPreference presentation_preference,
        bool development_diagnostics_enabled,
        std::optional<InitialMeshData> initial_mesh,
        std::optional<TextureUploadData> initial_texture,
        std::optional<InitialCameraData> initial_camera,
        std::optional<Shader::GraphicsShaderData> initial_graphics_shaders)
        : m_Window(&window),
          m_Instance(application_name, development_diagnostics_enabled),
          m_Surface(m_Instance.nativeHandle(), window.nativeHandle()),
          m_PhysicalDevice(selectRequiredPhysicalDevice(
              m_Instance.nativeHandle(), m_Surface.nativeHandle())),
          m_PresentationPreference(presentation_preference),
          m_LogicalDeviceFeatureConfiguration(
              deriveLogicalDeviceFeatureConfiguration(
                  deriveLogicalDeviceFeatureRequest(presentation_preference),
                  m_PhysicalDevice.capabilities)),
          m_Device(
              m_PhysicalDevice.handle, m_PhysicalDevice.queue_families,
              deriveUniqueQueueFamilyRequests(m_PhysicalDevice.queue_families),
              m_LogicalDeviceFeatureConfiguration,
              m_PhysicalDevice.capabilities.optional_capabilities),
          m_ImmediateSubmission(
              m_Device.nativeHandle(), m_Device.graphicsQueue(),
              m_PhysicalDevice.queue_families.graphics_family.family_index),
          m_MemoryAllocator(m_Instance.nativeHandle(), m_PhysicalDevice.handle,
                            m_Device.nativeHandle(), kRequiredApiVersion,
                            m_PhysicalDevice.capabilities.optional_capabilities
                                .memory_budget_extension_supported),
          m_InitialCamera(initial_camera),
          m_GraphicsSubmissionState(kFramesInFlight) {
        const std::uint32_t graphics_queue_family_index =
            m_PhysicalDevice.queue_families.graphics_family.family_index;

        m_FrameResources.reserve(kFramesInFlight);

        for (std::size_t i{}; i < kFramesInFlight; ++i) {
            m_FrameResources.emplace_back(m_Device.nativeHandle(),
                                          graphics_queue_family_index);
        }

        if (initial_mesh.has_value()) {
            m_MeshVertexAttributes.assign(
                initial_mesh->layout.attributes.begin(),
                initial_mesh->layout.attributes.end());

            m_MeshVertexStride = initial_mesh->mesh.vertex_stride;

            m_ModelMatrices.assign(initial_mesh->model_matrices.begin(),
                                   initial_mesh->model_matrices.end());

            uploadMesh(initial_mesh->mesh);
        }

        if (initial_texture.has_value()) {
            VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
            descriptor_set_layout_binding.binding = 0U;
            descriptor_set_layout_binding.descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptor_set_layout_binding.descriptorCount = 1U;
            descriptor_set_layout_binding.stageFlags =
                VK_SHADER_STAGE_FRAGMENT_BIT;

            VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
            descriptor_set_layout_create_info.sType =
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            descriptor_set_layout_create_info.bindingCount = 1U;
            descriptor_set_layout_create_info.pBindings =
                &descriptor_set_layout_binding;

            const VulkanDescriptorSetLayout &descriptor_layout =
                m_TextureDescriptorSetLayout.emplace(
                    m_Device.nativeHandle(), descriptor_set_layout_create_info);

            TextureResources texture_resources =
                uploadTexture(initial_texture.value());

            GpuResourceHandle texture_handle =
                m_TextureRegistry.insert(std::move(texture_resources));

            const MaterialResourceBinding material_binding{
                .set = 0U,
                .binding = 0U,
                .array_element = 0U,
                .type = Shader::ShaderResourceType::CombinedImageSampler,
                .resource_handle = texture_handle,
            };

            m_TextureBindingResources.emplace(createTextureBindingResources(
                material_binding, descriptor_layout));

            m_MaterialBindings.bindings.push_back(material_binding);
        }

        if (initial_graphics_shaders.has_value()) {
            m_Material.emplace(MaterialDescription{
                .shaders = std::move(initial_graphics_shaders.value()),
                .pipeline_state = {},
            });

            const Shader::ShaderStageInput &fragment =
                m_Material->shaders.fragment;

            const Shader::ShaderInterface fragment_shader_interface =
                reflectSpirvInterface(fragment.artifact.words,
                                      fragment.selection);

            const bool fragment_material_compatible =
                areMaterialResourceBindingsCompatible(
                    m_MaterialBindings.bindings,
                    fragment_shader_interface.descriptor_bindings);

            if (!fragment_material_compatible) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer initial material bindings do not satisfy "
                    "fragment shader descriptor requirements");
            }

            const Shader::ShaderStageInput &vertex = m_Material->shaders.vertex;

            const Shader::ShaderInterface vertex_shader_interface =
                reflectSpirvInterface(vertex.artifact.words, vertex.selection);

            const bool vertex_material_compatible =
                areMaterialResourceBindingsCompatible(
                    m_MaterialBindings.bindings,
                    vertex_shader_interface.descriptor_bindings);

            if (!vertex_material_compatible) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer initial material bindings do not satisfy "
                    "vertex shader descriptor requirements");
            }
        }

        if (isFramebufferDrawable()) {
            static_cast<void>(recreateSwapchainResources());
        }
    }

    VulkanRenderer::~VulkanRenderer() noexcept {
        const VkResult result = vkDeviceWaitIdle(m_Device.nativeHandle());

        if (result != VK_SUCCESS) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer failed to confirm GPU completion during "
                "shutdown");
        }

        for (std::size_t i{}; i < m_FrameResources.size(); ++i) {
            m_GraphicsSubmissionState.recordCompletion(i);
        }

        m_GraphicsResourceRetirementQueue.collectCompleted(
            m_GraphicsSubmissionState);
    }

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

    auto VulkanRenderer::createTextureBindingResources(
        const MaterialResourceBinding &resource_binding,
        const VulkanDescriptorSetLayout &layout) -> TextureBindingResources {
        if (resource_binding.set != 0U || resource_binding.binding != 0U ||
            resource_binding.array_element != 0U ||
            resource_binding.type !=
                Shader::ShaderResourceType::CombinedImageSampler) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::createTextureBindingResources requires a "
                "supported sampled-texture material binding");
        }

        const GpuResourceHandle &texture_handle =
            resource_binding.resource_handle;

        const TextureResources *texture_resources =
            m_TextureRegistry.find(texture_handle);

        if (texture_resources == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer::createTextureBindingResources requires a "
                "registered texture handle");
        }

        VkDescriptorPoolSize descriptor_pool_size{};
        descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_pool_size.descriptorCount = 1U;

        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.maxSets = 1U;
        descriptor_pool_create_info.poolSizeCount = 1U;
        descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;

        VulkanDescriptorPool descriptor_pool{m_Device.nativeHandle(),
                                             descriptor_pool_create_info};

        std::array<VkDescriptorSetLayout, 1> layouts{
            layout.nativeHandle(),
        };

        std::vector<VkDescriptorSet> descriptor_sets =
            descriptor_pool.allocateDescriptorSets(layouts);

        VkDescriptorImageInfo descriptor_image_info{};
        descriptor_image_info.sampler =
            texture_resources->m_Sampler.nativeHandle();
        descriptor_image_info.imageView =
            texture_resources->m_ImageView.nativeHandle();
        descriptor_image_info.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkWriteDescriptorSet write_descriptor_set{};
        write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write_descriptor_set.dstSet = descriptor_sets.front();
        write_descriptor_set.dstBinding = 0U;
        write_descriptor_set.dstArrayElement = 0U;
        write_descriptor_set.descriptorCount = 1U;
        write_descriptor_set.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write_descriptor_set.pImageInfo = &descriptor_image_info;

        std::array<VkWriteDescriptorSet, 1> writes{
            write_descriptor_set,
        };

        updateDescriptorSets(m_Device.nativeHandle(), writes);

        return TextureBindingResources{
            .m_DescriptorPool = std::move(descriptor_pool),
            .m_DescriptorSet = descriptor_sets.front(),
            .m_TextureHandle = texture_handle,
        };
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

        if (m_MaterialBindings.bindings.size() != 1U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires exactly one active material texture "
                "binding when replacing a texture");
        }

        const GpuResourceHandle old_texture_handle =
            m_MaterialBindings.bindings.front().resource_handle;

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

        m_MaterialBindings.bindings.front() = resource_binding;
    }

    auto VulkanRenderer::isFramebufferDrawable() const -> bool {
        const Platform::FramebufferSize framebuffer_size =
            m_Window->framebufferSize();
        return framebuffer_size.width > 0 && framebuffer_size.height > 0;
    }

    auto VulkanRenderer::waitForDrawableFramebuffer() const -> bool {
        while (!isFramebufferDrawable() && !m_Window->shouldClose()) {
            Platform::Window::waitEvents();
        }

        return isFramebufferDrawable();
    }

    auto VulkanRenderer::recreateSwapchainResources() -> bool {
        if (!waitForDrawableFramebuffer()) {
            return false;
        }

        m_Device.waitIdle();

        for (std::size_t i{}; i < m_FrameResources.size(); ++i) {
            m_GraphicsSubmissionState.recordCompletion(i);
        }

        m_GraphicsResourceRetirementQueue.collectCompleted(
            m_GraphicsSubmissionState);

        VkSwapchainKHR old_swapchain = VK_NULL_HANDLE;
        if (m_SwapchainResources.has_value()) {
            old_swapchain =
                m_SwapchainResources.value().m_VulkanSwapchain.nativeHandle();
        }

        const Platform::FramebufferSize new_framebuffer_size =
            m_Window->framebufferSize();

        replaceRetiringResource(
            m_SwapchainResources, [&] -> SwapchainResources {
                VulkanSwapchain new_swapchain = VulkanSwapchain(
                    m_PhysicalDevice.handle, m_Device.nativeHandle(),
                    m_Surface.nativeHandle(), m_PhysicalDevice.queue_families,
                    new_framebuffer_size, m_PresentationPreference,
                    m_LogicalDeviceFeatureConfiguration
                            .fifo_latest_ready_feature
                            .presentModeFifoLatestReady == VK_TRUE,
                    old_swapchain);

                const std::optional<VkFormat> depth_format =
                    selectSupportedDepthFormat(m_PhysicalDevice.handle);

                if (!depth_format.has_value()) {
                    throw Core::Error::EngineError(
                        Core::Error::Code::VulkanDepthFormatUnavailable,
                        "No supported Vulkan depth attachment format available",
                        "Select Vulkan Depth Format");
                }

                const VulkanImageCreateInfo depth_image_create_info{
                    .extent =
                        {
                            .width = new_swapchain.extent().width,
                            .height = new_swapchain.extent().height,
                            .depth = 1U,
                        },
                    .image_type = VK_IMAGE_TYPE_2D,
                    .format = depth_format.value(),
                    .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                };

                std::vector<DepthAttachmentResources> new_depth_attachments;
                new_depth_attachments.reserve(m_FrameResources.size());

                for (std::size_t i{}; i < m_FrameResources.size(); ++i) {
                    VulkanImage depth_image{m_MemoryAllocator,
                                            depth_image_create_info};

                    VkImageViewCreateInfo depth_view_create_info{};
                    depth_view_create_info.sType =
                        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                    depth_view_create_info.image = depth_image.nativeHandle();
                    depth_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                    depth_view_create_info.format = depth_format.value();
                    depth_view_create_info.subresourceRange.aspectMask =
                        VK_IMAGE_ASPECT_DEPTH_BIT;
                    depth_view_create_info.subresourceRange.baseMipLevel = 0U;
                    depth_view_create_info.subresourceRange.levelCount = 1U;
                    depth_view_create_info.subresourceRange.baseArrayLayer = 0U;
                    depth_view_create_info.subresourceRange.layerCount = 1U;

                    VulkanImageView depth_image_view{m_Device.nativeHandle(),
                                                     depth_view_create_info};

                    new_depth_attachments.emplace_back(DepthAttachmentResources{
                        .m_Image = std::move(depth_image),
                        .m_ImageView = std::move(depth_image_view),
                    });
                }

                const VulkanGraphicsPipeline *new_graphics_pipeline{nullptr};

                if (m_ActiveMeshHandle.has_value() &&
                    (!m_SwapchainResources.has_value() ||
                     new_swapchain.surfaceFormat().format !=
                         m_SwapchainResources->m_VulkanSwapchain.surfaceFormat()
                             .format ||
                     depth_format.value() !=
                         m_SwapchainResources->m_DepthFormat)) {

                    if (!m_Material.has_value()) {
                        Core::Assertion::failAssertion(
                            Core::Assertion::AssertionType::Precondition,
                            Core::Error::Subsystem::Vulkan,
                            "VulkanRenderer requires a material configuration "
                            "when rendering a mesh");
                    }

                    const Shader::GraphicsShaderData &graphics_data =
                        m_Material->shaders;

                    const VertexLayoutDescription vertex_layout{
                        .attributes = m_MeshVertexAttributes,
                    };

                    const VulkanVertexInputDescriptions
                        vertex_input_descriptions =
                            makeVulkanVertexInputDescriptions(
                                vertex_layout, m_MeshVertexStride);

                    const std::array<VkVertexInputBindingDescription, 1>
                        vertex_bindings{
                            vertex_input_descriptions.binding,
                    };

                    const GraphicsVertexInputData vertex_input_data{
                        .bindings = vertex_bindings,
                        .attributes = vertex_input_descriptions.attributes,
                    };

                    const std::array<VkPushConstantRange, 1>
                        push_constant_ranges{
                            {
                                {
                                    .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                                    .offset = 0U,
                                    .size = static_cast<std::uint32_t>(
                                        sizeof(Math::Matrix4f)),
                                },
                            },
                    };

                    std::array<const VulkanDescriptorSetLayout *, 1>
                        texture_layouts{};

                    std::span<const VulkanDescriptorSetLayout *const>
                        descriptor_set_layouts{};

                    if (m_TextureDescriptorSetLayout.has_value()) {
                        const auto &owned_layout =
                            m_TextureDescriptorSetLayout.value();
                        texture_layouts[0] = &owned_layout;
                        descriptor_set_layouts = texture_layouts;
                    }

                    const GraphicsPipelineLayoutData
                        graphics_pipeline_layout_data{
                            .push_constant_ranges = push_constant_ranges,
                            .descriptor_set_layouts = descriptor_set_layouts,
                    };

                    const GraphicsAttachmentFormats attachment_formats{
                        .color = new_swapchain.surfaceFormat().format,
                        .depth = depth_format.value(),
                    };

                    new_graphics_pipeline = m_GraphicsPipelineCache.getOrCreate(
                        m_Device.nativeHandle(), attachment_formats,
                        graphics_data, vertex_input_data,
                        graphics_pipeline_layout_data,
                        m_PhysicalDevice.capabilities.properties.limits
                            .maxPushConstantsSize,
                        m_Material->pipeline_state);
                }

                std::vector<VulkanSemaphore> new_render_finished_semaphores;
                new_render_finished_semaphores.reserve(
                    new_swapchain.images().size());

                for (std::size_t i{}; i < new_swapchain.images().size(); ++i) {
                    new_render_finished_semaphores.emplace_back(
                        m_Device.nativeHandle());
                }

                if (new_graphics_pipeline == nullptr &&
                    m_SwapchainResources.has_value()) {
                    new_graphics_pipeline =
                        m_SwapchainResources->m_VulkanGraphicsPipeline;
                }

                return SwapchainResources{
                    .m_VulkanSwapchain = std::move(new_swapchain),
                    .m_DepthFormat = depth_format.value(),
                    .m_VulkanGraphicsPipeline = new_graphics_pipeline,
                    .m_VulkanSemaphores =
                        std::move(new_render_finished_semaphores),
                    .m_DepthAttachments = std::move(new_depth_attachments),
                };
            });

        return true;
    }

    auto VulkanRenderer::recordMeshDraws(
        VkCommandBuffer command_buffer,
        const SwapchainResources &swapchain_resources) const -> void {

        const VulkanSwapchain &swapchain =
            swapchain_resources.m_VulkanSwapchain;

        if (swapchain_resources.m_VulkanGraphicsPipeline == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires a graphics pipeline when drawing");
        }

        if (!m_ActiveMeshHandle.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires an active mesh handle before "
                "recording draw commands");
        }

        const MeshBufferResources *active_mesh =
            m_MeshRegistry.find(m_ActiveMeshHandle.value());

        if (active_mesh == nullptr) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer active mesh handle does not resolve to a "
                "registered mesh");
        }

        const MeshBufferResources &mesh = *active_mesh;

        const VulkanGraphicsPipeline &graphics_pipeline =
            *swapchain_resources.m_VulkanGraphicsPipeline;

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphics_pipeline.nativeHandle());

        if (m_TextureBindingResources.has_value()) {
            if (m_MaterialBindings.bindings.size() != 1U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer requires exactly one active material "
                    "texture binding when drawing");
            }

            const GpuResourceHandle &resource_handle =
                m_MaterialBindings.bindings.front().resource_handle;

            if (resource_handle != m_TextureBindingResources->m_TextureHandle) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer active texture handle does not match its "
                    "descriptor binding");
            }

            const TextureResources *active_texture =
                m_TextureRegistry.find(resource_handle);

            if (active_texture == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer active texture handle does not resolve to "
                    "a registered texture");
            }

            vkCmdBindDescriptorSets(
                command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphics_pipeline.layoutHandle(), 0U, 1U,
                &m_TextureBindingResources->m_DescriptorSet, 0U, nullptr);
        }

        const VkDeviceSize vertex_offset = 0U;
        const VkBuffer vertex_buffer = mesh.m_VertexBuffer.nativeHandle();
        vkCmdBindVertexBuffers(command_buffer, 0U, 1U, &vertex_buffer,
                               &vertex_offset);

        const VkBuffer index_buffer = mesh.m_IndexBuffer.nativeHandle();
        vkCmdBindIndexBuffer(command_buffer, index_buffer, 0U,
                             VK_INDEX_TYPE_UINT16);

        VkViewport viewport{};
        viewport.x = 0.0F;
        viewport.y = static_cast<float>(swapchain.extent().height);
        viewport.width = static_cast<float>(swapchain.extent().width);
        viewport.height = -static_cast<float>(swapchain.extent().height);
        viewport.minDepth = 0.0F;
        viewport.maxDepth = 1.0F;
        vkCmdSetViewport(command_buffer, 0U, 1U, &viewport);

        VkRect2D scissor{};
        scissor.extent = swapchain.extent();
        scissor.offset.x = 0;
        scissor.offset.y = 0;
        vkCmdSetScissor(command_buffer, 0U, 1U, &scissor);

        Math::Matrix4f view_projection = Math::identity();

        if (m_InitialCamera.has_value()) {
            const float aspect_ratio =
                static_cast<float>(swapchain.extent().width) /
                static_cast<float>(swapchain.extent().height);

            const Math::PerspectiveProjectionParameters projection_parameters{
                .fov_y_radians = m_InitialCamera->fov_y_radians,
                .aspect_ratio = aspect_ratio,
                .near_plane = m_InitialCamera->near_plane,
                .far_plane = m_InitialCamera->far_plane,
            };

            const Math::Matrix4f perspective =
                Math::perspectiveRH_ZO(projection_parameters);

            view_projection =
                Math::multiply(perspective, m_InitialCamera->view);
        }

        for (const Math::Matrix4f &model : m_ModelMatrices) {
            const Math::Matrix4f mvp = Math::multiply(view_projection, model);

            vkCmdPushConstants(command_buffer, graphics_pipeline.layoutHandle(),
                               VK_SHADER_STAGE_VERTEX_BIT, 0U,
                               static_cast<std::uint32_t>(sizeof(mvp)), &mvp);

            vkCmdDrawIndexed(command_buffer, mesh.m_IndexCount, 1U, 0U, 0, 0U);
        }
    }

    auto VulkanRenderer::recordFrameCommands(
        VkCommandBuffer command_buffer,
        const SwapchainResources &swapchain_resources,
        const DepthAttachmentResources &depth_attachment,
        std::size_t image_index) -> void {
        const VulkanSwapchain &swapchain =
            swapchain_resources.m_VulkanSwapchain;
        VkCommandBufferBeginInfo command_buffer_info{};
        command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        const VkResult command_buffer_begin_result =
            vkBeginCommandBuffer(command_buffer, &command_buffer_info);

        if (command_buffer_begin_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandBufferBeginFailed,
                "Failed to begin Vulkan command buffer recording",
                Core::Error::NativeError(
                    static_cast<int>(command_buffer_begin_result),
                    std::string(toString(command_buffer_begin_result))),
                "Begin Vulkan Command Buffer");
        }

        const VkImage acquired_image = swapchain.images()[image_index];

        VkImageMemoryBarrier2 color_attachment_barrier{};
        color_attachment_barrier.sType =
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        color_attachment_barrier.image = acquired_image;
        color_attachment_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        color_attachment_barrier.newLayout =
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        color_attachment_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        color_attachment_barrier.srcAccessMask = VK_ACCESS_2_NONE;
        color_attachment_barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        color_attachment_barrier.dstAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        color_attachment_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        color_attachment_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        color_attachment_barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        VkImageMemoryBarrier2 depth_attachment_barrier{};
        depth_attachment_barrier.sType =
            VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        depth_attachment_barrier.image =
            depth_attachment.m_Image.nativeHandle();
        depth_attachment_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depth_attachment_barrier.newLayout =
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_attachment_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        depth_attachment_barrier.srcAccessMask = VK_ACCESS_2_NONE;
        depth_attachment_barrier.dstStageMask =
            static_cast<VkPipelineStageFlags2>(
                VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT) |
            static_cast<VkPipelineStageFlags2>(
                VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT);
        depth_attachment_barrier.dstAccessMask =
            static_cast<VkAccessFlags2>(
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_READ_BIT) |
            static_cast<VkAccessFlags2>(
                VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
        depth_attachment_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depth_attachment_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        depth_attachment_barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        const std::array<VkImageMemoryBarrier2, 2> attachment_barriers{
            color_attachment_barrier,
            depth_attachment_barrier,
        };

        VkDependencyInfo attachment_dependency_info{};
        attachment_dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        attachment_dependency_info.imageMemoryBarrierCount = 2U;
        attachment_dependency_info.pImageMemoryBarriers =
            attachment_barriers.data();

        vkCmdPipelineBarrier2(command_buffer, &attachment_dependency_info);

        const VkImageView acquired_image_view =
            swapchain.imageViews()[image_index];

        VkRenderingAttachmentInfo render_attachment_info{};
        render_attachment_info.sType =
            VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        render_attachment_info.imageView = acquired_image_view;
        render_attachment_info.imageLayout =
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        render_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        render_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        const VkClearColorValue color{{0.0F, 0.0F, 0.0F, 1.0F}};

        render_attachment_info.clearValue = {
            .color = color,
        };

        VkRenderingAttachmentInfo depth_render_attachment_info{};
        depth_render_attachment_info.sType =
            VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depth_render_attachment_info.imageView =
            depth_attachment.m_ImageView.nativeHandle();
        depth_render_attachment_info.imageLayout =
            VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depth_render_attachment_info.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depth_render_attachment_info.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depth_render_attachment_info.clearValue = {
            .depthStencil =
                {
                    .depth = 1.0F,
                    .stencil = 0U,
                },
        };

        VkRenderingInfo render_info{};
        render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        render_info.renderArea = {
            .offset = {.x = 0, .y = 0},
            .extent = swapchain.extent(),
        };
        render_info.layerCount = 1U;
        render_info.colorAttachmentCount = 1U;
        render_info.pColorAttachments = &render_attachment_info;
        render_info.pDepthAttachment = &depth_render_attachment_info;

        vkCmdBeginRendering(command_buffer, &render_info);

        if (m_ActiveMeshHandle.has_value()) {
            recordMeshDraws(command_buffer, swapchain_resources);
        }

        vkCmdEndRendering(command_buffer);

        VkImageMemoryBarrier2 present_barrier{};
        present_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        present_barrier.image = acquired_image;
        present_barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        present_barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        present_barrier.srcStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        present_barrier.srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        present_barrier.dstStageMask = VK_PIPELINE_STAGE_2_NONE;
        present_barrier.dstAccessMask = VK_ACCESS_2_NONE;
        present_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        present_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        present_barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        recordImageBarrier(command_buffer, present_barrier);

        const VkResult command_buffer_end_result =
            vkEndCommandBuffer(command_buffer);

        if (command_buffer_end_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandBufferEndFailed,
                "Failed to end Vulkan command buffer recording",
                Core::Error::NativeError(
                    static_cast<int>(command_buffer_end_result),
                    std::string(toString(command_buffer_end_result))),
                "End Vulkan Command Buffer");
        }
    }

    auto VulkanRenderer::submitFrame(VulkanFrameResources &current_frame,
                                     VkCommandBuffer command_buffer,
                                     VkSemaphore render_finished_semaphore)
        -> void {
        VkSemaphoreSubmitInfo image_available_wait_info{};
        image_available_wait_info.sType =
            VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        image_available_wait_info.semaphore =
            current_frame.imageAvailableSemaphore().nativeHandle();
        image_available_wait_info.value = std::uint64_t{0};
        image_available_wait_info.stageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_available_wait_info.deviceIndex = 0U;

        VkCommandBufferSubmitInfo command_buffer_submit_info{};
        command_buffer_submit_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_buffer_submit_info.commandBuffer = command_buffer;
        command_buffer_submit_info.deviceMask = 0U;

        VkSemaphoreSubmitInfo render_finished_signal_info{};
        render_finished_signal_info.sType =
            VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        render_finished_signal_info.semaphore = render_finished_semaphore;
        render_finished_signal_info.value = std::uint64_t{0};
        render_finished_signal_info.stageMask =
            VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
        render_finished_signal_info.deviceIndex = 0U;

        VkSubmitInfo2 submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.waitSemaphoreInfoCount = 1U;
        submit_info.pWaitSemaphoreInfos = &image_available_wait_info;
        submit_info.commandBufferInfoCount = 1U;
        submit_info.pCommandBufferInfos = &command_buffer_submit_info;
        submit_info.signalSemaphoreInfoCount = 1U;
        submit_info.pSignalSemaphoreInfos = &render_finished_signal_info;

        current_frame.inFlightFence().reset();

        const VkResult queue_submit_result =
            vkQueueSubmit2(m_Device.graphicsQueue(), 1U, &submit_info,
                           current_frame.inFlightFence().nativeHandle());

        if (queue_submit_result != VK_SUCCESS) {
            m_SubmissionFailed = true;
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanQueueSubmissionFailed,
                "Failed to submit Vulkan queue work",
                Core::Error::NativeError(
                    static_cast<int>(queue_submit_result),
                    std::string(toString(queue_submit_result))),
                "Submit Vulkan Queue Work");
        }
    }

    auto VulkanRenderer::renderFrame() -> void {
        if (m_SubmissionFailed) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanQueueSubmissionFailed,
                "Cannot render after a failed Vulkan queue submission",
                "Render Vulkan Frame");
        }

        if (!m_SwapchainResources.has_value() &&
            !recreateSwapchainResources()) {
            return;
        }

        if (!m_SwapchainResources.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires swapchain resources after successful "
                "creation");
        }

        SwapchainResources &swapchain_resources = m_SwapchainResources.value();

        VulkanFrameResources &current_frame =
            m_FrameResources[m_CurrentFrameIndex];

        current_frame.inFlightFence().wait();

        m_GraphicsSubmissionState.recordCompletion(m_CurrentFrameIndex);

        m_GraphicsResourceRetirementQueue.collectCompleted(
            m_GraphicsSubmissionState);

        if (swapchain_resources.m_DepthAttachments.size() !=
            m_FrameResources.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires one depth attachment per "
                "frame-in-flight slot");
        }

        const DepthAttachmentResources &depth_attachment =
            swapchain_resources.m_DepthAttachments[m_CurrentFrameIndex];

        std::uint32_t image_index{};

        const VkSwapchainKHR swapchain_handle =
            swapchain_resources.m_VulkanSwapchain.nativeHandle();

        const VulkanSwapchain &swapchain =
            swapchain_resources.m_VulkanSwapchain;

        const auto &semaphores = swapchain_resources.m_VulkanSemaphores;

        const VkResult image_result = vkAcquireNextImageKHR(
            m_Device.nativeHandle(), swapchain_handle, UINT32_MAX,
            current_frame.imageAvailableSemaphore().nativeHandle(),
            VK_NULL_HANDLE, &image_index);

        bool swapchain_recreation_requested = false;

        const SwapchainAcquireAction acquire_action =
            selectSwapchainAcquireAction(image_result);

        switch (acquire_action) {
        case SwapchainAcquireAction::ContinueFrame:
            break;

        case SwapchainAcquireAction::ContinueFrameAndRecreateAfterPresent:
            swapchain_recreation_requested = true;
            break;

        case SwapchainAcquireAction::ReturnWithoutAdvancingFrame:
            return;

        case SwapchainAcquireAction::RecreateAndReturnWithoutAdvancingFrame:
            static_cast<void>(recreateSwapchainResources());
            return;

        case SwapchainAcquireAction::Failure:
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSwapchainImageAcquisitionFailed,
                "Failed to acquire Vulkan swapchain image",
                Core::Error::NativeError(static_cast<int>(image_result),
                                         std::string(toString(image_result))),
                "Acquire Vulkan Swapchain Image");
        }

        if (swapchain.images().size() != swapchain.imageViews().size() ||
            swapchain.images().size() != semaphores.size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires swapchain images, image views, and "
                "render-finished semaphores to have matching counts");
        }

        const auto acquired_image_index = static_cast<std::size_t>(image_index);

        if (acquired_image_index >= swapchain.images().size()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer acquired a swapchain image index outside the "
                "tracked swapchain image range");
        }

        current_frame.resetCommandResources();

        if (current_frame.commandBuffers().empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer requires the current frame to contain at least "
                "one command buffer");
        }

        const VkCommandBuffer command_buffer =
            current_frame.commandBuffers().front();

        recordFrameCommands(command_buffer, swapchain_resources,
                            depth_attachment, acquired_image_index);

        const VulkanSemaphore &render_finished_semaphore =
            semaphores[acquired_image_index];

        const VkSemaphore render_finished_handle =
            render_finished_semaphore.nativeHandle();

        m_GraphicsSubmissionState.validateCanRecordSubmission(
            m_CurrentFrameIndex);

        submitFrame(current_frame, command_buffer, render_finished_handle);

        static_cast<void>(
            m_GraphicsSubmissionState.recordSubmission(m_CurrentFrameIndex));

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1U;
        present_info.pWaitSemaphores = &render_finished_handle;
        present_info.swapchainCount = 1U;
        present_info.pSwapchains = &swapchain_handle;
        present_info.pImageIndices = &image_index;

        const VkResult queue_present_result =
            vkQueuePresentKHR(m_Device.presentationQueue(), &present_info);

        const SwapchainPresentAction present_action =
            selectSwapchainPresentAction(queue_present_result);

        switch (present_action) {
        case SwapchainPresentAction::AdvanceFrame:
            break;

        case SwapchainPresentAction::RecreateAndAdvanceFrame:
            swapchain_recreation_requested = true;
            break;

        case SwapchainPresentAction::RecreateAndReturnWithoutAdvancingFrame:
            static_cast<void>(recreateSwapchainResources());
            return;

        case SwapchainPresentAction::Failure:
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanQueuePresentationFailed,
                "Failed to present Vulkan swapchain image",
                Core::Error::NativeError(
                    static_cast<int>(queue_present_result),
                    std::string(toString(queue_present_result))),
                "Present Vulkan Swapchain Image");
        }

        if (swapchain_recreation_requested) {
            static_cast<void>(recreateSwapchainResources());
        }

        m_CurrentFrameIndex =
            (m_CurrentFrameIndex + 1U) % m_FrameResources.size();
    }
} // namespace SNE::Engine::Renderer::Vulkan
