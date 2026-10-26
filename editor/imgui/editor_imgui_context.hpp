#pragma once

#include <GLFW/glfw3.h>
#include <filesystem>

struct ImGuiContext;

namespace SNE::Editor {

    /**
     * @brief Owns the Dear ImGui context and GLFW platform backend.
     *
     * Initializes the editor's ImGui context and connects its input
     * handling to an existing GLFW window.
     *
     * The GLFW window is borrowed and must outlive this object.
     *
     * Vulkan rendering backend initialization is managed separately.
     */
    class EditorImGuiContext {
      private:
        /**
         * @brief Dear ImGui context owned by the editor.
         *
         * Identifies the context created during initialization and destroyed
         * during shutdown.
         *
         * nullptr indicates that no context is currently owned.
         *
         * The context must be destroyed through ImGui::DestroyContext(),
         * not through delete.
         */
        ImGuiContext *m_Context = nullptr;

      public:
        /**
         * @brief Initializes Dear ImGui for an existing GLFW window.
         *
         * Creates the ImGui context, loads the editor's UI font,
         * and initializes the GLFW platform backend.
         *
         * The font is registered once during initialization.
         * The supplied font path is not retained.
         *
         * @param window Borrowed pointer to a valid GLFW window.
         * @param font_path Path to the editor's TrueType font file.
         *
         * @pre window must not be nullptr.
         * @pre GLFW must remain initialized throughout this object's lifetime.
         *
         * @throws SNE::Engine::Core::Error::EngineError if the
         * editor font cannot be loaded or GLFW backend initialization fails.
         */
        explicit EditorImGuiContext(GLFWwindow *window,
                                    const std::filesystem::path &font_path);

        /**
         * @brief Shuts down the GLFW backend and destroys the ImGui context.
         *
         * Releases ImGui resources without destroying the borrowed window.
         */
        ~EditorImGuiContext();

        EditorImGuiContext(const EditorImGuiContext &) = delete;
        auto operator=(const EditorImGuiContext &)
            -> EditorImGuiContext & = delete;

        EditorImGuiContext(EditorImGuiContext &&) = delete;
        auto operator=(EditorImGuiContext &&) -> EditorImGuiContext & = delete;
    };
} // namespace SNE::Editor
