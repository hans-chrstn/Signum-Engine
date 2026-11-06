#include "editor_workspace.hpp"
#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_boundary.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/core/math/quaternion.hpp"
#include "engine/core/math/vector3.hpp"
#include "engine/core/math/vector4.hpp"
#include "engine/platform/input/cursor_position.hpp"
#include "engine/renderer/camera/initial_camera_data.hpp"
#include "engine/renderer/geometry/mesh_draw_instance.hpp"
#include "engine/renderer/geometry/mesh_upload_data.hpp"
#include "engine/renderer/geometry/primitive_initial_mesh_data.hpp"
#include "engine/renderer/geometry/uv_sphere_mesh.hpp"
#include "engine/renderer/geometry/vertex_attribute_description.hpp"
#include "engine/renderer/geometry/vertex_format.hpp"
#include "engine/renderer/lighting/directional_light.hpp"
#include "engine/renderer/lighting/directional_light_uniform_data.hpp"
#include "engine/renderer/material/material_buffer_binding_input.hpp"
#include "engine/renderer/material/material_buffer_update_frequency.hpp"
#include "engine/renderer/material/material_description.hpp"
#include "engine/renderer/material/material_texture_binding_input.hpp"
#include "engine/renderer/pipeline/primitive_topology.hpp"
#include "engine/renderer/render_target_extent.hpp"
#include "engine/renderer/resource/gpu_resource_handle.hpp"
#include "engine/renderer/shader/graphics_shader_data.hpp"
#include "engine/renderer/shader/shader_compilation_result.hpp"
#include "engine/renderer/shader/shader_resource_type.hpp"
#include "engine/renderer/shader/shader_source_compiler.hpp"
#include "engine/renderer/shader/shader_source_description.hpp"
#include "engine/renderer/shader/shader_source_language.hpp"
#include "engine/renderer/shader/shader_stage.hpp"
#include "engine/renderer/texture/image_loader.hpp"
#include "engine/renderer/texture/rgba8_image.hpp"
#include "engine/renderer/texture/texture_format.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "engine/renderer/vulkan/vulkan_scene_viewport_image_info.hpp"
#include "imgui/editor_imgui_context.hpp"
#include "imgui/editor_imgui_style.hpp"
#include "imgui/editor_imgui_vulkan_backend.hpp"
#include "panels/scene_view_panel.hpp"
#include "scene_view/scene_grid_geometry.hpp"
#include "scene_view/scene_grid_mesh.hpp"
#include "scene_view/scene_grid_mesh_upload.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <iostream>
#include <limits>
#include <numbers>
#include <optional>
#include <span>
#include <utility>
#include <vector>
#include <vulkan/vulkan.h>

namespace Editor = SNE::Editor;
namespace Core = SNE::Engine::Core;
namespace Assertion = Core::Assertion;
namespace Error = Core::Error;
namespace Math = Core::Math;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;
namespace Platform = SNE::Engine::Platform;
namespace Shader = SNE::Engine::Renderer::Shader;

namespace {
    constexpr std::uint32_t kMaterialDescriptorSet{0U};
    constexpr std::uint32_t kMaterialArrayElement{0U};
    constexpr std::uint32_t kDayTextureBinding{0U};
    constexpr std::uint32_t kNightTextureBinding{2U};
    constexpr std::uint32_t kCloudTextureBinding{3U};
    constexpr std::uint32_t kSpecularTextureBinding{5U};
    constexpr std::uint32_t kNormalTextureBinding{7U};
    constexpr std::uint32_t kDirectionalLightUniformBinding{1U};
    constexpr std::uint32_t kCloudUniformBinding{4U};
    constexpr std::uint32_t kEarthViewUniformBinding{6U};
    constexpr float kCameraRotationSensitivity{0.005F};
    constexpr std::uint32_t kSceneGridHalfExtentCells{10U};
    constexpr float kSceneGridSpacing{1.0F};

    struct SceneViewCameraState {
        bool is_dragging{};
        Platform::CursorPosition previous_cursor{};
        Math::Quaternionf orientation{Math::quaternionIdentity()};
    };

    struct CloudUniformData {
        Math::Vector4f uv_offset{};
    };

    struct alignas(sizeof(Math::Vector4f)) EarthViewUniformData {
        Math::Vector4f camera_world_position{};
        Math::Vector4f sphere_center_radius{};
    };

    constexpr std::size_t kCloudUniformDataSize = 16U;

    static_assert(sizeof(CloudUniformData) == kCloudUniformDataSize);
    static_assert(alignof(EarthViewUniformData) == sizeof(Math::Vector4f));
    static_assert(offsetof(EarthViewUniformData, sphere_center_radius) ==
                  sizeof(Math::Vector4f));

    /**
     * @brief Determines whether a supported editor attachment format uses sRGB
     * encoding.
     *
     * @param format Vulkan color attachment format.
     * @return true for supported sRGB formats; false for supported UNORM
     * formats.
     * @throws Core::Error::EngineError If the attachment format is unsupported.
     */
    [[nodiscard]] auto isSrgbAttachmentFormat(VkFormat format) -> bool {
        switch (format) {
        case VK_FORMAT_B8G8R8A8_SRGB:
        case VK_FORMAT_R8G8B8A8_SRGB:
            return true;
        case VK_FORMAT_B8G8R8A8_UNORM:
        case VK_FORMAT_R8G8B8A8_UNORM:
            return false;
        default:
            throw Core::Error::EngineError(
                Core::Error::Code::VulkanImageFormatUnsupported,
                "Unsupported editor color attachment format",
                "isSrgbAttachmentFormat");
        }
    }

    /**
     * @brief Converts the Scene View content size to framebuffer pixels.
     *
     * Scales the panel's available content dimensions to produce an
     * integer render-target extent.
     *
     * Rounds positive dimensions upward to avoid allocating an image
     * smaller than the requested drawable area.
     *
     * Returns no extent when the panel is not drawable or when the
     * dimensions or framebuffer scale cannot produce a valid extent.
     *
     * This conversion does not allocate Vulkan resources.
     *
     * @param panel_state Scene View dimensions and visibility state.
     * @param framebuffer_scale Horizontal and vertical framebuffer scale.
     *
     * @return Pixel extent when valid; otherwise std::nullopt.
     */
    [[nodiscard]] auto tryMakeSceneViewportExtent(
        const Editor::Panels::SceneViewPanelState &panel_state,
        ImVec2 framebuffer_scale)
        -> std::optional<Renderer::RenderTargetExtent> {
        if (!panel_state.m_IsVisible) {
            return std::nullopt;
        }

        if (!std::isfinite(panel_state.m_ContentWidth) ||
            !std::isfinite(panel_state.m_ContentHeight) ||
            !std::isfinite(framebuffer_scale.x) ||
            !std::isfinite(framebuffer_scale.y) ||
            panel_state.m_ContentWidth <= 0.0F ||
            panel_state.m_ContentHeight <= 0.0F ||
            framebuffer_scale.x <= 0.0F || framebuffer_scale.y <= 0.0F) {
            return std::nullopt;
        }

        constexpr auto kMaxExtent =
            static_cast<double>(std::numeric_limits<std::uint32_t>::max());

        const double pixels_width =
            std::ceil(static_cast<double>(panel_state.m_ContentWidth) *
                      framebuffer_scale.x);

        const double pixels_height =
            std::ceil(static_cast<double>(panel_state.m_ContentHeight) *
                      framebuffer_scale.y);

        if (!std::isfinite(pixels_width) || !std::isfinite(pixels_height) ||
            pixels_width <= 0.0 || pixels_height <= 0.0 ||
            pixels_width > kMaxExtent || pixels_height > kMaxExtent) {
            return std::nullopt;
        }

        return Renderer::RenderTargetExtent{
            .width = static_cast<std::uint32_t>(pixels_width),
            .height = static_cast<std::uint32_t>(pixels_height),
        };
    }

    auto updateSceneViewCamera(
        SceneViewCameraState &camera_state,
        const Editor::Panels::SceneViewPanelState &panel_state,
        Core::Application &application, float camera_distance)
        -> std::optional<Math::Vector3f> {

        if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
            camera_state.is_dragging = false;
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
                   panel_state.m_IsImageHovered) {
            camera_state.is_dragging = true;
            camera_state.previous_cursor =
                application.window().cursorPosition();
        }

        if (camera_state.is_dragging) {

            const Platform::CursorPosition current_cursor_pos =
                application.window().cursorPosition();

            const double delta_x =
                current_cursor_pos.x - camera_state.previous_cursor.x;
            const double delta_y =
                current_cursor_pos.y - camera_state.previous_cursor.y;

            if (delta_x == 0.0 && delta_y == 0.0) {
                return std::nullopt;
            }

            const auto yaw_delta =
                static_cast<float>(delta_x) * kCameraRotationSensitivity;
            const auto pitch_delta =
                static_cast<float>(delta_y) * kCameraRotationSensitivity;

            const Math::Quaternionf yaw_rotation = Math::fromAxisAngle(
                {.x = 0.0F, .y = 1.0F, .z = 0.0F}, -yaw_delta);

            const Math::Quaternionf pitch_rotation = Math::fromAxisAngle(
                {.x = 1.0F, .y = 0.0F, .z = 0.0F}, -pitch_delta);

            const Math::Quaternionf drag_rotation =
                Math::multiply(yaw_rotation, pitch_rotation);

            camera_state.orientation = Math::normalize(
                Math::multiply(camera_state.orientation, drag_rotation));

            camera_state.previous_cursor = current_cursor_pos;

            const Math::Matrix4f camera_translation =
                Math::translate(0.0F, 0.0F, camera_distance);

            const Math::Matrix4f camera_rotation =
                Math::toRotationMatrix(camera_state.orientation);

            const Math::Matrix4f camera_world_matrix =
                Math::multiply(camera_rotation, camera_translation);

            const Math::Vector3f camera_world_position = Math::transformPoint(
                camera_world_matrix, {.x = 0.0F, .y = 0.0F, .z = 0.0F});

            const std::optional<Math::Matrix4f> inverted_matrix =
                Math::tryInverse(camera_world_matrix);

            if (!inverted_matrix.has_value()) {
                Core::Assertion::failAssertion(
                    Core::Assertion::AssertionType::Invariant,
                    Core::Error::Subsystem::Editor,
                    "Scene View camera world transform must be "
                    "invertible");
            }

            application.setCameraViewMatrix(inverted_matrix.value());

            return camera_world_position;
        }

        return std::nullopt;
    }
} // namespace

auto main() -> int {
    return SNE::Engine::Core::Error::runWithErrorBoundary(
        [] -> void {
            constexpr float kSphereRadius{0.75F};
            constexpr std::uint32_t kSphereLongitudeSegments{64U};
            constexpr std::uint32_t kSphereLatitudeSegments{32U};
            constexpr float kCameraFovRadians =
                std::numbers::pi_v<float> / 3.0F;
            constexpr float kCameraNearPlane{0.1F};
            constexpr float kCameraFarPlane{100.0F};
            constexpr float kCameraDistanceFromOrigin{3.0F};

            const Renderer::UvSphereParameters sphere_parameters{
                .radius = kSphereRadius,
                .longitude_segments = kSphereLongitudeSegments,
                .latitude_segments = kSphereLatitudeSegments,
            };

            const Renderer::UvSphereMesh reference_sphere =
                Renderer::generateUvSphereMesh(sphere_parameters);

            const Editor::SceneGridParameters grid_parameters{
                .half_extent_cells = kSceneGridHalfExtentCells,
                .grid_spacing = kSceneGridSpacing,
            };

            const std::vector<Editor::SceneGridLine> grid_lines =
                Editor::generateSceneGridLines(grid_parameters);

            const Editor::SceneGridMesh grid_mesh =
                Editor::makeSceneGridMesh(grid_lines);

            const std::array<Renderer::VertexAttributeDescription, 1>
                grid_vertex_attributes{
                    {
                        {
                            .location = 0U,
                            .format = Renderer::VertexFormat::Float32x3,
                            .offset = static_cast<std::uint32_t>(
                                offsetof(Editor::SceneGridVertex, position)),
                        },
                    },
            };

            const Renderer::VertexLayoutDescription grid_vertex_layout{
                .attributes = grid_vertex_attributes,
            };

            const std::array<Math::Matrix4f, 1> reference_model_matrices = {
                Math::identity(),
            };

            const Renderer::DirectionalLight directional_light{
                .direction_to_light = {.x = 3.0F, .y = 1.5F, .z = 1.0F},
                .color = {.x = 1.0F, .y = 1.0F, .z = 1.0F},
                .intensity = 1.0F,
            };
            const Renderer::DirectionalLightUniformData light_uniform_data =
                Renderer::makeDirectionalLightUniformData(directional_light);

            const CloudUniformData cloud_data{};
            EarthViewUniformData earth_data{
                .camera_world_position =
                    {
                        .x = 0.0F,
                        .y = 0.0F,
                        .z = kCameraDistanceFromOrigin,
                        .w = 0.0F,
                    },
                .sphere_center_radius =
                    {
                        .x = 0.0F,
                        .y = 0.0F,
                        .z = 0.0F,
                        .w = kSphereRadius,
                    },
            };

            const Shader::ShaderSourceDescription vertex_description{
                .path = "shaders/slang/basic/textured.slang",
                .language = Shader::ShaderSourceLanguage::Slang,
                .stage = Shader::ShaderStage::Vertex,
                .entry_point = "vertexMain",
            };

            Shader::ShaderCompilationResult vertex_result =
                Shader::compileShaderSource(vertex_description);

            const Shader::ShaderSourceDescription fragment_description{
                .path = "shaders/slang/basic/earth.slang",
                .language = Shader::ShaderSourceLanguage::Slang,
                .stage = Shader::ShaderStage::Fragment,
                .entry_point = "fragmentMain",
            };

            Shader::ShaderCompilationResult fragment_result =
                Shader::compileShaderSource(fragment_description);

            Shader::GraphicsShaderData shaders{
                .vertex = std::move(vertex_result.stage_input),
                .fragment = std::move(fragment_result.stage_input),
            };

            const Renderer::Rgba8Image day =
                Renderer::loadRgba8Image("playground/assets/earth_day.jpg");
            const Renderer::Rgba8Image night =
                Renderer::loadRgba8Image("playground/assets/earth_night.jpg");
            const Renderer::Rgba8Image clouds =
                Renderer::loadRgba8Image("playground/assets/earth_clouds.jpg");
            const Renderer::Rgba8Image specular = Renderer::loadRgba8Image(
                "playground/assets/earth_specular_map.png");
            const Renderer::Rgba8Image normal = Renderer::loadRgba8Image(
                "playground/assets/earth_normal_map.png");

            const std::array<Renderer::MaterialTextureBindingInput, 5>
                initial_textures{
                    {
                        Renderer::MaterialTextureBindingInput{
                            .set = kMaterialDescriptorSet,
                            .binding = kDayTextureBinding,
                            .array_element = kMaterialArrayElement,
                            .texture =
                                {
                                    .width = day.width,
                                    .height = day.height,
                                    .pixels = day.pixels,
                                    .format =
                                        Renderer::TextureFormat::Rgba8Srgb,
                                    .mip_levels = 1U,
                                },
                        },
                        Renderer::MaterialTextureBindingInput{
                            .set = kMaterialDescriptorSet,
                            .binding = kNightTextureBinding,
                            .array_element = kMaterialArrayElement,
                            .texture =
                                {
                                    .width = night.width,
                                    .height = night.height,
                                    .pixels = night.pixels,
                                    .format =
                                        Renderer::TextureFormat::Rgba8Srgb,
                                    .mip_levels = 1U,
                                },
                        },
                        Renderer::MaterialTextureBindingInput{
                            .set = kMaterialDescriptorSet,
                            .binding = kCloudTextureBinding,
                            .array_element = kMaterialArrayElement,
                            .texture =
                                {
                                    .width = clouds.width,
                                    .height = clouds.height,
                                    .pixels = clouds.pixels,
                                    .format =
                                        Renderer::TextureFormat::Rgba8Unorm,
                                    .mip_levels = 1U,
                                },
                        },
                        Renderer::MaterialTextureBindingInput{
                            .set = kMaterialDescriptorSet,
                            .binding = kSpecularTextureBinding,
                            .array_element = kMaterialArrayElement,
                            .texture =
                                {
                                    .width = specular.width,
                                    .height = specular.height,
                                    .pixels = specular.pixels,
                                    .format =
                                        Renderer::TextureFormat::Rgba8Unorm,
                                    .mip_levels = 1U,
                                },
                        },
                        Renderer::MaterialTextureBindingInput{
                            .set = kMaterialDescriptorSet,
                            .binding = kNormalTextureBinding,
                            .array_element = kMaterialArrayElement,
                            .texture =
                                {
                                    .width = normal.width,
                                    .height = normal.height,
                                    .pixels = normal.pixels,
                                    .format =
                                        Renderer::TextureFormat::Rgba8Unorm,
                                    .mip_levels = 1U,
                                },
                        },
                    },
            };

            const std::array<Renderer::MaterialBufferBindingInput, 3>
                initial_buffers{
                    {
                        Renderer::MaterialBufferBindingInput{
                            .set = kMaterialDescriptorSet,
                            .binding = kDirectionalLightUniformBinding,
                            .array_element = kMaterialArrayElement,
                            .type = Shader::ShaderResourceType::UniformBuffer,
                            .data = std::as_bytes(
                                std::span{&light_uniform_data, 1}),
                        },
                        Renderer::MaterialBufferBindingInput{
                            .set = kMaterialDescriptorSet,
                            .binding = kCloudUniformBinding,
                            .array_element = kMaterialArrayElement,
                            .type = Shader::ShaderResourceType::UniformBuffer,
                            .update_frequency = Renderer::
                                MaterialBufferUpdateFrequency::Frequent,
                            .data = std::as_bytes(std::span{&cloud_data, 1}),
                        },
                        Renderer::MaterialBufferBindingInput{
                            .set = kMaterialDescriptorSet,
                            .binding = kEarthViewUniformBinding,
                            .array_element = kMaterialArrayElement,
                            .type = Shader::ShaderResourceType::UniformBuffer,
                            .update_frequency = Renderer::
                                MaterialBufferUpdateFrequency::Frequent,
                            .data = std::as_bytes(std::span{&earth_data, 1}),
                        },
                    },
            };

            Core::ApplicationConfiguration configuration;
            configuration.initial_mesh = Renderer::makePrimitiveInitialMeshData(
                reference_sphere, reference_model_matrices);
            configuration.initial_camera = Renderer::InitialCameraData{
                .view = Math::translate(0.0F, 0.0F, -kCameraDistanceFromOrigin),
                .fov_y_radians = kCameraFovRadians,
                .near_plane = kCameraNearPlane,
                .far_plane = kCameraFarPlane,
            };
            configuration.initial_graphics_shaders = std::move(shaders);
            configuration.initial_buffer_bindings = initial_buffers;
            configuration.initial_texture_bindings = initial_textures;

            Core::Application application{std::move(configuration)};

            const Renderer::MeshUploadData grid_mesh_data =
                Editor::makeSceneGridMeshUploadData(grid_mesh);

            const Renderer::GpuResourceHandle grid_mesh_handle =
                application.registerMesh(grid_mesh_data, grid_vertex_layout);

            const Shader::ShaderSourceDescription grid_vertex_description{
                .path = "shaders/slang/editor/grid.slang",
                .language = Shader::ShaderSourceLanguage::Slang,
                .stage = Shader::ShaderStage::Vertex,
                .entry_point = "vertexMain",
            };

            const Shader::ShaderSourceDescription grid_fragment_description{
                .path = "shaders/slang/editor/grid.slang",
                .language = Shader::ShaderSourceLanguage::Slang,
                .stage = Shader::ShaderStage::Fragment,
                .entry_point = "fragmentMain",
            };

            Shader::ShaderCompilationResult grid_vertex_result =
                Shader::compileShaderSource(grid_vertex_description);

            Shader::ShaderCompilationResult grid_fragment_result =
                Shader::compileShaderSource(grid_fragment_description);

            Shader::GraphicsShaderData grid_shaders{
                .vertex = std::move(grid_vertex_result.stage_input),
                .fragment = std::move(grid_fragment_result.stage_input),
            };

            Renderer::MaterialDescription grid_material_description{
                .shaders = std::move(grid_shaders),
                .pipeline_state =
                    {
                        .depth_test_enabled = true,
                        .depth_write_enabled = false,
                        .topology = Renderer::PrimitiveTopology::LineList,
                    },
            };

            const Renderer::GpuResourceHandle
                grid_material_configuration_handle =
                    application.registerMaterialConfiguration(
                        std::move(grid_material_description));

            std::vector<Renderer::MeshDrawInstance> draw_instances =
                application.drawInstances();

            const Renderer::MeshDrawInstance grid_draw_instance{
                .mesh_handle = grid_mesh_handle,
                .model_matrix = Math::identity(),
                .material_configuration_handle =
                    grid_material_configuration_handle,
            };

            draw_instances.push_back(grid_draw_instance);

            application.setDrawInstances(draw_instances);

            float x_scale{};
            float y_scale{};

            glfwGetWindowContentScale(application.window().nativeHandle(),
                                      &x_scale, &y_scale);

            Vulkan::VulkanExternalRenderingInfo active_rendering_info =
                application.externalRenderingInfo();

            const std::filesystem::path executable_path =
                std::filesystem::read_symlink("/proc/self/exe");

            const std::filesystem::path executable_directory =
                executable_path.parent_path();

            const std::filesystem::path font_path =
                executable_directory / "assets/fonts/Inter-Regular.ttf";

            Editor::EditorImGuiContext editor_context{
                application.window().nativeHandle(), font_path};

            const bool srgb_attachment = isSrgbAttachmentFormat(
                active_rendering_info.color_attachment_format);

            Editor::applyEditorImGuiStyle(srgb_attachment, x_scale);

            std::optional<Editor::EditorImGuiVulkanBackend> editor_backend;

            editor_backend.emplace(active_rendering_info);

            Editor::EditorWorkspace workspace;

            std::optional<VkDescriptorSet> scene_descriptor_this_frame;

            SceneViewCameraState camera_state{};

            application.run(
                [&active_rendering_info, &editor_backend, &workspace, &x_scale,
                 &scene_descriptor_this_frame, &camera_state,
                 &earth_data](Core::Application &app) -> void {
                    scene_descriptor_this_frame.reset();

                    float current_x_scale{};
                    float current_y_scale{};

                    glfwGetWindowContentScale(app.window().nativeHandle(),
                                              &current_x_scale,
                                              &current_y_scale);

                    const bool dpi_changed = current_x_scale != x_scale;

                    const Vulkan::VulkanExternalRenderingInfo
                        new_rendering_info = app.externalRenderingInfo();

                    const bool color_format_changed =
                        active_rendering_info.color_attachment_format !=
                        new_rendering_info.color_attachment_format;

                    if (dpi_changed || color_format_changed) {
                        Editor::applyEditorImGuiStyle(
                            isSrgbAttachmentFormat(
                                new_rendering_info.color_attachment_format),
                            current_x_scale);

                        x_scale = current_x_scale;
                    }

                    if (active_rendering_info.color_attachment_format !=
                            new_rendering_info.color_attachment_format ||
                        active_rendering_info.depth_attachment_format !=
                            new_rendering_info.depth_attachment_format ||
                        active_rendering_info.swapchain_image_count !=
                            new_rendering_info.swapchain_image_count) {

                        editor_backend.reset();

                        editor_backend.emplace(new_rendering_info);

                        active_rendering_info = new_rendering_info;
                    }

                    editor_backend->collectRetiredSceneViewportTextures(
                        app.graphicsSubmissionProgress());

                    ImGui_ImplVulkan_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    if (workspace.draw(
                            [&app, &editor_backend,
                             &scene_descriptor_this_frame](
                                const Editor::Panels::SceneViewPanelState &
                                    panel_state) -> std::optional<ImTextureID> {
                                app.setSceneViewportExtent(
                                    tryMakeSceneViewportExtent(
                                        panel_state,
                                        ImGui::GetIO()
                                            .DisplayFramebufferScale));

                                const std::optional<
                                    Vulkan::VulkanSceneViewportImageInfo>
                                    image_info = app.sceneViewportImageInfo();

                                if (!image_info.has_value()) {
                                    return std::nullopt;
                                }

                                scene_descriptor_this_frame =
                                    editor_backend->sceneViewportTexture(
                                        image_info.value());

                                return static_cast<ImTextureID>(
                                    reinterpret_cast<std::uintptr_t>(
                                        scene_descriptor_this_frame.value()));
                            })) {
                        app.requestClose();
                    }

                    const Editor::Panels::SceneViewPanelState panel_state =
                        workspace.sceneViewPanelState();

                    const std::optional<Math::Vector3f>
                        updated_camera_position =
                            updateSceneViewCamera(camera_state, panel_state,
                                                  app,
                                                  kCameraDistanceFromOrigin);

                    if (updated_camera_position.has_value()) {
                        earth_data.camera_world_position.x =
                            updated_camera_position->x;
                        earth_data.camera_world_position.y =
                            updated_camera_position->y;
                        earth_data.camera_world_position.z =
                            updated_camera_position->z;

                        app.updateMaterialBuffer({
                            .set = kMaterialDescriptorSet,
                            .binding = kEarthViewUniformBinding,
                            .array_element = kMaterialArrayElement,
                            .data = std::as_bytes(std::span{&earth_data, 1U}),
                        });
                    }

                    const ImVec2 framebuffer_scale =
                        ImGui::GetIO().DisplayFramebufferScale;

                    const std::optional<Renderer::RenderTargetExtent> extent =
                        tryMakeSceneViewportExtent(panel_state,
                                                   framebuffer_scale);

                    app.setSceneViewportExtent(extent);

                    ImGui::Render();
                },
                [](VkCommandBuffer command_buffer) -> void {
                    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                                    command_buffer);
                },
                [&editor_backend, &scene_descriptor_this_frame](
                    std::uint64_t submission_id) -> void {
                    if (!scene_descriptor_this_frame.has_value()) {
                        return;
                    }

                    editor_backend->recordSceneViewportTextureSubmission(
                        scene_descriptor_this_frame.value(), submission_id);
                });
        },
        std::cerr);
}
