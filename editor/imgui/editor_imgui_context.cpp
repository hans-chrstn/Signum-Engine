#include "editor_imgui_context.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include "engine/core/error/subsystem.hpp"
#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>
#include <string>

namespace Assertion = SNE::Engine::Core::Assertion;
namespace Error = SNE::Engine::Core::Error;

namespace SNE::Editor {
    EditorImGuiContext::EditorImGuiContext(
        GLFWwindow *window, const std::filesystem::path &font_path) {
        if (window == nullptr) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition,
                Error::Subsystem::Editor,
                "EditorImGuiContext requires a non-null GLFW window");
        }

        IMGUI_CHECKVERSION();

        m_Context = ImGui::CreateContext();

        ImGui::SetCurrentContext(m_Context);

        ImGuiIO &gui_io = ImGui::GetIO();

        gui_io.ConfigDpiScaleFonts = true;

        const std::string font_filename = font_path.string();

        ImFontConfig font_config{};
        font_config.Flags = ImFontFlags_NoLoadError;

        ImFont *inter = gui_io.Fonts->AddFontFromFileTTF(font_filename.c_str(),
                                                         0.0F, &font_config);

        if (inter == nullptr) {
            ImGui::DestroyContext(m_Context);

            throw Error::EngineError(Error::Code::EditorFontLoadFailed,
                                     "Failed to load the editor UI font",
                                     "ImFontAtlas::AddFontFromFileTTF");
        }

        auto imgui_flags = static_cast<unsigned int>(gui_io.ConfigFlags);
        imgui_flags |=
            static_cast<unsigned int>(ImGuiConfigFlags_NavEnableKeyboard) |
            static_cast<unsigned int>(ImGuiConfigFlags_DockingEnable);

        gui_io.ConfigFlags = static_cast<ImGuiConfigFlags>(imgui_flags);

        const bool imgui_result = ImGui_ImplGlfw_InitForVulkan(window, true);

        if (!imgui_result) {
            ImGui::DestroyContext(m_Context);

            throw Error::EngineError(
                Error::Code::EditorGlfwBackendInitializationFailed,
                "Failed to initialize the Dear ImGui GLFW platform backend",
                "ImGui_ImplGlfw_InitForVulkan");
        }
    }

    EditorImGuiContext::~EditorImGuiContext() {
        ImGui::SetCurrentContext(m_Context);

        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext(m_Context);
    }
} // namespace SNE::Editor
