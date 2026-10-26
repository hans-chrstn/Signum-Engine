#include "editor_imgui_style.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <cmath>
#include <cstdint>
#include <imgui.h>

namespace Assertion = SNE::Engine::Core::Assertion;
namespace Error = SNE::Engine::Core::Error;

namespace SNE::Editor {
    namespace {
        /**
         * @brief Converts an sRGB color channel to linear light.
         *
         * Applies the standard sRGB transfer function to a normalized
         * color channel.
         *
         * @param channel Normalized sRGB channel value in [0, 1].
         * @return Corresponding linear-light channel value.
         */
        [[nodiscard]] auto srgbToLinearChannel(float channel) -> float {
            constexpr float kSrgbThreshold = 0.04045F;
            constexpr float kLinearScale = 12.92F;
            constexpr float kNonlinearOffset = 0.055F;
            constexpr float kNonlinearScale = 1.055F;
            constexpr float kNonlinearExponent = 2.4F;

            if (channel <= kSrgbThreshold) {
                return channel / kLinearScale;
            }

            return std::pow((channel + kNonlinearOffset) / kNonlinearScale,
                            kNonlinearExponent);
        }

        /**
         * @brief Converts an sRGB color to linear light.
         *
         * Converts the red, green, and blue channels using the
         * standard sRGB transfer function.
         *
         * Preserves the alpha component without conversion.
         *
         * @param color Normalized sRGB color with alpha.
         * @return Color with linear RGB channels and unchanged alpha.
         */
        [[nodiscard]] auto srgbToLinearColor(const ImVec4 &color) -> ImVec4 {
            return ImVec4{
                srgbToLinearChannel(color.x),
                srgbToLinearChannel(color.y),
                srgbToLinearChannel(color.z),
                color.w,
            };
        }

        /**
         * @brief Converts a packed 24-bit RGB color to normalized sRGB.
         *
         * Extracts the red, green, and blue channels from a 0xRRGGBB
         * value and normalizes them to [0, 1].
         *
         * Does not perform sRGB-to-linear conversion.
         *
         * @param rgb Packed 24-bit RGB color in 0xRRGGBB format.
         * @return Normalized sRGB color with full opacity.
         */
        [[nodiscard]] auto hexRgbToSrgb(std::uint32_t rgb) -> ImVec4 {
            constexpr std::uint32_t kChannelMask{0xFFU};
            constexpr std::uint32_t kRedShift{16U};
            constexpr std::uint32_t kGreenShift{8U};
            constexpr float kMaxChannelValue{255.0F};

            const std::uint32_t red = (rgb >> kRedShift) & kChannelMask;
            const std::uint32_t green = (rgb >> kGreenShift) & kChannelMask;
            const std::uint32_t blue = rgb & kChannelMask;

            const float normalized_red =
                static_cast<float>(red) / kMaxChannelValue;
            const float normalized_green =
                static_cast<float>(green) / kMaxChannelValue;
            const float normalized_blue =
                static_cast<float>(blue) / kMaxChannelValue;

            return ImVec4{
                normalized_red,
                normalized_green,
                normalized_blue,
                1.0F,
            };
        }
    } // namespace

    auto applyEditorImGuiStyle(bool srgb_attachment, float dpi_scale) -> void {
        if (!std::isfinite(dpi_scale) || dpi_scale <= 0.0F) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "Editor ImGui style requires a finite, positive DPI scale");
        }

        ImGuiStyle style{};
        ImGui::StyleColorsDark(&style);

        const ImVec2 window_padding{8.0F, 8.0F};
        const ImVec2 frame_padding{6.0F, 4.0F};
        const float window_rounding{4.0F};
        const float frame_rounding{3.0F};

        const ImVec4 panel_background_srgb = hexRgbToSrgb(0x242830U);
        const ImVec4 menu_background_srgb = hexRgbToSrgb(0x191C22U);
        const ImVec4 tab_background_srgb = hexRgbToSrgb(0x313742U);
        const ImVec4 selected_tab_srgb = hexRgbToSrgb(0x385673U);
        const ImVec4 dimmed_selected_tab_srgb = hexRgbToSrgb(0x2D4056U);
        const ImVec4 hovered_tab_srgb = hexRgbToSrgb(0x456584U);

        constexpr float kBaseFontSize{16.0F};

        style.WindowPadding = window_padding;
        style.FramePadding = frame_padding;
        style.WindowRounding = window_rounding;
        style.FrameRounding = frame_rounding;

        style.Colors[ImGuiCol_WindowBg] = panel_background_srgb;
        style.Colors[ImGuiCol_MenuBarBg] = menu_background_srgb;
        style.Colors[ImGuiCol_Tab] = tab_background_srgb;
        style.Colors[ImGuiCol_TabDimmed] = tab_background_srgb;
        style.Colors[ImGuiCol_TabSelected] = selected_tab_srgb;
        style.Colors[ImGuiCol_TabDimmedSelected] = dimmed_selected_tab_srgb;
        style.Colors[ImGuiCol_TitleBg] = menu_background_srgb;
        style.Colors[ImGuiCol_TitleBgActive] = panel_background_srgb;
        style.Colors[ImGuiCol_TabHovered] = hovered_tab_srgb;

        style.FontSizeBase = kBaseFontSize;

        if (srgb_attachment) {
            for (ImVec4 &color : style.Colors) {
                color = srgbToLinearColor(color);
            }
        }

        style.ScaleAllSizes(dpi_scale);

        const ImGuiStyle &previous_style = ImGui::GetStyle();

        style.FontScaleDpi = previous_style.FontScaleDpi;
        style.FontScaleMain = previous_style.FontScaleMain;

        ImGui::GetStyle() = style;
    }
} // namespace SNE::Editor
