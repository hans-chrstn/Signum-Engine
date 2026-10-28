#include "editor_workspace.hpp"
#include "engine/core/application.hpp"
#include "engine/core/application_configuration.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_boundary.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/math/matrix4.hpp"
#include "engine/renderer/geometry/primitive_initial_mesh_data.hpp"
#include "engine/renderer/geometry/uv_sphere_mesh.hpp"
#include "engine/renderer/render_target_extent.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "engine/renderer/vulkan/vulkan_scene_viewport_image_info.hpp"
#include "imgui/editor_imgui_context.hpp"
#include "imgui/editor_imgui_style.hpp"
#include "imgui/editor_imgui_vulkan_backend.hpp"
#include "panels/scene_view_panel.hpp"
#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <iostream>
#include <limits>
#include <optional>
#include <vulkan/vulkan.h>

namespace Editor = SNE::Editor;
namespace Core = SNE::Engine::Core;
namespace Math = Core::Math;
namespace Renderer = SNE::Engine::Renderer;
namespace Vulkan = Renderer::Vulkan;

namespace {
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
} // namespace

auto main() -> int {
    return SNE::Engine::Core::Error::runWithErrorBoundary(
        [] -> void {
            const Renderer::UvSphereParameters sphere_parameters{
                .radius = 0.75F,
                .longitude_segments = 32U,
                .latitude_segments = 16U,
            };

            const Renderer::UvSphereMesh reference_sphere =
                Renderer::generateUvSphereMesh(sphere_parameters);

            const std::array<Math::Matrix4f, 1> reference_model_matrices = {
                Math::identity(),
            };

            Core::ApplicationConfiguration configuration;
            configuration.initial_mesh = Renderer::makePrimitiveInitialMeshData(
                reference_sphere, reference_model_matrices);

            Core::Application application{std::move(configuration)};

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

            application.run(
                [&active_rendering_info, &editor_backend, &workspace, &x_scale,
                 &scene_descriptor_this_frame](Core::Application &app) -> void {
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
