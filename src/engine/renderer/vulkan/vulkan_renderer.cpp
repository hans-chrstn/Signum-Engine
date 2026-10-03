#include "vulkan_renderer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/platform/window.hpp"
#include "engine/renderer/presentation_preference.hpp"
#include "engine/renderer/vulkan/vulkan_device_discovery.hpp"
#include "engine/renderer/vulkan/vulkan_device_features.hpp"
#include "engine/renderer/vulkan/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"
#include "engine/renderer/vulkan/vulkan_result.hpp"
#include "engine/renderer/vulkan/vulkan_semaphore.hpp"
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

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
        bool development_diagnostics_enabled)
        : m_Instance(application_name, development_diagnostics_enabled),
          m_Surface(m_Instance.nativeHandle(), window.nativeHandle()),
          m_PhysicalDevice(selectRequiredPhysicalDevice(
              m_Instance.nativeHandle(), m_Surface.nativeHandle())),
          m_LogicalDeviceFeatureConfiguration(
              deriveLogicalDeviceFeatureConfiguration(
                  deriveLogicalDeviceFeatureRequest(presentation_preference),
                  m_PhysicalDevice.capabilities)),
          m_Device(
              m_PhysicalDevice.handle, m_PhysicalDevice.queue_families,
              deriveUniqueQueueFamilyRequests(m_PhysicalDevice.queue_families),
              m_LogicalDeviceFeatureConfiguration),
          m_Swapchain(
              m_PhysicalDevice.handle, m_Device.nativeHandle(),
              m_Surface.nativeHandle(), m_PhysicalDevice.queue_families,
              window.framebufferSize(), presentation_preference,
              m_LogicalDeviceFeatureConfiguration.fifo_latest_ready_feature
                      .presentModeFifoLatestReady == VK_TRUE) {
        const std::size_t images = m_Swapchain.images().size();
        m_RenderFinishedSemaphores.reserve(images);

        for (std::size_t i{}; i < images; ++i) {
            m_RenderFinishedSemaphores.emplace_back(m_Device.nativeHandle());
        }

        const std::uint32_t graphics_queue_family_index =
            m_PhysicalDevice.queue_families.graphics_family.family_index;

        m_FrameResources.reserve(kFramesInFlight);

        for (std::size_t i{}; i < kFramesInFlight; ++i) {
            m_FrameResources.emplace_back(m_Device.nativeHandle(),
                                          graphics_queue_family_index);
        }
    }

    VulkanRenderer::~VulkanRenderer() noexcept {
        const VkResult result = vkDeviceWaitIdle(m_Device.nativeHandle());
        static_cast<void>(result);
    }

    auto VulkanRenderer::renderFrame() -> void {
        VulkanFrameResources &current_frame =
            m_FrameResources[m_CurrentFrameIndex];

        current_frame.inFlightFence().wait();

        current_frame.resetCommandResources();

        const VkCommandBuffer command_buffer =
            current_frame.commandBuffers().front();

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

        std::uint32_t image_index{};
        const VkSwapchainKHR swapchain_handle = m_Swapchain.nativeHandle();

        const VkResult image_result = vkAcquireNextImageKHR(
            m_Device.nativeHandle(), swapchain_handle, UINT32_MAX,
            current_frame.imageAvailableSemaphore().nativeHandle(),
            VK_NULL_HANDLE, &image_index);

        switch (image_result) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_TIMEOUT:
        case VK_NOT_READY:
        case VK_ERROR_OUT_OF_DATE_KHR:
            return;
        default:
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSwapchainImageAcquisitionFailed,
                "Failed to acquire Vulkan swapchain image",
                Core::Error::NativeError(static_cast<int>(image_result),
                                         std::string(toString(image_result))),
                "Acquire Vulkan Swapchain Image");
        }

        const VkImage acquired_image = m_Swapchain.images()[image_index];

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

        VkDependencyInfo color_attachment_dependency_info{};
        color_attachment_dependency_info.sType =
            VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        color_attachment_dependency_info.imageMemoryBarrierCount = 1U;
        color_attachment_dependency_info.pImageMemoryBarriers =
            &color_attachment_barrier;

        vkCmdPipelineBarrier2(command_buffer,
                              &color_attachment_dependency_info);

        const VkImageView acquired_image_view =
            m_Swapchain.imageViews()[image_index];

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

        VkRenderingInfo render_info{};
        render_info.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        render_info.renderArea = {
            .offset = {.x = 0, .y = 0},
            .extent = m_Swapchain.extent(),
        };
        render_info.layerCount = 1U;
        render_info.colorAttachmentCount = 1U;
        render_info.pColorAttachments = &render_attachment_info;

        vkCmdBeginRendering(command_buffer, &render_info);
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

        VkDependencyInfo present_dependency_info{};
        present_dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        present_dependency_info.imageMemoryBarrierCount = 1U;
        present_dependency_info.pImageMemoryBarriers = &present_barrier;

        vkCmdPipelineBarrier2(command_buffer, &present_dependency_info);

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

        const VulkanSemaphore &render_finished_semaphore =
            m_RenderFinishedSemaphores[image_index];
        const VkSemaphore render_finished_handle =
            render_finished_semaphore.nativeHandle();

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
        render_finished_signal_info.semaphore = render_finished_handle;
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
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanQueueSubmissionFailed,
                "Failed to submit Vulkan queue work",
                Core::Error::NativeError(
                    static_cast<int>(queue_submit_result),
                    std::string(toString(queue_submit_result))),
                "Submit Vulkan Queue Work");
        }

        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1U;
        present_info.pWaitSemaphores = &render_finished_handle;
        present_info.swapchainCount = 1U;
        present_info.pSwapchains = &swapchain_handle;
        present_info.pImageIndices = &image_index;

        const VkResult queue_present_result =
            vkQueuePresentKHR(m_Device.presentationQueue(), &present_info);

        switch (queue_present_result) {
        case VK_SUCCESS:
        case VK_SUBOPTIMAL_KHR:
            break;
        case VK_ERROR_OUT_OF_DATE_KHR:
            return;
        default:
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanQueuePresentationFailed,
                "Failed to present Vulkan swapchain image",
                Core::Error::NativeError(
                    static_cast<int>(queue_present_result),
                    std::string(toString(queue_present_result))),
                "Present Vulkan Swapchain Image");
        }

        m_CurrentFrameIndex =
            (m_CurrentFrameIndex + 1U) % m_FrameResources.size();
    }
} // namespace SNE::Engine::Renderer::Vulkan
