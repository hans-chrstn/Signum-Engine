#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/error/diagnostic_writer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/core/math/vector4.hpp"
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
#include "engine/renderer/material/material_buffer_update_frequency.hpp"
#include "engine/renderer/material/material_texture_binding_input.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/texture/image_loader.hpp"
#include "engine/renderer/texture/texture_format.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include <array>
#include <chrono>
#include <cmath>
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

        constexpr std::array<Renderer::VertexAttributeDescription, 4>
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
                    {
                        .location = 3U,
                        .format = Renderer::VertexFormat::Float32x4,
                        .offset = static_cast<std::uint32_t>(
                            offsetof(Renderer::UvSphereVertex, tangent)),
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
        constexpr float kRotateX = 20.0F;
        constexpr float kRotateY = -30.0F;
        constexpr float kRotateZ = 0.0F;

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

        const Renderer::Rgba8Image cloud_image =
            Renderer::loadRgba8Image("playground/assets/earth_clouds.jpg");

        const Renderer::Rgba8Image specular_image = Renderer::loadRgba8Image(
            "playground/assets/earth_specular_map.png");

        const Renderer::Rgba8Image normal_image =
            Renderer::loadRgba8Image("playground/assets/earth_normal_map.png");

        constexpr std::size_t kInitialTextureBindingsSize{5U};
        constexpr std::uint32_t kMaterialDescriptorSet{0U};
        constexpr std::uint32_t kDayTextureBinding{0U};
        constexpr std::uint32_t kMaterialArrayElement{0U};
        constexpr std::uint32_t kNightTextureBinding{2U};
        constexpr std::uint32_t kCloudTextureBinding{3U};
        constexpr std::uint32_t kSpecularTextureBinding{5U};
        constexpr std::uint32_t kNormalTextureBinding{7U};
        std::array<Renderer::MaterialTextureBindingInput,
                   kInitialTextureBindingsSize>
            initial_texture_bindings{
                Renderer::MaterialTextureBindingInput{
                    .set = kMaterialDescriptorSet,
                    .binding = kDayTextureBinding,
                    .array_element = kMaterialArrayElement,
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
                    .set = kMaterialDescriptorSet,
                    .binding = kNightTextureBinding,
                    .array_element = kMaterialArrayElement,
                    .texture =
                        {
                            .width = night_image.width,
                            .height = night_image.height,
                            .pixels = night_image.pixels,
                            .format = Renderer::TextureFormat::Rgba8Srgb,
                            .mip_levels = 1U,
                        },
                },
                Renderer::MaterialTextureBindingInput{
                    .set = kMaterialDescriptorSet,
                    .binding = kCloudTextureBinding,
                    .array_element = kMaterialArrayElement,
                    .texture =
                        {
                            .width = cloud_image.width,
                            .height = cloud_image.height,
                            .pixels = cloud_image.pixels,
                            .format = Renderer::TextureFormat::Rgba8Unorm,
                            .mip_levels = 1U,
                        },
                },

                Renderer::MaterialTextureBindingInput{
                    .set = kMaterialDescriptorSet,
                    .binding = kSpecularTextureBinding,
                    .array_element = kMaterialArrayElement,
                    .texture =
                        {
                            .width = specular_image.width,
                            .height = specular_image.height,
                            .pixels = specular_image.pixels,
                            .format = Renderer::TextureFormat::Rgba8Unorm,
                            .mip_levels = 1U,
                        },
                },

                Renderer::MaterialTextureBindingInput{
                    .set = kMaterialDescriptorSet,
                    .binding = kNormalTextureBinding,
                    .array_element = kMaterialArrayElement,
                    .texture =
                        {
                            .width = normal_image.width,
                            .height = normal_image.height,
                            .pixels = normal_image.pixels,
                            .format = Renderer::TextureFormat::Rgba8Unorm,
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

        struct CloudUniformData {
            Math::Vector4f uv_offset{};
        };

        constexpr std::size_t kCloudUniformDataSize = 16U;

        static_assert(sizeof(CloudUniformData) == kCloudUniformDataSize);

        CloudUniformData cloud_data{};

        struct alignas(sizeof(Math::Vector4f)) EarthViewUniformData {
            Math::Vector4f camera_world_position{};
            Math::Vector4f sphere_center_radius{};
        };

        constexpr std::size_t kEarthViewUniformDataSize = 32U;

        static_assert(sizeof(EarthViewUniformData) ==
                      kEarthViewUniformDataSize);
        static_assert(alignof(EarthViewUniformData) == sizeof(Math::Vector4f));
        static_assert(offsetof(EarthViewUniformData, sphere_center_radius) ==
                      sizeof(Math::Vector4f));

        const EarthViewUniformData earth_data{
            .camera_world_position =
                {
                    .x = 0.0F,
                    .y = 0.0F,
                    .z = kCameraDistance,
                    .w = 0.0F,
                },
            .sphere_center_radius =
                {
                    .x = translation.x,
                    .y = translation.y,
                    .z = translation.z,
                    .w = kSphereRadius,
                },
        };

        constexpr std::uint32_t kDirectionalLightUniformBinding{1U};
        constexpr std::uint32_t kCloudUniformBinding{4U};
        constexpr std::uint32_t kEarthViewUniformBinding{6U};
        std::array<Renderer::MaterialBufferBindingInput, 3>
            initial_buffer_bindings{
                Renderer::MaterialBufferBindingInput{
                    .set = kMaterialDescriptorSet,
                    .binding = kDirectionalLightUniformBinding,
                    .array_element = kMaterialArrayElement,
                    .type = Renderer::Shader::ShaderResourceType::UniformBuffer,
                    .data = std::as_bytes(std::span{&uniform_data, 1}),
                },
                Renderer::MaterialBufferBindingInput{
                    .set = kMaterialDescriptorSet,
                    .binding = kCloudUniformBinding,
                    .array_element = kMaterialArrayElement,
                    .type = Renderer::Shader::ShaderResourceType::UniformBuffer,
                    .update_frequency =
                        Renderer::MaterialBufferUpdateFrequency::Frequent,
                    .data = std::as_bytes(std::span{&cloud_data, 1}),
                },
                Renderer::MaterialBufferBindingInput{
                    .set = kMaterialDescriptorSet,
                    .binding = kEarthViewUniformBinding,
                    .array_element = kMaterialArrayElement,
                    .type = Renderer::Shader::ShaderResourceType::UniformBuffer,
                    .data = std::as_bytes(std::span{&earth_data, 1}),
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

        constexpr double kCloudScrollSpeed{0.005};

        const auto cloud_animation_start = std::chrono::steady_clock::now();

        double planet_spin_radians{};

        constexpr double kAutoRotationRadiansPerSecond{0.05};

        auto previous_frame_time = std::chrono::steady_clock::now();

        application.run([&](Core::Application &app) -> void {
            const auto frame_time = std::chrono::steady_clock::now();

            const double frame_delta_seconds =
                std::chrono::duration<double>{frame_time - previous_frame_time}
                    .count();

            previous_frame_time = frame_time;

            const double clamped_delta_seconds =
                frame_delta_seconds > 0.1 ? 0.1 : frame_delta_seconds;

            const auto duration = frame_time - cloud_animation_start;

            const double elapsed_seconds =
                std::chrono::duration<double>{duration}.count();

            const double cloud_offset =
                std::fmod(elapsed_seconds * kCloudScrollSpeed, 1.0);

            cloud_data.uv_offset.x = static_cast<float>(cloud_offset);

            app.updateMaterialBuffer({
                .set = kMaterialDescriptorSet,
                .binding = kCloudUniformBinding,
                .array_element = kMaterialArrayElement,
                .data = std::as_bytes(std::span{&cloud_data, 1}),
            });

            constexpr double kFullRotationRadians = 2.0;
            planet_spin_radians =
                std::fmod(planet_spin_radians + (clamped_delta_seconds *
                                                 kAutoRotationRadiansPerSecond),
                          kFullRotationRadians * std::numbers::pi);

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
                    .set = kMaterialDescriptorSet,
                    .binding = kDirectionalLightUniformBinding,
                    .array_element = kMaterialArrayElement,
                    .data = std::as_bytes(std::span{&uniform_data, 1}),
                });
            }

            if (is_dragging && !was_dragging) {
                previous_cursor = current_cursor;
            } else if (is_dragging) {
                const double delta_x = current_cursor.x - previous_cursor.x;
                const double delta_y = current_cursor.y - previous_cursor.y;

                rotation.y +=
                    static_cast<float>(delta_x) * kRotationSensitivity;

                rotation.x +=
                    static_cast<float>(delta_y) * kRotationSensitivity;

                previous_cursor = current_cursor;
            }

            const Math::Matrix4f orientation_matrix = Math::composeTRS(
                translation, rotation,
                Math::Vector3f{.x = 1.0F, .y = 1.0F, .z = 1.0F});

            const Math::Matrix4f spin_matrix =
                Math::rotateY(static_cast<float>(planet_spin_radians));

            const Math::Matrix4f scale_matrix =
                Math::scale(scale.x, scale.y, scale.z);

            const Math::Matrix4f oriented_spin_matrix =
                Math::multiply(orientation_matrix, spin_matrix);

            const Math::Matrix4f updated_model_matrix =
                Math::multiply(oriented_spin_matrix, scale_matrix);

            app.setDrawInstanceModelMatrix(0U, updated_model_matrix);

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
