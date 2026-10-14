
#include "../vulkan_integration_test_fixture.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_shader_module.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include <filesystem>
#include <gtest/gtest.h>
#include <type_traits>
#include <utility>
#include <vulkan/vulkan.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Shader = SNE::Engine::Renderer::Shader;

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
