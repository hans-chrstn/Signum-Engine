
#pragma once

#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include <cstdint>
#include <vector>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns Vulkan vertex input descriptions translated from an
     * engine-defined vertex layout.
     *
     * Contains a single vertex buffer binding description and a
     * collection of Vulkan vertex attribute descriptions.
     *
     * The descriptions are owned by this object and may be borrowed
     * during graphics-pipeline creation.
     *
     * This structure does not own vertex buffers, vertex data,
     * graphics pipelines, or other Vulkan resources.
     *
     * @note The initial representation supports one vertex buffer
     * binding with per-vertex input rates.
     */
    struct VulkanVertexInputDescriptions {
        /**
         * @brief Description of the single vertex buffer binding.
         *
         * Specifies the binding index, vertex stride, and input rate.
         *
         * The binding is value-initialized until populated by the
         * vertex input translation operation.
         */
        VkVertexInputBindingDescription binding{};

        /**
         * @brief Owned Vulkan vertex attribute descriptions.
         *
         * Each element specifies the shader input location, binding
         * index, Vulkan format, and byte offset of one attribute.
         *
         * The storage must remain valid while a graphics-pipeline
         * creation operation borrows these descriptions.
         */
        std::vector<VkVertexInputAttributeDescription> attributes;
    };

    /**
     * @brief Translates an engine vertex layout into Vulkan descriptions.
     *
     * Converts backend-independent vertex attributes into Vulkan
     * vertex input attribute descriptions.
     *
     * The initial translation uses vertex buffer binding zero and
     * per-vertex input rates.
     *
     * The returned structure owns its translated descriptions and
     * does not retain references to the source layout.
     *
     * The conversion does not require a Vulkan device or create GPU
     * resources.
     *
     * @param layout Borrowed engine vertex attribute descriptions
     * to translate.
     * @param vertex_stride Number of bytes occupied by each vertex.
     *
     * @return Owned Vulkan binding and attribute descriptions.
     *
     * @pre vertex_stride must be nonzero when attributes are present.
     * @pre Every attribute must have a supported vertex format.
     * @pre Every attribute's byte range must fit within vertex_stride.
     * @pre Attribute shader input locations must be unique.
     *
     * @note Invalid layout descriptions violate preconditions and
     * trigger fatal assertions.
     */
    [[nodiscard]] auto
    makeVulkanVertexInputDescriptions(const VertexLayoutDescription &layout,
                                      std::uint32_t vertex_stride)
        -> VulkanVertexInputDescriptions;
} // namespace SNE::Engine::Renderer::Vulkan
