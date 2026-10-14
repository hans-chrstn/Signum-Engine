#include "vulkan_descriptor_updates.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/numeric/checked_conversion.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto updateDescriptorSets(VkDevice device,
                              std::span<const VkWriteDescriptorSet> writes)
        -> void {
        if (device == VK_NULL_HANDLE) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Descriptor updates require a valid Vulkan device");
        }

        const auto writes_count =
            Core::Numeric::tryConvertToUint32(writes.size());
        if (!writes_count.has_value()) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Descriptor update count exceeds uint32_t");
        }

        for (const VkWriteDescriptorSet &write : writes) {
            if (write.dstSet == VK_NULL_HANDLE) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Descriptor updates require valid destination "
                    "descriptor-set handles");
            }
        }

        vkUpdateDescriptorSets(device, writes_count.value(), writes.data(), 0U,
                               nullptr);
    }
} // namespace SNE::Engine::Renderer::Vulkan
