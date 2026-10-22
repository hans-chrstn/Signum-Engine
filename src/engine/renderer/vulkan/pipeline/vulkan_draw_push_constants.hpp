#pragma once

#include "engine/core/math/matrix4.hpp"
#include <cstddef>
#include <type_traits>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Stores per-draw transformations supplied through Vulkan push
     * constants.
     *
     * Contains the model-view-projection matrix used to transform vertex
     * positions into clip space and the normal matrix used to transform
     * surface normals into world space.
     *
     * The normal matrix is derived from the inverse transpose of the
     * model transformation and supports nonuniform scaling.
     *
     * Both matrices are stored in column-major order using the engine's
     * Matrix4f representation.
     *
     * The structure contains only CPU-side transformation data and
     * does not own GPU resources.
     *
     * @note The layout must match the corresponding shader push-constant
     * structure and fit within the device's push-constant size limit.
     */
    struct VulkanDrawPushConstants {
        /** @brief Model-view-projection transformation for vertex positions. */
        Core::Math::Matrix4f mvp{};

        /** @brief Inverse-transpose model transformation for surface normals.
         */
        Core::Math::Matrix4f normal_matrix{};
    };
    static_assert(std::is_standard_layout_v<VulkanDrawPushConstants>);
    static_assert(std::is_trivially_copyable_v<VulkanDrawPushConstants>);

    static_assert(offsetof(VulkanDrawPushConstants, mvp) == 0U);

    static_assert(offsetof(VulkanDrawPushConstants, normal_matrix) ==
                  sizeof(VulkanDrawPushConstants::mvp));

    static_assert(sizeof(VulkanDrawPushConstants) ==
                  sizeof(VulkanDrawPushConstants::mvp) +
                      sizeof(VulkanDrawPushConstants::normal_matrix));
} // namespace SNE::Engine::Renderer::Vulkan
