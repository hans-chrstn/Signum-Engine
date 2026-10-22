#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Describes an update to an existing material buffer.
     *
     * Identifies a material descriptor binding and provides replacement
     * CPU data for its uniform or storage buffer.
     *
     * The descriptor location consists of a set index, binding index,
     * and array element. The consuming renderer validates that the
     * location references an existing buffer resource.
     *
     * The structure does not own CPU memory, GPU resources, or descriptor
     * allocations and is independent of rendering backends.
     *
     * @note The supplied CPU data must remain valid until the consuming
     * update operation completes.
     */
    struct MaterialBufferUpdateInput {
        /** @brief Descriptor-set index containing the buffer binding. */
        std::uint32_t set{};

        /** @brief Binding index within the descriptor set. */
        std::uint32_t binding{};

        /** @brief Element index within the descriptor binding's array. */
        std::uint32_t array_element{};

        /**
         * @brief Borrowed CPU data used to replace the buffer contents.
         *
         * Must be nonempty and match the size of the existing GPU buffer.
         * The renderer copies the supplied data during the update operation.
         */
        std::span<const std::byte> data;
    };
} // namespace SNE::Engine::Renderer
