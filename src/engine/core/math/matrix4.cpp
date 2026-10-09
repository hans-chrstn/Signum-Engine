#include "matrix4.hpp"
#include <cstddef>

namespace SNE::Engine::Core::Math {
    auto multiply(const Matrix4f &first, const Matrix4f &second) -> Matrix4f {
        Matrix4f result{};
        for (std::size_t column{}; column < kMatrix4Dimension; ++column) {
            for (std::size_t row{}; row < kMatrix4Dimension; ++row) {
                float sum = 0.0F;
                for (std::size_t k{}; k < kMatrix4Dimension; ++k) {
                    sum += first.at(row, k) * second.at(k, column);
                }
                result.at(row, column) = sum;
            }
        }

        return result;
    }
} // namespace SNE::Engine::Core::Math
