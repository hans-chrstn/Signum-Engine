#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/error/diagnostic_writer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/renderer/camera/initial_camera_data.hpp"
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

namespace {
    [[nodiscard]] constexpr auto toRadians(float degrees) -> float {
        const float half_rotation = 180.0F;
        return degrees * (std::numbers::pi_v<float> / half_rotation);
    }
} // namespace

auto main() -> int {
    try {
        struct CubeVertex {
            std::array<float, 3> position;
            std::array<float, 3> color;
        };

        constexpr std::array<CubeVertex, 8> kCubeVertices{
            {
                {
                    .position = {-0.5F, -0.5F, -0.5F},
                    .color = {1.0F, 0.0F, 0.0F}, // Red
                },
                {
                    .position = {0.5F, -0.5F, -0.5F},
                    .color = {0.0F, 1.0F, 0.0F}, // Green
                },
                {
                    .position = {-0.5F, -0.5F, 0.5F},
                    .color = {0.0F, 0.0F, 1.0F}, // Blue
                },
                {
                    .position = {0.5F, -0.5F, 0.5F},
                    .color = {1.0F, 1.0F, 0.0F}, // Yellow
                },
                {
                    .position = {-0.5F, 0.5F, -0.5F},
                    .color = {1.0F, 0.0F, 1.0F}, // Magenta
                },
                {
                    .position = {0.5F, 0.5F, -0.5F},
                    .color = {0.0F, 1.0F, 1.0F}, // Cyan
                },
                {
                    .position = {-0.5F, 0.5F, 0.5F},
                    .color = {1.0F, 0.5F, 0.0F}, // Orange
                },
                {
                    .position = {0.5F, 0.5F, 0.5F},
                    .color = {1.0F, 1.0F, 1.0F}, // White
                },
            },
        };

        constexpr std::array<Renderer::VertexAttributeDescription, 2>
            kCubeVertexAttributes{
                Renderer::VertexAttributeDescription{
                    .location = 0U,
                    .format = Renderer::VertexFormat::Float32x3,
                    .offset = static_cast<std::uint32_t>(
                        offsetof(CubeVertex, position)),
                },
                Renderer::VertexAttributeDescription{
                    .location = 1U,
                    .format = Renderer::VertexFormat::Float32x3,
                    .offset =
                        static_cast<std::uint32_t>(offsetof(CubeVertex, color)),
                },
        };

        // clang-format off
        constexpr std::array<std::uint16_t, 36> kCubeIndices{
            // front face
            2, 3, 6,
            6, 3, 7,
            // top face
            4, 6, 7,
            7, 5, 4,
            // left face
            4, 0, 2,
            2, 6, 4,
            // back face
            5, 1, 0,
            0, 4, 5,
            // right face
            7, 3, 1,
            1, 5, 7,
            // bottom face
            1, 3, 2,
            2, 0, 1,
        };
        // clang-format on

        const Renderer::MeshUploadData mesh{
            .vertex_bytes = std::as_bytes(std::span{kCubeVertices}),
            .vertex_stride = static_cast<std::uint32_t>(sizeof(CubeVertex)),
            .indices = std::span{kCubeIndices},
        };

        const Renderer::VertexLayoutDescription layout{
            .attributes = kCubeVertexAttributes,
        };

        constexpr float kCameraDistance = 3.0F;
        constexpr float kCameraFovRadians = std::numbers::pi_v<float> / 3.0F;
        constexpr float kCameraNearPlane = 0.1F;
        constexpr float kCameraFarPlane = 100.0F;
        constexpr float kCubeRotationXDegrees = -35.0F;
        constexpr float kCubeRotationYDegrees = 45.0F;

        const Math::Vector3f translate{
            .x = 0.0F,
            .y = 0.0F,
            .z = 0.0F,
        };

        const Math::Vector3f rotation{
            .x = toRadians(kCubeRotationXDegrees),
            .y = toRadians(kCubeRotationYDegrees),
            .z = 0.0F,
        };

        const Math::Vector3f scale{
            .x = 1.0F,
            .y = 1.0F,
            .z = 1.0F,
        };

        const Math::Matrix4f matrix =
            Math::composeTRS(translate, rotation, scale);

        const std::array<Math::Matrix4f, 1> model_matrices{
            matrix,
        };

        const Renderer::InitialMeshData initial_mesh{
            .mesh = mesh,
            .layout = layout,
            .model_matrices = model_matrices,
        };

        const Renderer::InitialCameraData initial_camera{
            .view = Math::translate(0.0F, 0.0F, -kCameraDistance),
            .fov_y_radians = kCameraFovRadians,
            .near_plane = kCameraNearPlane,
            .far_plane = kCameraFarPlane,
        };

        Core::ApplicationConfiguration configuration{};
        configuration.initial_mesh = initial_mesh;
        configuration.initial_camera = initial_camera;

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
