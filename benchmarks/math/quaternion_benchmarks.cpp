#include "benchmark_data.hpp"
#include "engine/core/math/quaternion.hpp"
#include "engine/core/math/vector3.hpp"
#include <array>
#include <benchmark/benchmark.h>
#include <random>

namespace Math = SNE::Engine::Core::Math;

namespace {
    enum class QuaternionOperation {
        Multiply,
        Normalize,
        ToRotationMatrix,
        RotateViaMatrix,
    };

    struct QuaternionSample {
        Math::Quaternionf first{};
        Math::Quaternionf second{};
        Math::Quaternionf non_unit{};
        Math::Vector3f point{};
    };

    [[nodiscard]] auto createQuaternionSamples()
        -> std::array<QuaternionSample, SignumBench::kSampleCount> {
        constexpr float kMinimumComponent = -10.0F;
        constexpr float kMaximumComponent = 10.0F;
        constexpr float kMinimumAngle = -1.0F;
        constexpr float kMaximumAngle = 1.0F;
        constexpr float kMinimumScale = 0.5F;
        constexpr float kMaximumScale = 2.0F;

        // NOLINTBEGIN(bugprone-random-generator-seed)
        std::mt19937 generator{SignumBench::kRandomSeed};
        // NOLINTEND(bugprone-random-generator-seed)

        auto components = SignumBench::makeDistribution(kMinimumComponent,
                                                       kMaximumComponent);
        auto angles = SignumBench::makeDistribution(kMinimumAngle, kMaximumAngle);
        auto scales = SignumBench::makeDistribution(kMinimumScale, kMaximumScale);
        std::array<QuaternionSample, SignumBench::kSampleCount> samples{};

        for (auto &sample : samples) {
            Math::Vector3f first_axis =
                SignumBench::generateVector<Math::Vector3f>(generator, components);
            Math::Vector3f second_axis =
                SignumBench::generateVector<Math::Vector3f>(generator, components);
            // Ensure each axis is nonzero before fromAxisAngle normalizes it.
            first_axis.x += (first_axis.x < 0.0F) ? -kMinimumScale : kMinimumScale;
            second_axis.x += (second_axis.x < 0.0F) ? -kMinimumScale : kMinimumScale;
            sample.first = Math::fromAxisAngle(first_axis, angles(generator));
            sample.second = Math::fromAxisAngle(second_axis, angles(generator));
            sample.non_unit = Math::scale(sample.first, scales(generator));
            sample.point =
                SignumBench::generateVector<Math::Vector3f>(generator, components);
        }
        return samples;
    }

    template <QuaternionOperation Operation>
    void BM_QuaternionOperation(benchmark::State &state) {
        auto samples = createQuaternionSamples();
        SignumBench::run(state, samples, [](auto &sample) {
            if constexpr (Operation == QuaternionOperation::Multiply) {
                return Math::multiply(sample.first, sample.second);
            } else if constexpr (Operation == QuaternionOperation::Normalize) {
                return Math::normalize(sample.non_unit);
            } else if constexpr (Operation == QuaternionOperation::ToRotationMatrix) {
                return Math::toRotationMatrix(sample.first);
            } else {
                // Composite fallback: there is currently no public Quaternionf
                // direct vector rotation operation in the engine API.
                return Math::transformDirection(Math::toRotationMatrix(sample.first),
                                                sample.point);
            }
        });
    }
} // namespace

BENCHMARK_TEMPLATE(BM_QuaternionOperation, QuaternionOperation::Multiply)
    ->Name("BM_Quaternionf_Multiply");
BENCHMARK_TEMPLATE(BM_QuaternionOperation, QuaternionOperation::Normalize)
    ->Name("BM_Quaternionf_Normalize");
BENCHMARK_TEMPLATE(BM_QuaternionOperation, QuaternionOperation::ToRotationMatrix)
    ->Name("BM_Quaternionf_ToRotationMatrix");
BENCHMARK_TEMPLATE(BM_QuaternionOperation, QuaternionOperation::RotateViaMatrix)
    ->Name("BM_Quaternionf_RotateViaMatrix");
