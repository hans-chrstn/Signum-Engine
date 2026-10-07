#pragma once
#include <cstdint>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Minimum Vulkan API version required by the Signum Vulkan backend.
     *
     * Defines the Vulkan API baseline that must be supported by both the Vulkan
     * loader and any physical device selected for renderer use.
     *
     * Keeping the requirement in one shared location prevents instance creation
     * and physical-device selection from using inconsistent Vulkan-version
     * requirements.
     */
    inline constexpr std::uint32_t kRequiredApiVersion = VK_API_VERSION_1_4;

    /**
     * @brief Determines whether a reported Vulkan API version satisfies
     * Signum's required Vulkan version.
     *
     * Compares a supported Vulkan API version against the Vulkan API baseline
     * required by the renderer.
     *
     * @param version Vulkan API version reported as supported.
     *
     * @return true if the supplied version meets or exceeds the required Vulkan
     *         API version; otherwise false.
     */
    [[nodiscard]] constexpr auto
    supportsRequiredApiVersion(std::uint32_t version) noexcept -> bool {
        return version >= kRequiredApiVersion;
    }
} // namespace SNE::Engine::Renderer::Vulkan
