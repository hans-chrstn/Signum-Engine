#pragma once

#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Updates resource bindings in Vulkan descriptor sets.
     *
     * Writes descriptor information into existing descriptor sets.
     * The descriptor sets must already be allocated from a valid
     * descriptor pool.
     *
     * Each write identifies a destination descriptor set, binding,
     * descriptor type, and the resource information to assign.
     *
     * This function does not own the device, descriptor sets,
     * or resources referenced by the descriptor writes.
     *
     * @param device Logical device used to update the descriptor sets.
     * @param writes Descriptor writes to apply.
     *
     * @pre device must be a valid logical-device handle.
     * @pre The number of writes must fit within std::uint32_t.
     * @pre Every write must reference a valid destination descriptor set.
     * @pre Every write must satisfy Vulkan descriptor-update requirements.
     *
     * @note Descriptor updates must follow Vulkan's synchronization
     * and descriptor-set lifetime requirements.
     */
    auto updateDescriptorSets(VkDevice device,
                              std::span<const VkWriteDescriptorSet> writes)
        -> void;
} // namespace SNE::Engine::Renderer::Vulkan
