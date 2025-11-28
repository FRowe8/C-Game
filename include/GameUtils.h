#pragma once

#include "Types.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>

// Utility functions for the game

namespace GameUtils {

// Number format options
enum class NumberFormat {
    Suffix,      // 1.23M, 4.56B, 7.89T (default)
    Scientific   // 1.23e6, 4.56e9, 7.89e12
};

// Number formatting with extended suffix support
inline std::string FormatNumber(f64 num, NumberFormat format = NumberFormat::Suffix) {
    // Handle special cases
    if (num < 0.0) {
        return "-" + FormatNumber(-num, format);
    }
    if (num == 0.0) {
        return "0";
    }

    // Scientific notation mode
    if (format == NumberFormat::Scientific) {
        if (num < 1000.0) {
            // Small numbers don't need scientific notation
            if (num < 10.0) {
                std::ostringstream ss;
                ss << std::fixed << std::setprecision(1) << num;
                return ss.str();
            }
            return std::to_string(static_cast<i32>(num));
        }

        // Calculate exponent
        i32 exponent = static_cast<i32>(std::floor(std::log10(num)));
        f64 mantissa = num / std::pow(10.0, exponent);

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << mantissa << "e" << exponent;
        return ss.str();
    }

    // Suffix notation mode (default)
    // Extended suffix support - following Shark Incremental style
    struct Suffix {
        f64 value;
        const char* name;
    };

    static const Suffix suffixes[] = {
        {1e303, "Cg"},  // Centillion
        {1e153, "Uvg"}, // Unvigintillion
        {1e120, "Uvg"}, // Unvigintillion
        {1e102, "Tg"},  // Trigintillion
        {1e99,  "Vg"},  // Vigintillion
        {1e96,  "UDc"}, // Untrigintillion
        {1e93,  "DTg"}, // Duotrigintillion
        {1e90,  "TVg"}, // Trevigintillion
        {1e87,  "UVg"}, // Unvigintillion
        {1e84,  "DVg"}, // Duovigintillion
        {1e81,  "TVg"}, // Trevigintillion
        {1e78,  "QVg"}, // Quattuorvigintillion
        {1e75,  "QiVg"},// Quinvigintillion
        {1e72,  "SxVg"},// Sexvigintillion
        {1e69,  "SpVg"},// Septvigintillion
        {1e66,  "OcVg"},// Octovigintillion
        {1e63,  "NoVg"},// Novevig intillion
        {1e60,  "Vg"},  // Vigintillion
        {1e57,  "UDc"}, // Undevigintillion
        {1e54,  "DDc"}, // Duodevigintillion
        {1e51,  "TDc"}, // Tredevigintillion
        {1e48,  "QDc"}, // Quattuordevigintillion
        {1e45,  "QiDc"},// Quindevigintillion
        {1e42,  "SxDc"},// Sexdevigintillion
        {1e39,  "SpDc"},// Septdevigintillion
        {1e36,  "UDc"}, // Undecillion
        {1e33,  "Dc"},  // Decillion
        {1e30,  "No"},  // Nonillion
        {1e27,  "Oc"},  // Octillion
        {1e24,  "Sp"},  // Septillion
        {1e21,  "Sx"},  // Sextillion
        {1e18,  "Qi"},  // Quintillion
        {1e15,  "Qa"},  // Quadrillion
        {1e12,  "T"},   // Trillion
        {1e9,   "B"},   // Billion
        {1e6,   "M"},   // Million
        {1e3,   "K"}    // Thousand
    };

    // Find appropriate suffix
    for (const auto& suffix : suffixes) {
        if (num >= suffix.value) {
            f64 scaled = num / suffix.value;
            std::ostringstream ss;
            ss << std::fixed << std::setprecision(2) << scaled << suffix.name;
            return ss.str();
        }
    }

    // Small numbers (< 1000)
    if (num < 10.0) {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << num;
        return ss.str();
    }
    return std::to_string(static_cast<i32>(num));
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
