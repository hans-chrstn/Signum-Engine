#include "benchmark_data.hpp"
#include "engine/core/math/vector2.hpp"
#include "engine/core/math/vector2d.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/core/math/vector3d.hpp"
#include "engine/core/math/vector4.hpp"
#include "engine/core/math/vector4d.hpp"
#include <array>
#include <benchmark/benchmark.h>
#include <random>

namespace Math = SNE::Engine::Core::Math;

namespace {
    enum class VectorOperation {
        Add,
        Subtract,
        Scale,
        Dot,
        Cross,
        LengthSquared,
        Length,
        Normalize,
    };

    template <typename Vector>
    struct VectorSample {
        Vector first{};
        Vector second{};
        SignumBench::Component<Vector> scalar{};
    };

    template <typename Vector>
    [[nodiscard]] auto createVectorSamples()
        -> std::array<VectorSample<Vector>, SignumBench::kSampleCount> {
        using Scalar = SignumBench::Component<Vector>;
        constexpr Scalar kMinimumValue = static_cast<Scalar>(-10.0);
        constexpr Scalar kMaximumValue = static_cast<Scalar>(10.0);
        constexpr Scalar kMinimumScale = static_cast<Scalar>(0.5);
        constexpr Scalar kMaximumScale = static_cast<Scalar>(2.0);

        // NOLINTBEGIN(bugprone-random-generator-seed)
        std::mt19937 generator{SignumBench::kRandomSeed};
        // NOLINTEND(bugprone-random-generator-seed)

        auto distribution = SignumBench::makeDistribution(kMinimumValue, kMaximumValue);
        auto scale_distribution = SignumBench::makeDistribution(kMinimumScale, kMaximumScale);
        std::array<VectorSample<Vector>, SignumBench::kSampleCount> samples{};

        for (auto &sample : samples) {
            sample.first = SignumBench::generateVector<Vector>(generator, distribution);
            sample.second = SignumBench::generateVector<Vector>(generator, distribution);
            sample.scalar = scale_distribution(generator);
            // Make normalization inputs nonzero without excluding negative components.
            if (sample.first.x >= static_cast<Scalar>(0.0)) {
                sample.first.x += kMinimumScale;
            } else {
                sample.first.x -= kMinimumScale;
            }
        }
        return samples;
    }

    template <typename Vector, VectorOperation Operation>
    void BM_VectorOperation(benchmark::State &state) {
        auto samples = createVectorSamples<Vector>();
        SignumBench::run(state, samples, [](auto &sample) {
            if constexpr (Operation == VectorOperation::Add) {
                return Math::add(sample.first, sample.second);
            } else if constexpr (Operation == VectorOperation::Subtract) {
                return Math::subtract(sample.first, sample.second);
            } else if constexpr (Operation == VectorOperation::Scale) {
                return Math::scale(sample.first, sample.scalar);
            } else if constexpr (Operation == VectorOperation::Dot) {
                return Math::dot(sample.first, sample.second);
            } else if constexpr (Operation == VectorOperation::Cross) {
                return Math::cross(sample.first, sample.second);
            } else if constexpr (Operation == VectorOperation::LengthSquared) {
                return Math::lengthSquared(sample.first);
            } else if constexpr (Operation == VectorOperation::Length) {
                return Math::length(sample.first);
            } else {
                return Math::normalize(sample.first);
            }
        });
    }
} // namespace

BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2f, VectorOperation::Add)
    ->Name("BM_Vector2f_Add");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2f, VectorOperation::Subtract)
    ->Name("BM_Vector2f_Subtract");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2f, VectorOperation::Scale)
    ->Name("BM_Vector2f_Scale");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2f, VectorOperation::Dot)
    ->Name("BM_Vector2f_Dot");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2f, VectorOperation::LengthSquared)
    ->Name("BM_Vector2f_LengthSquared");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2f, VectorOperation::Length)
    ->Name("BM_Vector2f_Length");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2f, VectorOperation::Normalize)
    ->Name("BM_Vector2f_Normalize");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3f, VectorOperation::Add)
    ->Name("BM_Vector3f_Add");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3f, VectorOperation::Subtract)
    ->Name("BM_Vector3f_Subtract");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3f, VectorOperation::Scale)
    ->Name("BM_Vector3f_Scale");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3f, VectorOperation::Dot)
    ->Name("BM_Vector3f_Dot");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3f, VectorOperation::LengthSquared)
    ->Name("BM_Vector3f_LengthSquared");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3f, VectorOperation::Length)
    ->Name("BM_Vector3f_Length");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3f, VectorOperation::Normalize)
    ->Name("BM_Vector3f_Normalize");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3f, VectorOperation::Cross)
    ->Name("BM_Vector3f_Cross");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4f, VectorOperation::Add)
    ->Name("BM_Vector4f_Add");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4f, VectorOperation::Subtract)
    ->Name("BM_Vector4f_Subtract");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4f, VectorOperation::Scale)
    ->Name("BM_Vector4f_Scale");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4f, VectorOperation::Dot)
    ->Name("BM_Vector4f_Dot");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4f, VectorOperation::LengthSquared)
    ->Name("BM_Vector4f_LengthSquared");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4f, VectorOperation::Length)
    ->Name("BM_Vector4f_Length");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4f, VectorOperation::Normalize)
    ->Name("BM_Vector4f_Normalize");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2d, VectorOperation::Add)
    ->Name("BM_Vector2d_Add");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2d, VectorOperation::Subtract)
    ->Name("BM_Vector2d_Subtract");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2d, VectorOperation::Scale)
    ->Name("BM_Vector2d_Scale");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2d, VectorOperation::Dot)
    ->Name("BM_Vector2d_Dot");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2d, VectorOperation::LengthSquared)
    ->Name("BM_Vector2d_LengthSquared");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2d, VectorOperation::Length)
    ->Name("BM_Vector2d_Length");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector2d, VectorOperation::Normalize)
    ->Name("BM_Vector2d_Normalize");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3d, VectorOperation::Add)
    ->Name("BM_Vector3d_Add");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3d, VectorOperation::Subtract)
    ->Name("BM_Vector3d_Subtract");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3d, VectorOperation::Scale)
    ->Name("BM_Vector3d_Scale");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3d, VectorOperation::Dot)
    ->Name("BM_Vector3d_Dot");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3d, VectorOperation::LengthSquared)
    ->Name("BM_Vector3d_LengthSquared");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3d, VectorOperation::Length)
    ->Name("BM_Vector3d_Length");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3d, VectorOperation::Normalize)
    ->Name("BM_Vector3d_Normalize");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector3d, VectorOperation::Cross)
    ->Name("BM_Vector3d_Cross");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4d, VectorOperation::Add)
    ->Name("BM_Vector4d_Add");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4d, VectorOperation::Subtract)
    ->Name("BM_Vector4d_Subtract");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4d, VectorOperation::Scale)
    ->Name("BM_Vector4d_Scale");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4d, VectorOperation::Dot)
    ->Name("BM_Vector4d_Dot");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4d, VectorOperation::LengthSquared)
    ->Name("BM_Vector4d_LengthSquared");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4d, VectorOperation::Length)
    ->Name("BM_Vector4d_Length");
BENCHMARK_TEMPLATE(BM_VectorOperation, Math::Vector4d, VectorOperation::Normalize)
    ->Name("BM_Vector4d_Normalize");
