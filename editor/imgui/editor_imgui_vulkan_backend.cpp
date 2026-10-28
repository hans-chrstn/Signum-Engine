#include "editor_imgui_vulkan_backend.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include <algorithm>
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

        VkSamplerCreateInfo sampler_info{};
        sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        sampler_info.magFilter = VK_FILTER_LINEAR;
        sampler_info.minFilter = VK_FILTER_LINEAR;
        sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        sampler_info.mipLodBias = 0.0F;
        sampler_info.anisotropyEnable = VK_FALSE;
        sampler_info.maxAnisotropy = 1.0F;
        sampler_info.compareEnable = VK_FALSE;
        sampler_info.minLod = 0.0F;
        sampler_info.maxLod = 0.0F;
        sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        sampler_info.unnormalizedCoordinates = VK_FALSE;

        m_SceneViewportSampler.emplace(rendering_info.device, sampler_info);

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

        for (const SceneViewportTextureRegistration &registration :
             m_SceneViewportTextureRegistrations) {
            if (registration.m_DescriptorSet == VK_NULL_HANDLE) {
                continue;
            }

            ImGui_ImplVulkan_RemoveTexture(registration.m_DescriptorSet);
        }

        for (const SceneViewportTextureRegistration &registration :
             m_RetiredSceneViewportTextureRegistrations) {
            if (registration.m_DescriptorSet == VK_NULL_HANDLE) {
                continue;
            }

            ImGui_ImplVulkan_RemoveTexture(registration.m_DescriptorSet);
        }

        ImGui_ImplVulkan_Shutdown();
    }

    auto EditorImGuiVulkanBackend::sceneViewportTexture(
        const Vulkan::VulkanSceneViewportImageInfo &image_info)
        -> VkDescriptorSet {
        if (!m_SceneViewportSampler.has_value()) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene viewport texture registration requires an initialized "
                "sampler");
        }

        if (image_info.image_view == VK_NULL_HANDLE) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Scene viewport texture registration requires a valid image "
                "view");
        }

        if (image_info.frame_slot >=
            m_SceneViewportTextureRegistrations.size()) {
            m_SceneViewportTextureRegistrations.resize(image_info.frame_slot +
                                                       1);
        }

        SceneViewportTextureRegistration &registration =
            m_SceneViewportTextureRegistrations[image_info.frame_slot];

        if (registration.m_DescriptorSet != VK_NULL_HANDLE &&
            registration.m_ImageView == image_info.image_view &&
            registration.m_Generation == image_info.generation) {
            return registration.m_DescriptorSet;
        }

        if (registration.m_DescriptorSet != VK_NULL_HANDLE) {
            m_RetiredSceneViewportTextureRegistrations.push_back(registration);

            registration = {};
        }

        VkDescriptorSet descriptor_set = ImGui_ImplVulkan_AddTexture(
            m_SceneViewportSampler->nativeHandle(), image_info.image_view,
            image_info.image_layout);

        if (descriptor_set == VK_NULL_HANDLE) {
            Assertion::failAssertion(
                Assertion::AssertionType::Invariant, Error::Subsystem::Editor,
                "Scene viewport texture registration returned a null "
                "descriptor set");
        }

        registration.m_DescriptorSet = descriptor_set;
        registration.m_ImageView = image_info.image_view;
        registration.m_Generation = image_info.generation;

        return descriptor_set;
    }

    auto EditorImGuiVulkanBackend::recordSceneViewportTextureSubmission(
        VkDescriptorSet descriptor_set, std::uint64_t submission_id) -> void {
        if (descriptor_set == VK_NULL_HANDLE) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "Scene viewport submission tracking "
                                     "requires a valid descriptor set");
        }

        if (submission_id == 0U) {
            Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                     Error::Subsystem::Editor,
                                     "Scene viewport submission tracking "
                                     "requires a nonzero submission ID");
        }

        for (SceneViewportTextureRegistration &registration :
             m_SceneViewportTextureRegistrations) {
            if (registration.m_DescriptorSet != descriptor_set) {
                continue;
            }

            registration.m_LastUsedSubmissionId =
                std::max(registration.m_LastUsedSubmissionId, submission_id);

            return;
        }

        for (SceneViewportTextureRegistration &registration :
             m_RetiredSceneViewportTextureRegistrations) {
            if (registration.m_DescriptorSet != descriptor_set) {
                continue;
            }

            registration.m_LastUsedSubmissionId =
                std::max(registration.m_LastUsedSubmissionId, submission_id);

            return;
        }

        Assertion::failAssertion(Assertion::AssertionType::Precondition,
                                 Error::Subsystem::Editor,
                                 "Scene viewport submission references an "
                                 "untracked texture descriptor");
    }

    auto EditorImGuiVulkanBackend::collectRetiredSceneViewportTextures(
        const Vulkan::VulkanGraphicsSubmissionProgress &submission_progress)
        -> void {
        auto retired_it = m_RetiredSceneViewportTextureRegistrations.begin();

        while (retired_it != m_RetiredSceneViewportTextureRegistrations.end()) {
            if (retired_it->m_LastUsedSubmissionId >
                submission_progress.last_completed) {
                ++retired_it;
                continue;
            }

            if (retired_it->m_DescriptorSet != VK_NULL_HANDLE) {
                ImGui_ImplVulkan_RemoveTexture(retired_it->m_DescriptorSet);
            }

            retired_it =
                m_RetiredSceneViewportTextureRegistrations.erase(retired_it);
        }
    }
} // namespace SNE::Editor
