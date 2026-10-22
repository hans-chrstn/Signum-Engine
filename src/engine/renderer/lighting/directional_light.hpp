#pragma once

#include "engine/core/math/vector3.hpp"
namespace SNE::Engine::Renderer {

    /**
     * @brief Describes a directional light in world space.
     *
     * Represents a light source whose rays travel in parallel,
     * independent of the positions of illuminated objects.
     *
     * Stores the direction toward the light, its linear RGB color,
     * and a scalar intensity multiplier.
     *
     * The light direction is expressed in world coordinates and
     * must be nonzero when used for lighting calculations.
     *
     * This structure is independent of the rendering backend and
     * does not own GPU resources.
     *
     * @note Light properties must be validated before use by
     * the renderer. The direction must be finite and nonzero,
     * the color components must be finite and nonnegative,
     * and the intensity must be finite and nonnegative.
     */
    struct DirectionalLight {
        /**
         * @brief World-space direction pointing toward the light source.
         *
         * Must be finite and nonzero when used for lighting.
         * The direction is normalized before lighting calculations.
         */
        Core::Math::Vector3f direction_to_light{};

        /**
         * @brief Linear RGB color of the directional light.
         *
         * Each component represents the relative contribution
         * of red, green, and blue light.
         *
         * Components must be finite and nonnegative.
         */
        Core::Math::Vector3f color{};

        /**
         * @brief Brightness multiplier applied to the light's color.
         *
         * Must be finite and nonnegative.
         * Zero disables the light's direct contribution.
         */
        float intensity{};
    };

    /**
     * @brief Validates a directional light before it is used for rendering.
     *
     * Checks that the light direction contains finite components
     * and has a nonzero magnitude.
     *
     * Also verifies that the light color components and intensity
     * are finite and nonnegative.
     *
     * @param light Directional light configuration to validate.
     *
     * @pre The light must satisfy all directional light constraints.
     *
     * @note Invalid properties trigger fatal precondition assertions
     * in the Renderer subsystem.
     */
    auto validateDirectionalLight(const DirectionalLight &light) -> void;
} // namespace SNE::Engine::Renderer
