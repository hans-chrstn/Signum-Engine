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
         * @brief Creates a Vulkan shader module from SPIR-V bytecode.
         *
         * Creates and owns a Vulkan shader module using the supplied logical
         * device and SPIR-V code.
         *
         * The logical device is borrowed and must remain valid for the lifetime
         * of this shader module. The supplied SPIR-V data is borrowed only
         * during construction and is not retained.
         *
         * @param device Logical device used to create and later destroy the
         * shader module.
         * @param spirv SPIR-V code used to create the shader module.
         *
         * @pre device must be a valid Vulkan logical-device handle.
         * @pre spirv must not be empty.
         *
         * @post Successful creation produces a non-null Vulkan shader-module
         * handle.
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
