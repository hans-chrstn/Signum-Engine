#include "engine/renderer/vulkan/vulkan_api_version.hpp"
#include "engine/renderer/vulkan/vulkan_device_capabilities.hpp"
#include "engine/renderer/vulkan/vulkan_device_selection.hpp"
#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/vulkan_swapchain_support.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    [[nodiscard]] auto makeQueueFamilyProperties()
        -> std::vector<VkQueueFamilyProperties> {
        VkQueueFamilyProperties graphics_properties{};
        graphics_properties.queueFlags = VK_QUEUE_GRAPHICS_BIT;
        graphics_properties.queueCount = 2U;

        VkQueueFamilyProperties second_properties{};
        second_properties.queueFlags = VK_QUEUE_GRAPHICS_BIT;
        second_properties.queueCount = 4U;

        return {
            graphics_properties,
            second_properties,
        };
    }

    [[nodiscard]] auto
    makePhysicalDeviceCapabilities(std::uint32_t api_version,
                                   bool dynamic_rendering_supported = true,
                                   bool synchronization2_supported = true)
        -> Vulkan::PhysicalDeviceCapabilities {
        Vulkan::PhysicalDeviceCapabilities capabilities{};
        capabilities.properties.apiVersion = api_version;
        capabilities.required_capabilities.dynamic_rendering_supported =
            dynamic_rendering_supported;
        capabilities.required_capabilities.synchronization2_supported =
            synchronization2_supported;
        return capabilities;
    }

    [[nodiscard]] auto makeAdequateSwapchainSupport()
        -> Vulkan::SwapchainSupportDetails {
        const std::vector<VkPresentModeKHR> available_presentation_modes{
            VK_PRESENT_MODE_FIFO_KHR,
        };

        const VkSurfaceFormatKHR surface_format{
            .format = VK_FORMAT_B8G8R8A8_SRGB,
            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
        };

        const std::vector<VkSurfaceFormatKHR> available_surface_formats{
            surface_format,
        };

        Vulkan::SwapchainSupportDetails swapchain_support_details{
            .available_surface_formats = available_surface_formats,
            .available_presentation_modes = available_presentation_modes,
        };

        return swapchain_support_details;
    }
} // namespace

TEST(VulkanDeviceSelectionTests,
     ReturnsTrueWhenRequiredDeviceExtensionsAreAvailable) {
    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    EXPECT_TRUE(Vulkan::supportsRequiredDeviceExtensions(available_extensions));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenRequiredDeviceExtensionIsMissing) {
    const VkExtensionProperties unrelated_extension{
        .extensionName = VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        unrelated_extension,
    };

    EXPECT_FALSE(
        Vulkan::supportsRequiredDeviceExtensions(available_extensions));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsTrueWhenRequiredDeviceExtensionsAreAvailableWithExtras) {
    const VkExtensionProperties unrelated_extension{
        .extensionName = "VK_TEST",
        .specVersion = std::uint32_t{0},
    };

    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        unrelated_extension,
        swapchain_extension,
    };

    EXPECT_TRUE(Vulkan::supportsRequiredDeviceExtensions(available_extensions));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenNoDeviceExtensionsAreAvailable) {
    const std::vector<VkExtensionProperties> available_extensions{};

    EXPECT_FALSE(
        Vulkan::supportsRequiredDeviceExtensions(available_extensions));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsTrueWhenSwapchainSupportHasFormatsAndPresentationModes) {
    EXPECT_TRUE(
        Vulkan::hasRequiredSwapchainSupport(makeAdequateSwapchainSupport()));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenSwapchainSupportHasNoSurfaceFormats) {
    const std::vector<VkPresentModeKHR> available_presentation_modes{
        VK_PRESENT_MODE_FIFO_KHR,
    };

    const Vulkan::SwapchainSupportDetails swapchain_support_details{
        .available_surface_formats = {},
        .available_presentation_modes = available_presentation_modes,
    };

    EXPECT_FALSE(
        Vulkan::hasRequiredSwapchainSupport(swapchain_support_details));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenSwapchainSupportHasNoPresentationModes) {
    const VkSurfaceFormatKHR surface_format{
        .format = VK_FORMAT_B8G8R8A8_SRGB,
        .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
    };

    const std::vector<VkSurfaceFormatKHR> available_surface_formats{
        surface_format,
    };

    const Vulkan::SwapchainSupportDetails swapchain_support_details{
        .available_surface_formats = available_surface_formats,
        .available_presentation_modes = {},
    };

    EXPECT_FALSE(
        Vulkan::hasRequiredSwapchainSupport(swapchain_support_details));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenSwapchainSupportHasNoFormatsOrPresentationModes) {
    const Vulkan::SwapchainSupportDetails swapchain_support_details{
        .available_surface_formats = {},
        .available_presentation_modes = {},
    };

    EXPECT_FALSE(
        Vulkan::hasRequiredSwapchainSupport(swapchain_support_details));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsTrueWhenPhysicalDeviceMeetsAllRequirements) {
    const Vulkan::QueueFamilyIndices queue_family_indices{
        .graphics_family = std::uint32_t{0},
        .presentation_family = std::uint32_t{0},
    };

    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    EXPECT_TRUE(Vulkan::isPhysicalDeviceSuitable(
        queue_family_indices, available_extensions,
        makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        makeAdequateSwapchainSupport()));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenDynamicRenderingIsUnsupported) {
    const Vulkan::QueueFamilyIndices queue_family_indices{
        .graphics_family = std::uint32_t{0},
        .presentation_family = std::uint32_t{0},
    };

    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    EXPECT_FALSE(Vulkan::isPhysicalDeviceSuitable(
        queue_family_indices, available_extensions,
        makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion, false,
                                       true),
        makeAdequateSwapchainSupport()));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenSynchronization2IsUnsupported) {
    const Vulkan::QueueFamilyIndices queue_family_indices{
        .graphics_family = std::uint32_t{0},
        .presentation_family = std::uint32_t{0},
    };

    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    EXPECT_FALSE(Vulkan::isPhysicalDeviceSuitable(
        queue_family_indices, available_extensions,
        makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion, true,
                                       false),
        makeAdequateSwapchainSupport()));
}

TEST(VulkanDeviceSelectionTests, ReturnsFalseWhenGraphicsQueueFamilyIsMissing) {
    const Vulkan::QueueFamilyIndices queue_family_indices{
        .graphics_family = std::nullopt,
        .presentation_family = std::uint32_t{0},
    };

    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    EXPECT_FALSE(Vulkan::isPhysicalDeviceSuitable(
        queue_family_indices, available_extensions,
        makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        makeAdequateSwapchainSupport()));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenPresentationQueueFamilyIsMissing) {
    const Vulkan::QueueFamilyIndices queue_family_indices{
        .graphics_family = std::uint32_t{0},
        .presentation_family = std::nullopt,
    };

    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    EXPECT_FALSE(Vulkan::isPhysicalDeviceSuitable(
        queue_family_indices, available_extensions,
        makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        makeAdequateSwapchainSupport()));
}

TEST(VulkanDeviceSelectionTests,
     ReturnsFalseWhenRequiredDeviceExtensionsAreMissing) {
    const Vulkan::QueueFamilyIndices queue_family_indices{
        .graphics_family = std::uint32_t{0},
        .presentation_family = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{};

    EXPECT_FALSE(Vulkan::isPhysicalDeviceSuitable(
        queue_family_indices, available_extensions,
        makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        makeAdequateSwapchainSupport()));
}

TEST(VulkanDeviceSelectionTests, ReturnsFalseWhenSwapchainSupportIsInadequate) {
    const Vulkan::QueueFamilyIndices queue_family_indices{
        .graphics_family = std::uint32_t{0},
        .presentation_family = std::uint32_t{0},
    };

    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    const Vulkan::SwapchainSupportDetails swapchain_support_details{
        .available_surface_formats = {},
        .available_presentation_modes = {},
    };

    EXPECT_FALSE(Vulkan::isPhysicalDeviceSuitable(
        queue_family_indices, available_extensions,
        makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        swapchain_support_details));
}

TEST(VulkanDeviceSelectionTests, ReturnsFirstSuitablePhysicalDevice) {
    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    const Vulkan::DiscoveredPhysicalDevice first_device{
        .handle = VK_NULL_HANDLE,
        .queue_family_indices =
            {
                .graphics_family = std::uint32_t{0},
                .presentation_family = std::uint32_t{0},
            },
        .queue_family_properties = makeQueueFamilyProperties(),
        .available_extensions = available_extensions,
        .capabilities =
            makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        .swapchain_support = makeAdequateSwapchainSupport(),
    };

    const Vulkan::DiscoveredPhysicalDevice second_device{
        .handle = VK_NULL_HANDLE,
        .queue_family_indices =
            {
                .graphics_family = std::uint32_t{1},
                .presentation_family = std::uint32_t{1},
            },
        .queue_family_properties = makeQueueFamilyProperties(),
        .available_extensions = available_extensions,
        .capabilities =
            makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        .swapchain_support = makeAdequateSwapchainSupport(),
    };

    const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices{
        first_device,
        second_device,
    };

    const std::optional<Vulkan::SelectedPhysicalDevice> selected =
        Vulkan::selectPhysicalDevice(discovered_devices);

    if (!selected.has_value()) {
        FAIL() << "Expected a physical device to be selected";
        return;
    }

    const Vulkan::SelectedPhysicalDevice &selected_device = selected.value();

    EXPECT_EQ(selected_device.queue_families.graphics_family.family_index,
              std::uint32_t{0});

    EXPECT_EQ(selected_device.queue_families.presentation_family.family_index,
              std::uint32_t{0});

    EXPECT_EQ(
        selected_device.queue_families.graphics_family.available_queue_count,
        std::uint32_t{2});

    EXPECT_EQ(selected_device.queue_families.presentation_family
                  .available_queue_count,
              std::uint32_t{2});
}

TEST(VulkanDeviceSelectionTests, SkipsUnsuitablePhysicalDevices) {
    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const VkExtensionProperties unrelated_extension{
        .extensionName = VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> swapchain_extensions{
        swapchain_extension,
    };

    const std::vector<VkExtensionProperties> unrelated_extensions{
        unrelated_extension,
    };

    const Vulkan::DiscoveredPhysicalDevice suitable_device{
        .handle = VK_NULL_HANDLE,
        .queue_family_indices =
            {
                .graphics_family = std::uint32_t{0},
                .presentation_family = std::uint32_t{0},
            },
        .queue_family_properties = makeQueueFamilyProperties(),
        .available_extensions = swapchain_extensions,
        .capabilities =
            makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        .swapchain_support = makeAdequateSwapchainSupport(),
    };

    const Vulkan::DiscoveredPhysicalDevice unsuitable_device{
        .handle = VK_NULL_HANDLE,
        .queue_family_indices =
            {
                .graphics_family = std::uint32_t{1},
                .presentation_family = std::uint32_t{1},
            },
        .queue_family_properties = makeQueueFamilyProperties(),
        .available_extensions = unrelated_extensions,
        .capabilities =
            makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        .swapchain_support = makeAdequateSwapchainSupport(),
    };

    const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices{
        unsuitable_device,
        suitable_device,
    };

    const std::optional<Vulkan::SelectedPhysicalDevice> selected =
        Vulkan::selectPhysicalDevice(discovered_devices);

    if (!selected.has_value()) {
        FAIL() << "Expected a physical device to be selected";
        return;
    }

    const Vulkan::SelectedPhysicalDevice &selected_device = selected.value();

    EXPECT_EQ(selected_device.queue_families.graphics_family.family_index,
              std::uint32_t{0});

    EXPECT_EQ(selected_device.queue_families.presentation_family.family_index,
              std::uint32_t{0});
}

TEST(VulkanDeviceSelectionTests, ReturnsNulloptWhenNoPhysicalDeviceIsSuitable) {
    const VkExtensionProperties unrelated_extension{
        .extensionName = VK_KHR_DEVICE_GROUP_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> unrelated_extensions{
        unrelated_extension,
    };

    const Vulkan::DiscoveredPhysicalDevice first_unsuitable_device{
        .handle = VK_NULL_HANDLE,
        .queue_family_indices =
            {
                .graphics_family = std::uint32_t{0},
                .presentation_family = std::uint32_t{0},
            },
        .queue_family_properties = makeQueueFamilyProperties(),
        .available_extensions = unrelated_extensions,
        .capabilities =
            makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        .swapchain_support = makeAdequateSwapchainSupport(),
    };

    const Vulkan::DiscoveredPhysicalDevice second_unsuitable_device{
        .handle = VK_NULL_HANDLE,
        .queue_family_indices =
            {
                .graphics_family = std::uint32_t{1},
                .presentation_family = std::uint32_t{1},
            },
        .queue_family_properties = makeQueueFamilyProperties(),
        .available_extensions = unrelated_extensions,
        .capabilities =
            makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        .swapchain_support = makeAdequateSwapchainSupport(),
    };

    const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices{
        first_unsuitable_device,
        second_unsuitable_device,
    };

    const std::optional<Vulkan::SelectedPhysicalDevice> selected =
        Vulkan::selectPhysicalDevice(discovered_devices);

    EXPECT_FALSE(selected.has_value());
}

TEST(VulkanDeviceSelectionTests,
     ReturnsNulloptWhenApiVersionIsBelowRequiredVersion) {
    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> swapchain_extensions{
        swapchain_extension,
    };

    const Vulkan::DiscoveredPhysicalDevice unsuitable_device{
        .handle = VK_NULL_HANDLE,
        .queue_family_indices =
            {
                .graphics_family = std::uint32_t{0},
                .presentation_family = std::uint32_t{0},
            },
        .queue_family_properties = makeQueueFamilyProperties(),
        .available_extensions = swapchain_extensions,
        .capabilities =
            makePhysicalDeviceCapabilities(VK_MAKE_API_VERSION(0, 1, 3, 999)),
        .swapchain_support = makeAdequateSwapchainSupport(),
    };

    const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices{
        unsuitable_device,
    };

    const std::optional<Vulkan::SelectedPhysicalDevice> selected =
        Vulkan::selectPhysicalDevice(discovered_devices);

    EXPECT_FALSE(selected.has_value());
}

TEST(VulkanDeviceSelectionTests,
     ReturnsNulloptWhenDiscoveredDeviceListIsEmpty) {
    const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices{};

    const std::optional<Vulkan::SelectedPhysicalDevice> selected =
        Vulkan::selectPhysicalDevice(discovered_devices);

    EXPECT_FALSE(selected.has_value());
}

TEST(VulkanDeviceSelectionTests, PreservesSelectedQueueFamilyCapacity) {
    const VkExtensionProperties swapchain_extension{
        .extensionName = VK_KHR_SWAPCHAIN_EXTENSION_NAME,
        .specVersion = std::uint32_t{0},
    };

    const std::vector<VkExtensionProperties> available_extensions{
        swapchain_extension,
    };

    const Vulkan::DiscoveredPhysicalDevice device{
        .handle = VK_NULL_HANDLE,
        .queue_family_indices =
            {
                .graphics_family = std::uint32_t{0},
                .presentation_family = std::uint32_t{1},
            },
        .queue_family_properties = makeQueueFamilyProperties(),
        .available_extensions = available_extensions,
        .capabilities =
            makePhysicalDeviceCapabilities(Vulkan::kRequiredApiVersion),
        .swapchain_support = makeAdequateSwapchainSupport(),
    };

    const std::vector<Vulkan::DiscoveredPhysicalDevice> discovered_devices{
        device,
    };

    const std::optional<Vulkan::SelectedPhysicalDevice> selected =
        Vulkan::selectPhysicalDevice(discovered_devices);

    if (!selected.has_value()) {
        FAIL() << "Expected a physical device to be selected";
        return;
    }

    const Vulkan::SelectedPhysicalDevice &selected_device = selected.value();

    EXPECT_EQ(selected_device.queue_families.graphics_family.family_index,
              std::uint32_t{0});

    EXPECT_EQ(
        selected_device.queue_families.graphics_family.available_queue_count,
        std::uint32_t{2});

    EXPECT_EQ(selected_device.queue_families.presentation_family.family_index,
              std::uint32_t{1});

    EXPECT_EQ(selected_device.queue_families.presentation_family
                  .available_queue_count,
              std::uint32_t{4});
}
