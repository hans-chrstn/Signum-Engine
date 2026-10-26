#include "editor_imgui_vulkan_backend.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include <backends/imgui_impl_vulkan.h>
#include <cstdint>
#include <imgui.h>
#include <vulkan/vulkan.h>

namespace Assertion = SNE::Engine::Core::Assertion;
namespace Error = SNE::Engine::Core::Error;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace SNE::Editor {
    EditorImGuiVulkanBackend::EditorImGuiVulkanBackend(
        const Vulkan::VulkanExternalRenderingInfo &rendering_info) {
        if (ImGui::GetCurrentContext() == nullptr) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "EditorImGuiVulkanBackend requires an "
                                     "active Dear ImGui context");
        }

        if (rendering_info.instance == VK_NULL_HANDLE ||
            rendering_info.physical_device == VK_NULL_HANDLE ||
            rendering_info.device == VK_NULL_HANDLE ||
            rendering_info.graphics_queue == VK_NULL_HANDLE) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "EditorImGuiVulkanBackend requires valid Vulkan handles");
        }

        if (rendering_info.color_attachment_format == VK_FORMAT_UNDEFINED) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "EditorImGuiVulkanBackend requires a "
                                     "valid color attachment format");
        }

        if (rendering_info.depth_attachment_format == VK_FORMAT_UNDEFINED) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "EditorImGuiVulkanBackend requires a "
                                     "valid depth attachment format");
        }

        if (rendering_info.swapchain_image_count < 2U) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "EditorImGuiVulkanBackend requires at "
                                     "least two swapchain images");
        }

        if (rendering_info.api_version == 0U) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "EditorImGuiVulkanBackend requires a "
                                     "nonzero Vulkan API version");
        }

        ImGui_ImplVulkan_InitInfo init_info{};
        init_info.ApiVersion = rendering_info.api_version;
        init_info.Instance = rendering_info.instance;
        init_info.PhysicalDevice = rendering_info.physical_device;
        init_info.Device = rendering_info.device;
        init_info.QueueFamily = rendering_info.graphics_queue_family_index;
        init_info.Queue = rendering_info.graphics_queue;
        init_info.ImageCount = rendering_info.swapchain_image_count;
        init_info.MinImageCount = rendering_info.swapchain_image_count;
        constexpr std::uint32_t kDescriptorPoolSize = 64U;
        init_info.DescriptorPoolSize = kDescriptorPoolSize;
        init_info.UseDynamicRendering = true;

        VkPipelineRenderingCreateInfo pipeline_rendering_info{};
        pipeline_rendering_info.sType =
            VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
        pipeline_rendering_info.colorAttachmentCount = 1U;
        pipeline_rendering_info.pColorAttachmentFormats =
            &rendering_info.color_attachment_format;
        pipeline_rendering_info.depthAttachmentFormat =
            rendering_info.depth_attachment_format;
        pipeline_rendering_info.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

        init_info.PipelineInfoMain.PipelineRenderingCreateInfo =
            pipeline_rendering_info;
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

        const bool init_result = ImGui_ImplVulkan_Init(&init_info);
        if (!init_result) {
            throw Error::EngineError(
                Error::Code::EditorVulkanBackendInitializationFailed,
                "Failed to initialize the Dear ImGui Vulkan rendering backend",
                "ImGui_ImplVulkan_Init");
        }

        m_Device = rendering_info.device;
    }

    EditorImGuiVulkanBackend::~EditorImGuiVulkanBackend() {
        const VkResult result = vkDeviceWaitIdle(m_Device);

        if (result != VK_SUCCESS) {
            Assertion::failAssertion(
                Assertion::AssertionType::Invariant, Error::Subsystem::Editor,
                "EditorImGuiVulkanBackend failed to wait for the Vulkan device "
                "to become idle before shutdown");
        }

        ImGui_ImplVulkan_Shutdown();
    }
} // namespace SNE::Editor
