#include "engine/core/application.hpp"
#include <exception>
#include <iostream>

auto main() -> int {
    try {
        SNE::Engine::Core::Application application;
        application.run();
    } catch (const std::exception &e) {
        std::cerr << "[FATAL] " << e.what() << "\n";
        return 1;
    }
    return 0;
}
