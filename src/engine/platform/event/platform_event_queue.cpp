#include "platform_event_queue.hpp"
#include <utility>
#include <vector>

namespace SNE::Engine::Platform {
    auto PlatformEventQueue::push(PlatformEvent event) -> void {
        m_Events.emplace_back(event);
    }

    auto PlatformEventQueue::takeAll() -> std::vector<PlatformEvent> {
        return std::exchange(m_Events, {});
    }
} // namespace SNE::Engine::Platform
