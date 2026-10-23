#include "gpu_resource_dependency_tracker.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/resource/gpu_resource_dependency.hpp"
#include <algorithm>
#include <cstddef>
#include <vector>

namespace SNE::Engine::Renderer {
    auto GpuResourceDependencyTracker::wouldCreateCycle(
        GpuResourceDependency proposed_relationship) const -> bool {
        const auto &start = proposed_relationship.dependency;
        const auto &target = proposed_relationship.dependent;
        std::vector<GpuResourceHandle> pending;
        std::vector<GpuResourceHandle> visited;

        pending.push_back(start);

        while (!pending.empty()) {
            const GpuResourceHandle current = pending.back();

            pending.pop_back();

            const auto visited_it = std::ranges::find(visited, current);

            if (visited_it != visited.end()) {
                continue;
            }

            if (current == target) {
                return true;
            }

            visited.push_back(current);

            for (const GpuResourceDependency &relationship : m_Dependencies) {
                if (relationship.dependent == current) {
                    pending.push_back(relationship.dependency);
                }
            }
        }

        return false;
    }

    auto GpuResourceDependencyTracker::addDependency(
        GpuResourceDependency relationship) -> void {
        const auto &dependent = relationship.dependent;
        const auto &dependency = relationship.dependency;

        if (dependent.registry_id == 0U || dependent.generation == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "GPU resource dependency registration requires a valid "
                "dependent handle");
        }

        if (dependency.registry_id == 0U || dependency.generation == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "GPU resource dependency registration requires a valid "
                "dependency handle");
        }

        if (dependent == dependency) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "GPU resource dependency registration cannot reference the "
                "same resource");
        }

        if (contains(relationship)) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "GPU resource dependency registration requires a unique "
                "relationship");
        }

        if (wouldCreateCycle(relationship)) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Renderer,
                "GPU resource dependency registration cannot create a "
                "dependency cycle");
        }

        m_Dependencies.push_back(relationship);
    }

    auto GpuResourceDependencyTracker::removeDependency(
        GpuResourceDependency relationship) noexcept -> bool {
        auto relationship_it = std::ranges::find(m_Dependencies, relationship);

        if (relationship_it == m_Dependencies.end()) {
            return false;
        }

        static_cast<void>(m_Dependencies.erase(relationship_it));

        return true;
    }

    auto GpuResourceDependencyTracker::removeRelationshipsInvolving(
        GpuResourceHandle resource_handle) noexcept -> std::size_t {
        const std::size_t removed_count = std::erase_if(
            m_Dependencies,
            [resource_handle](
                const GpuResourceDependency &relationship) -> bool {
                return relationship.dependent == resource_handle ||
                       relationship.dependency == resource_handle;
            });

        return removed_count;
    }

    auto GpuResourceDependencyTracker::relationships() const noexcept
        -> std::span<const GpuResourceDependency> {
        return m_Dependencies;
    }

    auto GpuResourceDependencyTracker::contains(
        GpuResourceDependency relationship) const noexcept -> bool {
        const auto dependency_it =
            std::ranges::find(m_Dependencies, relationship);

        return dependency_it != m_Dependencies.end();
    }

    auto GpuResourceDependencyTracker::hasDependents(
        GpuResourceHandle resource_handle) const noexcept -> bool {
        const bool has_dependents = std::ranges::any_of(
            m_Dependencies,
            [resource_handle](const GpuResourceDependency &relationship)
                -> bool { return relationship.dependency == resource_handle; });

        return has_dependents;
    }

    auto GpuResourceDependencyTracker::hasDependentsOtherThan(
        GpuResourceDependency excluded_relationship) const noexcept -> bool {
        const bool has_other_dependents = std::ranges::any_of(
            m_Dependencies,
            [excluded_relationship](
                const GpuResourceDependency &relationship) -> bool {
                return relationship.dependency ==
                           excluded_relationship.dependency &&
                       relationship.dependent !=
                           excluded_relationship.dependent;
            });

        return has_other_dependents;
    }

    auto GpuResourceDependencyTracker::hasDependentsOutside(
        GpuResourceHandle resource_handle,
        std::span<const GpuResourceHandle> excluded_dependents) const noexcept
        -> bool {
        return std::ranges::any_of(
            m_Dependencies,
            [resource_handle, excluded_dependents](
                const GpuResourceDependency &relationship) -> bool {
                if (relationship.dependency != resource_handle) {
                    return false;
                }

                const auto dependent_it = std::ranges::find(
                    excluded_dependents, relationship.dependent);

                return dependent_it == excluded_dependents.end();
            });
    }
} // namespace SNE::Engine::Renderer
