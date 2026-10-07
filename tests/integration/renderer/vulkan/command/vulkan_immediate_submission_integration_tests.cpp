#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/command/vulkan_immediate_submission.hpp"
#include <cstdint>
#include <gtest/gtest.h>
#include <stdexcept>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

TEST_F(VulkanIntegrationTest, ExecutesImmediateSubmissionCallback) {
    VkQueue graphics_queue{VK_NULL_HANDLE};
    vkGetDeviceQueue(m_Device, m_SelectedQueueFamily.family_index, 0U,
                     &graphics_queue);

    ASSERT_NE(graphics_queue, VK_NULL_HANDLE);

    Vulkan::VulkanImmediateSubmission submission{
        m_Device,
        graphics_queue,
        m_SelectedQueueFamily.family_index,
    };

    bool callback_called{false};

    submission.execute(
        [&](VkCommandBuffer) -> void { callback_called = true; });

    EXPECT_TRUE(callback_called);
}

TEST_F(VulkanIntegrationTest, ReusesImmediateSubmissionAcrossExecutions) {
    VkQueue graphics_queue{VK_NULL_HANDLE};
    vkGetDeviceQueue(m_Device, m_SelectedQueueFamily.family_index, 0U,
                     &graphics_queue);

    ASSERT_NE(graphics_queue, VK_NULL_HANDLE);

    Vulkan::VulkanImmediateSubmission submission{
        m_Device,
        graphics_queue,
        m_SelectedQueueFamily.family_index,
    };

    std::uint32_t execution_count{};

    submission.execute([&](VkCommandBuffer) -> void { ++execution_count; });

    submission.execute([&](VkCommandBuffer) -> void { ++execution_count; });

    EXPECT_EQ(execution_count, 2U);
}

TEST_F(VulkanIntegrationTest, RecoversAfterRecordingCallbackThrows) {
    VkQueue graphics_queue{VK_NULL_HANDLE};
    vkGetDeviceQueue(m_Device, m_SelectedQueueFamily.family_index, 0U,
                     &graphics_queue);

    ASSERT_NE(graphics_queue, VK_NULL_HANDLE);

    Vulkan::VulkanImmediateSubmission submission{
        m_Device,
        graphics_queue,
        m_SelectedQueueFamily.family_index,
    };

    bool recording_failure_observed{false};

    try {
        submission.execute([](VkCommandBuffer) -> void {
            throw std::runtime_error("Recording callback failure");
        });
    } catch (const std::runtime_error &) {
        recording_failure_observed = true;
    }

    EXPECT_TRUE(recording_failure_observed);

    bool callback_called{false};

    submission.execute(
        [&](VkCommandBuffer) -> void { callback_called = true; });

    EXPECT_TRUE(callback_called);
}
