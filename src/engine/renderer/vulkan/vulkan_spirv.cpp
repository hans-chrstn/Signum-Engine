#include "vulkan_spirv.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"

#include <cstddef>
#include <fstream>
#include <limits>

namespace SNE::Engine::Renderer::Vulkan {
    auto loadSpirv(const std::filesystem::path &path)
        -> std::vector<std::uint32_t> {
        std::ifstream file(path, std::ios::binary | std::ios::ate);

        if (!file.is_open()) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderBytecodeLoadFailed,
                "Failed to open compiled SPIR-V shader bytecode",
                "Open SPIR-V Shader Bytecode");
        }

        const std::streampos end_position = file.tellg();

        if (end_position == std::streampos{-1}) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderBytecodeLoadFailed,
                "Failed to determine compiled SPIR-V shader bytecode size",
                "Determine SPIR-V Shader Bytecode Size");
        }

        const auto byte_size = static_cast<std::streamoff>(end_position);

        if (byte_size <= 0) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderBytecodeLoadFailed,
                "Compiled SPIR-V shader bytecode is empty",
                "Validate SPIR-V Shader Bytecode Size");
        }

        if (byte_size % static_cast<std::streamoff>(sizeof(std::uint32_t)) !=
            0) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderBytecodeLoadFailed,
                "Compiled SPIR-V shader bytecode has an invalid size",
                "Validate SPIR-V Shader Bytecode Size");
        }

        if (byte_size > std::numeric_limits<std::streamsize>::max()) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderBytecodeLoadFailed,
                "Compiled SPIR-V shader bytecode is too large to read",
                "Validate SPIR-V Shader Bytecode Size");
        }

        const std::size_t word_count =
            static_cast<std::size_t>(byte_size) / sizeof(std::uint32_t);

        std::vector<std::uint32_t> spirv(word_count);

        file.seekg(0, std::ios::beg);

        if (!file) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderBytecodeLoadFailed,
                "Failed to seek to the beginning of compiled SPIR-V shader "
                "bytecode",
                "Seek SPIR-V Shader Bytecode");
        }

        file.read(reinterpret_cast<char *>(spirv.data()),
                  static_cast<std::streamsize>(byte_size));

        if (!file) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderBytecodeLoadFailed,
                "Failed to read compiled SPIR-V shader bytecode completely",
                "Read SPIR-V Shader Bytecode");
        }

        return spirv;
    }
} // namespace SNE::Engine::Renderer::Vulkan
