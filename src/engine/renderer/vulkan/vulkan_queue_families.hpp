#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Stores queue-family indices selected for rendering and
     * presentation.
     *
     * Contains the queue-family indices discovered for graphics operations and
     * presentation to a Vulkan surface.
     *
     * A missing value indicates that no suitable queue family was found for
     * that capability.
     */
    struct QueueFamilyIndices {
        std::optional<std::uint32_t> graphics_family;
        std::optional<std::uint32_t> presentation_family;
    };

    /**
     * @brief Queries the queue-family properties exposed by a physical device.
     *
     * Retrieves the queue families reported by Vulkan and returns their core
     * properties, including supported queue capabilities and queue counts.
     *
     * An empty result indicates that no queue families were reported.
     *
     * @param device Physical device whose queue families are queried.
     *
     * @return Queue-family properties reported for the physical device.
     */
    [[nodiscard]] auto queryQueueFamilyProperties(VkPhysicalDevice device)
        -> std::vector<VkQueueFamilyProperties>;

    /**
     * @brief Finds a queue family that supports graphics operations.
     *
     * Searches the supplied queue-family properties for the first family whose
     * queue flags contain VK_QUEUE_GRAPHICS_BIT.
     *
     * @param properties Queue-family properties to search.
     *
     * @return The queue-family index when a graphics-capable family is found,
     *         or std::nullopt when none is available.
     */
    [[nodiscard]] auto findGraphicsQueueFamily(
        const std::vector<VkQueueFamilyProperties> &properties)
        -> std::optional<std::uint32_t>;

    /**
     * @brief Determines whether a queue family can present to a Vulkan surface.
     *
     * Queries Vulkan for presentation support between the specified physical
     * device queue family and surface.
     *
     * @param device Physical device that owns the queue family.
     * @param queue_family_index Index of the queue family to query.
     * @param surface Vulkan surface against which presentation support is
     * tested.
     *
     * @return true when the queue family supports presentation to the surface;
     *         otherwise false.
     *
     * @throws Core::Error::EngineError if Vulkan fails to query surface
     * support.
     */
    [[nodiscard]] auto supportsPresentation(VkPhysicalDevice device,
                                            std::uint32_t queue_family_index,
                                            VkSurfaceKHR surface) -> bool;

    /**
     * @brief Finds a queue family that supports presentation to a Vulkan
     * surface.
     *
     * Searches the supplied queue families and returns the first family whose
     * queue can present to the specified surface.
     *
     * @param device Physical device that owns the queue families.
     * @param surface Vulkan surface against which presentation support is
     * tested.
     * @param properties Queue-family properties used to determine valid family
     *                   indices.
     *
     * @return The queue-family index when a presentation-capable family is
     * found, or std::nullopt when none is available.
     *
     * @throws Core::Error::EngineError if Vulkan fails to query presentation
     *         support for a queue family.
     */
    [[nodiscard]] auto findPresentationQueueFamily(
        VkPhysicalDevice device, VkSurfaceKHR surface,
        const std::vector<VkQueueFamilyProperties> &properties)
        -> std::optional<std::uint32_t>;

    /**
     * @brief Finds the graphics and presentation queue-family indices for a
     * device.
     *
     * Searches the supplied queue-family properties for a graphics-capable
     * family and queries presentation support against the specified Vulkan
     * surface.
     *
     * Missing queue-family capabilities are represented by empty std::optional
     * values in the returned QueueFamilyIndices structure.
     *
     * @param device Physical device that owns the queue families.
     * @param surface Vulkan surface used when checking presentation support.
     * @param properties Queue-family properties to search.
     *
     * @return Graphics and presentation queue-family indices discovered for the
     *         physical device and surface.
     *
     * @throws Core::Error::EngineError if Vulkan fails to query presentation
     *         support for a queue family.
     */
    [[nodiscard]] auto findQueueFamilyIndices(
        VkPhysicalDevice device, VkSurfaceKHR surface,
        const std::vector<VkQueueFamilyProperties> &properties)
        -> QueueFamilyIndices;
} // namespace SNE::Engine::Renderer::Vulkan
