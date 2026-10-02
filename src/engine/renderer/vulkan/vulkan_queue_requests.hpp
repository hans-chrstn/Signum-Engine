#pragma once

#include <cstdint>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    struct SelectedQueueFamilies;

    /**
     * @brief Describes queues requested from one Vulkan queue family.
     *
     * Represents one unique Vulkan queue family from which Signum intends to
     * create one or more queues.
     *
     * The number of entries in priorities determines the number of queues
     * requested from the family. Each entry specifies the Vulkan scheduling
     * priority for the corresponding queue.
     */
    struct QueueFamilyRequest {
        /** Index of the Vulkan queue family from which queues are requested. */
        std::uint32_t family_index{};
        /** Queue priorities, with one entry for each requested queue. */
        std::vector<float> priorities;
    };

    /**
     * @brief Derives queue requests for the selected renderer queue families.
     *
     * Creates one queue-family request for each unique graphics and
     * presentation queue family selected for renderer use.
     *
     * When graphics and presentation use the same queue family, only one
     * request is produced for that family.
     *
     * The current queue policy requests one queue with priority 1.0 from each
     * required unique queue family.
     *
     * @param queue_families Selected graphics and presentation queue families.
     *
     * @return Unique queue-family requests required by the current renderer
     * policy.
     */
    [[nodiscard]] auto
    deriveUniqueQueueFamilyRequests(const SelectedQueueFamilies &queue_families)
        -> std::vector<QueueFamilyRequest>;
} // namespace SNE::Engine::Renderer::Vulkan
