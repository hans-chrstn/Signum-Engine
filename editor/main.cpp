#include "engine/core/application.hpp"
#include "engine/core/error/error_boundary.hpp"
#include "engine/renderer/vulkan/vulkan_external_rendering_info.hpp"
#include "imgui/editor_imgui_context.hpp"
#include "imgui/editor_imgui_vulkan_backend.hpp"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <iostream>
#include <optional>

namespace Core = SNE::Engine::Core;
namespace Vulkan = SNE::Engine::Renderer::Vulkan;
namespace Editor = SNE::Editor;

auto main() -> int {
    return SNE::Engine::Core::Error::runWithErrorBoundary(
        [] -> void {
            Core::Application application;

            Vulkan::VulkanExternalRenderingInfo active_rendering_info =
                application.externalRenderingInfo();

            Editor::EditorImGuiContext editor_context{
                application.window().nativeHandle()};

            std::optional<Editor::EditorImGuiVulkanBackend> editor_backend;

            editor_backend.emplace(active_rendering_info);

            application.run(
                [&active_rendering_info,
                 &editor_backend](Core::Application &app) -> void {
                    const Vulkan::VulkanExternalRenderingInfo
                        new_rendering_info = app.externalRenderingInfo();

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

                    ImGui::Begin("Editor Status");
                    ImGui::TextUnformatted("Signum Editor");
                    ImGui::End();
                    ImGui::Render();
                },
                [](VkCommandBuffer command_buffer) -> void {
                    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                                    command_buffer);
                });
        },
        std::cerr);
}
