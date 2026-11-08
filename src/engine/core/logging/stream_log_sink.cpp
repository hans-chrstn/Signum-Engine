#include "stream_log_sink.hpp"
#include "log_formatter.hpp"
#include <ostream>

namespace SNE::Engine::Core::Logging {
    StreamLogSink::StreamLogSink(std::ostream &output) : m_Output(output) {}

    auto StreamLogSink::write(const LogRecord &record) -> void {
        m_Output << formatLogRecord(record) << '\n';
    }
} // namespace SNE::Engine::Core::Logging
