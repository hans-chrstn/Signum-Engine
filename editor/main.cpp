#include "editor_workspace.hpp"
#include "engine/core/application.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_boundary.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "imgui/editor_imgui_context.hpp"
#include "imgui/editor_imgui_style.hpp"
#include "imgui/editor_imgui_vulkan_backend.hpp"
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <iostream>
#include <optional>
#include <vulkan/vulkan.h>

namespace Core = SNE::Engine::Core;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Editor = SNE::Editor;

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
} // namespace

auto main() -> int {
    return SNE::Engine::Core::Error::runWithErrorBoundary(
        [] -> void {
            Core::Application application;

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

            application.run(
                [&active_rendering_info, &editor_backend, &workspace,
                 &x_scale](Core::Application &app) -> void {
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

                    ImGui_ImplVulkan_NewFrame();
                    ImGui_ImplGlfw_NewFrame();
                    ImGui::NewFrame();

                    if (workspace.draw()) {
                        app.requestClose();
                    }

                    ImGui::Render();
                },
                [](VkCommandBuffer command_buffer) -> void {
                    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                                    command_buffer);
                });
        },
        std::cerr);
}
