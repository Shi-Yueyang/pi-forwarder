#include "log.hpp"

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace forwarder {

namespace {

// Global log level, default to Info.
LogLevel g_log_level = LogLevel::Info;

/// Truncate the file path to just the filename portion (after the last separator).
const char* short_filename(const char* path)
{
#if defined(_WIN32)
    const char sep = '\\';
#else
    const char sep = '/';
#endif
    const char* last = std::strrchr(path, sep);
    if (last != nullptr) {
        return last + 1;
    }
    // Also check forward slash on Windows (for consistency)
#if defined(_WIN32)
    last = std::strrchr(path, '/');
    if (last != nullptr) {
        return last + 1;
    }
#endif
    return path;
}

} // anonymous namespace

void set_log_level(LogLevel level)
{
    g_log_level = level;
}

LogLevel get_log_level()
{
    return g_log_level;
}

LogLevel log_level_from_string(const std::string& name)
{
    if (name == "trace") { return LogLevel::Trace; }
    if (name == "debug") { return LogLevel::Debug; }
    if (name == "info")  { return LogLevel::Info; }
    if (name == "warn")  { return LogLevel::Warn; }
    if (name == "error") { return LogLevel::Error; }

    // Unknown value: default to Info and warn.
    std::cerr << "[WARN] Unknown log level '" << name << "', defaulting to 'info'\n";
    return LogLevel::Info;
}

const char* log_level_to_string(LogLevel level)
{
    switch (level) {
    case LogLevel::Trace: return "TRACE";
    case LogLevel::Debug: return "DEBUG";
    case LogLevel::Info:  return "INFO";
    case LogLevel::Warn:  return "WARN";
    case LogLevel::Error: return "ERROR";
    }
    return "UNKNOWN";
}

void log_message(LogLevel level, const char* file, int line, const std::string& message)
{
    // Discard messages below the current threshold.
    if (level < g_log_level) {
        return;
    }

    // Build timestamp.
    auto now = std::chrono::system_clock::now();
    auto tt = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  now.time_since_epoch()) % 1000;

    // localtime_s on Windows, localtime_r on POSIX.
    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &tt);
#else
    localtime_r(&tt, &tm_buf);
#endif

    std::ostringstream oss;
    oss << '[' << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setw(3) << std::setfill('0') << ms.count() << ']'
        << " [" << log_level_to_string(level) << ']'
        << " (" << short_filename(file) << ':' << line << ')'
        << ' ' << message << '\n';

    // Trace/Debug -> stdout; Info/Warn/Error -> stderr.
    const std::string& line_str = oss.str();
    if (level <= LogLevel::Debug) {
        std::cout << line_str << std::flush;
    } else {
        std::cerr << line_str << std::flush;
    }
}

} // namespace forwarder
