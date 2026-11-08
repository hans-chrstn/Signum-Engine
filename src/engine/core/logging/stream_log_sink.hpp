#pragma once
#include "log_sink.hpp"
#include <iosfwd>

namespace SNE::Engine::Core::Logging {
    /**
     * @brief Writes structured logging events to a borrowed output stream.
     *
     * Provides a concrete logging sink for stream-based destinations
     * such as terminals, files, and in-memory test streams.
     *
     * The output stream is not owned by this sink and must remain
     * valid throughout the sink's lifetime.
     *
     * Record formatting is delegated to the logging formatter,
     * keeping presentation logic separate from stream ownership.
     */
    class StreamLogSink final : public LogSink {
      private:
        /** Borrowed output stream used to write formatted logging events. */
        std::ostream &m_Output;

      public:
        /**
         * @brief Creates a sink that writes to the supplied output stream.
         *
         * @param output Destination stream, which must outlive this sink.
         */
        explicit StreamLogSink(std::ostream &output);

        /**
         * @brief Writes a formatted log record to the output stream.
         *
         * @param record Structured logging event to write.
         *
         * @throws Exceptions propagated from formatting or stream operations.
         */
        auto write(const LogRecord &record) -> void override;
    };
} // namespace SNE::Engine::Core::Logging
