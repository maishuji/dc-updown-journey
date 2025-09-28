// Copyright 2025 Quentin Cartier
#pragma once
#include <atomic>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

namespace Logger {

enum class Level { Info, Debug, Warn, Error, Critical, None };

inline std::atomic<Level> current_level{Level::Info};
inline std::unique_ptr<std::ofstream> file_stream = nullptr;

// Log prefixes
constexpr const char* kLogPrefixInfo = "[INFO] ";
constexpr const char* kLogPrefixDebug = "[DEBUG] ";
constexpr const char* kLogPrefixWarn = "[WARN] ";
constexpr const char* kLogPrefixError = "[ERROR] ";
constexpr const char* kLogPrefixCritical = "[CRITICAL] ";

inline void set_level(Level level) { current_level.store(level); }

inline void set_output_file(const std::string& filename) {
    file_stream = std::make_unique<std::ofstream>(filename, std::ios::app);
}

// Helper to get output stream
inline std::ostream& get_output_stream(Level level) {
    if (file_stream && file_stream->is_open()) {
        return *file_stream;
    }
    return (level == Level::Error || level == Level::Critical) ? std::cerr
                                                               : std::cout;
}

// Helper for parameter expansion - corrected version
template <typename T, typename... Rest>
static void log_impl(std::ostream& output, const char* msg, T value,
                     Rest... rest) {
    while (*msg) {
        if (*msg == '%' && *(++msg) != '%') {
            output << value;
            log_impl(output, msg, rest...);
            return;
        }
        output << *msg++;
    }
    output << std::endl;
}

static void log_impl(std::ostream& output, const char* msg) {
    while (*msg != '\0') {
        output << *msg++;
    }
    output << std::endl;
}

// Main logging function with variadic template
template <typename T, typename... Rest>
static void log(Level log_level, const char* msg, T value, Rest... rest) {
    if (current_level.load() <= log_level) {
        auto& output = get_output_stream(log_level);

        switch (log_level) {
            case Level::Info:
                output << kLogPrefixInfo;
                break;
            case Level::Debug:
                output << kLogPrefixDebug;
                break;
            case Level::Warn:
                output << kLogPrefixWarn;
                break;
            case Level::Error:
                output << kLogPrefixError;
                break;
            case Level::Critical:
                output << kLogPrefixCritical;
                break;
            case Level::None:
                break;
        }

        while (*msg) {
            if (*msg == '%' && *(++msg) != '%') {
                output << value;
                log_impl(output, msg, rest...);
                return;
            }
            output << *msg++;
        }
        output << std::endl;
    }
}

// Base case for recursion
static void log(Level log_level, const char* msg) {
    if (current_level.load() <= log_level) {
        auto& output = get_output_stream(log_level);

        switch (log_level) {
            case Level::Info:
                output << kLogPrefixInfo;
                break;
            case Level::Debug:
                output << kLogPrefixDebug;
                break;
            case Level::Warn:
                output << kLogPrefixWarn;
                break;
            case Level::Error:
                output << kLogPrefixError;
                break;
            case Level::Critical:
                output << kLogPrefixCritical;
                break;
            case Level::None:
                break;
        }
        output << msg << std::endl;
    }
}

// String version
static void log(Level log_level, const std::string& msg) {
    log(log_level, msg.c_str());
}

// Convenience functions
template <typename... Args> inline void info(const char* msg, Args&&... args) {
    log(Level::Info, msg, std::forward<Args>(args)...);
}

template <typename... Args> inline void debug(const char* msg, Args&&... args) {
    log(Level::Debug, msg, std::forward<Args>(args)...);
}

template <typename... Args> inline void warn(const char* msg, Args&&... args) {
    log(Level::Warn, msg, std::forward<Args>(args)...);
}

template <typename... Args> inline void error(const char* msg, Args&&... args) {
    log(Level::Error, msg, std::forward<Args>(args)...);
}

template <typename... Args>
inline void critical(const char* msg, Args&&... args) {
    log(Level::Critical, msg, std::forward<Args>(args)...);
}
}  // namespace Logger