#include "../vulkan_integration_test_fixture.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_shader_module.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include <cstddef>
#include <filesystem>
#include <gtest/gtest.h>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Core = SNE::Engine::Core;

namespace {
    [[nodiscard]] auto loadTestShader() -> Shader::ShaderArtifact {
        return Vulkan::loadSpirv(std::filesystem::path{SIGNUM_TEST_SHADER_DIR} /
                                 "triangle.vert.spv");
    }

    static_assert(!std::is_copy_constructible_v<Vulkan::VulkanShaderModule>);
    static_assert(!std::is_copy_assignable_v<Vulkan::VulkanShaderModule>);
    static_assert(
        std::is_nothrow_move_constructible_v<Vulkan::VulkanShaderModule>);
    static_assert(
        std::is_nothrow_move_assignable_v<Vulkan::VulkanShaderModule>);
} // namespace

TEST_F(VulkanIntegrationTest, MoveConstructsShaderModule) {
    const auto shader = loadTestShader();

    Vulkan::VulkanShaderModule source{m_Device, shader.words};
    const VkShaderModule original_handle = source.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    Vulkan::VulkanShaderModule destination{std::move(source)};

    EXPECT_EQ(destination.nativeHandle(), original_handle);
}

TEST_F(VulkanIntegrationTest, MoveAssignsShaderModule) {
    const auto shader = loadTestShader();

    Vulkan::VulkanShaderModule source{m_Device, shader.words};
    Vulkan::VulkanShaderModule destination{m_Device, shader.words};

    const VkShaderModule source_handle = source.nativeHandle();
    const VkShaderModule previous_destination_handle =
        destination.nativeHandle();

    ASSERT_NE(source_handle, VK_NULL_HANDLE);
    ASSERT_NE(previous_destination_handle, VK_NULL_HANDLE);
    ASSERT_NE(source_handle, previous_destination_handle);

    destination = std::move(source);

    EXPECT_EQ(destination.nativeHandle(), source_handle);
    EXPECT_NE(destination.nativeHandle(), previous_destination_handle);
}

TEST_F(VulkanIntegrationTest, SelfMoveAssignsShaderModule) {
    const auto shader = loadTestShader();

    Vulkan::VulkanShaderModule module{m_Device, shader.words};
    const VkShaderModule original_handle = module.nativeHandle();

    ASSERT_NE(original_handle, VK_NULL_HANDLE);

    Vulkan::VulkanShaderModule &self = module;
    module = std::move(self);

    EXPECT_EQ(module.nativeHandle(), original_handle);
}

TEST_F(VulkanIntegrationTest,
       InvalidShaderArtifactDoesNotAffectExistingShaderModule) {
    const Shader::ShaderArtifact valid_artifact = loadTestShader();

    const Vulkan::VulkanShaderModule existing_module{
        m_Device,
        valid_artifact.words,
    };

    const VkShaderModule existing_handle = existing_module.nativeHandle();

    ASSERT_NE(existing_handle, VK_NULL_HANDLE);

    Shader::ShaderArtifact invalid_artifact = valid_artifact;
    invalid_artifact.words.front() = 0U;

    try {
        static_cast<void>(Vulkan::VulkanShaderModule{
            m_Device,
            invalid_artifact.words,
        });

        FAIL() << "Expected invalid shader module creation to fail";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(),
                  Core::Error::Code::VulkanSpirvValidationFailed);
    }

    EXPECT_EQ(existing_module.nativeHandle(), existing_handle);

    const Vulkan::VulkanShaderModule second_valid_module{
        m_Device,
        valid_artifact.words,
    };

    ASSERT_NE(second_valid_module.nativeHandle(), VK_NULL_HANDLE);

    EXPECT_NE(second_valid_module.nativeHandle(), existing_handle);
}

TEST_F(VulkanIntegrationTest,
       RepeatedInvalidShaderArtifactsDoNotPreventLaterCreation) {
    constexpr std::size_t kFailureAttemptCount{4U};

    const Shader::ShaderArtifact valid_artifact = loadTestShader();

    Shader::ShaderArtifact invalid_artifact = valid_artifact;
    invalid_artifact.words.front() = 0U;

    for (std::size_t attempt = 0U; attempt < kFailureAttemptCount; ++attempt) {
        EXPECT_THROW(static_cast<void>(Vulkan::VulkanShaderModule{
                         m_Device,
                         invalid_artifact.words,
                     }),
                     Core::Error::EngineError);
    }

    const Vulkan::VulkanShaderModule valid_module{
        m_Device,
        valid_artifact.words,
    };

    EXPECT_NE(valid_module.nativeHandle(), VK_NULL_HANDLE);
}
