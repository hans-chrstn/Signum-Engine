#pragma once

#include <functional>
#include <imgui.h>
#include <optional>

namespace SNE::Editor::Panels {
    /**
     * @brief Describes the Scene View panel's available area and input state.
     *
     * Records the drawable content dimensions and interaction state
     * reported by the editor UI.
     *
     * Content dimensions are expressed in ImGui coordinates and must
     * be converted to framebuffer pixels before requesting GPU resources.
     *
     * A hidden or collapsed panel reports no drawable content.
     *
     * The structure does not own rendering resources or depend on
     * the Vulkan renderer.
     */
    struct SceneViewPanelState {
        /** @brief Available Scene View content width in ImGui coordinates. */
        float m_ContentWidth{};

        /** @brief Available Scene View content height in ImGui coordinates. */
        float m_ContentHeight{};

        /** @brief Whether the Scene View currently has input focus. */
        bool m_IsFocused{};

        /** @brief Whether the pointer is hovering over the Scene View. */
        bool m_IsHovered{};

        /** @brief Whether the Scene View has drawable content. */
        bool m_IsVisible{};
    };

    /**
     * @brief Draws the editor's Scene View panel.
     *
     * Provides the dockable window used to display and interact with
     * the scene rendered by the engine.
     *
     * Reports the panel's available content dimensions, visibility,
     * and input focus state for the current editor frame.
     *
     * Scene rendering integration is managed separately.
     *
     * The panel is drawn only when visible is true. Closing the
     * panel updates the supplied visibility state.
     *
     * @param visible Reference to the panel's visibility state.
     * Updated when the user closes the panel.
     * @param resolve_texture Callback invoked when the panel has
     * drawable content.
     *
     * Receives the current panel dimensions and interaction state.
     * Returns an ImGui texture identifier when a compatible scene
     * image is available, or std::nullopt otherwise.
     *
     * @return Scene View content dimensions and interaction state.
     * Returns a default-initialized state when the panel has no
     * drawable content.
     *
     * @note The callback must not retain the panel state reference.
     * @note The caller manages texture registration and GPU lifetime.
     */
    auto drawSceneViewPanel(bool &visible,
                            const std::function<std::optional<ImTextureID>(
                                const SceneViewPanelState &)> &resolve_texture)
        -> SceneViewPanelState;
} // namespace SNE::Editor::Panels
