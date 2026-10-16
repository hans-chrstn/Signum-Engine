#include "vulkan_push_constant_compatibility.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/renderer/shader/push_constant_range.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "vulkan_shader_stage.hpp"
#include <cstdint>
#include <span>
#include <vulkan/vulkan.h>

namespace Shader = SNE::Engine::Renderer::Shader;
namespace Core = SNE::Engine::Core;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    auto validatePipelinePushConstantRanges(
        std::span<const VkPushConstantRange> pipeline_ranges,
        std::uint32_t max_push_constants_size) -> void {
        if (max_push_constants_size == 0U) {
            Core::Assertion::failAssertion(
                Core::Assertion::AssertionType::Precondition,
                Core::Error::Subsystem::Vulkan,
                "Push-constant compatibility validation requires a nonzero "
                "device limit");
        }

        VkShaderStageFlags seen_stages = 0U;

        for (const VkPushConstantRange &pipeline_range : pipeline_ranges) {
            if (pipeline_range.size == 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Pipeline push-constant range must have a nonzero size");
            }

            if (pipeline_range.offset % 4U != 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Pipeline push-constant range offset must be aligned to 4 "
                    "bytes");
            }

            if (pipeline_range.size % 4U != 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Pipeline push-constant range size must be aligned to 4 "
                    "bytes");
            }

            if (pipeline_range.stageFlags == 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Pipeline push-constant range must specify at least one "
                    "shader stage");
            }

            if ((pipeline_range.stageFlags & seen_stages) != 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Pipeline push-constant ranges must not repeat shader "
                    "stages");
            }

            const std::uint64_t range_end =
                std::uint64_t{pipeline_range.offset} + pipeline_range.size;

            if (range_end > max_push_constants_size) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Pipeline push-constant range exceeds the physical device "
                    "limit");
            }

            seen_stages |= pipeline_range.stageFlags;
        }
    }

    [[nodiscard]] auto
    coversPushConstantRequirement(const Shader::PushConstantRange &required,
                                  const VkPushConstantRange &provided) -> bool {

        const auto required_stage = static_cast<VkShaderStageFlags>(
            Vulkan::toVkShaderStage(required.stage));

        const std::uint64_t required_end =
            std::uint64_t{required.offset} + required.size;

        const std::uint64_t provided_end =
            std::uint64_t{provided.offset} + provided.size;

        if ((provided.stageFlags & required_stage) == 0U) {
            return false;
        }

        if (provided.offset > required.offset) {
            return false;
        }

        if (provided_end < required_end) {
            return false;
        }

        return true;
    }
} // namespace

namespace SNE::Engine::Renderer::Vulkan {
    auto validatePushConstantCompatibility(
        const Shader::ShaderInterface &shader_interface,
        std::span<const VkPushConstantRange> pipeline_ranges,
        std::uint32_t max_push_constants_size) -> void {

        validatePipelinePushConstantRanges(pipeline_ranges,
                                           max_push_constants_size);

        for (const auto &required : shader_interface.push_constant_ranges) {
            if (required.size == 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Shader push-constant requirement must have a nonzero "
                    "size");
            }

            if (required.offset % 4U != 0U || required.size % 4U != 0U) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Shader push-constant requirement must be aligned to 4 "
                    "bytes");
            }

            const std::uint64_t required_end =
                std::uint64_t{required.offset} + required.size;

            if (required_end > max_push_constants_size) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Precondition,
                    Core::Error::Subsystem::Vulkan,
                    "Shader push-constant requirement exceeds the physical "
                    "device limit");
            }

            bool found_compatible_range = false;

            for (const auto &provided : pipeline_ranges) {
                if (coversPushConstantRequirement(required, provided)) {
                    found_compatible_range = true;
                    break;
                }
            }

            if (!found_compatible_range) {
                throw Core::Error::EngineError(
                    Core::Error::Code::VulkanPushConstantLayoutIncompatible,
                    "Pipeline layout does not satisfy shader push-constant "
                    "requirements",
                    "Validate Push-Constant Compatibility");
            }
        }
    }
} // namespace SNE::Engine::Renderer::Vulkan
