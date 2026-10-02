#include "vulkan_queue_requests.hpp"
#include "vulkan_queue_families.hpp"
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    auto
    deriveUniqueQueueFamilyRequests(const SelectedQueueFamilies &queue_families)
        -> std::vector<QueueFamilyRequest> {
        std::vector<float> priorities{};
        priorities.push_back(1.0F);
        std::vector<QueueFamilyRequest> requests{
            {
                .family_index = queue_families.graphics_family.family_index,
                .priorities = priorities,
            },
        };

        if (queue_families.presentation_family.family_index !=
            queue_families.graphics_family.family_index) {
            requests.push_back({
                .family_index = queue_families.presentation_family.family_index,
                .priorities = priorities,
            });
        }
        return requests;
    }
} // namespace SNE::Engine::Renderer::Vulkan
