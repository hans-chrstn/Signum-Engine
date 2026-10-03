#include "vulkan_swapchain.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/platform/window.hpp"
#include "vulkan_queue_families.hpp"
#include "vulkan_result.hpp"
#include "vulkan_swapchain_selection.hpp"
#include "vulkan_swapchain_support.hpp"
#include <array>
#include <cstdint>
#include <span>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanSwapchain::VulkanSwapchain(
        VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface,
        const SelectedQueueFamilies &queue_families,
        const Platform::FramebufferSize &framebuffer_size,
        PresentationPreference presentation_preference,
        bool fifo_latest_ready_enabled)
        : m_Device(device) {
        if (physical_device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanSwapchain requires a valid Vulkan physical device");
        }

        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanSwapchain requires a valid Vulkan logical device");
        }

        if (surface == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanSwapchain requires a valid Vulkan surface");
        }

        const SwapchainSupportDetails swapchain_support =
            querySwapchainSupport(physical_device, surface);

        m_SurfaceFormat =
            selectSurfaceFormat(swapchain_support.available_surface_formats);

        const VkPresentModeKHR presentation_mode = selectPresentationMode(
            presentation_preference,
            swapchain_support.available_presentation_modes,
            fifo_latest_ready_enabled);

        m_Extent = selectSwapExtent(swapchain_support.surface_capabilities,
                                    framebuffer_size);

        const std::uint32_t selected_swapchain_image_count =
            selectSwapchainImageCount(swapchain_support.surface_capabilities);

        const std::uint32_t queue_graphics_index =
            queue_families.graphics_family.family_index;
        const std::uint32_t queue_presentation_index =
            queue_families.presentation_family.family_index;

        const std::array<std::uint32_t, 2> queue_family{
            {queue_graphics_index, queue_presentation_index},
        };

        VkSwapchainCreateInfoKHR swapchain_create_info{};
        swapchain_create_info.sType =
            VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;

        if (queue_graphics_index != queue_presentation_index) {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapchain_create_info.queueFamilyIndexCount = 2U;
            swapchain_create_info.pQueueFamilyIndices = queue_family.data();
        } else {
            swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapchain_create_info.queueFamilyIndexCount = 0U;
        }

        swapchain_create_info.surface = surface;
        swapchain_create_info.minImageCount = selected_swapchain_image_count;
        swapchain_create_info.imageFormat = m_SurfaceFormat.format;
        swapchain_create_info.imageColorSpace = m_SurfaceFormat.colorSpace;
        swapchain_create_info.imageExtent = m_Extent;
        swapchain_create_info.presentMode = presentation_mode;
        swapchain_create_info.imageArrayLayers = 1U;
        swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        swapchain_create_info.preTransform =
            swapchain_support.surface_capabilities.currentTransform;

        swapchain_create_info.compositeAlpha =
            selectCompositeAlpha(swapchain_support.surface_capabilities);

        swapchain_create_info.clipped = VK_TRUE;
        swapchain_create_info.oldSwapchain = VK_NULL_HANDLE;

        const VkResult result = vkCreateSwapchainKHR(
            m_Device, &swapchain_create_info, nullptr, &m_Swapchain);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSwapchainCreationFailed,
                "Failed to create a Vulkan swapchain",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Swapchain");
        }

        try {
            while (true) {
                std::uint32_t swapchain_image_count{};
                const VkResult swapchain_image_count_result =
                    vkGetSwapchainImagesKHR(m_Device, m_Swapchain,
                                            &swapchain_image_count, nullptr);

                if (swapchain_image_count_result != VK_SUCCESS) {
                    throw Core::Error::EngineError(
                        Core::Error::Code::
                            VulkanSwapchainImageEnumerationFailed,
                        "Vulkan swapchain image enumeration failed",
                        Core::Error::NativeError(
                            static_cast<int>(swapchain_image_count_result),
                            std::string(
                                toString(swapchain_image_count_result))),
                        "Enumerate Vulkan Swapchain Images");
                }

                m_Images.resize(swapchain_image_count);

                const VkResult swapchain_images_result =
                    vkGetSwapchainImagesKHR(m_Device, m_Swapchain,
                                            &swapchain_image_count,
                                            m_Images.data());

                if (swapchain_images_result == VK_SUCCESS) {
                    m_Images.resize(swapchain_image_count);
                    break;
                }

                if (swapchain_images_result == VK_INCOMPLETE) {
                    continue;
                }

                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanSwapchainImageEnumerationFailed,
                    "Vulkan swapchain image enumeration failed",
                    Core::Error::NativeError(
                        static_cast<int>(swapchain_images_result),
                        std::string(toString(swapchain_images_result))),
                    "Enumerate Vulkan Swapchain Images");
            }

            m_ImageViews.reserve(m_Images.size());

            for (std::size_t i{}; i < m_Images.size(); ++i) {
                VkImageViewCreateInfo image_view_create_info{};
                image_view_create_info.sType =
                    VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
                image_view_create_info.image = m_Images[i];
                image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
                image_view_create_info.format = m_SurfaceFormat.format;
                image_view_create_info.subresourceRange = {
                    .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                    .baseMipLevel = 0U,
                    .levelCount = 1U,
                    .baseArrayLayer = 0U,
                    .layerCount = 1U,
                };
                image_view_create_info.components = {
                    .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                    .a = VK_COMPONENT_SWIZZLE_IDENTITY,
                };

                VkImageView temporary_image_view{};

                const VkResult image_view_result =
                    vkCreateImageView(m_Device, &image_view_create_info,
                                      nullptr, &temporary_image_view);

                if (image_view_result != VK_SUCCESS) {
                    throw Core::Error::EngineError(
                        Core::Error::Code::
                            VulkanSwapchainImageViewCreationFailed,
                        "Vulkan swapchain image view creation failed",
                        Core::Error::NativeError(
                            static_cast<int>(image_view_result),
                            std::string(toString(image_view_result))),
                        "Create Vulkan Image Views");
                }

                m_ImageViews.push_back(temporary_image_view);
            }
        } catch (...) {
            for (const VkImageView &image_view : m_ImageViews) {
                vkDestroyImageView(m_Device, image_view, nullptr);
            }
            vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
            m_Swapchain = VK_NULL_HANDLE;
            throw;
        }
    }

    VulkanSwapchain::~VulkanSwapchain() noexcept {
        if (m_Swapchain != VK_NULL_HANDLE) {
            for (const VkImageView &image_view : m_ImageViews) {
                vkDestroyImageView(m_Device, image_view, nullptr);
            }
            vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
        }
        m_Swapchain = VK_NULL_HANDLE;
    }

    auto VulkanSwapchain::nativeHandle() const noexcept -> VkSwapchainKHR {
        return m_Swapchain;
    }

    auto VulkanSwapchain::surfaceFormat() const noexcept -> VkSurfaceFormatKHR {
        return m_SurfaceFormat;
    }

    auto VulkanSwapchain::extent() const noexcept -> VkExtent2D {
        return m_Extent;
    }

    auto VulkanSwapchain::images() const noexcept -> std::span<const VkImage> {
        return m_Images;
    }

    auto VulkanSwapchain::imageViews() const noexcept
        -> std::span<const VkImageView> {
        return m_ImageViews;
    }
} // namespace SNE::Engine::Renderer::Vulkan
