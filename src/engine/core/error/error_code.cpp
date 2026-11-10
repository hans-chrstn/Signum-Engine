#include "error_code.hpp"
#include "engine/core/config/compiler_features.hpp"
#include "error_metadata.hpp"

#if SNE_HAS_CPP26_REFLECTION
    #include <meta>
#endif

namespace SNE::Engine::Core::Error {
    auto toString(Code code) noexcept -> std::string_view {
        if (code == Code::Count) {
            return "Unknown";
        }

#if SNE_HAS_CPP26_REFLECTION
        template for (constexpr auto enumerator : std::define_static_array(
                          std::meta::enumerators_of(^^Code))) {
            static_assert(!std::meta::identifier_of(enumerator).empty());
            if (code == [:enumerator:]) {
                return std::meta::identifier_of(enumerator);
            }
        }
#else

        const ErrorMetadata *metadata = findErrorMetadata(code);
        if (metadata != nullptr) {
            return metadata->name;
        }
#endif
        return "Unknown";
    }
} // namespace SNE::Engine::Core::Error
