#include "vulkan_vertex_format.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto toVkFormat(VertexFormat format) -> VkFormat {
        switch (format) {
        case VertexFormat::Float32x2:
            return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::Float32x3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan vertex-format conversion requires a supported VertexFormat "
            "value");
    }
} // namespace SNE::Engine::Renderer::Vulkan
