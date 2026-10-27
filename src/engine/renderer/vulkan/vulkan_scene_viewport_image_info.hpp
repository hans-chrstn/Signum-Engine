#pragma once

#include <cstddef>
#include <cstdint>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Describes a borrowed offscreen scene image for external sampling.
     *
     * Provides the image view, layout, dimensions, and resource identity
     * required by an external Vulkan rendering consumer.
     *
     * The renderer retains ownership of the image and its image view.
     * Consumers must not destroy the borrowed Vulkan handles.
     *
     * Frame-slot and generation information allow consumers to distinguish
     * independently allocated frame targets and resource replacements.
     *
     * The information is a snapshot and does not extend the lifetime of
     * the referenced image.
     *
     * @note This type does not depend on Dear ImGui.
     * @note Consumers must coordinate resource lifetime and synchronization
     * with the renderer before sampling the image.
     */
    struct VulkanSceneViewportImageInfo {
        /**
         * @brief Borrowed image view of the offscreen scene color attachment.
         *
         * References an image view owned by the Vulkan renderer.
         * The consumer must not destroy or retain the handle beyond
         * its guaranteed lifetime.
         *
         * VK_NULL_HANDLE indicates that no image view is available.
         */
        VkImageView image_view{VK_NULL_HANDLE};

        /**
         * @brief Image layout expected when sampling the scene image.
         *
         * Describes the layout required by the external consumer's
         * sampled-image descriptor.
         *
         * Defaults to VK_IMAGE_LAYOUT_UNDEFINED when no valid
         * sampling layout has been provided.
         *
         * The layout does not perform an image transition.
         */
        VkImageLayout image_layout{VK_IMAGE_LAYOUT_UNDEFINED};

        /**
         * @brief Pixel dimensions of the offscreen scene image.
         *
         * Contains the width and height of the image referenced
         * by image_view.
         *
         * A valid scene image requires nonzero dimensions.
         */
        VkExtent2D extent{};

        /**
         * @brief Frame-in-flight slot associated with the image.
         *
         * Identifies which per-frame offscreen render target
         * owns the referenced image view.
         *
         * Valid indices are determined by the renderer's
         * initialized frame-resource count.
         */
        std::size_t frame_slot{};

        /**
         * @brief Resource generation identifying an offscreen allocation.
         *
         * Changes when the renderer replaces its offscreen scene
         * resources, such as when the viewport dimensions or
         * attachment formats change.
         *
         * Allows external consumers to detect resource replacement
         * and update dependent image descriptors.
         *
         * A value of zero represents an uninitialized generation.
         *
         * @note Generation changes do not independently guarantee
         * that previously borrowed resources remain alive.
         */
        std::uint64_t generation{};
    };
} // namespace SNE::Engine::Renderer::Vulkan
