#include "vulkan_texture_filter.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto toVkFilter(TextureFilter filter) -> VkFilter {
        switch (filter) {
        case TextureFilter::Nearest:
            return VK_FILTER_NEAREST;
        case TextureFilter::Linear:
            return VK_FILTER_LINEAR;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan texture-filter conversion requires a supported "
            "TextureFilter value");
    }
} // namespace SNE::Engine::Renderer::Vulkan
