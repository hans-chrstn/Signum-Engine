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
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/texture/texture_upload_data.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <numbers>
#include <span>
#include <utility>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;
namespace Math = SNE::Engine::Core::Math;
namespace Shader = SNE::Engine::Renderer::Shader;

auto main() -> int {
    try {
        struct CubeVertex {
            std::array<float, 3> position;
            std::array<float, 3> color;
            std::array<float, 2> uv;
        };

        constexpr std::array<float, 3> kWhiteColor{
            1.0F,
            1.0F,
            1.0F,
        };

        constexpr std::array<CubeVertex, 24> kCubeVertices{
            {
                // Front (+Z)
                {
                    .position = {-0.5F, -0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 0.0F},
                },
                {
                    .position = {0.5F, -0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 0.0F},
                },
                {
                    .position = {-0.5F, 0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 1.0F},
                },
                {
                    .position = {0.5F, 0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 1.0F},
                },

                // Back (-Z)
                {
                    .position = {0.5F, -0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 0.0F},
                },
                {
                    .position = {-0.5F, -0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 0.0F},
                },
                {
                    .position = {0.5F, 0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 1.0F},
                },
                {
                    .position = {-0.5F, 0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 1.0F},
                },

                // Right (+X)
                {
                    .position = {0.5F, -0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 0.0F},
                },
                {
                    .position = {0.5F, -0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 0.0F},
                },
                {
                    .position = {0.5F, 0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 1.0F},
                },
                {
                    .position = {0.5F, 0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 1.0F},
                },

                // Left (-X)
                {
                    .position = {-0.5F, -0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 0.0F},
                },
                {
                    .position = {-0.5F, -0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 0.0F},
                },
                {
                    .position = {-0.5F, 0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 1.0F},
                },
                {
                    .position = {-0.5F, 0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 1.0F},
                },

                // Top (+Y)
                {
                    .position = {-0.5F, 0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 0.0F},
                },
                {
                    .position = {0.5F, 0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 0.0F},
                },
                {
                    .position = {-0.5F, 0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 1.0F},
                },
                {
                    .position = {0.5F, 0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 1.0F},
                },

                // Bottom (-Y)
                {
                    .position = {-0.5F, -0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 0.0F},
                },
                {
                    .position = {0.5F, -0.5F, -0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 0.0F},
                },
                {
                    .position = {-0.5F, -0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {0.0F, 1.0F},
                },
                {
                    .position = {0.5F, -0.5F, 0.5F},
                    .color = kWhiteColor,
                    .uv = {1.0F, 1.0F},
                },
            },
        };

        constexpr std::array<Renderer::VertexAttributeDescription, 3>
            kCubeVertexAttributes{
                {
                    {
                        .location = 0U,
                        .format = Renderer::VertexFormat::Float32x3,
                        .offset = static_cast<std::uint32_t>(
                            offsetof(CubeVertex, position)),
                    },
                    {
                        .location = 1U,
                        .format = Renderer::VertexFormat::Float32x3,
                        .offset = static_cast<std::uint32_t>(
                            offsetof(CubeVertex, color)),
                    },
                    {
                        .location = 2U,
                        .format = Renderer::VertexFormat::Float32x2,
                        .offset = static_cast<std::uint32_t>(
                            offsetof(CubeVertex, uv)),
                    },
                },
        };

        // clang-format off
        constexpr std::array<std::uint16_t, 36> kCubeIndices{
            0,  1,  2,   2,  1,  3, // Front
            4,  5,  6,   6,  5,  7, // Back
            8,  9, 10,  10,  9, 11, // Right
            12, 13, 14,  14, 13, 15, // Left
            16, 17, 18,  18, 17, 19, // Top
            20, 21, 22,  22, 21, 23, // Bottom
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

        constexpr float kDegreesToRadians = std::numbers::pi_v<float> / 180.0F;

        const Math::Vector3f translation{
            .x = 0.0F,
            .y = 0.0F,
            .z = 0.0F,
        };

        const Math::Vector3f rotation{
            .x = 20.0F * kDegreesToRadians,
            .y = -30.0F * kDegreesToRadians,
            .z = 0.0F,
        };

        const Math::Vector3f scale{
            .x = 1.0F,
            .y = 1.0F,
            .z = 1.0F,
        };

        const Math::Matrix4f model_matrix =
            Math::composeTRS(translation, rotation, scale);

        const std::array<Math::Matrix4f, 1> model_matrices{
            model_matrix,
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

        constexpr std::size_t kCheckerboardWidth = 8U;
        constexpr std::size_t kCheckerboardHeight = 8U;
        constexpr std::size_t kRgba8BytesPerPixel = 4U;

        constexpr std::size_t kCheckerboardByteCount =
            kCheckerboardWidth * kCheckerboardHeight * kRgba8BytesPerPixel;

        constexpr auto kCheckerboardPixels =
            []() -> std::array<std::byte, kCheckerboardByteCount> {
            constexpr std::byte kMaximumChannelValue{
                std::numeric_limits<std::uint8_t>::max(),
            };

            constexpr std::byte kMinimumChannelValue{};

            std::array<std::byte, kCheckerboardByteCount> pixels{};

            for (std::size_t row_index{}; row_index < kCheckerboardHeight;
                 ++row_index) {
                for (std::size_t column_index{};
                     column_index < kCheckerboardWidth; ++column_index) {
                    const bool is_white =
                        ((row_index + column_index) % 2U) == 0U;

                    const std::byte color_channel =
                        is_white ? kMaximumChannelValue : kMinimumChannelValue;

                    const std::size_t pixel_byte_offset =
                        ((row_index * kCheckerboardWidth) + column_index) *
                        kRgba8BytesPerPixel;

                    pixels[pixel_byte_offset + 0U] = color_channel;
                    pixels[pixel_byte_offset + 1U] = color_channel;
                    pixels[pixel_byte_offset + 2U] = color_channel;
                    pixels[pixel_byte_offset + 3U] = kMaximumChannelValue;
                }
            }

            return pixels;
        }();

        const Renderer::TextureUploadData initial_texture{
            .width = static_cast<std::uint32_t>(kCheckerboardWidth),
            .height = static_cast<std::uint32_t>(kCheckerboardHeight),
            .pixels = std::span{kCheckerboardPixels},
        };

        auto vertex_data = Vulkan::loadSpirv("build/shaders/triangle.vert.spv");
        auto fragment_data =
            Vulkan::loadSpirv("build/shaders/triangle.frag.spv");

        Shader::GraphicsShaderData graphics_data{
            .vertex =
                {
                    .artifact = std::move(vertex_data),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Vertex,
                            .entry_point = "main",
                        },
                },
            .fragment =
                {
                    .artifact = std::move(fragment_data),
                    .selection =
                        {
                            .stage = Shader::ShaderStage::Fragment,
                            .entry_point = "main",
                        },
                },
        };

        Core::ApplicationConfiguration configuration{};
        configuration.initial_mesh = initial_mesh;
        configuration.initial_texture = initial_texture;
        configuration.initial_camera = initial_camera;
        configuration.initial_graphics_shaders = std::move(graphics_data);

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
