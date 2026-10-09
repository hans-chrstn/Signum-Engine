#include "matrix4.hpp"
#include <cstddef>

namespace SNE::Engine::Core::Math {
    auto multiply(const Matrix4f &first, const Matrix4f &second) -> Matrix4f {
        Matrix4f result{};
        for (std::size_t column{}; column < kMatrix4Dimension; ++column) {
            for (std::size_t row{}; row < kMatrix4Dimension; ++row) {
                float sum = 0.0F;
                for (std::size_t k{}; k < kMatrix4Dimension; ++k) {
                    const float first_element =
                        first.elements[(k * kMatrix4Dimension) + row];
                    const float second_element =
                        second.elements[(column * kMatrix4Dimension) + k];

                    sum += first_element * second_element;
                }
                // Store the completed row-by-column product using column-major
                // indexing.
                result.elements[(column * kMatrix4Dimension) + row] = sum;
            }
        }

        return result;
    }
} // namespace SNE::Engine::Core::Math
