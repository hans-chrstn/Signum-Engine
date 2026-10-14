#pragma once

#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/vulkan/descriptor/vulkan_descriptor_set_layout.hpp"
#include <span>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Provides a borrowed view of one descriptor-set layout's bindings.
     *
     * Contains the binding descriptions needed for shader compatibility
     * validation without requiring a Vulkan device or owning GPU resources.
     *
     * The caller must keep the referenced binding descriptions valid
     * throughout validation.
     *
     * The descriptor set index is determined by this view's position
     * in the collection supplied to the validator.
     */
    struct DescriptorSetLayoutBindingView {
        /** @brief Borrowed binding descriptions for this descriptor set. */
        std::span<const VulkanDescriptorBindingDescription> bindings;
    };

    /**
     * @brief Validates shader descriptor requirements against pipeline layouts.
     *
     * Compares the descriptor bindings required by a selected shader entry
     * point with the binding descriptions supplied by the pipeline layouts.
     *
     * Validation considers descriptor set indices, binding indices,
     * descriptor types, descriptor counts, and shader-stage visibility.
     *
     * Additional layout bindings not required by the shader are permitted.
     *
     * Validation does not create Vulkan resources or require a Vulkan device.
     *
     * @param shader_interface Reflected descriptor requirements of the
     * selected shader entry point.
     * @param pipeline_layouts Borrowed descriptor-set layout descriptions
     * ordered by descriptor set index.
     *
     * @pre Shader and pipeline layout descriptions must satisfy their
     * respective input contracts.
     *
     * @throws Core::Error::EngineError if the pipeline layouts do not
     * satisfy the shader's descriptor requirements.
     *
     * @note Invalid input descriptions trigger precondition assertions.
     */
    auto validateDescriptorCompatibility(
        const Shader::ShaderInterface &shader_interface,
        std::span<const DescriptorSetLayoutBindingView> pipeline_layouts)
        -> void;
} // namespace SNE::Engine::Renderer::Vulkan
