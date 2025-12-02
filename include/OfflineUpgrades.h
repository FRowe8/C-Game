#pragma once

#include "Types.h"
#include <string>

// Forward declarations
class TimeManager;

// Cryo-Stasis and offline upgrade system
class OfflineUpgrades {
public:
    OfflineUpgrades();

    void Initialize();
    void SetTimeManager(TimeManager* timeManager) { m_TimeManager = timeManager; }

    // === Cryo-Stasis Upgrades ===

    // Upgrade levels (0-5)
    i32 GetCryoStasisLevel() const { return m_CryoStasisLevel; }
    void SetCryoStasisLevel(i32 level);
    bool CanUpgradeCryoStasis() const;
    void UpgradeCryoStasis();

    // Get offline cap hours based on Cryo-Stasis level
    f64 GetOfflineCapHours() const;

    // Get upgrade costs
    f64 GetCryoStasisUpgradeCost() const;
    const char* GetCryoStasisLevelName() const;
    const char* GetCryoStasisDescription() const;

    // === Offline Efficiency Upgrades ===

    i32 GetOfflineEfficiencyLevel() const { return m_OfflineEfficiencyLevel; }
    void SetOfflineEfficiencyLevel(i32 level);
    bool CanUpgradeOfflineEfficiency() const;
    void UpgradeOfflineEfficiency();

    // Get offline multiplier based on efficiency level
    f64 GetOfflineMultiplier() const;

    f64 GetOfflineEfficiencyUpgradeCost() const;
    const char* GetOfflineEfficiencyDescription() const;

    // === Combat Simulation Upgrades ===

    i32 GetCombatSimulationLevel() const { return m_CombatSimulationLevel; }
    void SetCombatSimulationLevel(i32 level);
    bool CanUpgradeCombatSimulation() const;
    void UpgradeCombatSimulation();

    // Get combat rate based on simulation level
    f64 GetOfflineCombatRate() const;

    f64 GetCombatSimulationUpgradeCost() const;
    const char* GetCombatSimulationDescription() const;

    // === Loot Drop Upgrades ===

    i32 GetLootDropLevel() const { return m_LootDropLevel; }
    void SetLootDropLevel(i32 level);
    bool CanUpgradeLootDrop() const;
    void UpgradeLootDrop();

    // Get loot chance based on drop level
    f64 GetOfflineLootChance() const;

    f64 GetLootDropUpgradeCost() const;
    const char* GetLootDropDescription() const;

    // === Statistics ===

    i32 GetTotalUpgradeLevels() const;
    f64 GetTotalSpent() const { return m_TotalSpent; }

    // === Apply Upgrades ===

    // Apply all current upgrade levels to TimeManager
    void ApplyUpgrades();

    // === Save/Load ===

    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    TimeManager* m_TimeManager;

    // Upgrade levels (0-5 for each)
    i32 m_CryoStasisLevel;          // Extends offline cap (4h → 24h)
    i32 m_OfflineEfficiencyLevel;   // Improves offline multiplier (50% → 100%)
    i32 m_CombatSimulationLevel;    // Increases combat rate (10/h → 30/h)
    i32 m_LootDropLevel;             // Increases loot chance (20% → 50%)

    // Tracking
    f64 m_TotalSpent;                // Total resources spent on upgrades
};
