#pragma once

#include "engine/platform/event/platform_event.hpp"
#include <vector>

namespace SNE::Engine::Platform {
    /**
     * @brief Stores pending backend-independent platform events.
     *
     * Owns an ordered collection of platform events awaiting
     * processing by the application.
     *
     * Events are appended in arrival order and transferred to
     * consumers as a batch.
     *
     * The queue does not perform native event polling, input-state
     * tracking, or platform-specific event translation.
     *
     * Each platform window may own an independent event queue.
     *
     * This class is not thread-safe. Producers and consumers must
     * synchronize access externally when operating across threads.
     */
    class PlatformEventQueue {
      private:
        /**
         * @brief Pending platform events in their original arrival order.
         *
         * Events remain stored until transferred by takeAll().
         */
        std::vector<PlatformEvent> m_Events;

      public:
        /**
         * @brief Appends a platform event to the queue.
         *
         * Transfers the supplied event into the pending event collection
         * without changing the order of previously queued events.
         *
         * @param event Platform event to enqueue.
         *
         * @post On success, the event is appended to the queue.
         *
         * @throws std::bad_alloc if event storage allocation fails.
         */
        auto push(PlatformEvent event) -> void;

        /**
         * @brief Retrieves and removes all pending platform events.
         *
         * Transfers the current event collection to the caller while
         * preserving the original arrival order.
         *
         * The queue becomes empty and remains available for subsequent
         * event insertion.
         *
         * @return Vector containing all previously pending events,
         * or an empty vector when no events were queued.
         *
         * @post The queue contains no pending events.
         */
        [[nodiscard]] auto takeAll() -> std::vector<PlatformEvent>;
    };
} // namespace SNE::Engine::Platform
