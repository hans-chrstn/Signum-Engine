
#pragma once

#include "engine/renderer/shader/shader_interface.hpp"
#include <cstdint>
#include <span>
#include <vulkan/vulkan.h>

namespace Shader = SNE::Engine::Renderer::Shader;

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Validates shader push-constant requirements against a Vulkan
     * pipeline layout.
     *
     * Compares the reflected push-constant ranges required by a selected
     * shader entry point against the ranges provided by a pipeline layout.
     *
     * Every shader-required byte must be covered by a compatible pipeline
     * range for the corresponding shader execution stage.
     *
     * Validates pipeline range descriptions, including nonzero sizes,
     * four-byte alignment, shader-stage flags, duplicate stage usage,
     * and bounds imposed by the physical device's push-constant limit.
     *
     * Validation does not create Vulkan resources or require a logical
     * device. Device limits are supplied by the caller.
     *
     * @param shader_interface Reflected resource requirements of the
     * selected shader entry point.
     * @param pipeline_ranges Borrowed Vulkan push-constant range
     * descriptions supplied by the pipeline layout.
     * @param max_push_constants_size Maximum push-constant storage size
     * in bytes supported by the selected physical device.
     *
     * @pre max_push_constants_size must be a valid, nonzero device limit.
     * @pre Pipeline range descriptions and shader requirements must
     * satisfy their respective input contracts.
     *
     * @throws Core::Error::EngineError if the pipeline layout does not
     * satisfy the shader's push-constant requirements.
     *
     * @note Malformed input descriptions trigger precondition assertions
     * rather than compatibility errors.
     */
    auto validatePushConstantCompatibility(
        const Shader::ShaderInterface &shader_interface,
        std::span<const VkPushConstantRange> pipeline_ranges,
        std::uint32_t max_push_constants_size) -> void;
} // namespace SNE::Engine::Renderer::Vulkan
