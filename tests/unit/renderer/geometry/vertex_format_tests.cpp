
#include "engine/renderer/geometry/vertex_format.hpp"
#include <cstdint>
#include <gtest/gtest.h>

namespace Renderer = SNE::Engine::Renderer;

namespace {
    constexpr std::uint32_t kFloat32x2Size = 8U;
    constexpr std::uint32_t kFloat32x3Size = 12U;
} // namespace

TEST(VertexFormatTests, Float32x2OccupiesEightBytes) {
    EXPECT_EQ(
        Renderer::vertexFormatSizeInBytes(Renderer::VertexFormat::Float32x2),
        kFloat32x2Size);
}

TEST(VertexFormatTests, Float32x3OccupiesTwelveBytes) {
    EXPECT_EQ(
        Renderer::vertexFormatSizeInBytes(Renderer::VertexFormat::Float32x3),
        kFloat32x3Size);
}
