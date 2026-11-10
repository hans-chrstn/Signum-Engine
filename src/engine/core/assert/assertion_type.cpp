#include "assertion_type.hpp"
#include "engine/core/config/compiler_features.hpp"

#if SNE_HAS_CPP26_REFLECTION
    #include <meta>
#endif

namespace SNE::Engine::Core::Assertion {
    auto toString(AssertionType type) noexcept -> std::string_view {
#if SNE_HAS_CPP26_REFLECTION
        template for (constexpr auto enumerator : std::define_static_array(
                          std::meta::enumerators_of(^^AssertionType))) {
            static_assert(!std::meta::identifier_of(enumerator).empty());
            if (type == [:enumerator:]) {
                return std::meta::identifier_of(enumerator);
            }
        }
#else
        switch (type) {
        case AssertionType::Precondition:
            return "Precondition";
        case AssertionType::Postcondition:
            return "Postcondition";
        case AssertionType::Invariant:
            return "Invariant";
        }

#endif
        return "Unknown";
    }
} // namespace SNE::Engine::Core::Assertion
