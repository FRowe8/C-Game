#pragma once

#include <cstdint>
#include <memory>

// Common type aliases
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

// Smart pointer aliases
template<typename T>
using Ref = std::shared_ptr<T>;

template<typename T>
using Scope = std::unique_ptr<T>;

template<typename T, typename... Args>
Ref<T> CreateRef(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
Scope<T> CreateScope(Args&&... args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}

// Vector math structures
struct Vec2 {
    f32 x, y;

    Vec2() : x(0), y(0) {}
    Vec2(f32 x, f32 y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(f32 scalar) const { return Vec2(x * scalar, y * scalar); }
    Vec2 operator/(f32 scalar) const { return Vec2(x / scalar, y / scalar); }

    f32 Length() const;
    Vec2 Normalized() const;
    f32 Dot(const Vec2& other) const;
};

struct Vec3 {
    f32 x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& other) const { return Vec3(x + other.x, y + other.y, z + other.z); }
    Vec3 operator-(const Vec3& other) const { return Vec3(x - other.x, y - other.y, z - other.z); }
    Vec3 operator*(f32 scalar) const { return Vec3(x * scalar, y * scalar, z * scalar); }

    f32 Length() const;
    Vec3 Normalized() const;
    f32 Dot(const Vec3& other) const;
    Vec3 Cross(const Vec3& other) const;
};

struct Vec4 {
    f32 x, y, z, w;

    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(f32 x, f32 y, f32 z, f32 w) : x(x), y(y), z(z), w(w) {}
    Vec4(const Vec3& v, f32 w) : x(v.x), y(v.y), z(v.z), w(w) {}
};

struct Color {
    f32 r, g, b, a;

    Color() : r(1), g(1), b(1), a(1) {}
    Color(f32 r, f32 g, f32 b, f32 a = 1.0f) : r(r), g(g), b(b), a(a) {}

    // Operators for color manipulation
    Color operator*(f32 scalar) const { return Color(r * scalar, g * scalar, b * scalar, a); }

    static Color White() { return Color(1, 1, 1, 1); }
    static Color Black() { return Color(0, 0, 0, 1); }
    static Color Red() { return Color(1, 0, 0, 1); }
    static Color Green() { return Color(0, 1, 0, 1); }
    static Color Blue() { return Color(0, 0, 1, 1); }
    static Color Yellow() { return Color(1, 1, 0, 1); }
    static Color Cyan() { return Color(0, 1, 1, 1); }
    static Color Magenta() { return Color(1, 0, 1, 1); }

    // Quantum-themed colors
    static Color QuantumBlue() { return Color(0.2f, 0.5f, 1.0f, 1.0f); }
    static Color QuantumPurple() { return Color(0.6f, 0.2f, 0.9f, 1.0f); }
    static Color CoherenceGreen() { return Color(0.2f, 0.9f, 0.5f, 1.0f); }
    static Color EntanglementOrange() { return Color(1.0f, 0.5f, 0.1f, 1.0f); }

    // Cyberpunk neon colors for modern UI
    static Color NeonCyan() { return Color(0.0f, 0.9f, 0.9f, 1.0f); }
    static Color NeonPink() { return Color(1.0f, 0.0f, 0.6f, 1.0f); }
    static Color NeonPurple() { return Color(0.7f, 0.0f, 1.0f, 1.0f); }
    static Color ElectricBlue() { return Color(0.3f, 0.7f, 1.0f, 1.0f); }

    // Dark UI colors
    static Color DarkBackground() { return Color(0.08f, 0.08f, 0.12f, 1.0f); }
    static Color DarkPanel() { return Color(0.12f, 0.12f, 0.18f, 0.95f); }
    static Color DarkBorder() { return Color(0.2f, 0.2f, 0.3f, 1.0f); }

    static Color Transparent() { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
};

struct Rect {
    f32 x, y, width, height;

    Rect() : x(0), y(0), width(0), height(0) {}
    Rect(f32 x, f32 y, f32 width, f32 height) : x(x), y(y), width(width), height(height) {}

    bool Contains(const Vec2& point) const {
        return point.x >= x && point.x <= x + width &&
               point.y >= y && point.y <= y + height;
    }

    Vec2 Center() const { return Vec2(x + width * 0.5f, y + height * 0.5f); }
};

enum class QuantumResource {
    Qubits,        // Primary currency
    Coherence,     // Stability resource
    Entanglement   // Strategic resource
};



enum class ResearchID {
    // Tier 1 - Basic Research (Early game)
    QuantumMechanics101,      // Unlock basic concepts
    WaveFunctionTheory,       // +10% observation probability
    CoherenceStabilization,   // Slower coherence decay
    EntanglementBasics,       // Unlock entanglement system

    // Tier 2 - Intermediate (Mid game)
    QuantumComputing,         // +25% production
    SuperpositionMastery,     // Superposition accumulates faster
    ParticlePhysics,          // Unlock particle collection
    QuantumTunneling,         // Resources can "tunnel" between types

    // Tier 3 - Advanced (Late game)
    QuantumFieldTheory,       // Unlock quantum field effects
    StringTheory,             // +50% all production
    QuantumGravity,           // Gravity affects resource generation
    TimeDialation,            // Slow down/speed up time

    // Tier 4 - Exotic (End game)
    MultiverseTheory,         // Run parallel universes
    QuantumImmortality,       // Never lose progress
    ZeroPointEnergy,          // Infinite energy source
    QuantumSingularity,       // The ultimate discovery

    // Automation
    AutoObserver,             // Auto-observe every 10 seconds
    AutoUpgrade,              // Auto-buy cheapest upgrade
    AutoPrestige,             // Auto-prestige at photon threshold
    SmartInvestor,            // AI decides best purchases

    // Special
    QuantumLuck,              // Better event chances
    PhotonMultiplier,         // More photons on prestige
    OfflineBoost,             // Better offline production

    COUNT
};

enum class ResearchCategory {
    Production,      // Increase production
    Observation,     // Observation bonuses
    Coherence,       // Coherence bonuses
    Entanglement,    // Entanglement bonuses
    Automation,      // Quality of life
    Special          // Unique effects
};
