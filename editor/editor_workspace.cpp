#include "editor_workspace.hpp"
#include "panels/about_window.hpp"
#include "panels/content_browser_panel.hpp"
#include "panels/editor_status_panel.hpp"
#include "panels/hierarchy_panel.hpp"
#include "panels/inspector_panel.hpp"
#include "panels/performance_panel.hpp"
#include "panels/scene_view_panel.hpp"
#include <imgui.h>
#include <imgui_internal.h>

namespace SNE::Editor {
    namespace {
        /**
         * @brief Initializes the editor's default docking layout.
         *
         * Creates and arranges the workspace docking nodes when no
         * existing layout is available.
         *
         * Preserves an existing docking layout without modifying it.
         *
         * Must be called during an active ImGui frame before the
         * workspace dockspace is submitted.
         *
         * @param dockspace_id Identifier of the workspace dockspace.
         */
        auto initializeDefaultDockingLayout(ImGuiID dockspace_id) -> void {
            if (ImGui::DockBuilderGetNode(dockspace_id) != nullptr) {
                return;
            }

            ImGuiID left_node_id = 0;
            ImGuiID remaining_node_id = 0;
            ImGuiID right_node_id = 0;
            ImGuiID center_node_id = 0;
            ImGuiID bottom_node_id = 0;
            ImGuiID scene_view_node_id = 0;
            ImGuiID inspector_node_id = 0;
            ImGuiID performance_node_id = 0;

            ImGui::DockBuilderAddNode(dockspace_id,
                                      ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspace_id,
                                          ImGui::GetMainViewport()->WorkSize);

            const float hierarchy_size_ratio = 0.20F;
            ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Left,
                                        hierarchy_size_ratio, &left_node_id,
                                        &remaining_node_id);
            ImGui::DockBuilderDockWindow("Hierarchy", left_node_id);

            const float inspector_size_ratio = 0.25F;
            ImGui::DockBuilderSplitNode(remaining_node_id, ImGuiDir_Right,
                                        inspector_size_ratio, &right_node_id,
                                        &center_node_id);

            const float content_browser_size_ratio = 0.25F;
            ImGui::DockBuilderSplitNode(center_node_id, ImGuiDir_Down,
                                        content_browser_size_ratio,
                                        &bottom_node_id, &scene_view_node_id);
            ImGui::DockBuilderDockWindow("Content Browser", bottom_node_id);
            ImGui::DockBuilderDockWindow("Editor Status", bottom_node_id);

            ImGui::DockBuilderDockWindow("Scene View", scene_view_node_id);

            const float performance_size_ratio = 0.30F;
            ImGui::DockBuilderSplitNode(
                right_node_id, ImGuiDir_Down, performance_size_ratio,
                &performance_node_id, &inspector_node_id);
            ImGui::DockBuilderDockWindow("Inspector", inspector_node_id);
            ImGui::DockBuilderDockWindow("Performance", performance_node_id);

            ImGui::DockBuilderFinish(dockspace_id);
        }
    } // namespace

    auto EditorWorkspace::draw() -> bool {
        bool exit_requested = false;
        bool reset_layout_requested = false;

        const bool main_menu = ImGui::BeginMainMenuBar();

        if (main_menu) {
            const bool file_menu = ImGui::BeginMenu("File");
            if (file_menu) {
                const bool exit_item = ImGui::MenuItem("Exit");
                if (exit_item) {
                    exit_requested = true;
                }
                ImGui::EndMenu();
            }

            const bool view_menu = ImGui::BeginMenu("View");
            if (view_menu) {
                ImGui::MenuItem("Editor Status", nullptr, &m_ShowEditorStatus);
                ImGui::MenuItem("Hierarchy", nullptr, &m_ShowHierarchy);
                ImGui::MenuItem("Inspector", nullptr, &m_ShowInspector);
                ImGui::MenuItem("Scene View", nullptr, &m_ShowSceneView);
                ImGui::MenuItem("Performance", nullptr, &m_ShowPerformanceView);
                ImGui::MenuItem("Content Browser", nullptr,
                                &m_ShowContentBrowserView);
                ImGui::Separator();
                const bool reset_layout = ImGui::MenuItem("Reset Layout");
                if (reset_layout) {
                    reset_layout_requested = true;
                }
                ImGui::EndMenu();
            }

            const bool help_menu = ImGui::BeginMenu("Help");
            if (help_menu) {
                if (ImGui::MenuItem("About")) {
                    m_ShowAbout = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        const ImGuiID dockspace_id = ImGui::GetID("SignumEditorWorkspace");

        if (reset_layout_requested) {
            ImGui::DockBuilderRemoveNode(dockspace_id);
        }

        initializeDefaultDockingLayout(dockspace_id);

        ImGui::DockSpaceOverViewport(dockspace_id);
        Panels::drawEditorStatusPanel(m_ShowEditorStatus);
        Panels::drawAboutWindow(m_ShowAbout);
        Panels::drawHierarchyPanel(m_ShowHierarchy);
        Panels::drawInspectorPanel(m_ShowInspector);
        m_SceneViewPanelState = Panels::drawSceneViewPanel(m_ShowSceneView);
        Panels::drawPerformancePanel(m_ShowPerformanceView);
        Panels::drawContentBrowserPanel(m_ShowContentBrowserView);

        return exit_requested;
    }

    auto EditorWorkspace::sceneViewPanelState() const noexcept
        -> Panels::SceneViewPanelState {
        return m_SceneViewPanelState;
    }
} // namespace SNE::Editor
