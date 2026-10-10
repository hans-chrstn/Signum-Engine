#include "engine/core/math/matrix4.hpp"
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

BENCHMARK(BM_Matrix4Multiply);
BENCHMARK(BM_MatrixPairSelection);
