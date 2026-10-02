#include "engine/renderer/vulkan/vulkan_queue_families.hpp"
#include "engine/renderer/vulkan/vulkan_queue_requests.hpp"

#include <cstddef>
#include <gtest/gtest.h>
#include <vector>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanQueueRequestsTests,
     ReturnsSingleRequestWhenGraphicsAndPresentationShareQueueFamily) {
    const Vulkan::SelectedQueueFamilies queue_families{
        .graphics_family =
            {
                .family_index = 0U,
                .available_queue_count = 1U,
            },
        .presentation_family =
            {
                .family_index = 0U,
                .available_queue_count = 1U,
            },
    };

    const std::vector<Vulkan::QueueFamilyRequest> requests =
        Vulkan::deriveUniqueQueueFamilyRequests(queue_families);

    ASSERT_EQ(requests.size(), std::size_t{1});

    EXPECT_EQ(requests[0].family_index, 0U);

    ASSERT_EQ(requests[0].priorities.size(), std::size_t{1});
    EXPECT_FLOAT_EQ(requests[0].priorities[0], 1.0F);
}

TEST(
    VulkanQueueRequestsTests,
    ReturnsDistinctRequestsWhenGraphicsAndPresentationUseDifferentQueueFamilies) {
    const Vulkan::SelectedQueueFamilies queue_families{
        .graphics_family =
            {
                .family_index = 0U,
                .available_queue_count = 1U,
            },
        .presentation_family =
            {
                .family_index = 1U,
                .available_queue_count = 1U,
            },
    };

    const std::vector<Vulkan::QueueFamilyRequest> requests =
        Vulkan::deriveUniqueQueueFamilyRequests(queue_families);

    ASSERT_EQ(requests.size(), std::size_t{2});

    EXPECT_EQ(requests[0].family_index, 0U);
    ASSERT_EQ(requests[0].priorities.size(), std::size_t{1});
    EXPECT_FLOAT_EQ(requests[0].priorities[0], 1.0F);

    EXPECT_EQ(requests[1].family_index, 1U);
    ASSERT_EQ(requests[1].priorities.size(), std::size_t{1});
    EXPECT_FLOAT_EQ(requests[1].priorities[0], 1.0F);
}

TEST(VulkanQueueRequestsTests,
     RejectsGraphicsRequestWhenCapacityIsInsufficient) {
    const Vulkan::SelectedQueueFamilies requested{
        .graphics_family =
            {
                .family_index = 1U,
                .available_queue_count = 0U,
            },
        .presentation_family =
            {
                .family_index = 1U,
                .available_queue_count = 0U,
            },
    };

    ASSERT_DEATH(
        static_cast<void>(Vulkan::deriveUniqueQueueFamilyRequests(requested)),
        "Graphics queue request exceeds the selected queue family's "
        "available capacity");
}

TEST(VulkanQueueRequestsTests,
     RejectsDistinctPresentationRequestWhenCapacityIsInsufficient) {
    const Vulkan::SelectedQueueFamilies requested{
        .graphics_family =
            {
                .family_index = 1U,
                .available_queue_count = 1U,
            },
        .presentation_family =
            {
                .family_index = 2U,
                .available_queue_count = 0U,
            },
    };

    ASSERT_DEATH(
        static_cast<void>(Vulkan::deriveUniqueQueueFamilyRequests(requested)),
        "Presentation queue request exceeds the selected queue "
        "family's available capacity");
}
