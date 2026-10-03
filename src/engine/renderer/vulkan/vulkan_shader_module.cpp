#include "vulkan_shader_module.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/renderer/vulkan/vulkan_result.hpp"
#include <cstdint>
#include <span>
#include <string>

namespace SNE::Engine::Renderer::Vulkan {
    VulkanShaderModule::VulkanShaderModule(VkDevice device,
                                           std::span<const std::uint32_t> spirv)
        : m_Device(device) {
        VkShaderModuleCreateInfo shader_create_info{};
        shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_create_info.codeSize = spirv.size_bytes();
        shader_create_info.pCode = spirv.data();

        const VkResult result = vkCreateShaderModule(
            m_Device, &shader_create_info, nullptr, &m_ShaderModule);

        if (result != VK_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderModuleCreationFailed,
                "Failed to create Vulkan shader module",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Create Vulkan Shader Module");
        }
    }

    VulkanShaderModule::~VulkanShaderModule() noexcept {
        if (m_ShaderModule != VK_NULL_HANDLE) {
            vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
        }

        m_Device = VK_NULL_HANDLE;
        m_ShaderModule = VK_NULL_HANDLE;
    }

    auto VulkanShaderModule::nativeHandle() const noexcept -> VkShaderModule {
        return m_ShaderModule;
    }
} // namespace SNE::Engine::Renderer::Vulkan
