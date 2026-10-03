#pragma once

#include <cstdint>
#include <span>
#include <vulkan/vulkan.h>

namespace SNE::Engine::Renderer::Vulkan {
    /**
     * @brief Owns a Vulkan shader module created from compiled SPIR-V bytecode.
     *
     * Creates and manages the lifetime of a VkShaderModule associated with a
     * logical Vulkan device.
     *
     * The logical device is borrowed and must remain valid for the lifetime of
     * this object. The supplied SPIR-V bytecode is borrowed only during
     * construction and is not retained.
     *
     * The type is non-copyable because it exclusively owns the shader-module
     * handle.
     */
    class VulkanShaderModule {
      private:
        /**
         * @brief Logical device used to manage the shader module.
         *
         * The handle is borrowed and must remain valid for the lifetime of this
         * object.
         */
        VkDevice m_Device{VK_NULL_HANDLE};
        /**
         * @brief Vulkan shader-module handle owned by this object.
         */
        VkShaderModule m_ShaderModule{VK_NULL_HANDLE};

      public:
        /**
         * @brief Creates a Vulkan shader module from compiled SPIR-V bytecode.
         *
         * @param device Logical device used to create and destroy the shader
         * module.
         * @param spirv Compiled SPIR-V words used to create the shader module.
         *
         * @throws Core::Error::EngineError if Vulkan fails to create the shader
         * module.
         */
        VulkanShaderModule(VkDevice device,
                           std::span<const std::uint32_t> spirv);

        /**
         * @brief Destroys the owned Vulkan shader module.
         */
        ~VulkanShaderModule() noexcept;
        VulkanShaderModule(const VulkanShaderModule &) = delete;
        auto operator=(const VulkanShaderModule &)
            -> VulkanShaderModule & = delete;

        /**
         * @brief Returns the underlying Vulkan shader-module handle.
         *
         * The returned handle is non-owning and remains valid only while this
         * object owns the shader module.
         *
         * @return Vulkan shader-module handle owned by this object.
         */
        [[nodiscard]] auto nativeHandle() const noexcept -> VkShaderModule;
    };
} // namespace SNE::Engine::Renderer::Vulkan
