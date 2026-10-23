#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/error/diagnostic_writer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/platform/input/cursor_position.hpp"
#include "engine/platform/input/mouse_button.hpp"
#include "engine/renderer/camera/initial_camera_data.hpp"
#include "engine/renderer/geometry/initial_mesh_data.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/uv_sphere_mesh.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/geometry/vertex_layout_description.hpp"
#include "engine/renderer/lighting/directional_light.hpp"
#include "engine/renderer/lighting/directional_light_uniform_data.hpp"
#include "engine/renderer/material/material_buffer_binding_input.hpp"
#include "engine/renderer/material/material_texture_binding_input.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/texture/image_loader.hpp"
#include "engine/renderer/texture/texture_format.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
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
namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;
namespace Math = SNE::Engine::Core::Math;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Platform = SNE::Engine::Platform;

auto main() -> int {
    try {
        constexpr float kSphereRadius{0.75F};
        constexpr std::uint32_t kSphereLongitudeSegments{64U};
        constexpr std::uint32_t kSphereLatitudeSegments{32U};

        constexpr Renderer::UvSphereParameters sphere_param{
            .radius = kSphereRadius,
            .longitude_segments = kSphereLongitudeSegments,
            .latitude_segments = kSphereLatitudeSegments,
        };

        const Renderer::UvSphereMesh sphere =
            Renderer::generateUvSphereMesh(sphere_param);

        constexpr std::array<Renderer::VertexAttributeDescription, 3>
            kSphereVertexAttributes{
                {
                    {
                        .location = 0U,
                        .format = Renderer::VertexFormat::Float32x3,
                        .offset = static_cast<std::uint32_t>(
                            offsetof(Renderer::UvSphereVertex, position)),
                    },
                    {
                        .location = 1U,
                        .format = Renderer::VertexFormat::Float32x3,
                        .offset = static_cast<std::uint32_t>(
                            offsetof(Renderer::UvSphereVertex, normal)),
                    },
                    {
                        .location = 2U,
                        .format = Renderer::VertexFormat::Float32x2,
                        .offset = static_cast<std::uint32_t>(
                            offsetof(Renderer::UvSphereVertex, uv)),
                    },
                },
        };

        const Renderer::MeshUploadData mesh{
            .vertex_bytes = std::as_bytes(std::span{sphere.vertices}),
            .vertex_stride =
                static_cast<std::uint32_t>(sizeof(Renderer::UvSphereVertex)),
            .indices = sphere.indices,
        };

        const Renderer::VertexLayoutDescription layout{
            .attributes = kSphereVertexAttributes,
        };

        constexpr float kCameraDistance = 3.0F;
        constexpr float kCameraFovRadians = std::numbers::pi_v<float> / 3.0F;
        constexpr float kCameraNearPlane = 0.1F;
        constexpr float kCameraFarPlane = 100.0F;

        constexpr float kDegreesToRadians = std::numbers::pi_v<float> / 180.0F;
        const float kRotateX = 20.0F;
        const float kRotateY = -30.0F;
        const float kRotateZ = 0.0F;

        const Math::Vector3f translation{
            .x = 0.0F,
            .y = 0.0F,
            .z = 0.0F,
        };

        Math::Vector3f rotation{
            .x = kRotateX * kDegreesToRadians,
            .y = kRotateY * kDegreesToRadians,
            .z = kRotateZ * kDegreesToRadians,
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

        const Renderer::Rgba8Image day_image =
            Renderer::loadRgba8Image("playground/assets/earth_day.jpg");

        const Renderer::Rgba8Image night_image =
            Renderer::loadRgba8Image("playground/assets/earth_night.jpg");

        std::array<Renderer::MaterialTextureBindingInput, 2>
            initial_texture_bindings{
                Renderer::MaterialTextureBindingInput{
                    .set = 0U,
                    .binding = 0U,
                    .array_element = 0U,
                    .texture =
                        {
                            .width = day_image.width,
                            .height = day_image.height,
                            .pixels = day_image.pixels,
                            .format = Renderer::TextureFormat::Rgba8Srgb,
                            .mip_levels = 1U,
                        },
                },
                Renderer::MaterialTextureBindingInput{
                    .set = 0U,
                    .binding = 2U,
                    .array_element = 0U,
                    .texture =
                        {
                            .width = night_image.width,
                            .height = night_image.height,
                            .pixels = night_image.pixels,
                            .format = Renderer::TextureFormat::Rgba8Srgb,
                            .mip_levels = 1U,
                        },
                },
        };

        auto vertex_data = Vulkan::loadSpirv("build/shaders/textured.vert.spv");
        auto fragment_data = Vulkan::loadSpirv("build/shaders/earth.frag.spv");

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

        constexpr float kLightDirectionX = 3.0F;
        constexpr float kLightDirectionY = 1.5F;
        constexpr float kLightDirectionZ = 1.0F;

        Renderer::DirectionalLight directional_light{
            .direction_to_light =
                {
                    .x = kLightDirectionX,
                    .y = kLightDirectionY,
                    .z = kLightDirectionZ,
                },
            .color =
                {
                    .x = 1.0F,
                    .y = 1.0F,
                    .z = 1.0F,
                },
            .intensity = 1.0F,
        };

        Renderer::DirectionalLightUniformData uniform_data =
            Renderer::makeDirectionalLightUniformData(directional_light);

        std::array<Renderer::MaterialBufferBindingInput, 1>
            initial_buffer_bindings{
                Renderer::MaterialBufferBindingInput{
                    .set = 0U,
                    .binding = 1U,
                    .array_element = 0U,
                    .type = SNE::Engine::Renderer::Shader::ShaderResourceType::
                        UniformBuffer,
                    .data = std::as_bytes(std::span{&uniform_data, 1}),
                },
        };

        Core::ApplicationConfiguration configuration{};
        configuration.initial_mesh = initial_mesh;
        configuration.initial_camera = initial_camera;
        configuration.initial_graphics_shaders = std::move(graphics_data);
        configuration.initial_buffer_bindings = initial_buffer_bindings;
        configuration.initial_texture_bindings = initial_texture_bindings;

        Core::Application application{std::move(configuration)};

        Platform::CursorPosition previous_cursor{};
        bool was_dragging{};

        constexpr float kRotationSensitivity{0.005F};

        bool was_right_pressed{};
        bool light_enabled{true};

        application.run([&](Core::Application &app) -> void {
            const Platform::CursorPosition current_cursor =
                app.window().cursorPosition();

            const bool is_dragging =
                app.window().isMouseButtonPressed(Platform::MouseButton::Left);

            const bool is_right_pressed =
                app.window().isMouseButtonPressed(Platform::MouseButton::Right);

            if (is_right_pressed && !was_right_pressed) {
                light_enabled = !light_enabled;

                directional_light.intensity = light_enabled ? 1.0F : 0.0F;

                uniform_data = Renderer::makeDirectionalLightUniformData(
                    directional_light);

                app.updateMaterialBuffer({
                    .set = 0U,
                    .binding = 1U,
                    .array_element = 0U,
                    .data = std::as_bytes(std::span{&uniform_data, 1}),
                });
            }

            if (is_dragging && !was_dragging) {
                previous_cursor = current_cursor;
            }

            if (is_dragging && was_dragging) {
                const double delta_x = current_cursor.x - previous_cursor.x;
                const double delta_y = current_cursor.y - previous_cursor.y;

                rotation.y +=
                    static_cast<float>(delta_x) * kRotationSensitivity;

                rotation.x +=
                    static_cast<float>(delta_y) * kRotationSensitivity;

                const Math::Matrix4f updated_model_matrix =
                    Math::composeTRS(translation, rotation, scale);

                app.setDrawInstanceModelMatrix(0U, updated_model_matrix);
            }

            if (is_dragging) {
                previous_cursor = current_cursor;
            }

            was_dragging = is_dragging;
            was_right_pressed = is_right_pressed;
        });
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
