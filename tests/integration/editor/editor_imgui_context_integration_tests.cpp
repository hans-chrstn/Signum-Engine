#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/platform/glfw_context.hpp"
#include "engine/platform/window.hpp"
#include "imgui/editor_imgui_context.hpp"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <imgui.h>

namespace Editor = SNE::Editor;
namespace Platform = SNE::Engine::Platform;
namespace Core = SNE::Engine::Core;

namespace {
    constexpr int kWidth = 320;
    constexpr int kHeight = 240;

    [[nodiscard]] auto hasDisplay() -> bool {
        return std::getenv("DISPLAY") != nullptr ||
               std::getenv("WAYLAND_DISPLAY") != nullptr;
    }

    // GLFW defines the callback parameter types and order.
    // NOLINTBEGIN(bugprone-easily-swappable-parameters)
    auto testKeyCallback([[maybe_unused]] GLFWwindow *window,
                         [[maybe_unused]] int key,
                         [[maybe_unused]] int scancode,
                         [[maybe_unused]] int action, [[maybe_unused]] int mods)
        -> void {} // NOLINTEND(bugprone-easily-swappable-parameters)
} // namespace

TEST(EditorImGuiContextIntegrationTests, RejectsNullWindow) {
    EXPECT_DEATH(static_cast<void>(Editor::EditorImGuiContext{
                     nullptr, SIGNUM_TEST_EDITOR_FONT_PATH}),
                 "EditorImGuiContext requires a non-null GLFW window");
}

TEST(EditorImGuiContextIntegrationTests, InitializesAndDestroysContext) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires a window-system display";
    }

    Platform::GlfwContext glfw_context{};
    Platform::Window window{
        {.width = kWidth, .height = kHeight},
        "Signum ImGui Integration Test",
    };

    ASSERT_EQ(ImGui::GetCurrentContext(), nullptr);

    {
        Editor::EditorImGuiContext context{window.nativeHandle(),
                                           SIGNUM_TEST_EDITOR_FONT_PATH};

        ASSERT_NE(ImGui::GetCurrentContext(), nullptr);

        ImGuiIO &imgui_io = ImGui::GetIO();

        EXPECT_NE(imgui_io.BackendPlatformUserData, nullptr);
        EXPECT_NE(imgui_io.BackendPlatformName, nullptr);

        const auto config_flags =
            static_cast<unsigned int>(imgui_io.ConfigFlags);

        EXPECT_NE(config_flags & static_cast<unsigned int>(
                                     ImGuiConfigFlags_NavEnableKeyboard),
                  0U);

        EXPECT_NE(config_flags &
                      static_cast<unsigned int>(ImGuiConfigFlags_DockingEnable),
                  0U);
    }

    EXPECT_EQ(ImGui::GetCurrentContext(), nullptr);
}

TEST(EditorImGuiContextIntegrationTests, RestoresGlfwKeyCallbackOnDestruction) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires a window-system display";
    }

    Platform::GlfwContext glfw_context{};
    Platform::Window window{
        {.width = kWidth, .height = kHeight},
        "Signum ImGui Callback Integration Test",
    };

    glfwSetKeyCallback(window.nativeHandle(), testKeyCallback);

    {
        Editor::EditorImGuiContext context{window.nativeHandle(),
                                           SIGNUM_TEST_EDITOR_FONT_PATH};
    }

    const GLFWkeyfun restored_callback =
        glfwSetKeyCallback(window.nativeHandle(), nullptr);

    EXPECT_EQ(restored_callback, testKeyCallback);
}

TEST(EditorImGuiContextIntegrationTests, MissingFontThrowsAndDestroysContext) {
    if (!hasDisplay()) {
        GTEST_SKIP() << "Requires a window-system display";
    }

    Platform::GlfwContext glfw_context{};

    Platform::Window window{
        {.width = kWidth, .height = kHeight},
        "Signum ImGui Missing Font Test",
    };

    const std::filesystem::path missing_font_path =
        std::filesystem::path{SIGNUM_TEST_EDITOR_FONT_PATH}.parent_path() /
        "signum-intentionally-missing-font.ttf";

    ASSERT_FALSE(std::filesystem::exists(missing_font_path));
    ASSERT_EQ(ImGui::GetCurrentContext(), nullptr);

    try {
        [[maybe_unused]] Editor::EditorImGuiContext context{
            window.nativeHandle(), missing_font_path};

        FAIL() << "Expected font initialization to fail";
    } catch (const Core::Error::EngineError &error) {
        EXPECT_EQ(error.getCode(), Core::Error::Code::EditorFontLoadFailed);
    }

    EXPECT_EQ(ImGui::GetCurrentContext(), nullptr);
}
