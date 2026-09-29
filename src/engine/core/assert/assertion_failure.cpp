#include "assertion_failure.hpp"
#include <utility>

namespace SNE::Engine::Core::Assertion {
    AssertionFailure::AssertionFailure(AssertionType assertion_type,
                                       Error::Subsystem subsystem,
                                       std::string message,
                                       std::source_location source_location)
        : m_Type(assertion_type), m_Subsystem(subsystem),
          m_Message(std::move(message)), m_SourceLocation(source_location) {}

    auto AssertionFailure::getType() const noexcept -> AssertionType {
        return m_Type;
    }

    auto AssertionFailure::getSubsystem() const noexcept -> Error::Subsystem {
        return m_Subsystem;
    }

    auto AssertionFailure::getMessage() const noexcept -> const std::string & {
        return m_Message;
    }

    auto AssertionFailure::getSourceLocation() const noexcept
        -> std::source_location {
        return m_SourceLocation;
    }
} // namespace SNE::Engine::Core::Assertion
