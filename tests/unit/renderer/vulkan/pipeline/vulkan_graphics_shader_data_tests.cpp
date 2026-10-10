#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_graphics_pipeline.hpp"
#include <gtest/gtest.h>

namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Shader = SNE::Engine::Renderer::Shader;

TEST(GraphicsShaderDataTests, AcceptsValidShaderStageSelectionsAndEntryPoints) {
    Vulkan::GraphicsShaderData data;
    data.vertex.selection.stage = Shader::ShaderStage::Vertex;
    data.vertex.selection.entry_point = "main";
    data.fragment.selection.stage = Shader::ShaderStage::Fragment;
    data.fragment.selection.entry_point = "main";

    EXPECT_NO_THROW(Vulkan::validateGraphicsShaderData(data));
}

TEST(GraphicsShaderDataTests, RejectsIncorrectVertexStage) {
    Vulkan::GraphicsShaderData data;
    data.vertex.selection.stage = Shader::ShaderStage::Fragment;
    data.vertex.selection.entry_point = "main";
    data.fragment.selection.stage = Shader::ShaderStage::Fragment;
    data.fragment.selection.entry_point = "main";

    ASSERT_DEATH(Vulkan::validateGraphicsShaderData(data),
                 "VulkanGraphicsPipeline requires the vertex shader input "
                 "to specify ShaderStage::Vertex");
}

TEST(GraphicsShaderDataTests, RejectsIncorrectFragmentStage) {
    Vulkan::GraphicsShaderData data;
    data.vertex.selection.stage = Shader::ShaderStage::Vertex;
    data.vertex.selection.entry_point = "main";
    data.fragment.selection.stage = Shader::ShaderStage::Vertex;
    data.fragment.selection.entry_point = "main";

    ASSERT_DEATH(Vulkan::validateGraphicsShaderData(data),
                 "VulkanGraphicsPipeline requires the fragment shader input "
                 "to specify ShaderStage::Fragment");
}

TEST(GraphicsShaderDataTests, RejectsEmptyFragmentEntryPoint) {
    Vulkan::GraphicsShaderData data;
    data.vertex.selection.stage = Shader::ShaderStage::Vertex;
    data.vertex.selection.entry_point = "main";
    data.fragment.selection.stage = Shader::ShaderStage::Fragment;

    ASSERT_DEATH(Vulkan::validateGraphicsShaderData(data),
                 "VulkanGraphicsPipeline requires a nonempty fragment shader "
                 "entry-point name");
}

TEST(GraphicsShaderDataTests, RejectsEmptyVertexEntryPoint) {
    Vulkan::GraphicsShaderData data;
    data.vertex.selection.stage = Shader::ShaderStage::Vertex;
    data.fragment.selection.stage = Shader::ShaderStage::Fragment;
    data.fragment.selection.entry_point = "main";

    ASSERT_DEATH(Vulkan::validateGraphicsShaderData(data),
                 "VulkanGraphicsPipeline requires a nonempty vertex shader "
                 "entry-point name");
}
