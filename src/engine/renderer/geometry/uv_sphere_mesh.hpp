#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

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
     * @brief Stores the attributes of one generated UV sphere vertex.
     *
     * Contains the vertex position, outward-facing unit normal, and
     * normalized texture coordinates.
     *
     * The position is relative to the sphere center. Vertex attributes
     * are stored independently of rendering backend types.
     */
    struct UvSphereVertex {
        /** @brief Number of floating-point components in a 3D vector. */
        static constexpr std::size_t kSpatialComponentCount{3U};

        /** @brief Number of floating-point components in a UV coordinate. */
        static constexpr std::size_t kUvComponentCount{2U};

        /** @brief Position relative to the sphere center. */
        std::array<float, kSpatialComponentCount> position{};

        /** @brief Outward-facing unit surface normal. */
        std::array<float, kSpatialComponentCount> normal{};

        /**
         * @brief Normalized texture coordinates.
         *
         * The horizontal coordinate increases around the sphere, while
         * the vertical coordinate increases from the north pole toward
         * the south pole.
         */
        std::array<float, kUvComponentCount> uv{};
    };

    /**
     * @brief Owns CPU-side vertex and index data for a UV sphere.
     *
     * Stores independently owned geometry suitable for subsequent GPU
     * upload through the renderer's existing mesh upload infrastructure.
     *
     * The geometry does not own GPU resources or depend on Vulkan.
     */
    struct UvSphereMesh {
        /** @brief Generated sphere vertices. */
        std::vector<UvSphereVertex> vertices;

        /**
         * @brief Triangle indices referencing generated vertices.
         *
         * Indices use 16-bit storage to match the current mesh upload
         * contract.
         */
        std::vector<std::uint16_t> indices;
    };

    /**
     * @brief Generates an indexed UV sphere on the CPU.
     *
     * Creates positions, outward-facing unit normals, and normalized UV
     * coordinates for a sphere centered at the origin.
     *
     * Duplicates vertices along the texture seam to preserve correct
     * horizontal texture-coordinate interpolation.
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
