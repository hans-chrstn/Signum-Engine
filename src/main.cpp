#include "engine/core/application.hpp"
#include "engine/core/error/diagnostic_writer.hpp"
#include "engine/core/error/engine_error.hpp"
#include "engine/core/error/error_code.hpp"
#include <cstdlib>
#include <iostream>

auto main() -> int {
    try {
        SNE::Engine::Core::Application application;
        application.run();
    } catch (const SNE::Engine::Core::Error::EngineError &engine_error) {
        SNE::Engine::Core::Error::writeDiagnostic(std::cerr, engine_error);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
