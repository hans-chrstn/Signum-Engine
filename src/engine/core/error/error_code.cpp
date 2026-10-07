#include "error_code.hpp"
#include "error_metadata.hpp"

namespace SNE::Engine::Core::Error {
    auto toString(Code code) noexcept -> std::string_view {
        const ErrorMetadata *metadata = findErrorMetadata(code);
        if (metadata != nullptr) {
            return metadata->name;
        }

        return "Unknown";
    }
} // namespace SNE::Engine::Core::Error
