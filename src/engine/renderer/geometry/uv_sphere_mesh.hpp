#pragma once

#include "engine/renderer/geometry/primitive_mesh.hpp"
#include <cstdint>

namespace SNE::Engine::Renderer {
    /**
     * @brief Configures the geometry generated for a UV sphere.
     *
     * Describes a sphere centered at the origin, with the positive Y axis
     * pointing toward the north pole.
     *
     * Longitude segments subdivide the circumference, while latitude
     * segments subdivide the surface between the poles.
     */
    struct UvSphereParameters {
        /**
         * @brief Distance from the sphere center to its surface.
         *
         * Must be finite and greater than zero.
         */
        float radius{};

        /**
         * @brief Number of subdivisions around the vertical axis.
         *
         * Must be at least three.
         */
        std::uint32_t longitude_segments{};

        /**
         * @brief Number of subdivisions between the poles.
         *
         * Must be at least two.
         */
        std::uint32_t latitude_segments{};
    };

    /**
     * @brief Compatibility alias for the shared primitive vertex format.
     *
     * Preserves existing UV sphere interfaces while using the common
     * vertex representation for built-in primitives.
     */
    using UvSphereVertex = PrimitiveVertex;

    /**
     * @brief Compatibility alias for the shared primitive mesh representation.
     *
     * Preserves existing UV sphere interfaces while sharing the same
     * CPU-owned vertex and index storage.
     */
    using UvSphereMesh = PrimitiveMesh;

    /**
     * @brief Generates an indexed UV sphere with positions, normals, UVs,
     * and tangent-space information.
     *
     * Creates positions, outward-facing unit normals, normalized texture
     * coordinates, and tangent vectors for a sphere centered at the origin.
     *
     * Tangents follow increasing horizontal texture coordinates and store
     * their handedness in the fourth component, allowing shaders to
     * reconstruct the bitangent for tangent-space normal mapping.
     *
     * At the poles, tangents follow the longitude convention because the
     * tangent direction is geometrically ambiguous.
     *
     * Duplicates vertices along the texture seam to preserve correct
     * horizontal texture-coordinate interpolation and consistent tangents.
     *
     * The returned mesh owns its vertex and index storage. No GPU
     * resources are created or registered.
     *
     * @param parameters Radius and subdivision counts for the sphere.
     *
     * @return Owning CPU-side sphere geometry.
     *
     * @pre parameters.radius must be finite and greater than zero.
     * @pre parameters.longitude_segments must be at least three.
     * @pre parameters.latitude_segments must be at least two.
     * @pre Generated vertex indices must be representable by std::uint16_t.
     *
     * @note Invalid parameters trigger precondition assertions.
     *
     * @throws std::bad_alloc if geometry storage allocation fails.
     */
    [[nodiscard]] auto
    generateUvSphereMesh(const UvSphereParameters &parameters) -> UvSphereMesh;
} // namespace SNE::Engine::Renderer
