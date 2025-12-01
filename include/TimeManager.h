#pragma once

#include "Types.h"

/**
 * TimeManager - Centralized time management system
 *
 * Handles all time-related functionality including:
 * - Delta time and total play time tracking
 * - Offline progress calculation
 * - Time-based cooldowns (boost, events, anomalies, combo)
 * - Speed multipliers and time dilation effects
 * - Session timing for achievements
 *
 * This class extracts time management from GameState to establish
 * proper separation of concerns.
 */
class TimeManager {
public:
    TimeManager();
    ~TimeManager() = default;

    // Initialize
    void Initialize();

    // Update (call once per frame)
    void Update(f64 deltaTime);

    // === Time Tracking ===

    f64 GetTotalTimePlayed() const { return m_TotalTimePlayed; }
    f64 GetTimeSinceLastSave() const { return m_TimeSinceLastSave; }
    f64 GetTimeSinceLastPrestige() const { return m_TimeSinceLastPrestige; }

    void AddPlayTime(f64 deltaTime) { m_TotalTimePlayed += deltaTime; }
    void ResetTimeSincePrestige() { m_TimeSinceLastPrestige = 0.0; }
    void ResetTimeSinceSave() { m_TimeSinceLastSave = 0.0; }

    // === Offline Progress ===

    struct OfflineProgressData {
        f64 secondsOffline;
        f64 resourcesGenerated; // For display purposes
        bool hadProgress;
    };

    // Calculate offline progress (returns data for display)
    OfflineProgressData CalculateOfflineProgress(i64 lastSaveTimestamp);

    // Get offline cap (4 hours by default, can be upgraded)
    f64 GetOfflineCapHours() const { return m_OfflineCapHours; }
    void SetOfflineCapHours(f64 hours) { m_OfflineCapHours = hours; }

    f64 GetOfflineMultiplier() const { return m_OfflineMultiplier; }
    void SetOfflineMultiplier(f64 multiplier) { m_OfflineMultiplier = multiplier; }

    // === Time-Based Cooldowns ===

    // Boost system
    bool IsBoostActive() const { return m_BoostActive; }
    f64 GetBoostTimeRemaining() const { return m_BoostTimeRemaining; }
    f64 GetBoostCooldownRemaining() const { return m_BoostCooldownRemaining; }
    f64 GetBoostDuration() const { return m_BoostDuration; }
    f64 GetBoostCooldown() const { return m_BoostCooldown; }

    void ActivateBoost();
    void SetBoostDuration(f64 duration) { m_BoostDuration = duration; }
    void SetBoostCooldown(f64 cooldown) { m_BoostCooldown = cooldown; }

    // Event system
    f64 GetTimeSinceLastEvent() const { return m_TimeSinceLastEvent; }
    f64 GetEventCooldown() const { return m_EventCooldown; }

    void ResetEventTimer() { m_TimeSinceLastEvent = 0.0; }
    void SetEventCooldown(f64 cooldown) { m_EventCooldown = cooldown; }

    // Anomaly system
    f64 GetTimeSinceLastAnomaly() const { return m_TimeSinceLastAnomaly; }
    f64 GetAnomalySpawnInterval() const { return m_AnomalySpawnInterval; }

    void ResetAnomalyTimer() { m_TimeSinceLastAnomaly = 0.0; }
    void SetAnomalySpawnInterval(f64 interval) { m_AnomalySpawnInterval = interval; }

    // Combo system
    i32 GetComboCount() const { return m_ComboCount; }
    f64 GetComboTimeRemaining() const { return m_ComboTimeRemaining; }
    f64 GetComboWindow() const { return m_ComboWindow; }

    void IncrementCombo();
    void ResetCombo() { m_ComboCount = 0; m_ComboTimeRemaining = 0.0; }
    void SetComboWindow(f64 window) { m_ComboWindow = window; }

    // === Visual Effects Timing ===

    bool IsPrestigeFlashActive() const { return m_PrestigeFlashActive; }
    f64 GetPrestigeFlashTimer() const { return m_PrestigeFlashTimer; }

    void TriggerPrestigeFlash();

    // === Speed Multipliers ===

    f64 GetTimeScale() const { return m_TimeScale; }
    void SetTimeScale(f64 scale) { m_TimeScale = scale; }

    // Get effective delta time (with time scale applied)
    f64 GetEffectiveDeltaTime(f64 deltaTime) const { return deltaTime * m_TimeScale; }

private:
    // Play time tracking
    f64 m_TotalTimePlayed;
    f64 m_TimeSinceLastSave;
    f64 m_TimeSinceLastPrestige;

    // Offline progress settings
    f64 m_OfflineCapHours;       // Maximum offline time (default 4 hours)
    f64 m_OfflineMultiplier;     // Offline production multiplier (default 0.5 = 50%)

    // Boost system
    bool m_BoostActive;
    f64 m_BoostTimeRemaining;
    f64 m_BoostCooldownRemaining;
    f64 m_BoostDuration;         // How long boost lasts (default 30s)
    f64 m_BoostCooldown;         // Cooldown between boosts (default 60s)

    // Event system
    f64 m_TimeSinceLastEvent;
    f64 m_EventCooldown;         // Time between events (default 120s)

    // Anomaly system
    f64 m_TimeSinceLastAnomaly;
    f64 m_AnomalySpawnInterval;  // 30-60 seconds

    // Combo system
    i32 m_ComboCount;
    f64 m_ComboTimeRemaining;
    f64 m_ComboWindow;           // Time window for combo (default 5s)

    // Visual effects
    f64 m_PrestigeFlashTimer;
    bool m_PrestigeFlashActive;

    // Speed multipliers
    f64 m_TimeScale;             // Global time scale (default 1.0, can be modified by events)
};
