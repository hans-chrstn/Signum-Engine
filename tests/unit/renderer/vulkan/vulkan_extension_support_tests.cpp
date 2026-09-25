#include "engine/renderer/vulkan/vulkan_extension_support.hpp"
#include <gtest/gtest.h>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanExtensionSupportTests,
     ReturnsTrueWhenAllRequiredExtensionsAreAvailable) {
    const std::vector<const char *> required_extensions{
        "VK_KHR_surface",
        "VK_EXT_debug_utils",
    };

    const std::vector<VkExtensionProperties> available_extensions{
        {"VK_KHR_surface", 0},
        {"VK_EXT_debug_utils", 0},
    };

    EXPECT_TRUE(Vulkan::hasRequiredExtensions(required_extensions,
                                              available_extensions));
}

TEST(VulkanExtensionSupportTests, ReturnsFalseWhenRequiredExtensionIsMissing) {
    const std::vector<const char *> required_extensions{
        "VK_KHR_surface",
        "VK_EXT_debug_utils",
    };

    const std::vector<VkExtensionProperties> available_extensions{
        {"VK_KHR_surface", 0},
    };

    EXPECT_FALSE(Vulkan::hasRequiredExtensions(required_extensions,
                                               available_extensions));
}

TEST(VulkanExtensionSupportTests,
     ReturnsTrueWhenAvailableExtensionsContainExtras) {
    const std::vector<const char *> required_extensions{
        "VK_KHR_surface",
        "VK_EXT_debug_utils",
    };

    const std::vector<VkExtensionProperties> available_extensions{
        {"VK_KHR_surface", 0},
        {"VK_EXT_debug_utils", 0},
        {"VK_TEST_EXT", 0},
    };

    EXPECT_TRUE(Vulkan::hasRequiredExtensions(required_extensions,
                                              available_extensions));
}

TEST(VulkanExtensionSupportTests, ReturnsTrueWhenNoExtensionsAreRequired) {
    const std::vector<const char *> required_extensions{};

    const std::vector<VkExtensionProperties> available_extensions{
        {"VK_KHR_surface", 0},
        {"VK_EXT_debug_utils", 0},
    };

    EXPECT_TRUE(Vulkan::hasRequiredExtensions(required_extensions,
                                              available_extensions));
}

TEST(VulkanExtensionSupportTests,
     ReturnsFalseWhenNoExtensionsAreAvailableButExtensionsAreRequired) {
    const std::vector<const char *> required_extensions{
        "VK_KHR_surface",
        "VK_EXT_debug_utils",
    };

    const std::vector<VkExtensionProperties> available_extensions{};

    EXPECT_FALSE(Vulkan::hasRequiredExtensions(required_extensions,
                                               available_extensions));
}
