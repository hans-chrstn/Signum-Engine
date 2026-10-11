#include "engine/core/error/engine_error.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_validation.hpp"
#include <array>
#include <cstdint>
#include <gtest/gtest.h>
#include <span>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;

TEST(SpirvValidationTests, RejectsEmptyModule) {
    const std::span<const std::uint32_t> words{};

    EXPECT_THROW(Vulkan::validateSpirvModule(words), Core::Error::EngineError);
}

TEST(SpirvValidationTests, RejectsMalformedModule) {
    const std::array<std::uint32_t, 5> words{0x00000000, 0x00010600, 0, 1, 0};

    EXPECT_THROW(Vulkan::validateSpirvModule(words), Core::Error::EngineError);
}

TEST(SpirvValidationTests, AcceptsValidModule) {
    const std::filesystem::path shader_path =
        std::filesystem::path{SIGNUM_TEST_FIXTURE_DIR} / "triangle.vert.spv";

    const auto artifact = Vulkan::loadSpirv(shader_path);

    EXPECT_NO_THROW(Vulkan::validateSpirvModule(artifact.words));
}

TEST(SpirvValidationTests, ReportsValidationFailureCode) {
    const std::span<const std::uint32_t> words{};

    try {
        Vulkan::validateSpirvModule(words);
        FAIL() << "Expected SPIR-V validation to fail";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanSpirvValidationFailed);
    }
}

TEST(SpirvValidationTests, ProvidesNonemptyDiagnostic) {
    const std::span<const std::uint32_t> words{};

    try {
        Vulkan::validateSpirvModule(words);
        FAIL() << "Expected SPIR-V validation to fail";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_FALSE(std::string_view{error.what()}.empty());
    }
}
