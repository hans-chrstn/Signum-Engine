#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanQueueRequestsTests,
     ReturnsSingleRequestWhenGraphicsAndPresentationShareQueueFamily) {
    const Vulkan::SelectedQueueFamilyIndices queue_family_indices{
        .graphics_family = std::uint32_t{0},
        .presentation_family = std::uint32_t{0},
    };

    const std::vector<Vulkan::QueueFamilyRequest> requests =
        Vulkan::deriveUniqueQueueFamilyRequests(queue_family_indices);

    ASSERT_EQ(requests.size(), std::size_t{1});
    EXPECT_EQ(requests[0].family_index, std::uint32_t{0});
}

TEST(
    VulkanQueueRequestsTests,
    ReturnsDistinctRequestsWhenGraphicsAndPresentationUseDifferentQueueFamilies) {
    const Vulkan::SelectedQueueFamilyIndices queue_family_indices{
        .graphics_family = std::uint32_t{0},
        .presentation_family = std::uint32_t{1},
    };

    const std::vector<Vulkan::QueueFamilyRequest> requests =
        Vulkan::deriveUniqueQueueFamilyRequests(queue_family_indices);

    ASSERT_EQ(requests.size(), std::size_t{2});
    EXPECT_EQ(requests[0].family_index, std::uint32_t{0});
    EXPECT_EQ(requests[1].family_index, std::uint32_t{1});
}
