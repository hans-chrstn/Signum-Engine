#include "engine/renderer/vulkan/vulkan_result.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <string_view>

struct VulkanResultTestCase {
    VkResult result;
    std::string_view expected_name;
};

class VulkanResultTests : public testing::TestWithParam<VulkanResultTestCase> {
};

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
constexpr std::int32_t kUnknownVkResultValue = 123456789;

TEST_P(VulkanResultTests, HasReadableName) {
    const VulkanResultTestCase &test_case = GetParam();

    EXPECT_EQ(Vulkan::toString(test_case.result), test_case.expected_name);
}

INSTANTIATE_TEST_SUITE_P(
    KnownResults, VulkanResultTests,
    ::testing::Values(VulkanResultTestCase{VK_SUCCESS, "VK_SUCCESS"},
                      VulkanResultTestCase{VK_TIMEOUT, "VK_TIMEOUT"},
                      VulkanResultTestCase{VK_ERROR_OUT_OF_HOST_MEMORY,
                                           "VK_ERROR_OUT_OF_HOST_MEMORY"},
                      VulkanResultTestCase{VK_ERROR_EXTENSION_NOT_PRESENT,
                                           "VK_ERROR_EXTENSION_NOT_PRESENT"},
                      VulkanResultTestCase{VK_SUBOPTIMAL_KHR,
                                           "VK_SUBOPTIMAL_KHR"}));

TEST(VulkanResultFallbackTests, ReturnsUnknownForInvalidValue) {
    // NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    const auto unknown_result = static_cast<VkResult>(kUnknownVkResultValue);

    EXPECT_EQ(Vulkan::toString(unknown_result), "VK_UNKNOWN_RESULT");
}
