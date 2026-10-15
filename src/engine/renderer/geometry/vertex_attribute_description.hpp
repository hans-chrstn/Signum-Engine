#pragma once

#include "engine/renderer/geometry/vertex_format.hpp"
#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes a single backend-independent vertex attribute.
     *
     * Defines the shader input location, attribute format, and byte
     * offset within an individual vertex record.
     *
     * Rendering backends translate this description into the
     * corresponding native vertex input attribute configuration.
     *
     * This description does not own vertex data or GPU resources.
     *
     * @note The initial vertex input model uses a single vertex
     * buffer binding. Additional bindings and input rates may be
     * introduced as rendering requirements expand.
     */
    struct VertexAttributeDescription {
        /**
         * @brief Shader input location associated with the attribute.
         *
         * Must correspond to the location expected by the selected
         * vertex shader entry point.
         */
        std::uint32_t location{};

        /**
         * @brief Backend-independent format of the vertex attribute.
         *
         * Specifies the component type and component count used
         * to interpret the attribute's bytes.
         */
        VertexFormat format{};

        /**
         * @brief Byte offset of the attribute within each vertex record.
         *
         * The attribute's byte range must fit within the vertex stride
         * supplied by the associated vertex layout.
         */
        std::uint32_t offset{};

        /**
         * @brief Compares two vertex attribute descriptions.
         *
         * @param other Vertex attribute description to compare.
         *
         * @return True when location, format, and offset match.
         */
        [[nodiscard]] constexpr auto
        operator==(const VertexAttributeDescription &other) const noexcept
            -> bool = default;
    };
} // namespace SNE::Engine::Renderer
