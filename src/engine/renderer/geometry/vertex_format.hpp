#pragma once

#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Identifies a backend-independent vertex attribute format.
     *
     * Describes the component type and component count of an individual
     * vertex attribute without exposing graphics API-specific formats.
     *
     * Rendering backends translate these values into their corresponding
     * native vertex attribute formats.
     *
     * @note This enumeration initially supports the floating-point formats
     * required by Signum's reference geometry. Additional formats may be
     * introduced as rendering requirements expand.
     */
    enum class VertexFormat : std::uint8_t {
        /**
         * @brief Two 32-bit floating-point components.
         *
         * Occupies 8 bytes per attribute.
         */
        Float32x2,

        /**
         * @brief Three 32-bit floating-point components.
         *
         * Occupies 12 bytes per attribute.
         */
        Float32x3,

        /**
         * @brief Four 32-bit floating-point components.
         *
         * Occupies 16 bytes per attribute.
         */
        Float32x4,
    };

    /**
     * @brief Returns the storage size of a vertex attribute format.
     *
     * Determines the number of bytes occupied by one attribute using
     * the specified backend-independent vertex format.
     *
     * The result depends only on the format and does not require a
     * graphics device or allocate GPU resources.
     *
     * @param vertex_format Vertex attribute format to inspect.
     *
     * @return Size of the vertex attribute in bytes.
     *
     * @pre vertex_format must identify a supported VertexFormat value.
     *
     * @note Unsupported format values trigger a fatal precondition
     * assertion in the Renderer subsystem.
     */
    [[nodiscard]] auto vertexFormatSizeInBytes(VertexFormat vertex_format)
        -> std::uint32_t;
} // namespace SNE::Engine::Renderer
