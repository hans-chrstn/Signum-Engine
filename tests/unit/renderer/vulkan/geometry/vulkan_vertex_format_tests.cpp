#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/vulkan/geometry/vulkan_vertex_format.hpp"
#include <gtest/gtest.h>
#include <vulkan/vulkan.h>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;

TEST(VulkanVertexFormatTests, ConvertsFloat32x2) {
    EXPECT_EQ(Vulkan::toVkFormat(Renderer::VertexFormat::Float32x2),
              VK_FORMAT_R32G32_SFLOAT);
}

TEST(VulkanVertexFormatTests, ConvertsFloat32x3) {
    EXPECT_EQ(Vulkan::toVkFormat(Renderer::VertexFormat::Float32x3),
              VK_FORMAT_R32G32B32_SFLOAT);
}
