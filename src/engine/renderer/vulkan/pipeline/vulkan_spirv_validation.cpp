#include "vulkan_spirv_validation.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include <cstdint>
#include <span>
#include <spirv-tools/libspirv.hpp>
#include <string>

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
} // namespace SNE::Engine::Renderer::Vulkan
