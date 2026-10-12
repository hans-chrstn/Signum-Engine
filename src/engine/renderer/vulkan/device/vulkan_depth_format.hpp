#pragma once

#include <optional>
#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes a candidate depth format and its supported features.
     *
     * Associates a Vulkan image format with the optimal-tiling features
     * reported by a physical device.
     *
     * Used by depth-format selection policy without requiring Vulkan
     * API calls during selection.
     */
    struct DepthFormatCandidate {
        /**
         * @brief Vulkan image format being evaluated.
         */
        VkFormat format{};

        /**
         * @brief Format features supported with optimal image tiling.
         */
        VkFormatFeatureFlags optimal_tiling_features{};
    };

    /**
     * @brief Selects the first depth-attachment-compatible format.
     *
     * Evaluates candidate formats in the supplied order of preference.
     *
     * A candidate is supported when its optimal-tiling features contain
     * VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT.
     *
     * Selection does not query Vulkan or allocate GPU resources.
     *
     * @param candidates Ordered depth-format candidates and their
     * optimal-tiling feature capabilities.
     *
     * @return The first supported candidate format, or std::nullopt
     * when no candidate supports depth attachments.
     */
    [[nodiscard]] auto
    selectPreferredDepthFormat(std::span<const DepthFormatCandidate> candidates)
        -> std::optional<VkFormat>;

    /**
     * @brief Selects a supported depth-attachment format for a physical device.
     *
     * Evaluates preferred depth-only formats in priority order and queries
     * the physical device for optimal-tiling format support.
     *
     * A format is considered supported when it provides the
     * VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT capability.
     *
     * @param physical_device Physical device whose depth-format capabilities
     * are queried.
     *
     * @pre physical_device must be a valid Vulkan physical-device handle.
     *
     * @return The highest-priority supported depth format, or std::nullopt
     * if none of the preferred formats is supported.
     */
    [[nodiscard]] auto
    selectSupportedDepthFormat(VkPhysicalDevice physical_device)
        -> std::optional<VkFormat>;
} // namespace SNE::Engine::Renderer::Vulkan
