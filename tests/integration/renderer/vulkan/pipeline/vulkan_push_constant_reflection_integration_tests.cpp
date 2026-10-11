#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/shader/shader_entry_point_selection.hpp"
#include "engine/renderer/shader/shader_interface.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_push_constant_compatibility.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv_reflection.hpp"
#include <array>
#include <cstdint>
#include <filesystem>
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Core = SNE::Engine::Core;
namespace Math = SNE::Engine::Core::Math;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

namespace {
    constexpr std::uint32_t kRangeOffset = 0U;
    constexpr std::uint32_t kModelSize =
        static_cast<std::uint32_t>(sizeof(Math::Matrix4f));
    constexpr std::uint32_t kPartialModelSize = kModelSize / 2U;
    constexpr std::uint32_t kTestDeviceLimit = kModelSize * 2U;
    constexpr std::size_t kExpectedRangeCount = 1U;

    [[nodiscard]] auto reflectCompiledShader(const char *filename,
                                             Shader::ShaderStage stage)
        -> Shader::ShaderInterface {
        const std::filesystem::path path =
            std::filesystem::path{SIGNUM_TEST_SHADER_DIR} / filename;

        const auto artifact = Vulkan::loadSpirv(path);

        const Shader::ShaderEntryPointSelection selection{
            .stage = stage,
            .entry_point = "main",
        };

        return Vulkan::reflectSpirvInterface(artifact.words, selection);
    }
} // namespace

TEST(PushConstantReflectionIntegrationTests,
     AcceptsRealVertexAndFragmentShaders) {
    const auto vertex_interface =
        reflectCompiledShader("triangle.vert.spv", Shader::ShaderStage::Vertex);

    const auto fragment_interface = reflectCompiledShader(
        "triangle.frag.spv", Shader::ShaderStage::Fragment);

    ASSERT_EQ(vertex_interface.push_constant_ranges.size(),
              kExpectedRangeCount);

    const auto &required = vertex_interface.push_constant_ranges.front();

    EXPECT_EQ(required.offset, kRangeOffset);
    EXPECT_EQ(required.size, kModelSize);
    EXPECT_EQ(required.stage, Shader::ShaderStage::Vertex);
    EXPECT_TRUE(fragment_interface.push_constant_ranges.empty());

    constexpr std::array ranges{
        VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = kRangeOffset,
            .size = kModelSize,
        },
    };

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        vertex_interface, ranges, kTestDeviceLimit));

    EXPECT_NO_THROW(Vulkan::validatePushConstantCompatibility(
        fragment_interface, ranges, kTestDeviceLimit));
}

TEST(PushConstantReflectionIntegrationTests,
     RejectsInsufficientLayoutForRealVertexShader) {
    const auto vertex_interface =
        reflectCompiledShader("triangle.vert.spv", Shader::ShaderStage::Vertex);

    constexpr std::array ranges{
        VkPushConstantRange{
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = kRangeOffset,
            .size = kPartialModelSize,
        },
    };

    try {
        Vulkan::validatePushConstantCompatibility(vertex_interface, ranges,
                                                  kTestDeviceLimit);

        FAIL() << "Expected incompatible push-constant layout";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanPushConstantLayoutIncompatible);
    }
}
