#include "error_boundary.hpp"
#include "diagnostic_writer.hpp"
#include "engine_error.hpp"
#include <cstdlib>
#include <exception>

namespace SNE::Engine::Core::Error {
    auto runWithErrorBoundary(const std::function<void()> &operation,
                              std::ostream &diagnostics) -> int {
        try {
            operation();
        } catch (const EngineError &err) {
            writeDiagnostic(diagnostics, err);
            return EXIT_FAILURE;
        } catch (const std::exception &err) {
            diagnostics << "Unexpected standard exception: " << err.what()
                        << '\n';
            return EXIT_FAILURE;
        } catch (...) {
            diagnostics << "Unknown exception reached application boundary\n";
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
} // namespace SNE::Engine::Core::Error
