#include "imgui/editor_imgui_style.hpp"
#include <array>
#include <cmath>
#include <cstddef>
#include <gtest/gtest.h>
#include <imgui.h>
#include <limits>

namespace Editor = SNE::Editor;

namespace {
    [[nodiscard]] auto expectedSrgbToLinear(float channel) -> float {
        constexpr float kThreshold{0.04045F};
        constexpr float kLinearDivisor{12.92F};
        constexpr float kOffset{0.055F};
        constexpr float kScale{1.055F};
        constexpr float kExponent{2.4F};

        if (channel <= kThreshold) {
            return channel / kLinearDivisor;
        }

        return std::pow((channel + kOffset) / kScale, kExponent);
    }
} // namespace

class EditorImGuiStyleIntegrationTests : public ::testing::Test {
  protected:
    void SetUp() override {
        ASSERT_EQ(ImGui::GetCurrentContext(), nullptr);
        m_Context = ImGui::CreateContext();
        ASSERT_NE(m_Context, nullptr);
    }

    void TearDown() override {
        if (m_Context != nullptr) {
            ImGui::DestroyContext(m_Context);
        }
    }

  private:
    ImGuiContext *m_Context = nullptr;
};

TEST_F(EditorImGuiStyleIntegrationTests, ScalesLayoutWithoutAccumulation) {
    constexpr float kDpiScale = 1.5F;

    Editor::applyEditorImGuiStyle(false, kDpiScale);

    EXPECT_FLOAT_EQ(ImGui::GetStyle().WindowPadding.x, 12.0F);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().FramePadding.x, 9.0F);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().WindowRounding, 6.0F);

    Editor::applyEditorImGuiStyle(false, kDpiScale);

    EXPECT_FLOAT_EQ(ImGui::GetStyle().WindowPadding.x, 12.0F);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().FramePadding.x, 9.0F);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().WindowRounding, 6.0F);
}

TEST_F(EditorImGuiStyleIntegrationTests, PreservesFontScaleValues) {
    constexpr float kFontScaleDpi{1.25F};
    constexpr float kFontScaleMain{0.875F};
    constexpr float kInitialDpiScale{1.5F};
    constexpr float kUpdatedDpiScale{2.0F};

    ImGuiStyle &style = ImGui::GetStyle();

    style.FontScaleDpi = kFontScaleDpi;
    style.FontScaleMain = kFontScaleMain;

    Editor::applyEditorImGuiStyle(false, kInitialDpiScale);

    EXPECT_FLOAT_EQ(ImGui::GetStyle().FontScaleDpi, kFontScaleDpi);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().FontScaleMain, kFontScaleMain);

    Editor::applyEditorImGuiStyle(false, kUpdatedDpiScale);

    EXPECT_FLOAT_EQ(ImGui::GetStyle().FontScaleDpi, kFontScaleDpi);
    EXPECT_FLOAT_EQ(ImGui::GetStyle().FontScaleMain, kFontScaleMain);
}

TEST_F(EditorImGuiStyleIntegrationTests, RejectsZeroDpiScale) {
    constexpr float kInvalidDpiScale{0.0F};

    EXPECT_DEATH(Editor::applyEditorImGuiStyle(false, kInvalidDpiScale), ".*");
}

TEST_F(EditorImGuiStyleIntegrationTests, RejectsNegativeDpiScale) {
    constexpr float kInvalidDpiScale{-1.0F};

    EXPECT_DEATH(Editor::applyEditorImGuiStyle(false, kInvalidDpiScale), ".*");
}

TEST_F(EditorImGuiStyleIntegrationTests, RejectsNanDpiScale) {
    const float invalid_dpi_scale = std::numeric_limits<float>::quiet_NaN();

    EXPECT_DEATH(Editor::applyEditorImGuiStyle(false, invalid_dpi_scale), ".*");
}

TEST_F(EditorImGuiStyleIntegrationTests, RejectsPositiveInfinityDpiScale) {
    const float invalid_dpi_scale = std::numeric_limits<float>::infinity();

    EXPECT_DEATH(Editor::applyEditorImGuiStyle(false, invalid_dpi_scale), ".*");
}

TEST_F(EditorImGuiStyleIntegrationTests, RejectsNegativeInfinityDpiScale) {
    const float invalid_dpi_scale = -std::numeric_limits<float>::infinity();

    EXPECT_DEATH(Editor::applyEditorImGuiStyle(false, invalid_dpi_scale), ".*");
}

TEST_F(EditorImGuiStyleIntegrationTests, ConvertsColorsForSrgbAttachments) {
    constexpr float kDpiScale{1.0F};
    constexpr float kColorTolerance{0.00001F};

    constexpr std::array<int, 3> kColorIndices{
        ImGuiCol_Text,
        ImGuiCol_WindowBg,
        ImGuiCol_Button,
    };

    std::array<ImVec4, kColorIndices.size()> unorm_colors{};

    Editor::applyEditorImGuiStyle(false, kDpiScale);

    for (std::size_t index = 0; index < kColorIndices.size(); ++index) {
        unorm_colors[index] = ImGui::GetStyle().Colors[kColorIndices[index]];
    }

    Editor::applyEditorImGuiStyle(true, kDpiScale);

    for (std::size_t index = 0; index < kColorIndices.size(); ++index) {
        SCOPED_TRACE(kColorIndices[index]);

        const ImVec4 &original = unorm_colors[index];
        const ImVec4 &converted =
            ImGui::GetStyle().Colors[kColorIndices[index]];

        EXPECT_NEAR(converted.x, expectedSrgbToLinear(original.x),
                    kColorTolerance);

        EXPECT_NEAR(converted.y, expectedSrgbToLinear(original.y),
                    kColorTolerance);

        EXPECT_NEAR(converted.z, expectedSrgbToLinear(original.z),
                    kColorTolerance);

        EXPECT_FLOAT_EQ(converted.w, original.w);
    }
}

TEST_F(EditorImGuiStyleIntegrationTests, ReapplyingSrgbStyleDoesNotAccumulate) {
    constexpr float kDpiScale{1.0F};

    Editor::applyEditorImGuiStyle(true, kDpiScale);

    const ImVec4 initial_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

    Editor::applyEditorImGuiStyle(true, kDpiScale);

    const ImVec4 reapplied_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

    EXPECT_FLOAT_EQ(reapplied_color.x, initial_color.x);
    EXPECT_FLOAT_EQ(reapplied_color.y, initial_color.y);
    EXPECT_FLOAT_EQ(reapplied_color.z, initial_color.z);
    EXPECT_FLOAT_EQ(reapplied_color.w, initial_color.w);
}

TEST_F(EditorImGuiStyleIntegrationTests,
       SwitchingFromSrgbToUnormRestoresOriginalColors) {
    constexpr float kDpiScale{1.0F};

    Editor::applyEditorImGuiStyle(false, kDpiScale);

    const ImVec4 original_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

    Editor::applyEditorImGuiStyle(true, kDpiScale);
    Editor::applyEditorImGuiStyle(false, kDpiScale);

    const ImVec4 restored_color = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];

    EXPECT_FLOAT_EQ(restored_color.x, original_color.x);
    EXPECT_FLOAT_EQ(restored_color.y, original_color.y);
    EXPECT_FLOAT_EQ(restored_color.z, original_color.z);
    EXPECT_FLOAT_EQ(restored_color.w, original_color.w);
}
