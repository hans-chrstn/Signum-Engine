#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes non-owning CPU geometry data for GPU upload.
     *
     * Provides contiguous vertex bytes, the stride of each vertex,
     * and 16-bit indices defining indexed geometry.
     *
     * This description is independent of graphics APIs and does not
     * allocate, own, or upload GPU resources.
     *
     * The caller retains ownership of the referenced CPU memory.
     * Both spans must remain valid until the upload operation has
     * finished consuming or copying their contents.
     *
     * The receiving upload operation is responsible for validating
     * the description before accessing or uploading its data.
     *
     * @note Vertex attribute formats, locations, and bindings are
     * not described by this structure.
     *
     * @note The initial contract supports 16-bit indices. Other
     * index formats may be introduced as rendering requirements grow.
     */
    struct MeshUploadData {
        /**
         * @brief Borrowed contiguous vertex data represented as bytes.
         *
         * The byte sequence contains vertex records whose size is
         * determined by vertex_stride.
         *
         * For nonempty vertex data, its byte count must be divisible
         * by vertex_stride.
         *
         * The underlying memory remains owned by the caller.
         */
        std::span<const std::byte> vertex_bytes;

        /**
         * @brief Number of bytes occupied by a single vertex record.
         *
         * Must be greater than zero when vertex_bytes is nonempty.
         *
         * The stride describes vertex record spacing, not the formats
         * or locations of individual vertex attributes.
         */
        std::uint32_t vertex_stride{};

        /**
         * @brief Borrowed 16-bit indices defining indexed geometry.
         *
         * Each index identifies a vertex within the supplied vertex
         * sequence. Every index must be less than the number of
         * vertices represented by vertex_bytes and vertex_stride.
         *
         * The underlying index storage remains owned by the caller.
         *
         * Primitive topology is determined separately by the
         * rendering pipeline.
         */
        std::span<const std::uint16_t> indices;
    };
} // namespace SNE::Engine::Renderer
