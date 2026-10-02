#pragma once

#include <cstdint>
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    struct SelectedQueueFamilies;

    /**
     * @brief Describes a queue family requested for logical-device creation.
     *
     * Represents one unique Vulkan queue family from which Signum intends to
     * create one or more queues.
     */
    struct QueueFamilyRequest {
        /** Index of the queue family to request from the logical device. */
        std::uint32_t family_index{};
        std::vector<float> priorities;
    };

    /**
     * @brief Derives the unique queue-family requests required by a logical
     * device.
     *
     * Converts the selected graphics and presentation queue-family indices into
     * unique queue-family requests. When both roles use the same family, that
     * family is requested only once.
     *
     * @param queue_families Graphics and presentation queue-family
     * indices.
     *
     * @return Unique queue-family requests derived from the supplied indices.
     */
    [[nodiscard]] auto
    deriveUniqueQueueFamilyRequests(const SelectedQueueFamilies &queue_families)
        -> std::vector<QueueFamilyRequest>;
} // namespace SNE::Engine::Renderer::Vulkan
