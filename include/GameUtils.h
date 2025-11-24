#pragma once

#include "Types.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>

// Utility functions for the game

namespace GameUtils {

// Number formatting
inline std::string FormatNumber(f64 num) {
    if (num < 1000.0) {
        return std::to_string(static_cast<i32>(num));
    } else if (num < 1000000.0) {
        f64 thousands = num / 1000.0;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << thousands << "K";
        return ss.str();
    } else if (num < 1000000000.0) {
        f64 millions = num / 1000000.0;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << millions << "M";
        return ss.str();
    } else if (num < 1000000000000.0) {
        f64 billions = num / 1000000000.0;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << billions << "B";
        return ss.str();
    } else {
        f64 trillions = num / 1000000000000.0;
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << trillions << "T";
        return ss.str();
    }
}

// Format time (seconds to readable format)
inline std::string FormatTime(f64 seconds) {
    i32 totalSeconds = static_cast<i32>(seconds);
    i32 hours = totalSeconds / 3600;
    i32 minutes = (totalSeconds % 3600) / 60;
    i32 secs = totalSeconds % 60;

    std::ostringstream ss;
    if (hours > 0) {
        ss << hours << "h " << minutes << "m";
    } else if (minutes > 0) {
        ss << minutes << "m " << secs << "s";
    } else {
        ss << secs << "s";
    }
    return ss.str();
}

// Format percentage
inline std::string FormatPercent(f64 value) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(0) << (value * 100.0) << "%";
    return ss.str();
}

// Clamp value between min and max
template<typename T>
inline T Clamp(T value, T min, T max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Linear interpolation
inline f64 Lerp(f64 a, f64 b, f64 t) {
    return a + (b - a) * Clamp(t, 0.0, 1.0);
}

// Ease out cubic interpolation
inline f64 EaseOutCubic(f64 t) {
    return 1.0 - std::pow(1.0 - t, 3.0);
}

// Random float between 0 and 1
inline f64 RandomFloat() {
    return static_cast<f64>(rand()) / static_cast<f64>(RAND_MAX);
}

// Random float between min and max
inline f64 RandomRange(f64 min, f64 max) {
    return min + RandomFloat() * (max - min);
}

// Random integer between min and max (inclusive)
inline i32 RandomInt(i32 min, i32 max) {
    return min + rand() % (max - min + 1);
}

// Color manipulation
inline Color LerpColor(const Color& a, const Color& b, f32 t) {
    return Color(
        static_cast<f32>(Lerp(a.r, b.r, t)),
        static_cast<f32>(Lerp(a.g, b.g, t)),
        static_cast<f32>(Lerp(a.b, b.b, t)),
        static_cast<f32>(Lerp(a.a, b.a, t))
    );
}

// Check if point is in rectangle
inline bool PointInRect(const Vec2& point, const Rect& rect) {
    return point.x >= rect.x && point.x <= rect.x + rect.width &&
           point.y >= rect.y && point.y <= rect.y + rect.height;
}

// Calculate cost with multiplier (for exponential upgrade costs)
inline f64 CalculateUpgradeCost(f64 baseCost, i32 level, f64 multiplier) {
    return baseCost * std::pow(multiplier, level);
}

// Parse simple JSON value (basic implementation)
inline std::string ParseJsonString(const std::string& line, const std::string& key) {
    size_t keyPos = line.find("\"" + key + "\"");
    if (keyPos == std::string::npos) return "";

    size_t colonPos = line.find(':', keyPos);
    if (colonPos == std::string::npos) return "";

    size_t firstQuote = line.find('"', colonPos);
    if (firstQuote == std::string::npos) return "";

    size_t secondQuote = line.find('"', firstQuote + 1);
    if (secondQuote == std::string::npos) return "";

    return line.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

inline f64 ParseJsonNumber(const std::string& line, const std::string& key) {
    size_t keyPos = line.find("\"" + key + "\"");
    if (keyPos == std::string::npos) return 0.0;

    size_t colonPos = line.find(':', keyPos);
    if (colonPos == std::string::npos) return 0.0;

    size_t numStart = colonPos + 1;
    while (numStart < line.length() && (line[numStart] == ' ' || line[numStart] == '\t')) {
        numStart++;
    }

    try {
        return std::stod(line.substr(numStart));
    } catch (...) {
        return 0.0;
    }
}

inline bool ParseJsonBool(const std::string& line, const std::string& key) {
    size_t keyPos = line.find("\"" + key + "\"");
    if (keyPos == std::string::npos) return false;

    size_t colonPos = line.find(':', keyPos);
    if (colonPos == std::string::npos) return false;

    return line.find("true", colonPos) != std::string::npos;
}

} // namespace GameUtils
