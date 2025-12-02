#pragma once

#include "Types.h"
#include <vector>

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

    [[nodiscard]] f64 GetTotalTimePlayed() const { return m_TotalTimePlayed; }
    [[nodiscard]] f64 GetTimeSinceLastSave() const { return m_TimeSinceLastSave; }
    [[nodiscard]] f64 GetTimeSinceLastPrestige() const { return m_TimeSinceLastPrestige; }

    void AddPlayTime(f64 deltaTime) { m_TotalTimePlayed += deltaTime; }
    void ResetTimeSincePrestige() { m_TimeSinceLastPrestige = 0.0; }
    void ResetTimeSinceSave() { m_TimeSinceLastSave = 0.0; }

    // === Offline Progress ===

    struct LootDrop {
        enum class Type { Card, Crew, ShipPart, Resource };
        Type type;
        i32 itemID;
        std::string itemName;
        i32 rarity; // 0=Common, 4=Legendary
    };

    struct OfflineProgressData {
        f64 secondsOffline;
        bool hadProgress;

        // Resources generated
        f64 qubitsGenerated;
        f64 coherenceGenerated;
        f64 entanglementGenerated;
        i32 photonsEarned;

        // Combat simulation
        i32 combatsWon;
        i32 creditsEarned;
        i32 xpEarned;

        // Loot drops
        std::vector<LootDrop> lootDrops;
        i32 cardsDropped;
        i32 crewRecruited;
        i32 partsDropped;

        // Research progress
        i32 researchCompleted;

        OfflineProgressData()
            : secondsOffline(0.0), hadProgress(false),
              qubitsGenerated(0.0), coherenceGenerated(0.0), entanglementGenerated(0.0), photonsEarned(0),
              combatsWon(0), creditsEarned(0), xpEarned(0),
              lootDrops(), cardsDropped(0), crewRecruited(0), partsDropped(0),
              researchCompleted(0) {}
    };

    // Calculate offline progress with full simulation (returns data for display)
    OfflineProgressData CalculateOfflineProgress(i64 lastSaveTimestamp, class GameState* gameState = nullptr);

    // Get offline cap (4 hours by default, can be upgraded)
    [[nodiscard]] f64 GetOfflineCapHours() const { return m_OfflineCapHours; }
    void SetOfflineCapHours(f64 hours) { m_OfflineCapHours = hours; }

    [[nodiscard]] f64 GetOfflineMultiplier() const { return m_OfflineMultiplier; }
    void SetOfflineMultiplier(f64 multiplier) { m_OfflineMultiplier = multiplier; }

    // Offline loot settings
    [[nodiscard]] f64 GetOfflineLootChance() const { return m_OfflineLootChance; }
    void SetOfflineLootChance(f64 chance) { m_OfflineLootChance = chance; }

    [[nodiscard]] f64 GetOfflineCombatRate() const { return m_OfflineCombatRate; } // Combats per hour
    void SetOfflineCombatRate(f64 rate) { m_OfflineCombatRate = rate; }

    // === Time-Based Cooldowns ===

    // Boost system
    [[nodiscard]] bool IsBoostActive() const { return m_BoostActive; }
    [[nodiscard]] f64 GetBoostTimeRemaining() const { return m_BoostTimeRemaining; }
    [[nodiscard]] f64 GetBoostCooldownRemaining() const { return m_BoostCooldownRemaining; }
    [[nodiscard]] f64 GetBoostDuration() const { return m_BoostDuration; }
    [[nodiscard]] f64 GetBoostCooldown() const { return m_BoostCooldown; }

    void ActivateBoost();
    void SetBoostDuration(f64 duration) { m_BoostDuration = duration; }
    void SetBoostCooldown(f64 cooldown) { m_BoostCooldown = cooldown; }

    // Event system
    [[nodiscard]] f64 GetTimeSinceLastEvent() const { return m_TimeSinceLastEvent; }
    [[nodiscard]] f64 GetEventCooldown() const { return m_EventCooldown; }

    void ResetEventTimer() { m_TimeSinceLastEvent = 0.0; }
    void SetEventCooldown(f64 cooldown) { m_EventCooldown = cooldown; }

    // Anomaly system
    [[nodiscard]] f64 GetTimeSinceLastAnomaly() const { return m_TimeSinceLastAnomaly; }
    [[nodiscard]] f64 GetAnomalySpawnInterval() const { return m_AnomalySpawnInterval; }

    void ResetAnomalyTimer() { m_TimeSinceLastAnomaly = 0.0; }
    void SetAnomalySpawnInterval(f64 interval) { m_AnomalySpawnInterval = interval; }

    // Combo system
    [[nodiscard]] i32 GetComboCount() const { return m_ComboCount; }
    [[nodiscard]] f64 GetComboTimeRemaining() const { return m_ComboTimeRemaining; }
    [[nodiscard]] f64 GetComboWindow() const { return m_ComboWindow; }

    void IncrementCombo();
    void ResetCombo() { m_ComboCount = 0; m_ComboTimeRemaining = 0.0; }
    void SetComboWindow(f64 window) { m_ComboWindow = window; }

    // === Visual Effects Timing ===

    [[nodiscard]] bool IsPrestigeFlashActive() const { return m_PrestigeFlashActive; }
    [[nodiscard]] f64 GetPrestigeFlashTimer() const { return m_PrestigeFlashTimer; }

    void TriggerPrestigeFlash();

    // === Speed Multipliers ===

    [[nodiscard]] f64 GetTimeScale() const { return m_TimeScale; }
    void SetTimeScale(f64 scale) { m_TimeScale = scale; }

    // Get effective delta time (with time scale applied)
    [[nodiscard]] f64 GetEffectiveDeltaTime(f64 deltaTime) const { return deltaTime * m_TimeScale; }

private:
    // Play time tracking
    f64 m_TotalTimePlayed;
    f64 m_TimeSinceLastSave;
    f64 m_TimeSinceLastPrestige;

    // Offline progress settings
    f64 m_OfflineCapHours;       // Maximum offline time (default 4 hours)
    f64 m_OfflineMultiplier;     // Offline production multiplier (default 0.5 = 50%)
    f64 m_OfflineLootChance;     // Base loot drop chance per combat (default 0.2 = 20%)
    f64 m_OfflineCombatRate;     // Combats simulated per hour offline (default 10.0)

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

    // Helper methods
    i32 RollRarity(); // Roll for loot rarity (0=Common, 4=Legendary)
};
