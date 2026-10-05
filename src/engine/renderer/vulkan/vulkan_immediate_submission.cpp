#include "vulkan_immediate_submission.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/core/error/subsystem.hpp"
#include "vulkan_result.hpp"
#include <string>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanImmediateSubmission::VulkanImmediateSubmission(
        VkDevice device, VkQueue queue, std::uint32_t queue_family_index)
        : m_Queue(queue), m_VulkanCommandPool(device, queue_family_index),
          m_VulkanFence(device, false) {
        if (queue == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImmediateSubmission requires a valid Vulkan queue");
        }
        const std::vector<VkCommandBuffer> command_buffers =
            m_VulkanCommandPool.allocateCommandBuffers(
                VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1U);

        m_CommandBuffer = command_buffers[0];
    }

    auto VulkanImmediateSubmission::execute(
        const std::function<void(VkCommandBuffer)> &record) -> void {
        if (!record) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanImmediateSubmission::execute requires a valid recording "
                "callback");
        }

        m_VulkanCommandPool.reset();

        VkCommandBufferBeginInfo command_buffer_begin_info{};
        command_buffer_begin_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        command_buffer_begin_info.flags =
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        const VkResult command_buffer_begin_result =
            vkBeginCommandBuffer(m_CommandBuffer, &command_buffer_begin_info);

        if (command_buffer_begin_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandBufferBeginFailed,
                "Failed to begin immediate Vulkan command buffer recording",
                Core::Error::NativeError(
                    static_cast<int>(command_buffer_begin_result),
                    std::string(toString(command_buffer_begin_result))),
                "Begin Immediate Vulkan Command Buffer");
        }

        record(m_CommandBuffer);

        const VkResult command_buffer_end_result =
            vkEndCommandBuffer(m_CommandBuffer);

        if (command_buffer_end_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanCommandBufferEndFailed,
                "Failed to end immediate Vulkan command buffer recording",
                Core::Error::NativeError(
                    static_cast<int>(command_buffer_end_result),
                    std::string(toString(command_buffer_end_result))),
                "End Immediate Vulkan Command Buffer");
        }

        VkCommandBufferSubmitInfo command_buffer_submit_info{};
        command_buffer_submit_info.sType =
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        command_buffer_submit_info.commandBuffer = m_CommandBuffer;
        command_buffer_submit_info.deviceMask = 0U;

        VkSubmitInfo2 submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submit_info.commandBufferInfoCount = 1U;
        submit_info.pCommandBufferInfos = &command_buffer_submit_info;

        m_VulkanFence.reset();

        const VkResult queue_submit_result = vkQueueSubmit2(
            m_Queue, 1U, &submit_info, m_VulkanFence.nativeHandle());

        if (queue_submit_result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanQueueSubmissionFailed,
                "Failed to submit immediate Vulkan command buffer",
                Core::Error::NativeError(
                    static_cast<int>(queue_submit_result),
                    std::string(toString(queue_submit_result))),
                "Submit Immediate Vulkan Command Buffer");
        }

        m_VulkanFence.wait();
    }
} // namespace SNE::Engine::Renderer::Vulkan
