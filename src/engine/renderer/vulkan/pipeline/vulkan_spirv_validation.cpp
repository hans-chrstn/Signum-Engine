#include "vulkan_spirv_validation.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/native_error.hpp"
#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include <cstddef>
#include <cstdint>
#include <span>
#include <spirv-tools/libspirv.hpp>
#include <spirv/unified1/spirv.hpp>
#include <string>
#include <string_view>

namespace {
    [[nodiscard]] auto toString(spv_result_t result) noexcept
        -> std::string_view {
        switch (result) {
        case SPV_SUCCESS:
            return "SPV_SUCCESS";
        case SPV_UNSUPPORTED:
            return "SPV_UNSUPPORTED";
        case SPV_END_OF_STREAM:
            return "SPV_END_OF_STREAM";
        case SPV_WARNING:
            return "SPV_WARNING";
        case SPV_FAILED_MATCH:
            return "SPV_FAILED_MATCH";
        case SPV_REQUESTED_TERMINATION:
            return "SPV_REQUESTED_TERMINATION";
        case SPV_ERROR_INTERNAL:
            return "SPV_ERROR_INTERNAL";
        case SPV_ERROR_OUT_OF_MEMORY:
            return "SPV_ERROR_OUT_OF_MEMORY";
        case SPV_ERROR_INVALID_POINTER:
            return "SPV_ERROR_INVALID_POINTER";
        case SPV_ERROR_INVALID_BINARY:
            return "SPV_ERROR_INVALID_BINARY";
        case SPV_ERROR_INVALID_TEXT:
            return "SPV_ERROR_INVALID_TEXT";
        case SPV_ERROR_INVALID_TABLE:
            return "SPV_ERROR_INVALID_TABLE";
        case SPV_ERROR_INVALID_VALUE:
            return "SPV_ERROR_INVALID_VALUE";
        case SPV_ERROR_INVALID_DIAGNOSTIC:
            return "SPV_ERROR_INVALID_DIAGNOSTIC";
        case SPV_ERROR_INVALID_LOOKUP:
            return "SPV_ERROR_INVALID_LOOKUP";
        case SPV_ERROR_INVALID_ID:
            return "SPV_ERROR_INVALID_ID";
        case SPV_ERROR_INVALID_CFG:
            return "SPV_ERROR_INVALID_CFG";
        case SPV_ERROR_INVALID_LAYOUT:
            return "SPV_ERROR_INVALID_LAYOUT";
        case SPV_ERROR_INVALID_CAPABILITY:
            return "SPV_ERROR_INVALID_CAPABILITY";
        case SPV_ERROR_INVALID_DATA:
            return "SPV_ERROR_INVALID_DATA";
        case SPV_ERROR_MISSING_EXTENSION:
            return "SPV_ERROR_MISSING_EXTENSION";
        case SPV_ERROR_WRONG_VERSION:
            return "SPV_ERROR_WRONG_VERSION";
        case SPV_ERROR_FNVAR:
            return "SPV_ERROR_FNVAR";

        default:
            return "UNKNOWN_SPV_RESULT";
        }
    }
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    auto validateSpirvModule(std::span<const std::uint32_t> words) -> void {
        spvtools::SpirvTools tools{SPV_ENV_VULKAN_1_4};
        std::string diagnostic;
        tools.SetMessageConsumer(
            [&diagnostic](spv_message_level_t, const char *,
                          const spv_position_t &, const char *message) -> void {
                diagnostic = message;
            });

        const bool is_valid = tools.Validate(words.data(), words.size());

        if (!is_valid) {
            if (diagnostic.empty()) {
                diagnostic = "SPIR-V validation failed without diagnostics";
            }

            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSpirvValidationFailed,
                diagnostic.c_str(), "Validate SPIR-V Module");
        }
    }

    auto
    validateSpirvEntryPoint(std::span<const std::uint32_t> words,
                            const Shader::ShaderEntryPointSelection &selection)
        -> void {
        validateSpirvModule(words);

        struct EntryPointSearchState {
            const Shader::ShaderEntryPointSelection &selection;
            bool found{false};
        };

        EntryPointSearchState search_state{
            .selection = selection,
        };

        spvtools::Context context{SPV_ENV_VULKAN_1_4};
        spv_parsed_instruction_fn_t parse_instruction =
            // capture nothing, pointer to generic user_data, pointer to a
            // parsed SPIR-V instruction, return a spirv-tools result code
            [](void *user_data,
               const spv_parsed_instruction_t *instruction) -> spv_result_t {
            auto *state = static_cast<EntryPointSearchState *>(user_data);
            if (instruction->opcode == spv::OpEntryPoint &&
                instruction->num_operands >= 3U) {
                const spv_parsed_operand_t &execution_model_operand =
                    instruction->operands[0];
                const std::uint32_t execution_model =
                    instruction->words[execution_model_operand.offset];
                const spv_parsed_operand_t &entry_point_name_operand =
                    instruction->operands[2];
                const std::size_t name_offset = entry_point_name_operand.offset;
                const std::size_t name_word_count =
                    entry_point_name_operand.num_words;

                const char *name_data = reinterpret_cast<const char *>(
                    instruction->words + name_offset);

                const std::size_t name_byte_count =
                    name_word_count * sizeof(std::uint32_t);

                const std::string_view name_bytes{name_data, name_byte_count};

                const std::size_t terminator_position = name_bytes.find('\0');

                if (terminator_position == std::string_view::npos) {
                    return SPV_ERROR_INVALID_BINARY;
                }

                const std::string_view entry_point_name =
                    name_bytes.substr(0, terminator_position);

                const bool name_match =
                    entry_point_name == state->selection.entry_point;

                bool stage_matches = false;
                switch (state->selection.stage) {
                case Shader::ShaderStage::Vertex:
                    stage_matches =
                        execution_model == spv::ExecutionModelVertex;
                    break;
                case Shader::ShaderStage::Fragment:
                    stage_matches =
                        execution_model == spv::ExecutionModelFragment;
                    break;
                case Shader::ShaderStage::Compute:
                    stage_matches =
                        execution_model == spv::ExecutionModelGLCompute;
                    break;
                default:
                    break;
                }

                if (name_match && stage_matches) {
                    state->found = true;
                }
            }

            return SPV_SUCCESS;
        };

        const spv_result_t parse_result =
            spvBinaryParse(context.CContext(), &search_state, words.data(),
                           words.size(), nullptr, parse_instruction, nullptr);

        if (parse_result != SPV_SUCCESS) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanSpirvValidationFailed,
                "Failed to parse SPIR-V module while checking entry point",
                Core::Error::NativeError(static_cast<int>(parse_result),
                                         std::string(toString(parse_result))),
                "Validate SPIR-V Entry Point");
        }

        if (!search_state.found) {
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanShaderEntryPointUnavailable,
                "Requested SPIR-V shader entry point or stage was not found",
                "Validate SPIR-V Entry Point");
        }
    }
} // namespace SNE::Engine::Renderer::Vulkan
