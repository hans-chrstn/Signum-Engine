#include "vulkan_texture_address_mode.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/texture/texture_address_mode.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto toVkSamplerAddressMode(TextureAddressMode mode)
        -> VkSamplerAddressMode {
        switch (mode) {
        case TextureAddressMode::Repeat:
            return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case TextureAddressMode::MirroredRepeat:
            return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case TextureAddressMode::ClampToEdge:
            return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Vulkan,
            "Vulkan texture-address-mode conversion requires a supported "
            "TextureAddressMode value");
    }
} // namespace SNE::Engine::Renderer::Vulkan
