#pragma once

#include <optional>
#include <utility>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Replaces a resource group that cannot safely be
     * reused if replacement construction fails.
     *
     * Builds a replacement before installing it into the owning
     * resource slot.
     *
     * If replacement construction or installation throws,
     * clears the owning slot and propagates the original exception.
     *
     * @tparam Resource Owned resource-group type.
     * @tparam Builder Callable that constructs a Resource.
     *
     * @param current Owning slot containing the existing resources.
     * @param build_replacement Callable producing replacement resources.
     *
     * @post On success, current owns the replacement resource group.
     * @post On failure, current is empty and the exception propagates.
     *
     * @note The builder must not invalidate the current resource
     * group while potentially throwing construction work remains.
     */
    template <typename Resource, typename Builder>
    auto replaceRetiringResource(std::optional<Resource> &current,
                                 Builder &&build_replacement) -> void {
        try {
            auto new_resource = build_replacement();
            current.emplace(std::move(new_resource));

        } catch (...) {
            current.reset();
            throw;
        }
    }
} // namespace SNE::Engine::Renderer::Vulkan
