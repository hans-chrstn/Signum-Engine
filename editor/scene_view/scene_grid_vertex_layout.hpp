#pragma once

#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "scene_view/scene_grid_mesh.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace SNE::Editor {
    static_assert(std::is_standard_layout_v<SceneGridVertex>);

    /**
     * @brief Defines the position-only vertex layout for editor ground grid
     * geometry.
     *
     * Maps SceneGridVertex's three-dimensional position to vertex shader
     * input location zero using the Float32x3 attribute format.
     *
     * The attribute offset is derived from SceneGridVertex at compile time
     * to preserve consistency with its memory layout.
     *
     * The metadata is backend-independent and may be borrowed by
     * VertexLayoutDescription when configuring grid rendering.
     *
     * @note Grid vertices contain only position data.
     * @note The selected vertex shader must accept a three-component
     * floating-point position at location zero.
     * @note Grid line topology is configured through the graphics
     * pipeline state, not through this vertex layout.
     */
    inline constexpr std::array<Engine::Renderer::VertexAttributeDescription,
                                1U>
        kSceneGridVertexAttributes{
            {
                {
                    .location = 0U,
                    .format = Engine::Renderer::VertexFormat::Float32x3,
                    .offset = static_cast<std::uint32_t>(
                        offsetof(SceneGridVertex, position)),
                },
            },
    };
} // namespace SNE::Editor
