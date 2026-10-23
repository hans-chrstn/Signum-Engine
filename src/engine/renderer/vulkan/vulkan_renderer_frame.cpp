#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/renderer/material/material_resource_binding.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/vulkan/command/vulkan_image_barrier.hpp"
#include "engine/renderer/vulkan/common/vulkan_result.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_draw_push_constants.hpp"
#include "engine/renderer/vulkan/presentation/vulkan_swapchain_result_policy.hpp"
#include "vulkan_renderer.hpp"
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace Math = SNE::Engine::Core::Math;

namespace {
    constexpr std::size_t kMatrixSize = 64U;
    constexpr std::size_t kMatrixOffset = 0U;

    static_assert(sizeof(Math::Matrix4f) == kMatrixSize);
    static_assert(offsetof(Math::Matrix4f, elements) == kMatrixOffset);
    static_assert(std::is_trivially_copyable_v<Math::Matrix4f>);
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
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

        const VulkanGraphicsPipeline &graphics_pipeline =
            *swapchain_resources.m_VulkanGraphicsPipeline;

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          graphics_pipeline.nativeHandle());

        const MaterialDescriptorResources *active_material_descriptors =
            findMaterialDescriptorResourcesForFrame(m_CurrentFrameIndex);

        if (active_material_descriptors != nullptr) {
            const std::vector<VkDescriptorSet> &descriptor_sets =
                active_material_descriptors->m_DescriptorSets;

            if (descriptor_sets.empty()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer material descriptor resources require "
                    "at least one allocated descriptor set");
            }

            const std::optional<std::uint32_t> descriptor_count =
                Core::Numeric::tryConvertToUint32(descriptor_sets.size());

            if (!descriptor_count.has_value()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer material descriptor-set count exceeds "
                    "uint32_t");
            }

            vkCmdBindDescriptorSets(
                command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                graphics_pipeline.layoutHandle(), 0U, descriptor_count.value(),
                descriptor_sets.data(), 0U, nullptr);

        } else if (m_TextureBindingResources.has_value()) {
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
                    "VulkanRenderer active material texture binding was not "
                    "found");
            }

            const GpuResourceHandle &resource_handle =
                texture_binding_it->resource_handle;

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

        for (const MeshDrawInstance &instance : m_DrawInstances) {
            const MeshBufferResources *mesh =
                m_MeshRegistry.find(instance.mesh_handle);

            if (mesh == nullptr) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer draw instance mesh handle does not resolve "
                    "to a registered mesh");
            }

            const VkDeviceSize vertex_offset = 0U;
            const VkBuffer vertex_buffer = mesh->m_VertexBuffer.nativeHandle();
            vkCmdBindVertexBuffers(command_buffer, 0U, 1U, &vertex_buffer,
                                   &vertex_offset);

            const VkBuffer index_buffer = mesh->m_IndexBuffer.nativeHandle();
            vkCmdBindIndexBuffer(command_buffer, index_buffer, 0U,
                                 VK_INDEX_TYPE_UINT16);

            const Math::Matrix4f mvp =
                Math::multiply(view_projection, instance.model_matrix);

            const std::optional<Math::Matrix4f> inverse_matrix =
                Math::tryInverse(instance.model_matrix);

            if (!inverse_matrix.has_value()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "VulkanRenderer requires an invertible model matrix "
                    "for normal transformation");
            }

            VulkanDrawPushConstants draw_push_constants{
                .mvp = mvp,
                .normal_matrix = Math::transpose(inverse_matrix.value()),
            };

            vkCmdPushConstants(
                command_buffer, graphics_pipeline.layoutHandle(),
                VK_SHADER_STAGE_VERTEX_BIT, 0U,
                static_cast<std::uint32_t>(sizeof(draw_push_constants)),
                &draw_push_constants);

            vkCmdDrawIndexed(command_buffer, mesh->m_IndexCount, 1U, 0U, 0, 0U);
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

        if (!m_DrawInstances.empty()) {
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

        const Platform::FramebufferSize framebuffer_size =
            m_Window->framebufferSize();

        if (framebuffer_size.width <= 0 || framebuffer_size.height <= 0) {
            static_cast<void>(recreateSwapchainResources());
            return;
        }

        const VkExtent2D swapchain_extent =
            m_SwapchainResources->m_VulkanSwapchain.extent();

        if (std::cmp_not_equal(framebuffer_size.width,
                               swapchain_extent.width) ||
            std::cmp_not_equal(framebuffer_size.height,
                               swapchain_extent.height)) {

            static_cast<void>(recreateSwapchainResources());
            return;
        }

        SwapchainResources &swapchain_resources = m_SwapchainResources.value();

        VulkanFrameResources &current_frame =
            m_FrameResources[m_CurrentFrameIndex];

        current_frame.inFlightFence().wait();

        m_GraphicsSubmissionState.recordCompletion(m_CurrentFrameIndex);

        m_GraphicsResourceRetirementQueue.collectCompleted(
            m_GraphicsSubmissionState);

        uploadFrequentMaterialBuffersForFrame(m_CurrentFrameIndex);

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
