#include "vulkan_image_barrier.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto recordImageBarrier(VkCommandBuffer command_buffer,
                            const VkImageMemoryBarrier2 &barrier) -> void {
        if (command_buffer == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordImageBarrier requires a valid command buffer");
        }

        if (barrier.image == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "recordImageBarrier requires a valid Vulkan image");
        }

        VkDependencyInfo dependency_info{};
        dependency_info.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        dependency_info.imageMemoryBarrierCount = 1U;
        dependency_info.pImageMemoryBarriers = &barrier;

        vkCmdPipelineBarrier2(command_buffer, &dependency_info);
    }
} // namespace SNE::Engine::Renderer::Vulkan
