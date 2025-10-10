#pragma once

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

namespace PEREZLOG {
// spdlog wrapper
class Log {
  public:
    static void init();

    template <typename... Args>
    static void trace(fmt::format_string<Args...> fmt, Args &&...args) {
        spdlog::trace(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void debug(fmt::format_string<Args...> fmt, Args &&...args) {
        spdlog::debug(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void info(fmt::format_string<Args...> fmt, Args &&...args) {
        spdlog::info(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warn(fmt::format_string<Args...> fmt, Args &&...args) {
        spdlog::warn(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(fmt::format_string<Args...> fmt, Args &&...args) {
        spdlog::error(fmt, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void critical(fmt::format_string<Args...> fmt, Args &&...args) {
        spdlog::critical(fmt, std::forward<Args>(args)...);
    }
};

#define LOG_TRACE(...) PEREZLOG::Log::trace(__VA_ARGS__) // spdlog::trace(fmt)
#define LOG_DEBUG(...) PEREZLOG::Log::debug(__VA_ARGS__) // spdlog::debug(fmt)
#define LOG_INFO(...) PEREZLOG::Log::info(__VA_ARGS__)   // spdlog::info(fmt)
#define LOG_WARN(...) PEREZLOG::Log::warn(__VA_ARGS__)   // spdlog::warn(fmt)
#define LOG_ERROR(...) PEREZLOG::Log::error(__VA_ARGS__)    // spdlog::error(__VA_ARGS__)
#define LOG_CRITICAL(...) PEREZLOG::Log::critical(__VA_ARGS__) // spdlog::critical(fmt)

#define ASSERT_LOG(cond, msg, ...)                                                       \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            LOG_CRITICAL("Assertion failed:\n  expr : {}\n  msg  : " msg "\n  loc  : {}:{}" \
                      , #cond, ##__VA_ARGS__, __FILE__, __LINE__);                       \
            std::abort();                                                                \
        }                                                                                \
    } while (0)

}; // namespace PEREZLOG
