#include "vulkan_shader_stage.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"

namespace SNE::Engine::Renderer::Vulkan {
    auto toVkShaderStage(Shader::ShaderStage stage) -> VkShaderStageFlagBits {
        switch (stage) {
        case Shader::ShaderStage::Vertex:
            return VK_SHADER_STAGE_VERTEX_BIT;
        case Shader::ShaderStage::Fragment:
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        case Shader::ShaderStage::Compute:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Vulkan shader-stage conversion requires a supported shader "
                "stage");
        }
    }

} // namespace SNE::Engine::Renderer::Vulkan
