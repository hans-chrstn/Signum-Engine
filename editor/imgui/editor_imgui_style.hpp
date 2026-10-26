#pragma once

namespace SNE::Editor {
    /**
     * @brief Applies the editor's default Dear ImGui appearance.
     *
     * Configures editor-wide colors, spacing, and widget styling.
     *
     * Converts authored sRGB colors to linear RGB when the rendering
     * attachment uses sRGB encoding.
     *
     * Scales layout dimensions using the supplied DPI scale.
     * Each application starts from unscaled values to prevent
     * cumulative scaling.
     *
     * Must be called after an ImGui context has been created and
     * before the editor begins drawing frames.
     *
     * Does not manage the ImGui context or rendering backend.
     *
     * @param srgb_attachment true when the color attachment uses an
     * sRGB format; false when using an ordinary UNORM format.
     * @param dpi_scale Positive content-scale factor applied to layout
     * dimensions. A value of 1.0 means no additional scaling.
     *
     * @pre dpi_scale must be finite and greater than zero.
     */
    auto applyEditorImGuiStyle(bool srgb_attachment, float dpi_scale) -> void;
} // namespace SNE::Editor
