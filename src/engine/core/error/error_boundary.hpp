#pragma once

#include <functional>
#include <ostream>
namespace SNE::Engine::Core::Error {
    /**
     * @brief Executes an operation within an application-level error boundary.
     *
     * Invokes the supplied operation and catches exceptions that escape from
     * it.
     *
     * EngineError exceptions are reported using the engine's structured
     * diagnostic writer. Other standard exceptions and unknown exceptions
     * are reported using fallback diagnostic messages.
     *
     * Each caught failure is reported once to the supplied output stream.
     *
     * @param operation Operation to execute.
     * @param diagnostics Output stream receiving failure diagnostics.
     *
     * @return EXIT_SUCCESS if the operation completes successfully, or
     * EXIT_FAILURE if an exception escapes the operation.
     *
     * @note This function handles uncaught runtime failures at an application
     * boundary. Programmer-contract violations handled by the assertion
     * system are not recoverable through this boundary.
     *
     * @throws Any exception raised while writing diagnostics to the output
     * stream that is not handled by the stream itself.
     */
    [[nodiscard]] auto
    runWithErrorBoundary(const std::function<void()> &operation,
                         std::ostream &diagnostics) -> int;
} // namespace SNE::Engine::Core::Error
