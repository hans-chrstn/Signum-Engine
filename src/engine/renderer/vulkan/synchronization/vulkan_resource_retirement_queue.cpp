#include "vulkan_resource_retirement_queue.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "vulkan_queue_submission_state.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    VulkanResourceRetirementQueue::~VulkanResourceRetirementQueue() noexcept {
        if (!m_PendingResources.empty()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "VulkanResourceRetirementQueue must be empty before "
                "destruction");
        }
    }

    auto VulkanResourceRetirementQueue::collectCompleted(
        const VulkanQueueSubmissionState &submission_state) -> void {
        for (RetiredResource &resource : m_PendingResources) {
            if (submission_state.isComplete(resource.submission_id)) {
                resource.release();
            }
        }

        std::erase_if(
            m_PendingResources,
            [&submission_state](const RetiredResource &resource) -> bool {
                return submission_state.isComplete(resource.submission_id);
            });
    }
} // namespace SNE::Engine::Renderer::Vulkan
