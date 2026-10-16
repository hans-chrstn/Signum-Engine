#pragma once

#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/numeric/checked_conversion.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace SNE::Engine::Renderer {
    /**
     * @brief Stores resources using generation-checked handles.
     *
     * Associates resource ownership with logical slot identities.
     * The registry does not perform GPU synchronization.
     *
     * @tparam Resource Resource type owned by the registry.
     */
    template <typename Resource> class GpuResourceRegistry {
      private:
        /**
         * @brief Stores one resource and its current generation.
         *
         * An empty resource indicates an unoccupied slot.
         */
        struct Slot {
            /** @brief Generation assigned to the current slot lifetime. */
            std::uint32_t generation{1U};

            /** @brief Owned resource, or empty when the slot is unoccupied. */
            std::optional<Resource> resource;

            /**
             * @brief Index of the next reusable slot in the free list.
             *
             * Empty when this slot is the last entry in the free list
             * or is not linked into the free list.
             */
            std::optional<std::uint32_t> next_free_index;
        };

        /**
         * @brief Stores resource slots indexed by GPU resource handles.
         *
         * Slot indices remain stable even when the vector reallocates.
         * References and pointers to individual slots may be invalidated.
         */
        std::vector<Slot> m_Slots;

        /**
         * @brief Index of the first reusable slot.
         *
         * Empty when the registry has no reusable slots.
         */
        std::optional<std::uint32_t> m_FreeHead;

      public:
        /**
         * @brief Inserts a resource and returns its logical handle.
         *
         * Transfers ownership of the supplied resource into a registry slot.
         * The returned handle identifies that slot and its generation.
         *
         * @param resource Resource whose ownership is transferred.
         *
         * @return Handle identifying the inserted resource.
         *
         * @pre The registry must have an available slot index
         * representable by std::uint32_t.
         *
         * @throws std::bad_alloc if slot storage allocation fails.
         *
         * @note Resource must be nothrow move-constructible.
         * This requirement is enforced at compile time.
         */
        [[nodiscard]] auto insert(Resource &&resource) -> GpuResourceHandle {
            static_assert(
                std::is_nothrow_move_constructible_v<Resource>,
                "Registry resources must be nothrow move-constructible");

            if (m_FreeHead.has_value()) {
                const std::uint32_t free_index = *m_FreeHead;
                Slot &slot = m_Slots[free_index];
                slot.resource.emplace(std::move(resource));
                m_FreeHead = slot.next_free_index;
                slot.next_free_index.reset();

                return GpuResourceHandle{
                    .index = free_index,
                    .generation = slot.generation,
                };
            }

            const auto next_index =
                Core::Numeric::tryConvertToUint32(m_Slots.size());

            if (!next_index.has_value()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Renderer,
                    "GPU resource registry insertion exceeds the supported "
                    "index range");
            }

            m_Slots.emplace_back();
            m_Slots.back().resource.emplace(std::move(resource));

            return GpuResourceHandle{
                .index = next_index.value(),
                .generation = m_Slots.back().generation,
            };
        }

        /**
         * @brief Finds a resource using its logical handle.
         *
         * Validates the handle's slot index and generation before
         * returning access to the stored resource.
         *
         * @param handle Resource identity to resolve.
         *
         * @return Non-owning pointer to the resource, or nullptr if
         * the handle is invalid, stale, or references an empty slot.
         *
         * @note The returned pointer must not outlive the resource.
         * Inserting additional resources may invalidate the pointer.
         */
        [[nodiscard]] auto find(GpuResourceHandle handle) noexcept
            -> Resource * {
            if (std::cmp_greater_equal(handle.index, m_Slots.size())) {
                return nullptr;
            }

            Slot &slot = m_Slots[handle.index];

            if (handle.generation == 0U ||
                handle.generation != slot.generation) {
                return nullptr;
            }

            if (!slot.resource.has_value()) {
                return nullptr;
            }

            return std::addressof(*slot.resource);
        }

        /**
         * @brief Removes a resource and transfers ownership to the caller.
         *
         * Resolves the supplied handle and removes its associated resource.
         * Successful removal invalidates the handle.
         *
         * This operation does not synchronize with GPU execution.
         *
         * @param handle Resource identity to remove.
         *
         * @return Owned resource on success, or std::nullopt if the handle
         * is invalid, stale, or references an empty slot.
         *
         * @note The caller must keep the returned resource alive until
         * all GPU work referencing it has completed.
         */
        [[nodiscard]] auto remove(GpuResourceHandle handle)
            -> std::optional<Resource> {
            Resource *resource = find(handle);

            if (resource == nullptr) {
                return std::nullopt;
            }

            std::optional<Resource> removed = std::move(*resource);

            Slot &slot = m_Slots[handle.index];

            slot.resource.reset();

            if (slot.generation != std::numeric_limits<std::uint32_t>::max()) {
                ++slot.generation;
            }

            if (slot.generation < std::numeric_limits<std::uint32_t>::max()) {
                slot.next_free_index = m_FreeHead;
                m_FreeHead = handle.index;
            }

            return removed;
        }
    };
} // namespace SNE::Engine::Renderer
