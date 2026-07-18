#pragma once

// Structured console logger with severity levels.
// Thread safety: not required (the forwarder is single-threaded by design).

#include <string>

namespace forwarder {

enum class LogLevel {
    Trace,
    Debug,
    Info,
    Warn,
    Error
};

/// Set the global minimum log level. Messages below this threshold are discarded.
void set_log_level(LogLevel level);

/// Get the current global minimum log level.
LogLevel get_log_level();

/// Convert a string name to LogLevel. Case-sensitive.
/// Accepted: "trace", "debug", "info", "warn", "error".
/// Unknown values default to LogLevel::Info with a warning to stderr.
LogLevel log_level_from_string(const std::string& name);

/// Convert LogLevel to its string name.
const char* log_level_to_string(LogLevel level);

/// Core log function. All convenience macros delegate to this.
/// Format: [YYYY-MM-DD HH:MM:SS.mmm] [LEVEL] (file:line) message
/// Output: Trace/Debug -> stdout, Info/Warn/Error -> stderr.
void log_message(LogLevel level, const char* file, int line, const std::string& message);

} // namespace forwarder

// Convenience macros that capture __FILE__ and __LINE__ automatically.
// Usage: LOG_INFO("server started on port " + std::to_string(port));

#define LOG_TRACE(msg) forwarder::log_message(forwarder::LogLevel::Trace, __FILE__, __LINE__, (msg))
#define LOG_DEBUG(msg) forwarder::log_message(forwarder::LogLevel::Debug, __FILE__, __LINE__, (msg))
#define LOG_INFO(msg)  forwarder::log_message(forwarder::LogLevel::Info,  __FILE__, __LINE__, (msg))
#define LOG_WARN(msg)  forwarder::log_message(forwarder::LogLevel::Warn,  __FILE__, __LINE__, (msg))
#define LOG_ERROR(msg) forwarder::log_message(forwarder::LogLevel::Error, __FILE__, __LINE__, (msg))
