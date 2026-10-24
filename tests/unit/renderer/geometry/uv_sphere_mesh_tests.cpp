#include "engine/renderer/geometry/uv_sphere_mesh.hpp"
#include <cstddef>
#include <cstdint>
#include <gtest/gtest.h>
#include <limits>

namespace Renderer = SNE::Engine::Renderer;

namespace {
    constexpr std::size_t kPositionX{0U};
    constexpr std::size_t kPositionY{1U};
    constexpr std::size_t kPositionZ{2U};
    constexpr std::size_t kTangentX{0U};
    constexpr std::size_t kTangentY{1U};
    constexpr std::size_t kTangentZ{2U};
    constexpr std::size_t kTangentHandedness{3U};
    constexpr std::size_t kThirdTriangleIndexOffset{2U};
    constexpr std::size_t kIndicesPerTriangle{3U};
    constexpr std::size_t kIndicesPerCell{6U};

    constexpr float kFloatTolerance{0.0001F};
} // namespace

TEST(UvSphereMeshTests, GeneratesExpectedVertexAndIndexCounts) {
    constexpr std::uint32_t kLatitudeSegments{10U};
    constexpr std::uint32_t kLongitudeSegments{16U};

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    const std::size_t expected_vertex_count =
        (static_cast<std::size_t>(kLatitudeSegments) + 1U) *
        (static_cast<std::size_t>(kLongitudeSegments) + 1U);

    const std::size_t expected_index_count =
        kIndicesPerCell * (static_cast<std::size_t>(kLatitudeSegments) - 1U) *
        static_cast<std::size_t>(kLongitudeSegments);

    EXPECT_EQ(mesh.vertices.size(), expected_vertex_count);
    EXPECT_EQ(mesh.indices.size(), expected_index_count);
}

TEST(UvSphereMeshTests, AcceptsMinimumSegmentCounts) {
    constexpr std::uint32_t kMinimumLatitudeSegments{2U};
    constexpr std::uint32_t kMinimumLongitudeSegments{3U};

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kMinimumLongitudeSegments,
        .latitude_segments = kMinimumLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    constexpr std::size_t kExpectedVertexCount{12U};
    constexpr std::size_t kExpectedIndexCount{18U};

    EXPECT_EQ(mesh.vertices.size(), kExpectedVertexCount);
    EXPECT_EQ(mesh.indices.size(), kExpectedIndexCount);

    for (const std::uint16_t vertex_index : mesh.indices) {
        EXPECT_LT(static_cast<std::size_t>(vertex_index), mesh.vertices.size());
    }
}

TEST(UvSphereMeshTests, PreservesExactPolePositionsAndNormals) {
    constexpr std::uint32_t kLatitudeSegments{8U};
    constexpr std::uint32_t kLongitudeSegments{12U};
    constexpr float kRadius{2.0F};

    const Renderer::UvSphereParameters parameters{
        .radius = kRadius,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    const std::size_t vertices_per_row =
        static_cast<std::size_t>(kLongitudeSegments) + 1U;

    const std::size_t south_pole_row_start =
        static_cast<std::size_t>(kLatitudeSegments) * vertices_per_row;

    for (std::size_t column_index{}; column_index < vertices_per_row;
         ++column_index) {
        const auto &north = mesh.vertices[column_index];
        const auto &south = mesh.vertices[south_pole_row_start + column_index];

        EXPECT_FLOAT_EQ(north.position[kPositionX], 0.0F);
        EXPECT_FLOAT_EQ(north.position[kPositionY], kRadius);
        EXPECT_FLOAT_EQ(north.position[kPositionZ], 0.0F);

        EXPECT_FLOAT_EQ(north.normal[kPositionX], 0.0F);
        EXPECT_FLOAT_EQ(north.normal[kPositionY], 1.0F);
        EXPECT_FLOAT_EQ(north.normal[kPositionZ], 0.0F);

        EXPECT_FLOAT_EQ(south.position[kPositionX], 0.0F);
        EXPECT_FLOAT_EQ(south.position[kPositionY], -kRadius);
        EXPECT_FLOAT_EQ(south.position[kPositionZ], 0.0F);

        EXPECT_FLOAT_EQ(south.normal[kPositionX], 0.0F);
        EXPECT_FLOAT_EQ(south.normal[kPositionY], -1.0F);
        EXPECT_FLOAT_EQ(south.normal[kPositionZ], 0.0F);

        EXPECT_FLOAT_EQ(north.uv[1U], 0.0F);
        EXPECT_FLOAT_EQ(south.uv[1U], 1.0F);
    }
}

TEST(UvSphereMeshTests, DuplicatesLongitudeSeamWithDistinctUvCoordinates) {
    constexpr std::uint32_t kLatitudeSegments{8U};
    constexpr std::uint32_t kLongitudeSegments{12U};

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    const std::size_t vertices_per_row =
        static_cast<std::size_t>(kLongitudeSegments) + 1U;

    for (std::size_t latitude_row{};
         latitude_row <= static_cast<std::size_t>(kLatitudeSegments);
         ++latitude_row) {
        const std::size_t first_vertex_index = latitude_row * vertices_per_row;

        const std::size_t last_vertex_index =
            first_vertex_index + static_cast<std::size_t>(kLongitudeSegments);

        const auto &first = mesh.vertices[first_vertex_index];
        const auto &last = mesh.vertices[last_vertex_index];

        EXPECT_EQ(first.position, last.position);
        EXPECT_EQ(first.normal, last.normal);
        EXPECT_EQ(first.tangent, last.tangent);

        EXPECT_FLOAT_EQ(first.uv[0U], 0.0F);
        EXPECT_FLOAT_EQ(last.uv[0U], 1.0F);
        EXPECT_FLOAT_EQ(first.uv[1U], last.uv[1U]);
    }
}

TEST(UvSphereMeshTests, NormalsAreUnitLengthAndPositionsMatchRadius) {
    constexpr std::uint32_t kLatitudeSegments{16U};
    constexpr std::uint32_t kLongitudeSegments{24U};
    constexpr float kRadius{2.5F};

    const Renderer::UvSphereParameters parameters{
        .radius = kRadius,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    for (const Renderer::UvSphereVertex &vertex : mesh.vertices) {
        const float normal_length_squared =
            (vertex.normal[kPositionX] * vertex.normal[kPositionX]) +
            (vertex.normal[kPositionY] * vertex.normal[kPositionY]) +
            (vertex.normal[kPositionZ] * vertex.normal[kPositionZ]);

        EXPECT_NEAR(normal_length_squared, 1.0F, kFloatTolerance);

        for (std::size_t component_index{};
             component_index < Renderer::UvSphereVertex::kSpatialComponentCount;
             ++component_index) {
            EXPECT_NEAR(vertex.position[component_index],
                        vertex.normal[component_index] * kRadius,
                        kFloatTolerance);
        }
    }
}

TEST(UvSphereMeshTests, TangentsAreUnitLengthAndOrthogonalToNormals) {
    constexpr std::uint32_t kLatitudeSegments{16U};
    constexpr std::uint32_t kLongitudeSegments{24U};

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    for (const Renderer::UvSphereVertex &vertex : mesh.vertices) {
        const auto &normal = vertex.normal;
        const auto &tangent = vertex.tangent;

        const float tangent_length_squared =
            (tangent[kTangentX] * tangent[kTangentX]) +
            (tangent[kTangentY] * tangent[kTangentY]) +
            (tangent[kTangentZ] * tangent[kTangentZ]);

        const float normal_dot_tangent =
            (normal[kPositionX] * tangent[kTangentX]) +
            (normal[kPositionY] * tangent[kTangentY]) +
            (normal[kPositionZ] * tangent[kTangentZ]);

        EXPECT_NEAR(tangent_length_squared, 1.0F, kFloatTolerance);
        EXPECT_NEAR(normal_dot_tangent, 0.0F, kFloatTolerance);
        EXPECT_FLOAT_EQ(tangent[kTangentHandedness], 1.0F);
    }
}

TEST(UvSphereMeshTests, TangentsFollowLongitudeAndReconstructBitangent) {
    constexpr std::uint32_t kLatitudeSegments{8U};
    constexpr std::uint32_t kLongitudeSegments{12U};

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    const std::size_t vertices_per_row =
        static_cast<std::size_t>(kLongitudeSegments) + 1U;

    const std::size_t equator_index =
        static_cast<std::size_t>(kLatitudeSegments / 2U) * vertices_per_row;

    const auto quarter_longitude_index =
        equator_index + static_cast<std::size_t>(kLongitudeSegments / 4U);

    const auto &vertex = mesh.vertices[equator_index];
    const auto &normal = vertex.normal;
    const auto &tangent = vertex.tangent;

    EXPECT_NEAR(tangent[kTangentX], 0.0F, kFloatTolerance);
    EXPECT_NEAR(tangent[kTangentY], 0.0F, kFloatTolerance);
    EXPECT_NEAR(tangent[kTangentZ], 1.0F, kFloatTolerance);

    const auto &quarter_tangent =
        mesh.vertices[quarter_longitude_index].tangent;

    EXPECT_NEAR(quarter_tangent[kTangentX], -1.0F, kFloatTolerance);
    EXPECT_NEAR(quarter_tangent[kTangentY], 0.0F, kFloatTolerance);
    EXPECT_NEAR(quarter_tangent[kTangentZ], 0.0F, kFloatTolerance);

    const float bitangent_x = ((normal[kPositionY] * tangent[kTangentZ]) -
                               (normal[kPositionZ] * tangent[kTangentY])) *
                              tangent[kTangentHandedness];

    const float bitangent_y = ((normal[kPositionZ] * tangent[kTangentX]) -
                               (normal[kPositionX] * tangent[kTangentZ])) *
                              tangent[kTangentHandedness];

    const float bitangent_z = ((normal[kPositionX] * tangent[kTangentY]) -
                               (normal[kPositionY] * tangent[kTangentX])) *
                              tangent[kTangentHandedness];

    EXPECT_NEAR(bitangent_x, 0.0F, kFloatTolerance);
    EXPECT_NEAR(bitangent_y, -1.0F, kFloatTolerance);
    EXPECT_NEAR(bitangent_z, 0.0F, kFloatTolerance);
}

TEST(UvSphereMeshTests, PoleTangentsFollowLongitudeConvention) {
    constexpr std::uint32_t kLatitudeSegments{8U};
    constexpr std::uint32_t kLongitudeSegments{12U};

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    const std::size_t vertices_per_row =
        static_cast<std::size_t>(kLongitudeSegments) + 1U;

    const std::size_t south_pole_row_start =
        static_cast<std::size_t>(kLatitudeSegments) * vertices_per_row;

    const auto quarter_longitude =
        static_cast<std::size_t>(kLongitudeSegments / 4U);

    const auto &north_first = mesh.vertices[0U].tangent;
    const auto &south_first = mesh.vertices[south_pole_row_start].tangent;

    EXPECT_EQ(north_first, south_first);

    EXPECT_NEAR(north_first[kTangentX], 0.0F, kFloatTolerance);
    EXPECT_NEAR(north_first[kTangentY], 0.0F, kFloatTolerance);
    EXPECT_NEAR(north_first[kTangentZ], 1.0F, kFloatTolerance);

    const auto &north_quarter = mesh.vertices[quarter_longitude].tangent;

    const auto &south_quarter =
        mesh.vertices[south_pole_row_start + quarter_longitude].tangent;

    EXPECT_EQ(north_quarter, south_quarter);

    EXPECT_NEAR(north_quarter[kTangentX], -1.0F, kFloatTolerance);
    EXPECT_NEAR(north_quarter[kTangentY], 0.0F, kFloatTolerance);
    EXPECT_NEAR(north_quarter[kTangentZ], 0.0F, kFloatTolerance);
}

TEST(UvSphereMeshTests, GeneratesOutwardFacingNondegenerateTriangles) {
    constexpr std::uint32_t kLatitudeSegments{8U};
    constexpr std::uint32_t kLongitudeSegments{12U};

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    ASSERT_EQ(mesh.indices.size() % kIndicesPerTriangle, 0U);

    for (std::size_t index_offset{}; index_offset < mesh.indices.size();
         index_offset += kIndicesPerTriangle) {
        const std::size_t first_index = mesh.indices[index_offset];
        const std::size_t second_index = mesh.indices[index_offset + 1U];
        const std::size_t third_index =
            mesh.indices[index_offset + kThirdTriangleIndexOffset];

        ASSERT_LT(first_index, mesh.vertices.size());
        ASSERT_LT(second_index, mesh.vertices.size());
        ASSERT_LT(third_index, mesh.vertices.size());

        EXPECT_NE(first_index, second_index);
        EXPECT_NE(second_index, third_index);
        EXPECT_NE(first_index, third_index);

        const auto &first = mesh.vertices[first_index].position;
        const auto &second = mesh.vertices[second_index].position;
        const auto &third = mesh.vertices[third_index].position;

        const float first_edge_x = second[kPositionX] - first[kPositionX];
        const float first_edge_y = second[kPositionY] - first[kPositionY];
        const float first_edge_z = second[kPositionZ] - first[kPositionZ];

        const float second_edge_x = third[kPositionX] - first[kPositionX];
        const float second_edge_y = third[kPositionY] - first[kPositionY];
        const float second_edge_z = third[kPositionZ] - first[kPositionZ];

        const float face_normal_x =
            (first_edge_y * second_edge_z) - (first_edge_z * second_edge_y);

        const float face_normal_y =
            (first_edge_z * second_edge_x) - (first_edge_x * second_edge_z);

        const float face_normal_z =
            (first_edge_x * second_edge_y) - (first_edge_y * second_edge_x);

        const float outward_orientation = (face_normal_x * first[kPositionX]) +
                                          (face_normal_y * first[kPositionY]) +
                                          (face_normal_z * first[kPositionZ]);

        EXPECT_GT(outward_orientation, 0.0F);
    }
}

TEST(UvSphereMeshTests, ProducesDeterministicGeometry) {
    constexpr std::uint32_t kLatitudeSegments{10U};
    constexpr std::uint32_t kLongitudeSegments{16U};

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh first =
        Renderer::generateUvSphereMesh(parameters);

    const Renderer::UvSphereMesh second =
        Renderer::generateUvSphereMesh(parameters);

    ASSERT_EQ(first.vertices.size(), second.vertices.size());
    EXPECT_EQ(first.indices, second.indices);

    for (std::size_t vertex_index{}; vertex_index < first.vertices.size();
         ++vertex_index) {
        EXPECT_EQ(first.vertices[vertex_index].position,
                  second.vertices[vertex_index].position);

        EXPECT_EQ(first.vertices[vertex_index].normal,
                  second.vertices[vertex_index].normal);

        EXPECT_EQ(first.vertices[vertex_index].uv,
                  second.vertices[vertex_index].uv);

        EXPECT_EQ(first.vertices[vertex_index].tangent,
                  second.vertices[vertex_index].tangent);
    }
}

TEST(UvSphereMeshTests, SupportsMaximumIndexableVertexCount) {
    constexpr std::uint32_t kLatitudeSegments{255U};
    constexpr std::uint32_t kLongitudeSegments{255U};

    constexpr std::size_t kMaximumVertexCount =
        static_cast<std::size_t>(std::numeric_limits<std::uint16_t>::max()) +
        1U;

    const Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    const Renderer::UvSphereMesh mesh =
        Renderer::generateUvSphereMesh(parameters);

    ASSERT_EQ(mesh.vertices.size(), kMaximumVertexCount);

    for (const std::uint16_t vertex_index : mesh.indices) {
        EXPECT_LT(static_cast<std::size_t>(vertex_index), mesh.vertices.size());
    }
}

TEST(UvSphereMeshTests, RejectsNonPositiveAndNonFiniteRadius) {
    constexpr std::uint32_t kLatitudeSegments{8U};
    constexpr std::uint32_t kLongitudeSegments{12U};

    Renderer::UvSphereParameters parameters{
        .radius = 0.0F,
        .longitude_segments = kLongitudeSegments,
        .latitude_segments = kLatitudeSegments,
    };

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere radius must be finite and greater than zero");

    parameters.radius = -1.0F;

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere radius must be finite and greater than zero");

    parameters.radius = std::numeric_limits<float>::infinity();

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere radius must be finite and greater than zero");

    parameters.radius = std::numeric_limits<float>::quiet_NaN();

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere radius must be finite and greater than zero");
}

TEST(UvSphereMeshTests, RejectsInsufficientSegmentCounts) {
    constexpr std::uint32_t kValidLatitudeSegments{8U};
    constexpr std::uint32_t kValidLongitudeSegments{12U};
    constexpr std::uint32_t kInvalidLongitudeSegments{2U};

    Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kInvalidLongitudeSegments,
        .latitude_segments = kValidLatitudeSegments,
    };

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere longitude segment count must be at least three");

    parameters.longitude_segments = kValidLongitudeSegments;
    parameters.latitude_segments = 1U;

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere latitude segment count must be at least two");
}

TEST(UvSphereMeshTests, RejectsVertexCountsBeyondSixteenBitIndexRange) {
    constexpr std::uint32_t kValidSegmentCount{8U};
    constexpr std::uint32_t kExcessiveSegmentCount{256U};
    constexpr std::uint32_t kMaximumSegmentCount =
        std::numeric_limits<std::uint32_t>::max();

    Renderer::UvSphereParameters parameters{
        .radius = 1.0F,
        .longitude_segments = kMaximumSegmentCount,
        .latitude_segments = kValidSegmentCount,
    };

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere segment count exceeds the 16-bit indexable range");

    parameters.longitude_segments = kValidSegmentCount;
    parameters.latitude_segments = kMaximumSegmentCount;

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere segment count exceeds the 16-bit indexable range");

    parameters.longitude_segments = kExcessiveSegmentCount;
    parameters.latitude_segments = kExcessiveSegmentCount;

    ASSERT_DEATH(static_cast<void>(Renderer::generateUvSphereMesh(parameters)),
                 "UV sphere vertex count exceeds the 16-bit indexable range");
}
