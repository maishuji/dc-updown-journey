// Copyright 2025 Quentin Cartier

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include <utility>

namespace udjourney {

class Logger {
 public:
    enum class Level { Info, Warning, Error, Debug };

    template <typename... Args>
    static void info(const std::string& format, Args&&... args) {
        log(Level::Info, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void warning(const std::string& format, Args&&... args) {
        log(Level::Warning, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(const std::string& format, Args&&... args) {
        log(Level::Error, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void debug(const std::string& format, Args&&... args) {
        log(Level::Debug, format, std::forward<Args>(args)...);
    }

 private:
    template <typename... Args>
    static void log(Level level, const std::string& format, Args&&... args) {
        std::string prefix;
        switch (level) {
            case Level::Info:
                prefix = "[INFO] ";
                break;
            case Level::Warning:
                prefix = "[WARN] ";
                break;
            case Level::Error:
                prefix = "[ERROR] ";
                break;
            case Level::Debug:
                prefix = "[DEBUG] ";
                break;
        }

        std::string message =
            format_message(format, std::forward<Args>(args)...);
        std::cout << prefix << message << std::endl;
    }

    template <typename T>
    static std::string format_message(const std::string& format, T&& arg) {
        size_t pos = format.find('%');
        if (pos != std::string::npos) {
            std::ostringstream oss;
            oss << format.substr(0, pos) << arg << format.substr(pos + 1);
            return oss.str();
        }
        return format;
    }

    template <typename T, typename... Args>
    static std::string format_message(const std::string& format, T&& first,
                                      Args&&... rest) {
        size_t pos = format.find('%');
        if (pos != std::string::npos) {
            std::ostringstream oss;
            oss << format.substr(0, pos) << first;
            std::string remaining = format.substr(pos + 1);
            return oss.str() +
                   format_message(remaining, std::forward<Args>(rest)...);
        }
        return format;
    }

    static std::string format_message(const std::string& format) {
        return format;
    }
};

}  // namespace udjourney
