#include "benchmark_data.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/matrix4d.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/core/math/vector3d.hpp"
#include <array>
#include <benchmark/benchmark.h>
#include <random>
#include <type_traits>

namespace Math = SNE::Engine::Core::Math;

namespace {
    enum class MatrixOperation {
        Multiply,
        TransformPoint,
        TransformDirection,
        Translate,
        Scale,
        RotateX,
        RotateY,
        RotateZ,
        Transpose,
        Inverse,
        ComposeTRS,
    };

    template <typename Matrix>
    using VectorFor = std::conditional_t<std::is_same_v<Matrix, Math::Matrix4f>,
                                         Math::Vector3f, Math::Vector3d>;

    template <typename Matrix>
    struct MatrixSample {
        using Vector = VectorFor<Matrix>;
        using Scalar = SignumBench::Component<Vector>;
        Matrix first{};
        Matrix second{};
        Vector translation{};
        Vector rotation{};
        Vector scale_factors{};
        Vector point{};
        Scalar angle{};
    };

    template <typename Matrix>
    [[nodiscard]] auto createMatrixSamples()
        -> std::array<MatrixSample<Matrix>, SignumBench::kSampleCount> {
        using Vector = VectorFor<Matrix>;
        using Scalar = SignumBench::Component<Vector>;
        constexpr Scalar kMinimumPosition = static_cast<Scalar>(-10.0);
        constexpr Scalar kMaximumPosition = static_cast<Scalar>(10.0);
        constexpr Scalar kMinimumRotation = static_cast<Scalar>(-1.0);
        constexpr Scalar kMaximumRotation = static_cast<Scalar>(1.0);
        constexpr Scalar kMinimumScale = static_cast<Scalar>(0.5);
        constexpr Scalar kMaximumScale = static_cast<Scalar>(2.0);

        // NOLINTBEGIN(bugprone-random-generator-seed)
        std::mt19937 generator{SignumBench::kRandomSeed};
        // NOLINTEND(bugprone-random-generator-seed)

        auto positions = SignumBench::makeDistribution(kMinimumPosition, kMaximumPosition);
        auto rotations = SignumBench::makeDistribution(kMinimumRotation, kMaximumRotation);
        auto scales = SignumBench::makeDistribution(kMinimumScale, kMaximumScale);
        std::array<MatrixSample<Matrix>, SignumBench::kSampleCount> samples{};

        for (auto &sample : samples) {
            sample.translation = SignumBench::generateVector<Vector>(generator, positions);
            sample.rotation = SignumBench::generateVector<Vector>(generator, rotations);
            sample.scale_factors = SignumBench::generateVector<Vector>(generator, scales);
            sample.point = SignumBench::generateVector<Vector>(generator, positions);
            sample.angle = rotations(generator);
            sample.first = Math::composeTRS(sample.translation, sample.rotation,
                                            sample.scale_factors);

            const Vector second_translation =
                SignumBench::generateVector<Vector>(generator, positions);
            const Vector second_rotation =
                SignumBench::generateVector<Vector>(generator, rotations);
            const Vector second_scale =
                SignumBench::generateVector<Vector>(generator, scales);
            sample.second = Math::composeTRS(second_translation, second_rotation,
                                             second_scale);
        }
        return samples;
    }

    template <typename Matrix, MatrixOperation Operation>
    void BM_MatrixOperation(benchmark::State &state) {
        auto samples = createMatrixSamples<Matrix>();
        SignumBench::run(state, samples, [](auto &sample) {
            if constexpr (Operation == MatrixOperation::Multiply) {
                return Math::multiply(sample.first, sample.second);
            } else if constexpr (Operation == MatrixOperation::TransformPoint) {
                return Math::transformPoint(sample.first, sample.point);
            } else if constexpr (Operation == MatrixOperation::TransformDirection) {
                return Math::transformDirection(sample.first, sample.point);
            } else if constexpr (Operation == MatrixOperation::Translate) {
                return Math::translate(sample.translation.x, sample.translation.y,
                                       sample.translation.z);
            } else if constexpr (Operation == MatrixOperation::Scale) {
                return Math::scale(sample.scale_factors.x, sample.scale_factors.y,
                                   sample.scale_factors.z);
            } else if constexpr (Operation == MatrixOperation::RotateX) {
                return Math::rotateX(sample.angle);
            } else if constexpr (Operation == MatrixOperation::RotateY) {
                return Math::rotateY(sample.angle);
            } else if constexpr (Operation == MatrixOperation::RotateZ) {
                return Math::rotateZ(sample.angle);
            } else if constexpr (Operation == MatrixOperation::Transpose) {
                return Math::transpose(sample.first);
            } else if constexpr (Operation == MatrixOperation::Inverse) {
                return Math::tryInverse(sample.first);
            } else {
                return Math::composeTRS(sample.translation, sample.rotation,
                                        sample.scale_factors);
            }
        });
    }
} // namespace

BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::Multiply)
    ->Name("BM_Matrix4fTRS_Multiply");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::TransformDirection)
    ->Name("BM_Matrix4fTRS_TransformDirection");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::Translate)
    ->Name("BM_Matrix4fTRS_Translate");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::Scale)
    ->Name("BM_Matrix4fTRS_Scale");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::RotateX)
    ->Name("BM_Matrix4fTRS_RotateX");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::RotateY)
    ->Name("BM_Matrix4fTRS_RotateY");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::RotateZ)
    ->Name("BM_Matrix4fTRS_RotateZ");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::Transpose)
    ->Name("BM_Matrix4fTRS_Transpose");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::Inverse)
    ->Name("BM_Matrix4fTRS_Inverse");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4f, MatrixOperation::ComposeTRS)
    ->Name("BM_Matrix4fTRS_ComposeTRS");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::Multiply)
    ->Name("BM_Matrix4dTRS_Multiply");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::TransformDirection)
    ->Name("BM_Matrix4dTRS_TransformDirection");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::Translate)
    ->Name("BM_Matrix4dTRS_Translate");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::Scale)
    ->Name("BM_Matrix4dTRS_Scale");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::RotateX)
    ->Name("BM_Matrix4dTRS_RotateX");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::RotateY)
    ->Name("BM_Matrix4dTRS_RotateY");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::RotateZ)
    ->Name("BM_Matrix4dTRS_RotateZ");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::Transpose)
    ->Name("BM_Matrix4dTRS_Transpose");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::Inverse)
    ->Name("BM_Matrix4dTRS_Inverse");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::ComposeTRS)
    ->Name("BM_Matrix4dTRS_ComposeTRS");
BENCHMARK_TEMPLATE(BM_MatrixOperation, Math::Matrix4d, MatrixOperation::TransformPoint)
    ->Name("BM_Matrix4dTRS_TransformPoint");
