#pragma once
#include "engine/platform/glfw_context.hpp"

namespace SNE::Engine::Core {
    class Application {
      private:
        Platform::GlfwContext m_GlfwContext;

      public:
        auto run() -> void;
    };
} // namespace SNE::Engine::Core
