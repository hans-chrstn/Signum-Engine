#pragma once

#include "engine/renderer/shader/shader_resource_type.hpp"
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Converts a shader resource type to its Vulkan descriptor type.
     *
     * @param type Backend-independent shader resource type.
     *
     * @return Corresponding Vulkan descriptor type.
     *
     * @pre type must identify a supported ShaderResourceType value.
     *
     * @note Unsupported types trigger a fatal precondition assertion.
     */
    [[nodiscard]] auto toVkDescriptorType(Shader::ShaderResourceType type)
        -> VkDescriptorType;
} // namespace SNE::Engine::Renderer::Vulkan
