#include "vulkan_spirv_reflection.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/renderer/shader/push_constant_range.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_validation.hpp"
#include <spirv_reflect.h>
#include <string>
#include <string_view>
#include <vector>

namespace Core = SNE::Engine::Core;

namespace {
    [[nodiscard]] constexpr auto toString(SpvReflectResult result) noexcept
        -> std::string_view {
        switch (result) {
        case SPV_REFLECT_RESULT_SUCCESS:
            return "SPV_REFLECT_RESULT_SUCCESS";
        case SPV_REFLECT_RESULT_NOT_READY:
            return "SPV_REFLECT_RESULT_NOT_READY";
        case SPV_REFLECT_RESULT_ERROR_PARSE_FAILED:
            return "SPV_REFLECT_RESULT_ERROR_PARSE_FAILED";
        case SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED:
            return "SPV_REFLECT_RESULT_ERROR_ALLOC_FAILED";
        case SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED:
            return "SPV_REFLECT_RESULT_ERROR_RANGE_EXCEEDED";
        case SPV_REFLECT_RESULT_ERROR_NULL_POINTER:
            return "SPV_REFLECT_RESULT_ERROR_NULL_POINTER";
        case SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR:
            return "SPV_REFLECT_RESULT_ERROR_INTERNAL_ERROR";
        case SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH:
            return "SPV_REFLECT_RESULT_ERROR_COUNT_MISMATCH";
        case SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND:
            return "SPV_REFLECT_RESULT_ERROR_ELEMENT_NOT_FOUND";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_CODE_SIZE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_MAGIC_NUMBER";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_EOF";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ID_REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_SET_NUMBER_OVERFLOW";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_STORAGE_CLASS";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_RECURSION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_INSTRUCTION";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_UNEXPECTED_BLOCK_DATA";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_REFERENCE:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_BLOCK_MEMBER_"
                   "REFERENCE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_ENTRY_POINT";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_INVALID_EXECUTION_MODE";
        case SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED:
            return "SPV_REFLECT_RESULT_ERROR_SPIRV_MAX_RECURSIVE_EXCEEDED";

        default:
            return "UNKNOWN_SPV_REFLECT_RESULT";
        }
    }

} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    auto
    reflectSpirvInterface(std::span<const std::uint32_t> words,
                          const Shader::ShaderEntryPointSelection &selection)
        -> Shader::ShaderInterface {
        validateSpirvEntryPoint(words, selection);

        spv_reflect::ShaderModule shader_module{words.size_bytes(),
                                                words.data()};
        const SpvReflectResult result = shader_module.GetResult();
        if (result != SPV_REFLECT_RESULT_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSpirvReflectionFailed,
                "Failed to create SPIR-V reflection module",
                Core::Error::NativeError(static_cast<int>(result),
                                         std::string(toString(result))),
                "Reflect SPIR-V Interface");
        }

        Shader::ShaderInterface shader_interface{};
        shader_interface.entry_point = selection;

        std::uint32_t block_count{};

        const SpvReflectResult enumeration_result =
            shader_module.EnumerateEntryPointPushConstantBlocks(
                selection.entry_point.c_str(), &block_count, nullptr);

        if (enumeration_result != SPV_REFLECT_RESULT_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSpirvReflectionFailed,
                "Failed to enumerate SPIR-V push-constant blocks",
                Core::Error::NativeError(
                    static_cast<int>(enumeration_result),
                    std::string(toString(enumeration_result))),
                "Enumerate SPIR-V Push Constants");
        }

        std::vector<SpvReflectBlockVariable *> blocks(block_count);

        if (block_count > 0U) {
            const SpvReflectResult blocks_result =
                shader_module.EnumerateEntryPointPushConstantBlocks(
                    selection.entry_point.c_str(), &block_count, blocks.data());

            if (blocks_result != SPV_REFLECT_RESULT_SUCCESS) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanSpirvReflectionFailed,
                    "Failed to retrieve SPIR-V push-constant blocks",
                    Core::Error::NativeError(
                        static_cast<int>(blocks_result),
                        std::string(toString(blocks_result))),
                    "Retrieve SPIR-V Push Constants");
            }
        }

        shader_interface.push_constant_ranges.reserve(block_count);

        for (const SpvReflectBlockVariable *block : blocks) {
            Shader::PushConstantRange push_constant_range{
                .offset = block->offset,
                .size = block->size - block->offset,
                .stage = selection.stage,
            };

            shader_interface.push_constant_ranges.push_back(
                push_constant_range);
        }

        return shader_interface;
    }
} // namespace SNE::Engine::Renderer::Vulkan
