#include "vulkan_texture_format.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto toVkFormat(TextureFormat format) -> VkFormat {
        switch (format) {
        case TextureFormat::Rgba8Unorm:
            return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::Rgba8Srgb:
            return VK_FORMAT_R8G8B8A8_SRGB;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan texture-format conversion requires a supported "
            "TextureFormat value");
    }
} // namespace SNE::Engine::Renderer::Vulkan
