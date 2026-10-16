#pragma once

#include "engine/renderer/shader/shader_stage.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Converts a shader stage to its Vulkan shader-stage flag.
     *
     * @param stage Backend-independent shader stage.
     *
     * @return Corresponding Vulkan shader-stage flag.
     *
     * @pre stage must identify a supported ShaderStage value.
     *
     * @note Unsupported stages trigger a fatal precondition assertion.
     */
    [[nodiscard]] auto toVkShaderStage(Shader::ShaderStage stage)
        -> VkShaderStageFlagBits;
} // namespace SNE::Engine::Renderer::Vulkan
