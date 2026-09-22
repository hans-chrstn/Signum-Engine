#include "engine/core/application.hpp"
#include <exception>
#include <iostream>

int main() {
    try {
        SNE::Engine::Core::Application application;
        application.run();
    } catch (const std::exception &e) {
        std::cerr << "[FATAL] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
