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
#include <cstddef>
#include <cstdint>
#include <optional>
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

        const VkResult command_buffer_result =
            vkBeginCommandBuffer(command_buffer, &command_buffer_info);

        if (command_buffer_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandBufferBeginFailed,
                "Failed to begin Vulkan command buffer recording",
                Core::Error::NativeError(
                    static_cast<int>(command_buffer_result),
                    std::string(toString(command_buffer_result))),
                "Begin Vulkan Command Buffer");
        }

        std::uint32_t image_index{};

        const VkResult image_result = vkAcquireNextImageKHR(
            m_Device.nativeHandle(), m_Swapchain.nativeHandle(), UINT32_MAX,
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

        VkImageMemoryBarrier2 image_barrier{};
        image_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        image_barrier.image = acquired_image;
        image_barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_barrier.srcStageMask = VK_PIPELINE_STAGE_2_NONE;
        image_barrier.srcAccessMask = VK_ACCESS_2_NONE;
        image_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        image_barrier.dstStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
        image_barrier.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;
        image_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        image_barrier.subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0U,
            .levelCount = 1U,
            .baseArrayLayer = 0U,
            .layerCount = 1U,
        };

        VkDependencyInfo dependency_info{};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.imageMemoryBarrierCount = 1U;
        dependency_info.pImageMemoryBarriers = &image_barrier;

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);
    }
} // namespace SNE::Engine::Renderer::Vulkan
