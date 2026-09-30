#include "vulkan_queue_requests.hpp"
#include "vulkan_queue_families.hpp"
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    auto deriveUniqueQueueFamilyRequests(
        const QueueFamilyIndices &queue_family_indices)
        -> std::vector<QueueFamilyRequest> {
        std::vector<QueueFamilyRequest> requests{};
        if (queue_family_indices.graphics_family.has_value()) {
            requests.push_back(QueueFamilyRequest{
                .family_index = queue_family_indices.graphics_family.value(),
            });
        }

        if (queue_family_indices.presentation_family.has_value() &&
            queue_family_indices.presentation_family !=
                queue_family_indices.graphics_family) {
            requests.push_back(QueueFamilyRequest{
                .family_index =
                    queue_family_indices.presentation_family.value(),
            });
        }

        return requests;
    }
} // namespace SNE::Engine::Renderer::Vulkan
