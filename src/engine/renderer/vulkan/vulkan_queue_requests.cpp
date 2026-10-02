#include "vulkan_queue_requests.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "vulkan_queue_families.hpp"
#include <vector>

namespace SNE::Engine::Renderer::Vulkan {
    auto
    deriveUniqueQueueFamilyRequests(const SelectedQueueFamilies &queue_families)
        -> std::vector<QueueFamilyRequest> {
        std::vector<float> priorities{};
        priorities.push_back(1.0F);
        std::vector<QueueFamilyRequest> requests{};

        if (static_cast<std::uint32_t>(priorities.size()) >
            queue_families.graphics_family.available_queue_count) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Graphics queue request exceeds the selected queue family's "
                "available capacity");
        }

        requests.push_back({
            .family_index = queue_families.graphics_family.family_index,
            .priorities = priorities,
        });

        if (queue_families.presentation_family.family_index !=
            queue_families.graphics_family.family_index) {

            if (static_cast<std::uint32_t>(priorities.size()) >
                queue_families.presentation_family.available_queue_count) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Presentation queue request exceeds the selected queue "
                    "family's available capacity");
            }

            requests.push_back({
                .family_index = queue_families.presentation_family.family_index,
                .priorities = priorities,
            });
        }

        return requests;
    }
} // namespace SNE::Engine::Renderer::Vulkan
