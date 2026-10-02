#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"

#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanQueueRequestsTests,
     ReturnsSingleRequestWhenGraphicsAndPresentationShareQueueFamily) {
    const Vulkan::SelectedQueueFamilies queue_families{
        .graphics_family =
            {
                .family_index = std::uint32_t{0},
                .available_queue_count = 1U,
            },
        .presentation_family =
            {
                .family_index = std::uint32_t{0},
                .available_queue_count = 1U,
            },
    };

    const std::vector<Vulkan::QueueFamilyRequest> requests =
        Vulkan::deriveUniqueQueueFamilyRequests(queue_families);

    ASSERT_EQ(requests.size(), std::size_t{1});

    EXPECT_EQ(requests[0].family_index, std::uint32_t{0});

    ASSERT_EQ(requests[0].priorities.size(), std::size_t{1});
    EXPECT_FLOAT_EQ(requests[0].priorities[0], 1.0F);
}

TEST(
    VulkanQueueRequestsTests,
    ReturnsDistinctRequestsWhenGraphicsAndPresentationUseDifferentQueueFamilies) {
    const Vulkan::SelectedQueueFamilies queue_families{
        .graphics_family =
            {
                .family_index = std::uint32_t{0},
                .available_queue_count = 1U,
            },
        .presentation_family =
            {
                .family_index = std::uint32_t{1},
                .available_queue_count = 1U,
            },
    };

    const std::vector<Vulkan::QueueFamilyRequest> requests =
        Vulkan::deriveUniqueQueueFamilyRequests(queue_families);

    ASSERT_EQ(requests.size(), std::size_t{2});

    EXPECT_EQ(requests[0].family_index, std::uint32_t{0});
    ASSERT_EQ(requests[0].priorities.size(), std::size_t{1});
    EXPECT_FLOAT_EQ(requests[0].priorities[0], 1.0F);

    EXPECT_EQ(requests[1].family_index, std::uint32_t{1});
    ASSERT_EQ(requests[1].priorities.size(), std::size_t{1});
    EXPECT_FLOAT_EQ(requests[1].priorities[0], 1.0F);
}
