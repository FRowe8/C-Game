#pragma once

#include "Types.h"
#include "imgui.h"
#include <vector>
#include <string>

// Forward declarations
class Renderer;

namespace UI {

/**
 * FloatingText
 * Represents a single floating text instance with animation
 */
struct FloatingText {
    std::string text;
    Vec2 position;        // Current screen position
    Vec2 velocity;        // Movement speed (upward)
    ImVec4 color;         // Text color
    float lifetime;       // Total lifetime
    float timeRemaining;  // Time until removal
    float scale;          // Size multiplier (for pop-in effect)

    FloatingText(const std::string& txt, const Vec2& pos, const ImVec4& col, float life = 2.0f);

    void Update(float deltaTime);
    bool IsAlive() const { return timeRemaining > 0.0f; }
};

/**
 * FloatingTextManager
 *
 * Manages floating text animations that appear when resources are gained.
 * Provides visual feedback for resource changes (e.g., "+10 Qubits").
 *
 * Features:
 * - Spawn floating text at specific screen positions
 * - Automatic upward movement and fade-out
 * - Color-coded by resource type
 * - Scale-in animation for impact
 */
class FloatingTextManager {
public:
    FloatingTextManager();
    ~FloatingTextManager() = default;

    /**
     * Spawn a floating text at the given position
     * @param text Text to display (e.g., "+10 Qubits")
     * @param position Screen position to spawn at
     * @param color Text color
     * @param lifetime Duration before fade-out (default 2.0s)
     */
    void SpawnText(const std::string& text, const Vec2& position, const ImVec4& color, float lifetime = 2.0f);

    /**
     * Convenience method to spawn resource gain text
     * @param resourceType Type of resource (for color selection)
     * @param amount Amount gained
     * @param position Screen position
     */
    void SpawnResourceText(QuantumResource resourceType, f64 amount, const Vec2& position);

    /**
     * Update all active floating texts
     * @param deltaTime Time since last frame
     */
    void Update(float deltaTime);

    /**
     * Render all active floating texts
     * @param renderer Renderer reference (unused for ImGui-only rendering)
     */
    void Render(Renderer* renderer);

    /**
     * Clear all floating texts
     */
    void Clear();

private:
    std::vector<FloatingText> m_Texts;

    /**
     * Get color for resource type
     */
    ImVec4 GetResourceColor(QuantumResource type) const;

    /**
     * Format resource text with appropriate units
     */
    std::string FormatResourceText(QuantumResource type, f64 amount) const;
};

} // namespace UI
