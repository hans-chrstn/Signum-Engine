#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include <array>
#include <benchmark/benchmark.h>
#include <cstddef>
#include <cstdint>
#include <random>

namespace Math = SNE::Engine::Core::Math;

namespace {
    constexpr std::size_t kMatrixPairCount = 64U;
    struct MatrixPairData {
        std::array<Math::Matrix4f, kMatrixPairCount> first{};
        std::array<Math::Matrix4f, kMatrixPairCount> second{};
    };

    [[nodiscard]] auto createMatrixPairs() -> MatrixPairData {
        constexpr std::uint32_t kRandomSeed = 42U;
        constexpr float kMinimumValue = 0.25F;
        constexpr float kMaximumValue = 10.0F;

        // NOLINTBEGIN(bugprone-random-generator-seed)
        std::mt19937 generator{kRandomSeed};
        // NOLINTEND(bugprone-random-generator-seed)

        MatrixPairData matrix_pair{};
        std::uniform_real_distribution<float> distribution{kMinimumValue,
                                                           kMaximumValue};

        for (std::size_t matrix{}; matrix < kMatrixPairCount; ++matrix) {
            for (std::size_t row{}; row < Math::kMatrix4Dimension; ++row) {
                for (std::size_t column{}; column < Math::kMatrix4Dimension;
                     ++column) {
                    matrix_pair.first[matrix].at(row, column) =
                        distribution(generator);
                    matrix_pair.second[matrix].at(row, column) =
                        distribution(generator);
                }
            }
        }
        return matrix_pair;
    }

    constexpr std::size_t kTransformCount = 64U;

    struct TransformPointData {
        std::array<Math::Matrix4f, kTransformCount> matrices{};
        std::array<Math::Vector3f, kTransformCount> points{};
    };

    [[nodiscard]] auto createTransformPointData() -> TransformPointData {
        constexpr std::uint32_t kRandomSeed = 42U;
        constexpr float kMinimumTranslation = -10.0F;
        constexpr float kMaximumTranslation = 10.0F;
        constexpr float kMinimumRotation = -1.0F;
        constexpr float kMaximumRotation = 1.0F;
        constexpr float kMinimumScale = 0.5F;
        constexpr float kMaximumScale = 2.0F;
        constexpr float kMinimumPoint = -10.0F;
        constexpr float kMaximumPoint = 10.0F;

        // NOLINTBEGIN(bugprone-random-generator-seed)
        std::mt19937 generator{kRandomSeed};
        // NOLINTEND(bugprone-random-generator-seed)

        TransformPointData transform_point_data{};
        std::uniform_real_distribution<float> translation_distribution{
            kMinimumTranslation, kMaximumTranslation};
        std::uniform_real_distribution<float> rotation_distribution{
            kMinimumRotation, kMaximumRotation};
        std::uniform_real_distribution<float> scale_distribution{kMinimumScale,
                                                                 kMaximumScale};
        std::uniform_real_distribution<float> point_distribution{kMinimumPoint,
                                                                 kMaximumPoint};

        for (std::size_t point_data{}; point_data < kTransformCount;
             ++point_data) {
            const Math::Vector3f translation{
                .x = translation_distribution(generator),
                .y = translation_distribution(generator),
                .z = translation_distribution(generator),
            };
            const Math::Vector3f rotation{
                .x = rotation_distribution(generator),
                .y = rotation_distribution(generator),
                .z = rotation_distribution(generator),
            };
            const Math::Vector3f scale{
                .x = scale_distribution(generator),
                .y = scale_distribution(generator),
                .z = scale_distribution(generator),
            };
            const Math::Vector3f point{
                .x = point_distribution(generator),
                .y = point_distribution(generator),
                .z = point_distribution(generator),
            };

            transform_point_data.matrices[point_data] =
                Math::composeTRS(translation, rotation, scale);
            transform_point_data.points[point_data] = point;
        }
        return transform_point_data;
    }
} // namespace

static void BM_Matrix4Multiply(benchmark::State &state) {
    MatrixPairData matrix_pairs = createMatrixPairs();
    std::size_t matrix_index = 0U;
    for ([[maybe_unused]] auto iteration : state) {
        Math::Matrix4f &first = matrix_pairs.first[matrix_index];
        Math::Matrix4f &second = matrix_pairs.second[matrix_index];
        benchmark::DoNotOptimize(first);
        benchmark::DoNotOptimize(second);
        Math::Matrix4f result = Math::multiply(first, second);
        benchmark::DoNotOptimize(result);
        ++matrix_index;
        if (matrix_index == kMatrixPairCount) {
            matrix_index = 0U;
        }
    }
}

static void BM_MatrixPairSelection(benchmark::State &state) {
    MatrixPairData matrix_pairs = createMatrixPairs();
    std::size_t matrix_index = 0U;
    for ([[maybe_unused]] auto iteration : state) {
        Math::Matrix4f &first = matrix_pairs.first[matrix_index];
        Math::Matrix4f &second = matrix_pairs.second[matrix_index];
        benchmark::DoNotOptimize(first);
        benchmark::DoNotOptimize(second);
        ++matrix_index;
        if (matrix_index == kMatrixPairCount) {
            matrix_index = 0U;
        }
    }
}

static void BM_Matrix4TransformPoint(benchmark::State &state) {
    TransformPointData data = createTransformPointData();
    std::size_t transform_index = 0U;
    for ([[maybe_unused]] auto iteration : state) {
        Math::Matrix4f &matrix = data.matrices[transform_index];
        Math::Vector3f &point = data.points[transform_index];
        benchmark::DoNotOptimize(matrix);
        benchmark::DoNotOptimize(point);
        Math::Vector3f transform_point = Math::transformPoint(matrix, point);
        benchmark::DoNotOptimize(transform_point);
        ++transform_index;
        if (transform_index == kTransformCount) {
            transform_index = 0U;
        }
    }
}

static void BM_TransformPointSelection(benchmark::State &state) {
    TransformPointData data = createTransformPointData();
    std::size_t transform_index = 0U;
    for ([[maybe_unused]] auto iteration : state) {
        Math::Matrix4f &matrix = data.matrices[transform_index];
        Math::Vector3f &point = data.points[transform_index];
        benchmark::DoNotOptimize(matrix);
        benchmark::DoNotOptimize(point);
        ++transform_index;
        if (transform_index == kTransformCount) {
            transform_index = 0U;
        }
    }
}

BENCHMARK(BM_Matrix4Multiply);
BENCHMARK(BM_MatrixPairSelection);
BENCHMARK(BM_Matrix4TransformPoint);
BENCHMARK(BM_TransformPointSelection);
