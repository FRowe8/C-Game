#pragma once

#include "Types.h"
#include <vector>

// Forward declarations
class Renderer;

/**
 * VisualFXManager - Centralized visual effects system
 *
 * Handles all visual effects including:
 * - Particle system (feedback for interactions)
 * - Quantum Anomalies (clickable orbs for active gameplay)
 * - Screen effects (prestige flash, screen shake)
 * - Combo counter display
 *
 * As mentioned in IMPROVEMENTS.md, RenderQuantumAnomalies currently uses
 * ImGui::GetBackgroundDrawList() which is fine, but should be encapsulated
 * in a dedicated system rather than in GameState.
 *
 * This class extracts visual effects from GameState to establish
 * proper separation of concerns.
 */
class VisualFXManager {
public:
    VisualFXManager();
    ~VisualFXManager() = default;

    // Initialize
    void Initialize();

    // Update all visual effects
    void Update(f64 deltaTime);

    // Render all visual effects
    void Render(Renderer* renderer);

    // === Particle System ===

    struct Particle {
        Vec2 position;
        Vec2 velocity;
        Color color;
        f32 lifetime;
        f32 maxLifetime;
    };

    // Spawn a single particle
    void SpawnParticle(const Vec2& position, const Color& color, f64 lifetime = 1.0);

    // Spawn a burst of particles
    void SpawnParticleBurst(const Vec2& position, const Color& color, i32 count = 10);

    // Get particle list (for direct rendering if needed)
    const std::vector<Particle>& GetParticles() const { return m_Particles; }

    // === Quantum Anomaly System ===

    struct QuantumAnomaly {
        Vec2 position;
        f32 radius;
        f32 lifetime;
        f32 maxLifetime;
        f64 rewardMultiplier;  // How much bonus (10-100x production)
        Color color;
        bool clicked;
    };

    // Spawn a quantum anomaly at random position
    void SpawnQuantumAnomaly();

    // Click on anomaly (returns reward multiplier if clicked, 0 if missed)
    f64 ClickQuantumAnomaly(const Vec2& clickPos);

    // Get anomaly list (for rendering)
    const std::vector<QuantumAnomaly>& GetAnomalies() const { return m_Anomalies; }

    // Check if should spawn new anomaly (used by game logic)
    bool ShouldSpawnAnomaly() const;

    // === Screen Effects ===

    // Prestige flash effect
    bool IsPrestigeFlashActive() const { return m_PrestigeFlashActive; }
    f64 GetPrestigeFlashAlpha() const;

    void TriggerPrestigeFlash();

    // Screen shake (for future use)
    void TriggerScreenShake(f32 intensity = 1.0f, f32 duration = 0.3f);
    Vec2 GetScreenShakeOffset() const;

    // === Combo Display ===

    i32 GetComboCount() const { return m_ComboCount; }
    f32 GetComboAlpha() const;  // Fade based on time remaining

    void IncrementCombo();
    void ResetCombo() { m_ComboCount = 0; }

    // === Visual Settings ===

    void SetParticleScale(f32 scale) { m_ParticleScale = scale; }
    void SetAnomalyScale(f32 scale) { m_AnomalyScale = scale; }

    void EnableParticles(bool enabled) { m_ParticlesEnabled = enabled; }
    void EnableAnomalies(bool enabled) { m_AnomaliesEnabled = enabled; }

private:
    // Particle system
    std::vector<Particle> m_Particles;
    bool m_ParticlesEnabled;
    f32 m_ParticleScale;

    // Quantum Anomaly system
    std::vector<QuantumAnomaly> m_Anomalies;
    bool m_AnomaliesEnabled;
    f32 m_AnomalyScale;

    // Screen effects
    bool m_PrestigeFlashActive;
    f64 m_PrestigeFlashTimer;
    f64 m_PrestigeFlashDuration;

    bool m_ScreenShakeActive;
    f32 m_ScreenShakeIntensity;
    f32 m_ScreenShakeTimer;
    f32 m_ScreenShakeDuration;

    // Combo system
    i32 m_ComboCount;

    // Helper methods
    void UpdateParticles(f64 deltaTime);
    void UpdateAnomalies(f64 deltaTime);
    void UpdateScreenEffects(f64 deltaTime);

    void RenderParticles(Renderer* /*renderer*/);
    void RenderAnomalies(Renderer* /*renderer*/);
};
