#include "backlog_policy_controller.hpp"
#include "engine/core/assert/assertion_handler.hpp"
#include "engine/core/assert/assertion_type.hpp"
#include "engine/core/error/subsystem.hpp"
#include <utility>

namespace SNE::Engine::Core::Time {
    BacklogPolicyController::BacklogPolicyController(
        BacklogPolicy initial_policy)
        : m_Active(std::move(initial_policy)) {
        if (!m_Active) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "BacklogPolicyController requires a valid initial policy");
        }
    }

    auto BacklogPolicyController::commitPendingPolicy() noexcept -> bool {
        if (!m_Pending.has_value()) {
            return false;
        }

        m_Active = std::move(*m_Pending);

        m_Pending.reset();

        return true;
    }

    auto BacklogPolicyController::requestPolicy(BacklogPolicy policy) -> void {
        if (!policy) {
            Assertion::failAssertion(
                Assertion::AssertionType::Precondition, Error::Subsystem::Core,
                "BacklogPolicyController requires a valid requested policy");
        }

        m_Pending.emplace(std::move(policy));
    }

    auto BacklogPolicyController::activePolicy() const noexcept
        -> const BacklogPolicy & {
        return m_Active;
    }
} // namespace SNE::Engine::Core::Time
