#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/error/diagnostic_writer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <numbers>
#include <span>
#include <utility>

namespace Renderer = SNE::Engine::Renderer;
namespace Core = SNE::Engine::Core;
namespace Math = SNE::Engine::Core::Math;

auto main() -> int {
    try {
        struct QuadVertex {
            std::array<float, 2> position;
            std::array<float, 3> color;
        };

        constexpr std::array<QuadVertex, 4> kQuadVertices{
            {
                {
                    .position = {-0.5F, -0.5F},
                    .color = {1.0F, 0.0F, 0.0F},
                },
                {
                    .position = {0.5F, -0.5F},
                    .color = {0.0F, 1.0F, 0.0F},
                },
                {
                    .position = {-0.5F, 0.5F},
                    .color = {0.0F, 0.0F, 1.0F},
                },
                {
                    .position = {0.5F, 0.5F},
                    .color = {0.0F, 0.0F, 1.0F},
                },
            },
        };

        constexpr std::array<Renderer::VertexAttributeDescription, 2>
            kQuadVertexAttributes{
                Renderer::VertexAttributeDescription{
                    .location = 0U,
                    .format = Renderer::VertexFormat::Float32x2,
                    .offset = static_cast<std::uint32_t>(
                        offsetof(QuadVertex, position)),
                },
                Renderer::VertexAttributeDescription{
                    .location = 1U,
                    .format = Renderer::VertexFormat::Float32x3,
                    .offset =
                        static_cast<std::uint32_t>(offsetof(QuadVertex, color)),
                },
        };

        constexpr std::array<std::uint16_t, 6> kQuadIndices{
            0, 1, 2, 2, 1, 3,
        };

        const Renderer::MeshUploadData mesh{
            .vertex_bytes = std::as_bytes(std::span{kQuadVertices}),
            .vertex_stride = static_cast<std::uint32_t>(sizeof(QuadVertex)),
            .indices = std::span{kQuadIndices},
        };

        const Renderer::VertexLayoutDescription layout{
            .attributes = kQuadVertexAttributes,
        };

        constexpr float kQuadScale = 0.5F;
        constexpr float kQuadHorizontalOffset = 0.5F;
        constexpr float kQuadRotationDegrees = 45.0F;

        constexpr float kQuadRotationRadians =
            kQuadRotationDegrees * std::numbers::pi_v<float> / 180.0F;

        const Math::Vector3f scale{
            .x = kQuadScale,
            .y = kQuadScale,
            .z = 1.0F,
        };

        const Math::Matrix4f model_a = Math::composeTRS(
            Math::Vector3f{.x = -kQuadHorizontalOffset, .y = 0.0F, .z = 0.0F},
            Math::Vector3f{.x = 0.0F, .y = 0.0F, .z = kQuadRotationRadians},
            scale);

        const Math::Matrix4f model_b = Math::composeTRS(
            Math::Vector3f{.x = kQuadHorizontalOffset, .y = 0.0F, .z = 0.0F},
            Math::Vector3f{}, scale);

        const std::array<Math::Matrix4f, 2> model_matrices{
            model_a,
            model_b,
        };

        const Renderer::InitialMeshData initial_mesh{
            .mesh = mesh,
            .layout = layout,
            .model_matrices = model_matrices,
        };

        Core::ApplicationConfiguration configuration{};
        configuration.initial_mesh = initial_mesh;

        Core::Application application{std::move(configuration)};
        application.run();
    } catch (const Core::Error::EngineError &engine_error) {
        Core::Error::writeDiagnostic(std::cerr, engine_error);
        return EXIT_FAILURE;
    } catch (const std::exception &error) {
        std::cerr << "Unexpected standard exception: " << error.what() << '\n';
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown exception reached application boundary" << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
