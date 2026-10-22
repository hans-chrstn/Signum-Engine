#pragma once

#include "engine/core/math/vector4.hpp"
#include "engine/renderer/lighting/directional_light.hpp"
#include <cstddef>
#include <type_traits>

namespace SNE::Engine::Renderer {

    /**
     * @brief Required alignment of a four-component shader float vector.
     *
     * Derives the alignment from the storage size of Vector4f.
     *
     * Used when defining CPU structures that must match the
     * shader's uniform-buffer memory layout.
     */
    inline constexpr std::size_t kShaderFloat4Alignment =
        sizeof(Core::Math::Vector4f);

    /**
     * @brief Stores directional light properties in a GPU-compatible layout.
     *
     * Packs the light direction, color, and intensity into two
     * four-component vectors for use in a shader uniform buffer.
     *
     * The structure is independent of the rendering backend and
     * does not own GPU resources.
     *
     * @note The direction's W component is unused and must be zero.
     * The color_intensity W component stores the light intensity.
     *
     * @note The structure's size, alignment, and member offsets
     * must match the corresponding shader uniform-buffer layout.
     */
    struct alignas(kShaderFloat4Alignment) DirectionalLightUniformData {
        /**
         * @brief World-space direction pointing toward the light.
         *
         * XYZ contains the normalized light direction.
         * W is unused and must be zero.
         */
        Core::Math::Vector4f direction{};

        /**
         * @brief Linear RGB light color and intensity.
         *
         * XYZ contains the light's linear RGB color.
         * W contains its nonnegative intensity multiplier.
         */
        Core::Math::Vector4f color_intensity{};
    };
    static_assert(std::is_standard_layout_v<DirectionalLightUniformData>);

    static_assert(std::is_trivially_copyable_v<DirectionalLightUniformData>);

    static_assert(alignof(DirectionalLightUniformData) ==
                  kShaderFloat4Alignment);

    static_assert(offsetof(DirectionalLightUniformData, direction) == 0U);

    static_assert(offsetof(DirectionalLightUniformData, color_intensity) ==
                  sizeof(Core::Math::Vector4f));

    static_assert(sizeof(DirectionalLightUniformData) ==
                  sizeof(Core::Math::Vector4f) * 2U);

    /**
     * @brief Converts a directional light into GPU-compatible uniform data.
     *
     * Validates the supplied directional light and normalizes its
     * world-space direction.
     *
     * Packs the normalized direction into the XYZ components of the
     * direction vector, with W set to zero.
     *
     * Packs the linear RGB light color into the XYZ components of
     * color_intensity, with W containing the light intensity.
     *
     * @param light Directional light configuration to convert.
     *
     * @return Uniform-buffer data containing the normalized light
     * direction, color, and intensity.
     *
     * @pre The light must satisfy the constraints enforced by
     * validateDirectionalLight().
     *
     * @note Invalid light properties trigger fatal precondition
     * assertions in the Renderer subsystem.
     */
    [[nodiscard]] auto
    makeDirectionalLightUniformData(const DirectionalLight &light)
        -> DirectionalLightUniformData;
} // namespace SNE::Engine::Renderer
