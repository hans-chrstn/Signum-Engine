#pragma once

#include "engine/renderer/geometry/primitive_mesh.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace SNE::Engine::Renderer {
    static_assert(std::is_standard_layout_v<PrimitiveVertex>);
    /**
     * @brief Defines the vertex attribute layout shared by built-in primitives.
     *
     * Maps PrimitiveVertex position, normal, texture coordinates, and tangent
     * attributes to their corresponding shader input locations and formats.
     *
     * Attribute offsets are derived from PrimitiveVertex at compile time,
     * ensuring consistency with its memory layout.
     *
     * The metadata is backend-independent and may be borrowed by
     * VertexLayoutDescription during mesh initialization.
     *
     * @note Shader inputs must be compatible with this attribute convention.
     * Specialized meshes may use different vertex layouts.
     */
    inline constexpr std::array<VertexAttributeDescription, 4>
        kPrimitiveVertexAttributes{
            VertexAttributeDescription{
                .location = 0U,
                .format = VertexFormat::Float32x3,
                .offset = static_cast<std::uint32_t>(
                    offsetof(PrimitiveVertex, position)),
            },
            VertexAttributeDescription{
                .location = 1U,
                .format = VertexFormat::Float32x3,
                .offset = static_cast<std::uint32_t>(
                    offsetof(PrimitiveVertex, normal)),
            },
            VertexAttributeDescription{
                .location = 2U,
                .format = VertexFormat::Float32x2,
                .offset =
                    static_cast<std::uint32_t>(offsetof(PrimitiveVertex, uv)),
            },
            VertexAttributeDescription{
                .location = 3U,
                .format = VertexFormat::Float32x4,
                .offset = static_cast<std::uint32_t>(
                    offsetof(PrimitiveVertex, tangent)),
            },
    };

} // namespace SNE::Engine::Renderer
