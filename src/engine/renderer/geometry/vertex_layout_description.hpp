
#pragma once

#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes a backend-independent vertex attribute layout.
     *
     * Defines the collection of attributes used to interpret vertex
     * records without exposing graphics API-specific descriptions.
     *
     * Each attribute specifies its shader input location, component
     * format, and byte offset within a vertex record.
     *
     * The description is non-owning. The caller retains ownership
     * of the underlying attribute collection and must keep it valid
     * throughout any operation that consumes the layout.
     *
     * Vertex stride is supplied separately by the associated geometry
     * description to avoid maintaining conflicting stride values.
     *
     * @note The initial layout model supports a single vertex buffer
     * binding with per-vertex input rates.
     *
     * @note Attribute bounds, duplicate locations, and compatibility
     * with the selected vertex shader are validated by the consuming
     * rendering infrastructure.
     */
    struct VertexLayoutDescription {
        /**
         * @brief Borrowed descriptions of vertex input attributes.
         *
         * Each description identifies a shader input location,
         * its data format, and its byte offset within a vertex.
         *
         * An empty span describes a layout without vertex attributes.
         *
         * The underlying attribute storage must remain valid for
         * the duration of the consuming operation.
         */
        std::span<const VertexAttributeDescription> attributes;
    };
} // namespace SNE::Engine::Renderer
