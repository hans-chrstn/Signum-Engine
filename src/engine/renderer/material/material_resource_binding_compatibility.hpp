#pragma once

#include "engine/renderer/material/material_resource_binding.hpp"
#include "engine/renderer/shader/shader_descriptor_binding.hpp"
#include <span>

namespace SNE::Engine::Renderer {
    /**
     * @brief Checks compatibility between a material resource binding and
     * a shader descriptor requirement.
     *
     * Compares descriptor-set index, binding index, resource type, and
     * descriptor-array element against the shader's declared requirements.
     *
     * This function does not validate GPU resource existence or ownership.
     *
     * @param provided Material resource binding to check.
     * @param required Shader descriptor requirement to compare against.
     *
     * @return True when the binding location and resource type are compatible
     * and the array element is within the declared descriptor count;
     * otherwise false.
     */
    [[nodiscard]] auto isMaterialResourceBindingCompatible(
        const MaterialResourceBinding &provided,
        const Shader::ShaderDescriptorBinding &required) noexcept -> bool;

    /**
     * @brief Checks compatibility between material resource bindings and
     * shader descriptor requirements.
     *
     * Rejects duplicate material binding locations and verifies that every
     * required shader descriptor-array element has a compatible resource
     * binding.
     *
     * Additional material bindings not referenced by the supplied shader
     * requirements are permitted.
     *
     * This function does not validate GPU resource existence or ownership.
     *
     * @param provided Material resource bindings to check.
     * @param required Shader descriptor requirements to satisfy.
     *
     * @return True when no material binding locations are duplicated and
     * every required descriptor-array element is supplied by a compatible
     * material binding; otherwise false.
     */
    [[nodiscard]] auto areMaterialResourceBindingsCompatible(
        std::span<const MaterialResourceBinding> provided,
        std::span<const Shader::ShaderDescriptorBinding> required) noexcept
        -> bool;
} // namespace SNE::Engine::Renderer
