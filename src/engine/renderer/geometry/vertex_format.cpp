#include "vertex_format.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"

namespace SNE::Engine::Renderer {
    auto vertexFormatSizeInBytes(VertexFormat vertex_format) -> std::uint32_t {
        constexpr std::uint32_t float32x2_byte_size = 8U;
        constexpr std::uint32_t float32x3_byte_size = 12U;
        switch (vertex_format) {
        case VertexFormat::Float32x2:
            return float32x2_byte_size;
        case VertexFormat::Float32x3:
            return float32x3_byte_size;
        }

        Core::Assertion::failAssertion(
            Core::Assertion::AssertionType::Precondition,
            Core::Error::Subsystem::Renderer,
            "Vertex format size query requires a supported VertexFormat value");
    }
} // namespace SNE::Engine::Renderer
