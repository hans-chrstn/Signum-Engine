#include "vulkan_queue_requests.hpp"
#include "vulkan_queue_families.hpp"
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    auto deriveUniqueQueueFamilyRequests(
        const SelectedQueueFamilyIndices &queue_family_indices)
        -> std::vector<QueueFamilyRequest> {
        std::vector<QueueFamilyRequest> requests{
            {.family_index = queue_family_indices.graphics_family},
        };

        if (queue_family_indices.presentation_family !=
            queue_family_indices.graphics_family) {
            requests.push_back({
                .family_index = queue_family_indices.presentation_family,
            });
        }
        return requests;
    }
} // namespace SNE::Engine::Renderer::Vulkan
