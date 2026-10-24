#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <span>

namespace Renderer = SNE::Engine::Renderer;

namespace {
    constexpr std::uint32_t kFloat32x2Size = 8U;
    constexpr std::uint32_t kFloat32x3Size = 12U;
    constexpr std::uint32_t kFloat32x4Size = 16U;
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

TEST(VertexFormatTests, Float32x4OccupiesSixteenBytes) {
    EXPECT_EQ(
        Renderer::vertexFormatSizeInBytes(Renderer::VertexFormat::Float32x4),
        kFloat32x4Size);
}

TEST(MeshUploadDataTests, BorrowsVertexAndIndexStorage) {
    constexpr std::size_t kVertexByteCount{24U};
    constexpr std::size_t kIndexCount{3U};
    constexpr std::uint32_t kVertexStride{8U};
    constexpr std::uint16_t kLastVertexIndex{2U};
    constexpr std::byte kUpdatedVertexByte{0x42};

    std::array<std::byte, kVertexByteCount> vertex_bytes{};
    std::array<std::uint16_t, kIndexCount> indices{
        0U,
        1U,
        kLastVertexIndex,
    };

    const Renderer::MeshUploadData mesh{
        .vertex_bytes = vertex_bytes,
        .vertex_stride = kVertexStride,
        .indices = indices,
    };

    EXPECT_EQ(mesh.vertex_bytes.data(), vertex_bytes.data());
    EXPECT_EQ(mesh.vertex_bytes.size_bytes(), vertex_bytes.size());
    EXPECT_EQ(mesh.vertex_stride, kVertexStride);

    EXPECT_EQ(mesh.indices.data(), indices.data());
    EXPECT_EQ(mesh.indices.size(), indices.size());

    vertex_bytes[0] = kUpdatedVertexByte;
    indices[0] = kLastVertexIndex;

    EXPECT_EQ(mesh.vertex_bytes[0], kUpdatedVertexByte);
    EXPECT_EQ(mesh.indices[0], kLastVertexIndex);
}

TEST(MeshUploadDataTests, CopyingDescriptionDoesNotCopyStorage) {
    constexpr std::size_t kVertexByteCount{24U};
    constexpr std::size_t kIndexCount{3U};
    constexpr std::uint32_t kVertexStride{8U};
    constexpr std::uint16_t kLastVertexIndex{2U};
    constexpr std::byte kUpdatedVertexByte{0x7F};

    std::array<std::byte, kVertexByteCount> vertex_bytes{};
    std::array<std::uint16_t, kIndexCount> indices{
        0U,
        1U,
        kLastVertexIndex,
    };

    const Renderer::MeshUploadData original{
        .vertex_bytes = vertex_bytes,
        .vertex_stride = kVertexStride,
        .indices = indices,
    };

    const Renderer::MeshUploadData copied = original;

    EXPECT_EQ(copied.vertex_bytes.data(), original.vertex_bytes.data());
    EXPECT_EQ(copied.indices.data(), original.indices.data());
    EXPECT_EQ(copied.vertex_stride, original.vertex_stride);

    vertex_bytes[0] = kUpdatedVertexByte;

    EXPECT_EQ(copied.vertex_bytes[0], kUpdatedVertexByte);
    EXPECT_EQ(original.vertex_bytes[0], kUpdatedVertexByte);
}

TEST(VertexLayoutDescriptionTests, BorrowsAttributeDescriptions) {
    constexpr std::size_t kAttributeCount{2U};
    constexpr std::uint32_t kColorAttributeOffset{12U};
    constexpr std::uint32_t kUpdatedLocation{3U};

    std::array<Renderer::VertexAttributeDescription, kAttributeCount>
        attributes{
            Renderer::VertexAttributeDescription{
                .location = 0U,
                .format = Renderer::VertexFormat::Float32x3,
                .offset = 0U,
            },
            Renderer::VertexAttributeDescription{
                .location = 1U,
                .format = Renderer::VertexFormat::Float32x3,
                .offset = kColorAttributeOffset,
            },
    };

    const Renderer::VertexLayoutDescription layout{
        .attributes = attributes,
    };

    ASSERT_EQ(layout.attributes.size(), attributes.size());

    EXPECT_EQ(layout.attributes.data(), attributes.data());
    EXPECT_EQ(layout.attributes[0], attributes[0]);
    EXPECT_EQ(layout.attributes[1], attributes[1]);

    attributes[1].location = kUpdatedLocation;

    EXPECT_EQ(layout.attributes[1].location, kUpdatedLocation);
}

TEST(InitialMeshDataTests, BorrowsMeshLayoutAndModelMatrices) {
    constexpr std::size_t kVertexByteCount{36U};
    constexpr std::size_t kIndexCount{3U};
    constexpr std::size_t kExpectedMatrixCount{2U};
    constexpr std::size_t kZRow{2U};
    constexpr std::size_t kTranslationColumn{
        SNE::Engine::Core::Math::kMatrix4Dimension - 1U,
    };

    constexpr std::uint32_t kVertexStride{12U};
    constexpr std::uint16_t kLastVertexIndex{2U};

    constexpr float kTranslationY{2.0F};
    constexpr float kTranslationZ{3.0F};
    constexpr float kUpdatedTranslationX{5.0F};

    const std::array<std::byte, kVertexByteCount> vertex_bytes{};

    const std::array<std::uint16_t, kIndexCount> indices{
        0U,
        1U,
        kLastVertexIndex,
    };

    const std::array attributes{
        Renderer::VertexAttributeDescription{
            .location = 0U,
            .format = Renderer::VertexFormat::Float32x3,
            .offset = 0U,
        },
    };

    std::array model_matrices{
        SNE::Engine::Core::Math::identity(),
        SNE::Engine::Core::Math::translate(1.0F, kTranslationY, kTranslationZ),
    };

    const Renderer::InitialMeshData initial_mesh{
        .mesh =
            {
                .vertex_bytes = vertex_bytes,
                .vertex_stride = kVertexStride,
                .indices = indices,
            },
        .layout =
            {
                .attributes = attributes,
            },
        .model_matrices = model_matrices,
    };

    EXPECT_EQ(initial_mesh.mesh.vertex_bytes.data(), vertex_bytes.data());
    EXPECT_EQ(initial_mesh.mesh.indices.data(), indices.data());
    EXPECT_EQ(initial_mesh.layout.attributes.data(), attributes.data());
    EXPECT_EQ(initial_mesh.model_matrices.data(), model_matrices.data());

    ASSERT_EQ(initial_mesh.model_matrices.size(), kExpectedMatrixCount);

    EXPECT_FLOAT_EQ(initial_mesh.model_matrices[1].at(0U, kTranslationColumn),
                    1.0F);

    EXPECT_FLOAT_EQ(initial_mesh.model_matrices[1].at(1U, kTranslationColumn),
                    kTranslationY);

    EXPECT_FLOAT_EQ(
        initial_mesh.model_matrices[1].at(kZRow, kTranslationColumn),
        kTranslationZ);

    model_matrices[1].at(0U, kTranslationColumn) = kUpdatedTranslationX;

    EXPECT_FLOAT_EQ(initial_mesh.model_matrices[1].at(0U, kTranslationColumn),
                    kUpdatedTranslationX);
}

TEST(MeshDrawInstanceTests, StoresMeshIdentityAndTransformation) {
    constexpr std::uint32_t kMeshIndex{7U};
    constexpr std::uint32_t kMeshGeneration{3U};
    constexpr std::uint64_t kRegistryId{19U};

    constexpr float kTranslationY{2.0F};
    constexpr float kTranslationZ{3.0F};

    constexpr std::size_t kZRow{2U};
    constexpr std::size_t kTranslationColumn{
        SNE::Engine::Core::Math::kMatrix4Dimension - 1U,
    };

    const Renderer::GpuResourceHandle mesh_handle{
        .index = kMeshIndex,
        .generation = kMeshGeneration,
        .registry_id = kRegistryId,
    };

    const Renderer::MeshDrawInstance instance{
        .mesh_handle = mesh_handle,
        .model_matrix = SNE::Engine::Core::Math::translate(1.0F, kTranslationY,
                                                           kTranslationZ),
    };

    EXPECT_EQ(instance.mesh_handle, mesh_handle);

    EXPECT_FLOAT_EQ(instance.model_matrix.at(0U, kTranslationColumn), 1.0F);

    EXPECT_FLOAT_EQ(instance.model_matrix.at(1U, kTranslationColumn),
                    kTranslationY);

    EXPECT_FLOAT_EQ(instance.model_matrix.at(kZRow, kTranslationColumn),
                    kTranslationZ);

    EXPECT_FLOAT_EQ(
        instance.model_matrix.at(kTranslationColumn, kTranslationColumn), 1.0F);
}
