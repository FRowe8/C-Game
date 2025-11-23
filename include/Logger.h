#pragma once

#include <string>
#include <sstream>

namespace Log {

enum class Level {
    Debug,
    Info,
    Warning,
    Error
};

void Init();
void Shutdown();
void SetLevel(Level level);

void Debug(const std::string& message);
void Info(const std::string& message);
void Warning(const std::string& message);
void Error(const std::string& message);

// Template for easy formatting
template<typename... Args>
void Debugf(Args&&... args) {
    std::ostringstream oss;
    (oss << ... << args);
    Debug(oss.str());
}

template<typename... Args>
void Infof(Args&&... args) {
    std::ostringstream oss;
    (oss << ... << args);
    Info(oss.str());
}

template<typename... Args>
void Warningf(Args&&... args) {
    std::ostringstream oss;
    (oss << ... << args);
    Warning(oss.str());
}

template<typename... Args>
void Errorf(Args&&... args) {
    std::ostringstream oss;
    (oss << ... << args);
    Error(oss.str());
}

} // namespace Log
