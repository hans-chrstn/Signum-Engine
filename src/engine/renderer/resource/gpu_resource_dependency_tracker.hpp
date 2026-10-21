#pragma once

#include "engine/renderer/resource/gpu_resource_dependency.hpp"
#include <cstddef>
#include <span>
#include <vector>

namespace SNE::Engine::Renderer {
    /**
     * @brief Tracks dependencies between GPU resource identities.
     *
     * Stores directed relationships between registry-scoped GPU
     * resource handles.
     *
     * The tracker does not own GPU resources, validate resource
     * existence, or synchronize GPU execution.
     *
     * Registry operations and dependency tracking require external
     * synchronization when accessed from multiple threads.
     */
    class GpuResourceDependencyTracker {
      private:
        /**
         * @brief Registered GPU resource dependency relationships.
         *
         * Each entry identifies a dependent resource and the resource
         * it requires. Relationships are stored without duplicates.
         */
        std::vector<GpuResourceDependency> m_Dependencies;

        /**
         * @brief Checks whether registering a relationship would create a
         * cycle.
         *
         * Searches existing dependency relationships for a path from the
         * proposed dependency back to the proposed dependent.
         *
         * @param proposed_relationship Relationship being considered.
         *
         * @return True if registering the relationship would create a
         * dependency cycle, otherwise false.
         *
         * @throws std::bad_alloc if traversal storage allocation fails.
         *
         * @note Does not modify stored relationships or verify whether
         * referenced resources are alive.
         */
        [[nodiscard]] auto
        wouldCreateCycle(GpuResourceDependency proposed_relationship) const
            -> bool;

      public:
        /**
         * @brief Registers a dependency between two GPU resources.
         *
         * Validates the supplied handles and stores the directed
         * relationship.
         *
         * Handles must have nonzero registry identities and generations.
         * The dependent and dependency must identify different resources.
         * Duplicate relationships are not permitted.
         *
         * @param relationship Dependency relationship to register.
         *
         * @pre Both handles must have valid structural identities.
         * @pre The dependent must not reference itself.
         * @pre The relationship must not already be registered.
         * @pre The relationship must not introduce a dependency cycle.
         *
         * @post On success, the relationship is stored.
         * @post On failure, existing relationships remain unchanged.
         *
         * @throws std::bad_alloc if relationship storage allocation fails.
         *
         * @note Invalid relationships trigger precondition assertions.
         * Resource existence is not verified.
         */
        auto addDependency(GpuResourceDependency relationship) -> void;

        /**
         * @brief Removes a registered GPU resource dependency relationship.
         *
         * Searches for an exact relationship and removes it if present.
         * Other relationships remain unchanged.
         *
         * @param relationship Dependency relationship to remove.
         *
         * @return True if the relationship was removed, otherwise false.
         *
         * @post The specified relationship is no longer registered.
         *
         * @note This operation does not destroy GPU resources or synchronize
         * GPU execution.
         */
        [[nodiscard]] auto
        removeDependency(GpuResourceDependency relationship) noexcept -> bool;

        /**
         * @brief Removes all relationships involving a GPU resource.
         *
         * Removes every relationship in which the supplied resource is
         * either the dependent or the dependency.
         *
         * Relationships involving other resources remain unchanged.
         *
         * @param resource_handle Handle of the resource to remove from
         * dependency tracking.
         *
         * @return Number of relationships removed.
         *
         * @post No stored relationship references the supplied handle.
         *
         * @note This operation does not destroy resources, validate their
         * existence, or synchronize GPU execution.
         */
        [[nodiscard]] auto
        removeRelationshipsInvolving(GpuResourceHandle resource_handle) noexcept
            -> std::size_t;

        /**
         * @brief Returns the registered GPU resource relationships.
         *
         * Provides a read-only view of the dependency relationships currently
         * stored by the tracker.
         *
         * Each relationship identifies a dependent resource and the resource
         * it requires.
         *
         * @return Borrowed view of the registered dependency relationships.
         *
         * @note The returned view does not own the relationships.
         * Adding or removing relationships may invalidate the view.
         *
         * @note The order of relationships is not a scheduling or destruction
         * order.
         *
         * @note The caller must synchronize access with relationship
         * modifications.
         *
         * @note This operation does not validate resource existence or GPU
         * completion.
         */
        [[nodiscard]] auto relationships() const noexcept
            -> std::span<const GpuResourceDependency>;

        /**
         * @brief Checks whether a dependency relationship is registered.
         *
         * Searches for an exact match between the supplied dependent
         * and dependency handles.
         *
         * @param relationship Dependency relationship to find.
         *
         * @return True if the relationship exists, otherwise false.
         *
         * @note This operation does not modify stored relationships
         * or validate whether referenced resources still exist.
         */
        [[nodiscard]] auto
        contains(GpuResourceDependency relationship) const noexcept -> bool;

        /**
         * @brief Checks whether any registered relationship depends on a
         * resource.
         *
         * Searches for relationships in which the supplied resource is the
         * dependency, rather than the dependent.
         *
         * @param resource_handle Resource to check for incoming dependencies.
         *
         * @return True if at least one registered relationship depends on
         * the supplied resource, otherwise false.
         *
         * @note This operation does not verify whether the referenced
         * resources are still registered or alive.
         */
        [[nodiscard]] auto
        hasDependents(GpuResourceHandle resource_handle) const noexcept -> bool;

        /**
         * @brief Checks whether a resource has dependents other than one
         * excluded dependent.
         *
         * Searches for relationships referencing the supplied dependency,
         * excluding those whose dependent matches the supplied dependent.
         *
         * @param excluded_relationship Relationship identifying the resource
         * to examine and the dependent to exclude.
         *
         * @return true if another dependent references the resource;
         * otherwise false.
         *
         * @note The excluded relationship does not need to be registered.
         * @note This query does not validate resource lifetime.
         */
        [[nodiscard]] auto hasDependentsOtherThan(
            GpuResourceDependency excluded_relationship) const noexcept -> bool;
    };
} // namespace SNE::Engine::Renderer
