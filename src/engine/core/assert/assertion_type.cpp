#include "assertion_type.hpp"

namespace SNE::Engine::Core::Assertion {
    auto toString(AssertionType type) noexcept -> std::string_view {
        switch (type) {
        case AssertionType::Precondition:
            return "Precondition";
        case AssertionType::Postcondition:
            return "Postcondition";
        case AssertionType::Invariant:
            return "Invariant";
        }

        return "Unknown";
    }
} // namespace SNE::Engine::Core::Assertion
