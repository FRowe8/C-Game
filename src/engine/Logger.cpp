#include "Logger.h"
#include <iostream>
#include <ctime>
#include <iomanip>

namespace Log {

static Level s_LogLevel = Level::Debug;

void Init() {
    Info("Logger initialized");
}

void Shutdown() {
    Info("Logger shutdown");
}

void SetLevel(Level level) {
    s_LogLevel = level;
}

static std::string GetTimestamp() {
    auto now = std::time(nullptr);
    auto tm = *std::localtime(&now);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%H:%M:%S");
    return oss.str();
}

static const char* LevelToString(Level level) {
    switch (level) {
        case Level::Debug:   return "DEBUG";
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error:   return "ERROR";
    }
    return "UNKNOWN";
}

static void LogMessage(Level level, const std::string& message) {
    if (level < s_LogLevel) return;

    std::ostringstream oss;
    oss << "[" << GetTimestamp() << "] "
        << "[" << LevelToString(level) << "] "
        << message;

    if (level == Level::Error) {
        std::cerr << oss.str() << std::endl;
    } else {
        std::cout << oss.str() << std::endl;
    }
}

void Debug(const std::string& message) {
    LogMessage(Level::Debug, message);
}

void Info(const std::string& message) {
    LogMessage(Level::Info, message);
}

void Warning(const std::string& message) {
    LogMessage(Level::Warning, message);
}

void Error(const std::string& message) {
    LogMessage(Level::Error, message);
}

} // namespace Log
