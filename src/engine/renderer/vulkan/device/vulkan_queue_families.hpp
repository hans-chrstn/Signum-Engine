#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Stores queue-family indices discovered for rendering and
     * presentation.
     *
     * Contains queue-family indices discovered for graphics operations and
     * presentation to a Vulkan surface.
     *
     * A missing value indicates that no suitable queue family was found for
     * that capability.
     */
    struct QueueFamilyIndices {
        /** Graphics-capable queue-family index when available. */
        std::optional<std::uint32_t> graphics_family;
        /** Presentation-capable queue-family index when available. */
        std::optional<std::uint32_t> presentation_family;
    };

    /**
     * @brief Represents a queue family selected for renderer use.
     *
     * Stores the Vulkan queue-family index together with the number of queues
     * exposed by that family.
     *
     * This information is produced after physical-device selection and provides
     * the queue-family identity and hardware capacity required by
     * logical-device queue planning.
     */
    struct SelectedQueueFamily {
        /** Vulkan queue-family index. */
        std::uint32_t family_index{};
        /** Number of queues exposed by this Vulkan queue family. */
        std::uint32_t available_queue_count = 0;
    };

    /**
     * @brief Stores selected queue-family information guaranteed for renderer
     * use.
     *
     * Represents the graphics and presentation queue families chosen during
     * physical-device selection.
     *
     * Unlike QueueFamilyIndices, these values are not optional because device
     * selection has already established that both required queue families
     * exist.
     *
     * Each selected queue family contains both its Vulkan index and the queue
     * capacity reported by the physical device. This information is used when
     * creating the logical-device queue plan.
     */
    struct SelectedQueueFamilies {
        /** Selected queue family used for graphics operations. */
        SelectedQueueFamily graphics_family;
        /** Selected queue family used for presentation operations. */
        SelectedQueueFamily presentation_family;
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
     * @brief Finds the first queue family supporting graphics operations.
     *
     * Examines the supplied queue-family properties in Vulkan queue-family
     * order and returns the index of the first family advertising graphics
     * capability.
     *
     * @param properties Contiguous sequence of queue-family properties reported
     * by the physical device.
     *
     * @pre Any graphics queue-family index returned from properties must be
     * representable by std::uint32_t.
     *
     * @return Index of the first graphics-capable queue family, or std::nullopt
     * if no graphics-capable family is available.
     */
    [[nodiscard]] auto
    findGraphicsQueueFamily(std::span<const VkQueueFamilyProperties> properties)
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
     * @brief Finds the first queue family supporting presentation to a surface.
     *
     * Queries presentation support for each supplied queue-family index and
     * returns the first family capable of presenting to the supplied surface.
     *
     * @param device Physical device whose queue families are evaluated.
     * @param surface Surface against which presentation support is queried.
     * @param properties Contiguous sequence of queue-family properties reported
     * by the physical device.
     *
     * @return Index of the first presentation-capable queue family, or
     * std::nullopt if no suitable family is available.
     *
     * @pre Every queue-family index used to query presentation support must be
     * representable by std::uint32_t.
     *
     * @throws Core::Error::EngineError if Vulkan fails while querying surface
     * presentation support.
     */
    [[nodiscard]] auto findPresentationQueueFamily(
        VkPhysicalDevice device, VkSurfaceKHR surface,
        std::span<const VkQueueFamilyProperties> properties)
        -> std::optional<std::uint32_t>;

    /**
     * @brief Finds the graphics and presentation queue families required by the
     * renderer.
     *
     * Searches the supplied queue-family properties for graphics support and
     * presentation support for the supplied Vulkan surface.
     *
     * Graphics and presentation roles may resolve to the same queue family.
     *
     * @param device Physical device whose queue families are evaluated.
     * @param surface Surface against which presentation support is queried.
     * @param properties Contiguous sequence of queue-family properties reported
     * by the physical device.
     *
     * @pre Every queue-family index examined from properties must be
     * representable by std::uint32_t.
     *
     * @return Discovered graphics and presentation queue-family indices. Either
     * index may be empty when the corresponding capability is unavailable.
     *
     * @throws Core::Error::EngineError if Vulkan fails while querying surface
     * presentation support.
     */
    [[nodiscard]] auto
    findQueueFamilyIndices(VkPhysicalDevice device, VkSurfaceKHR surface,
                           std::span<const VkQueueFamilyProperties> properties)
        -> QueueFamilyIndices;
} // namespace SNE::Engine::Renderer::Vulkan
