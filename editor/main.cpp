#include "engine/core/application.hpp"
#include "engine/core/error/error_boundary.hpp"
#include <iostream>

auto main() -> int {
    return SNE::Engine::Core::Error::runWithErrorBoundary(
        [] -> void {
            SNE::Engine::Core::Application application;
            application.run();
        },
        std::cerr);
}
