#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/error/diagnostic_writer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_boundary.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/core/math/vector3d.hpp"
#include "engine/core/math/vector4.hpp"
#include "engine/platform/input/cursor_position.hpp"
#include "engine/platform/input/key.hpp"
#include "engine/platform/input/mouse_button.hpp"
#include "engine/renderer/camera/camera_relative_position.hpp"
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
#include "engine/renderer/shader/shader_artifact.hpp"
#include "engine/renderer/shader/shader_compilation_result.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_source_compiler.hpp"
#include "engine/renderer/shader/shader_source_description.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/shader/shader_stage_input.hpp"
#include "engine/renderer/texture/image_loader.hpp"
#include "engine/renderer/texture/texture_format.hpp"
#include "engine/renderer/vulkan/pipeline/vulkan_spirv.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <numbers>
#include <span>
#include <stdexcept>
#include <system_error>
#include <utility>

namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Core = SNE::Engine::Core;
namespace Math = SNE::Engine::Core::Math;
namespace Shader = SNE::Engine::Renderer::Shader;
namespace Platform = SNE::Engine::Platform;

namespace {
    [[nodiscard]] auto
    tryReplaceFragmentShader(Core::Application &application,
                             const std::filesystem::path &shader_path) -> bool {

        try {
            Shader::ShaderArtifact artifact = Vulkan::loadSpirv(shader_path);

            Shader::ShaderStageInput input{
                .artifact = std::move(artifact),
                .selection =
                    {
                        .stage = Shader::ShaderStage::Fragment,
                        .entry_point = "main",
                    },
            };

            application.replaceFragmentShader(std::move(input));

        } catch (const Core::Error::EngineError &error) {
            Core::Error::writeDiagnostic(std::cerr, error);
            return false;
        }
        return true;
    }

    [[nodiscard]] auto
    tryReplaceFragmentShaderSource(Core::Application &application,
                                   const std::filesystem::path &shader_path)
        -> bool {
        try {
            Shader::ShaderSourceDescription description{
                .path = shader_path,
                .language = Shader::ShaderSourceLanguage::Slang,
                .stage = Shader::ShaderStage::Fragment,
                .entry_point = "fragmentMain",
            };

            Shader::ShaderCompilationResult result =
                SNE::Engine::Renderer::Shader::compileShaderSource(description);

            application.replaceFragmentShader(std::move(result.stage_input));
        } catch (const Core::Error::EngineError &error) {
            Core::Error::writeDiagnostic(std::cerr, error);
            return false;
        }

        return true;
    }

    struct CloudUniformData {
        Math::Vector4f uv_offset{};
    };

    struct alignas(sizeof(Math::Vector4f)) EarthViewUniformData {
        Math::Vector4f camera_world_position{};
        Math::Vector4f sphere_center_radius{};
    };

    struct FrameState {
        Core::Application &app;
        Math::Vector3f rotation;
        Renderer::DirectionalLight directional_light;
        Renderer::DirectionalLightUniformData uniform_data;
        CloudUniformData cloud_data;
        EarthViewUniformData earth_data;
        Platform::CursorPosition previous_cursor{};
        bool was_dragging{false};
        bool was_right_pressed{false};
        bool was_middle_pressed{false};
        bool cursor_captured{false};
        bool was_tab_pressed{false};
        bool light_enabled{true};
        bool is_earth_shader_active{true};
        double planet_spin_radians{};
        std::chrono::steady_clock::time_point cloud_animation_start;
        std::chrono::steady_clock::time_point previous_frame_time;
        std::chrono::steady_clock::time_point last_shader_poll_time;
        std::filesystem::file_time_type observed_shader_write_time;
        std::filesystem::path earth_shader_path;
        Math::Vector3d earth_world_position{};
        Math::Vector3d camera_world_position{};
        double camera_yaw_radians{};
        double camera_pitch_radians{};
    };

    constexpr float kSphereRadius{0.75F};
    constexpr std::uint32_t kSphereLongitudeSegments{64U};
    constexpr std::uint32_t kSphereLatitudeSegments{32U};
    constexpr float kCameraFovRadians = std::numbers::pi_v<float> / 3.0F;
    constexpr float kCameraNearPlane = 0.1F;
    constexpr float kCameraFarPlane = 100.0F;
    constexpr float kDegreesToRadians = std::numbers::pi_v<float> / 180.0F;
    constexpr float kRotateX = 20.0F;
    constexpr float kRotateY = -30.0F;
    constexpr float kRotateZ = 0.0F;
    constexpr float kRotationSensitivity{0.005F};
    constexpr double kCloudScrollSpeed{0.005};
    constexpr double kAutoRotationRadiansPerSecond{0.05};
    constexpr double kFullRotationRadians = 2.0;
    constexpr auto kShaderPollInterval = std::chrono::milliseconds{250};
    constexpr std::size_t kInitialTextureBindingsSize{5U};
    constexpr std::uint32_t kMaterialDescriptorSet{0U};
    constexpr std::uint32_t kDayTextureBinding{0U};
    constexpr std::uint32_t kMaterialArrayElement{0U};
    constexpr std::uint32_t kNightTextureBinding{2U};
    constexpr std::uint32_t kCloudTextureBinding{3U};
    constexpr std::uint32_t kSpecularTextureBinding{5U};
    constexpr std::uint32_t kNormalTextureBinding{7U};
    constexpr std::uint32_t kDirectionalLightUniformBinding{1U};
    constexpr std::uint32_t kCloudUniformBinding{4U};
    constexpr std::uint32_t kEarthViewUniformBinding{6U};
    constexpr std::size_t kCloudUniformDataSize = 16U;
    constexpr std::size_t kEarthViewUniformDataSize = 32U;
    constexpr double kEarthOrbitDistanceMeters = 149'597'870'700.0;
    constexpr double kEarthMeanRadiusMeters = 6'371'000.0;
    [[maybe_unused]] constexpr double kSunMeanRadiusMeters = 695'700'000.0;
    constexpr double kMetersPerRenderUnit =
        kEarthMeanRadiusMeters / static_cast<double>(kSphereRadius);

    constexpr double kCameraMovementMetersPerSecond = kEarthMeanRadiusMeters;
    constexpr double kCameraSprintMultiplier = 10.0;
    constexpr double kCameraLookSensitivity = 0.003;
    constexpr double kMaxCameraPitchRadians =
        89.0 * std::numbers::pi_v<double> / 180.0;

    static_assert(sizeof(CloudUniformData) == kCloudUniformDataSize);
    static_assert(sizeof(EarthViewUniformData) == kEarthViewUniformDataSize);
    static_assert(alignof(EarthViewUniformData) == sizeof(Math::Vector4f));
    static_assert(offsetof(EarthViewUniformData, sphere_center_radius) ==
                  sizeof(Math::Vector4f));

    auto updateCloudUniforms(FrameState &state, double elapsed_seconds)
        -> void {
        const double cloud_offset =
            std::fmod(elapsed_seconds * kCloudScrollSpeed, 1.0);
        state.cloud_data.uv_offset.x = static_cast<float>(cloud_offset);

        state.app.updateMaterialBuffer({
            .set = kMaterialDescriptorSet,
            .binding = kCloudUniformBinding,
            .array_element = kMaterialArrayElement,
            .data = std::as_bytes(std::span{&state.cloud_data, 1}),
        });
    }

    auto updatePlanetSpin(FrameState &state, double clamped_delta_seconds)
        -> void {
        state.planet_spin_radians = std::fmod(
            state.planet_spin_radians +
                (clamped_delta_seconds * kAutoRotationRadiansPerSecond),
            kFullRotationRadians * std::numbers::pi);
    }

    auto pollShaderHotReload(FrameState &state,
                             std::chrono::steady_clock::time_point frame_time)
        -> void {
        if (frame_time - state.last_shader_poll_time < kShaderPollInterval) {
            return;
        }
        state.last_shader_poll_time = frame_time;

        std::error_code filesystem_error;
        const auto current_write_time = std::filesystem::last_write_time(
            state.earth_shader_path, filesystem_error);

        if (filesystem_error ||
            current_write_time == state.observed_shader_write_time) {
            return;
        }
        state.observed_shader_write_time = current_write_time;

        if (state.is_earth_shader_active) {
            static_cast<void>(tryReplaceFragmentShaderSource(
                state.app, state.earth_shader_path));
        }
    }

    auto handleCursorCaptureToggle(FrameState &state, bool is_tab_pressed)
        -> void {
        if (!is_tab_pressed || state.was_tab_pressed) {
            return;
        }

        state.cursor_captured = !state.cursor_captured;

        state.app.setCursorCaptured(state.cursor_captured);

        state.previous_cursor = state.app.window().cursorPosition();
        state.was_dragging = false;
    }

    auto handleLightToggle(FrameState &state, bool is_right_pressed) -> void {
        if (!is_right_pressed || state.was_right_pressed) {
            return;
        }
        state.light_enabled = !state.light_enabled;
        state.directional_light.intensity = state.light_enabled ? 1.0F : 0.0F;
        state.uniform_data =
            Renderer::makeDirectionalLightUniformData(state.directional_light);

        state.app.updateMaterialBuffer({
            .set = kMaterialDescriptorSet,
            .binding = kDirectionalLightUniformBinding,
            .array_element = kMaterialArrayElement,
            .data = std::as_bytes(std::span{&state.uniform_data, 1}),
        });
    }

    auto handleShaderToggle(FrameState &state, bool is_middle_pressed) -> void {
        if (!is_middle_pressed || state.was_middle_pressed) {
            return;
        }

        bool replacement_succeeded{false};

        if (state.is_earth_shader_active) {
            replacement_succeeded = tryReplaceFragmentShader(
                state.app, "build/shaders/textured.frag.spv");
        } else {
            replacement_succeeded = tryReplaceFragmentShaderSource(
                state.app, state.earth_shader_path);
        }

        if (replacement_succeeded) {
            state.is_earth_shader_active = !state.is_earth_shader_active;
        }
    }

    auto handleDragRotation(FrameState &state, bool is_dragging,
                            const Platform::CursorPosition &current_cursor)
        -> void {
        if (is_dragging && !state.was_dragging) {
            state.previous_cursor = current_cursor;
            return;
        }
        if (!is_dragging) {
            return;
        }

        const double delta_x = current_cursor.x - state.previous_cursor.x;
        const double delta_y = current_cursor.y - state.previous_cursor.y;

        state.rotation.y += static_cast<float>(delta_x) * kRotationSensitivity;
        state.rotation.x += static_cast<float>(delta_y) * kRotationSensitivity;
        state.previous_cursor = current_cursor;
    }

    auto updateCameraLook(FrameState &state,
                          const Platform::CursorPosition &current_cursor)
        -> void {
        if (!state.cursor_captured) {
            return;
        }

        const double delta_x = current_cursor.x - state.previous_cursor.x;
        const double delta_y = current_cursor.y - state.previous_cursor.y;

        state.camera_yaw_radians += delta_x * kCameraLookSensitivity;
        state.camera_pitch_radians -= delta_y * kCameraLookSensitivity;

        state.camera_pitch_radians =
            std::clamp(state.camera_pitch_radians, -kMaxCameraPitchRadians,
                       kMaxCameraPitchRadians);

        state.previous_cursor = current_cursor;
    }

    auto updateCameraView(FrameState &state) -> void {
        const auto yaw_radians = static_cast<float>(state.camera_yaw_radians);

        const Math::Matrix4f inverse_yaw = Math::rotateY(yaw_radians);

        const auto pitch_radians =
            static_cast<float>(state.camera_pitch_radians);

        const Math::Matrix4f inverse_pitch = Math::rotateX(-pitch_radians);

        const Math::Matrix4f view_matrix =
            Math::multiply(inverse_pitch, inverse_yaw);

        state.app.setCameraViewMatrix(view_matrix);
    }

    auto updateCameraMovement(FrameState &state, double clamped_delta_seconds)
        -> void {
        if (!state.cursor_captured) {
            return;
        }

        const double cos_yaw = std::cos(state.camera_yaw_radians);
        const double sin_yaw = std::sin(state.camera_yaw_radians);
        const double cos_pitch = std::cos(state.camera_pitch_radians);
        const double sin_pitch = std::sin(state.camera_pitch_radians);

        const Math::Vector3d forward{
            .x = sin_yaw * cos_pitch,
            .y = sin_pitch,
            .z = -cos_yaw * cos_pitch,
        };

        const Math::Vector3d right{
            .x = cos_yaw,
            .y = 0.0,
            .z = sin_yaw,
        };

        const Math::Vector3d upward = Math::cross(right, forward);

        const bool is_fast =
            state.app.window().isKeyPressed(Platform::Key::LeftShift);

        const double multiplier = is_fast ? kCameraSprintMultiplier : 1.0;

        const double distance =
            kCameraMovementMetersPerSecond * multiplier * clamped_delta_seconds;

        Math::Vector3d movement_direction{};

        if (state.app.window().isKeyPressed(Platform::Key::W)) {
            movement_direction = Math::add(movement_direction, forward);
        }

        if (state.app.window().isKeyPressed(Platform::Key::S)) {
            movement_direction = Math::subtract(movement_direction, forward);
        }

        if (state.app.window().isKeyPressed(Platform::Key::A)) {
            movement_direction = Math::subtract(movement_direction, right);
        }

        if (state.app.window().isKeyPressed(Platform::Key::D)) {
            movement_direction = Math::add(movement_direction, right);
        }

        if (state.app.window().isKeyPressed(Platform::Key::Q)) {
            movement_direction = Math::subtract(movement_direction, upward);
        }

        if (state.app.window().isKeyPressed(Platform::Key::E)) {
            movement_direction = Math::add(movement_direction, upward);
        }

        if (Math::lengthSquared(movement_direction) > 0.0) {
            const Math::Vector3d normalized_direction =
                Math::normalize(movement_direction);

            const Math::Vector3d displacement =
                Math::scale(normalized_direction, distance);

            state.camera_world_position =
                Math::add(state.camera_world_position, displacement);
        }
    }

    auto updateModelMatrix(FrameState &state) -> void {

        constexpr double kRenderUnitsPerMeter = 1.0 / kMetersPerRenderUnit;

        const Math::Vector3d earth_relative_meters = Math::subtract(
            state.earth_world_position, state.camera_world_position);

        const auto relative_position = Renderer::tryMakeCameraRelativePosition({
            .world_position =
                Math::scale(earth_relative_meters, kRenderUnitsPerMeter),
        });

        if (!relative_position.has_value()) {
            throw std::runtime_error("Camera relative conversion failed");
        }

        const Math::Vector3f translation = relative_position.value();

        state.earth_data.sphere_center_radius.x = translation.x;
        state.earth_data.sphere_center_radius.y = translation.y;
        state.earth_data.sphere_center_radius.z = translation.z;

        state.app.updateMaterialBuffer({
            .set = kMaterialDescriptorSet,
            .binding = kEarthViewUniformBinding,
            .array_element = kMaterialArrayElement,
            .data = std::as_bytes(std::span{&state.earth_data, 1}),
        });

        const Math::Vector3f scale{
            .x = 1.0F,
            .y = 1.0F,
            .z = 1.0F,
        };

        const Math::Matrix4f orientation_matrix =
            Math::composeTRS(translation, state.rotation, scale);
        const Math::Matrix4f spin_matrix =
            Math::rotateY(static_cast<float>(state.planet_spin_radians));
        const Math::Matrix4f oriented_spin_matrix =
            Math::multiply(orientation_matrix, spin_matrix);

        state.app.setDrawInstanceModelMatrix(0U, oriented_spin_matrix);
    }

    auto processFrame(FrameState &state) -> void {
        const auto frame_time = std::chrono::steady_clock::now();
        const double frame_delta_seconds =
            std::chrono::duration<double>{frame_time -
                                          state.previous_frame_time}
                .count();
        state.previous_frame_time = frame_time;
        const double clamped_delta_seconds =
            frame_delta_seconds > 0.1 ? 0.1 : frame_delta_seconds;

        const auto duration = frame_time - state.cloud_animation_start;
        const double elapsed_seconds =
            std::chrono::duration<double>{duration}.count();

        updateCloudUniforms(state, elapsed_seconds);
        updatePlanetSpin(state, clamped_delta_seconds);
        pollShaderHotReload(state, frame_time);

        const bool is_tab_pressed =
            state.app.window().isKeyPressed(Platform::Key::Tab);

        handleCursorCaptureToggle(state, is_tab_pressed);

        const Platform::CursorPosition current_cursor =
            state.app.window().cursorPosition();
        const bool is_dragging =
            !state.cursor_captured && state.app.window().isMouseButtonPressed(
                                          Platform::MouseButton::Left);
        const bool is_right_pressed = state.app.window().isMouseButtonPressed(
            Platform::MouseButton::Right);
        const bool is_middle_pressed = state.app.window().isMouseButtonPressed(
            Platform::MouseButton::Middle);

        handleLightToggle(state, is_right_pressed);
        handleShaderToggle(state, is_middle_pressed);
        handleDragRotation(state, is_dragging, current_cursor);

        updateCameraLook(state, current_cursor);
        updateCameraMovement(state, clamped_delta_seconds);
        updateCameraView(state);
        updateModelMatrix(state);

        state.was_dragging = is_dragging;
        state.was_right_pressed = is_right_pressed;
        state.was_middle_pressed = is_middle_pressed;
        state.was_tab_pressed = is_tab_pressed;
    }

    auto createInitialMesh(const Renderer::UvSphereMesh &sphere,
                           const std::array<Math::Matrix4f, 1> &model_matrices)
        -> Renderer::InitialMeshData {
        static constexpr std::array<Renderer::VertexAttributeDescription, 4>
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

        return Renderer::InitialMeshData{
            .mesh = mesh,
            .layout = layout,
            .model_matrices = model_matrices,
        };
    }

    auto createInitialCamera() -> Renderer::InitialCameraData {
        return Renderer::InitialCameraData{
            .view = Math::identity(),
            .fov_y_radians = kCameraFovRadians,
            .near_plane = kCameraNearPlane,
            .far_plane = kCameraFarPlane,
        };
    }

    auto createInitialShaders() -> Shader::GraphicsShaderData {
        auto vertex_data = Vulkan::loadSpirv("build/shaders/textured.vert.spv");
        auto fragment_data = Vulkan::loadSpirv("build/shaders/earth.frag.spv");

        return Shader::GraphicsShaderData{
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
    }

    struct InitialTextureImages {
        Renderer::Rgba8Image day;
        Renderer::Rgba8Image night;
        Renderer::Rgba8Image clouds;
        Renderer::Rgba8Image specular;
        Renderer::Rgba8Image normal;
    };

    auto loadInitialTextureImages() -> InitialTextureImages {
        return {
            .day = Renderer::loadRgba8Image("playground/assets/earth_day.jpg"),
            .night =
                Renderer::loadRgba8Image("playground/assets/earth_night.jpg"),
            .clouds =
                Renderer::loadRgba8Image("playground/assets/earth_clouds.jpg"),
            .specular = Renderer::loadRgba8Image(
                "playground/assets/earth_specular_map.png"),
            .normal = Renderer::loadRgba8Image(
                "playground/assets/earth_normal_map.png"),
        };
    }

    auto createInitialTextures(const InitialTextureImages &images)
        -> std::array<Renderer::MaterialTextureBindingInput,
                      kInitialTextureBindingsSize> {
        return std::array<Renderer::MaterialTextureBindingInput,
                          kInitialTextureBindingsSize>{
            Renderer::MaterialTextureBindingInput{
                .set = kMaterialDescriptorSet,
                .binding = kDayTextureBinding,
                .array_element = kMaterialArrayElement,
                .texture =
                    {
                        .width = images.day.width,
                        .height = images.day.height,
                        .pixels = images.day.pixels,
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
                        .width = images.night.width,
                        .height = images.night.height,
                        .pixels = images.night.pixels,
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
                        .width = images.clouds.width,
                        .height = images.clouds.height,
                        .pixels = images.clouds.pixels,
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
                        .width = images.specular.width,
                        .height = images.specular.height,
                        .pixels = images.specular.pixels,
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
                        .width = images.normal.width,
                        .height = images.normal.height,
                        .pixels = images.normal.pixels,
                        .format = Renderer::TextureFormat::Rgba8Unorm,
                        .mip_levels = 1U,
                    },
            },
        };
    }

    auto createInitialBuffers(
        const Renderer::DirectionalLightUniformData &uniform_data,
        const CloudUniformData &cloud_data,
        const EarthViewUniformData &earth_data)
        -> std::array<Renderer::MaterialBufferBindingInput, 3> {
        return std::array<Renderer::MaterialBufferBindingInput, 3>{
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
                .update_frequency =
                    Renderer::MaterialBufferUpdateFrequency::Frequent,
                .data = std::as_bytes(std::span{&earth_data, 1}),
            },
        };
    }
} // namespace

auto main() -> int {
    return Core::Error::runWithErrorBoundary(
        [] -> void {
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

            CloudUniformData cloud_data{};

            const EarthViewUniformData earth_data{
                .camera_world_position =
                    {
                        .x = 0.0F,
                        .y = 0.0F,
                        .z = 0.0F,
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

            constexpr Renderer::UvSphereParameters sphere_params{
                .radius = kSphereRadius,
                .longitude_segments = kSphereLongitudeSegments,
                .latitude_segments = kSphereLatitudeSegments,
            };

            const Renderer::UvSphereMesh sphere =
                Renderer::generateUvSphereMesh(sphere_params);

            const Math::Vector3f scale{
                .x = 1.0F,
                .y = 1.0F,
                .z = 1.0F,
            };

            const std::array<Math::Matrix4f, 1> model_matrices{
                Math::composeTRS(translation, rotation, scale),
            };

            const auto initial_mesh = createInitialMesh(sphere, model_matrices);
            const auto initial_camera = createInitialCamera();
            auto initial_shaders = createInitialShaders();

            const InitialTextureImages texture_images =
                loadInitialTextureImages();

            const auto initial_textures = createInitialTextures(texture_images);
            const auto initial_buffers =
                createInitialBuffers(uniform_data, cloud_data, earth_data);

            Core::ApplicationConfiguration configuration{};
            configuration.initial_mesh = initial_mesh;
            configuration.initial_camera = initial_camera;
            configuration.initial_graphics_shaders = std::move(initial_shaders);
            configuration.initial_buffer_bindings = initial_buffers;
            configuration.initial_texture_bindings = initial_textures;

            Core::Application application{std::move(configuration)};

            static_cast<void>(tryReplaceFragmentShaderSource(
                application, "shaders/slang/basic/earth.slang"));

            const auto now = std::chrono::steady_clock::now();
            const std::filesystem::path earth_shader_path =
                "shaders/slang/basic/earth.slang";

            constexpr double kCameraDistanceInEarthRadii = 3.0;

            FrameState state{
                .app = application,
                .rotation = rotation,
                .directional_light = directional_light,
                .uniform_data = uniform_data,
                .cloud_data = cloud_data,
                .earth_data = earth_data,
                .previous_cursor = {},
                .was_dragging = false,
                .was_right_pressed = false,
                .was_middle_pressed = false,
                .light_enabled = true,
                .is_earth_shader_active = true,
                .planet_spin_radians = 0.0,
                .cloud_animation_start = now,
                .previous_frame_time = now,
                .last_shader_poll_time = now,
                .observed_shader_write_time =
                    std::filesystem::last_write_time(earth_shader_path),
                .earth_shader_path = earth_shader_path,
                .earth_world_position =
                    {
                        .x = kEarthOrbitDistanceMeters,
                        .y = 0.0,
                        .z = 0.0,
                    },
                .camera_world_position =
                    {
                        .x = kEarthOrbitDistanceMeters,
                        .y = 0.0,
                        .z = kCameraDistanceInEarthRadii *
                             kEarthMeanRadiusMeters,
                    },
            };

            application.run(
                [&state](Core::Application &) -> void { processFrame(state); });
        },
        std::cerr);
}
