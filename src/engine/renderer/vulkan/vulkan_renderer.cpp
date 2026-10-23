#include "vulkan_renderer.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/material/material_resource_binding.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/vulkan/common/vulkan_api_version.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/device/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/device/vulkan_queue_requests.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <utility>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Error = SNE::Engine::Core::Error;

namespace {
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
        std::optional<Shader::GraphicsShaderData> initial_graphics_shaders,
        std::span<const MaterialBufferBindingInput> initial_buffer_bindings)
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
        initializeFrameResources();

        if (initial_mesh.has_value()) {
            initializeInitialMesh(initial_mesh.value());
        }

        std::optional<ReflectedGraphicsShaderInterfaces> shader_interfaces;

        if (initial_graphics_shaders.has_value()) {
            shader_interfaces.emplace(initializeInitialMaterial(
                std::move(initial_graphics_shaders.value())));
        }

        if (initial_texture.has_value()) {
            initializeInitialTexture(initial_texture.value());
        }

        for (const MaterialBufferBindingInput &input :
             initial_buffer_bindings) {
            m_MaterialBindings.bindings.push_back(
                registerMaterialBufferBinding(input));
        }

        if (shader_interfaces.has_value()) {
            validateInitialMaterialBindings(shader_interfaces.value());

            if (!m_DescriptorSetLayouts.empty()) {
                if (m_FrequentMaterialBuffers.empty()) {
                    m_ActiveMaterialDescriptorHandle.emplace(
                        registerMaterialDescriptorResources(
                            m_MaterialBindings.bindings));
                } else {
                    m_FrameMaterialDescriptorHandles =
                        registerFrameMaterialDescriptorResources(
                            m_MaterialBindings.bindings);
                }
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

    auto VulkanRenderer::initializeFrameResources() -> void {
        const std::uint32_t graphics_queue_family_index =
            m_PhysicalDevice.queue_families.graphics_family.family_index;

        m_FrameResources.reserve(kFramesInFlight);

        for (std::size_t i{}; i < kFramesInFlight; ++i) {
            m_FrameResources.emplace_back(m_Device.nativeHandle(),
                                          graphics_queue_family_index);
        }
    }

    auto VulkanRenderer::initializeInitialMesh(const InitialMeshData &mesh)
        -> void {
        m_MeshVertexAttributes.assign(mesh.layout.attributes.begin(),
                                      mesh.layout.attributes.end());

        m_MeshVertexStride = mesh.mesh.vertex_stride;

        uploadMesh(mesh.mesh);

        if (!m_ActiveMeshHandle.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Invariant,
                Core::Error::Subsystem::Vulkan,
                "VulkanRenderer initial mesh upload did not produce an active "
                "mesh handle");
        }

        m_DrawInstances.reserve(mesh.model_matrices.size());

        for (const Core::Math::Matrix4f &matrix : mesh.model_matrices) {
            MeshDrawInstance instance{
                .mesh_handle = *m_ActiveMeshHandle,
                .model_matrix = matrix,
            };

            m_DrawInstances.push_back(instance);
        }
    }

    auto
    VulkanRenderer::initializeInitialTexture(const TextureUploadData &texture)
        -> void {
        VkDescriptorSetLayoutBinding descriptor_set_layout_binding{};
        descriptor_set_layout_binding.binding = 0U;
        descriptor_set_layout_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_set_layout_binding.descriptorCount = 1U;
        descriptor_set_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

        VkDescriptorSetLayoutCreateInfo descriptor_set_layout_create_info{};
        descriptor_set_layout_create_info.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_layout_create_info.bindingCount = 1U;
        descriptor_set_layout_create_info.pBindings =
            &descriptor_set_layout_binding;

        const VulkanDescriptorSetLayout &descriptor_layout =
            m_TextureDescriptorSetLayout.emplace(
                m_Device.nativeHandle(), descriptor_set_layout_create_info);

        TextureResources texture_resources = uploadTexture(texture);

        GpuResourceHandle texture_handle =
            m_TextureRegistry.insert(std::move(texture_resources));

        const MaterialResourceBinding material_binding{
            .set = 0U,
            .binding = 0U,
            .array_element = 0U,
            .type = Shader::ShaderResourceType::CombinedImageSampler,
            .resource_handle = texture_handle,
        };

        m_TextureBindingResources.emplace(
            createTextureBindingResources(material_binding, descriptor_layout));

        m_MaterialBindings.bindings.push_back(material_binding);
    }

} // namespace SNE::Engine::Renderer::Vulkan
