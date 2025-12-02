#include "OfflineUpgrades.h"
#include "TimeManager.h"
#include "Log.h"
#include <fstream>
#include <cmath>

OfflineUpgrades::OfflineUpgrades()
    : m_TimeManager(nullptr)
    , m_CryoStasisLevel(0)
    , m_OfflineEfficiencyLevel(0)
    , m_CombatSimulationLevel(0)
    , m_LootDropLevel(0)
    , m_TotalSpent(0.0)
{
}

void OfflineUpgrades::Initialize() {
    m_CryoStasisLevel = 0;
    m_OfflineEfficiencyLevel = 0;
    m_CombatSimulationLevel = 0;
    m_LootDropLevel = 0;
    m_TotalSpent = 0.0;

    Log::Info("OfflineUpgrades initialized");
}

// === Cryo-Stasis Upgrades ===

void OfflineUpgrades::SetCryoStasisLevel(i32 level) {
    if (level < 0) level = 0;
    if (level > 5) level = 5;
    m_CryoStasisLevel = level;
    ApplyUpgrades();
}

bool OfflineUpgrades::CanUpgradeCryoStasis() const {
    return m_CryoStasisLevel < 5;
}

void OfflineUpgrades::UpgradeCryoStasis() {
    if (!CanUpgradeCryoStasis()) {
        Log::Warning("Cryo-Stasis already at max level");
        return;
    }

    f64 cost = GetCryoStasisUpgradeCost();
    m_TotalSpent += cost;

    m_CryoStasisLevel++;
    ApplyUpgrades();

    Log::Infof("Upgraded Cryo-Stasis to level ", m_CryoStasisLevel, " - Offline cap now ", GetOfflineCapHours(), " hours");
}

f64 OfflineUpgrades::GetOfflineCapHours() const {
    // Level 0: 4 hours (default)
    // Level 1: 8 hours
    // Level 2: 12 hours
    // Level 3: 16 hours
    // Level 4: 20 hours
    // Level 5: 24 hours
    return 4.0 + (m_CryoStasisLevel * 4.0);
}

f64 OfflineUpgrades::GetCryoStasisUpgradeCost() const {
    // Exponential cost scaling: 1000, 2500, 5000, 10000, 20000
    return 1000.0 * std::pow(2.5, m_CryoStasisLevel);
}

const char* OfflineUpgrades::GetCryoStasisLevelName() const {
    switch (m_CryoStasisLevel) {
        case 0: return "Basic Life Support";
        case 1: return "Cryo-Stasis I";
        case 2: return "Cryo-Stasis II";
        case 3: return "Cryo-Stasis III";
        case 4: return "Cryo-Stasis IV";
        case 5: return "Quantum Stasis";
        default: return "Unknown";
    }
}

const char* OfflineUpgrades::GetCryoStasisDescription() const {
    switch (m_CryoStasisLevel) {
        case 0: return "Basic 4-hour offline cap";
        case 1: return "Extended 8-hour offline cap";
        case 2: return "Advanced 12-hour offline cap";
        case 3: return "Superior 16-hour offline cap";
        case 4: return "Elite 20-hour offline cap";
        case 5: return "Maximum 24-hour offline cap";
        default: return "Unknown level";
    }
}

// === Offline Efficiency Upgrades ===

void OfflineUpgrades::SetOfflineEfficiencyLevel(i32 level) {
    if (level < 0) level = 0;
    if (level > 5) level = 5;
    m_OfflineEfficiencyLevel = level;
    ApplyUpgrades();
}

bool OfflineUpgrades::CanUpgradeOfflineEfficiency() const {
    return m_OfflineEfficiencyLevel < 5;
}

void OfflineUpgrades::UpgradeOfflineEfficiency() {
    if (!CanUpgradeOfflineEfficiency()) {
        Log::Warning("Offline Efficiency already at max level");
        return;
    }

    f64 cost = GetOfflineEfficiencyUpgradeCost();
    m_TotalSpent += cost;

    m_OfflineEfficiencyLevel++;
    ApplyUpgrades();

    Log::Infof("Upgraded Offline Efficiency to level ", m_OfflineEfficiencyLevel, " - Multiplier now ", GetOfflineMultiplier() * 100.0, "%");
}

f64 OfflineUpgrades::GetOfflineMultiplier() const {
    // Level 0: 50% (0.5x)
    // Level 1: 60% (0.6x)
    // Level 2: 70% (0.7x)
    // Level 3: 80% (0.8x)
    // Level 4: 90% (0.9x)
    // Level 5: 100% (1.0x)
    return 0.5 + (m_OfflineEfficiencyLevel * 0.1);
}

f64 OfflineUpgrades::GetOfflineEfficiencyUpgradeCost() const {
    // Exponential cost: 500, 1250, 2500, 5000, 10000
    return 500.0 * std::pow(2.5, m_OfflineEfficiencyLevel);
}

const char* OfflineUpgrades::GetOfflineEfficiencyDescription() const {
    i32 percent = static_cast<i32>(GetOfflineMultiplier() * 100.0);
    static char buffer[128];
    snprintf(buffer, sizeof(buffer), "Offline resources generate at %d%% of active rate", percent);
    return buffer;
}

// === Combat Simulation Upgrades ===

void OfflineUpgrades::SetCombatSimulationLevel(i32 level) {
    if (level < 0) level = 0;
    if (level > 5) level = 5;
    m_CombatSimulationLevel = level;
    ApplyUpgrades();
}

bool OfflineUpgrades::CanUpgradeCombatSimulation() const {
    return m_CombatSimulationLevel < 5;
}

void OfflineUpgrades::UpgradeCombatSimulation() {
    if (!CanUpgradeCombatSimulation()) {
        Log::Warning("Combat Simulation already at max level");
        return;
    }

    f64 cost = GetCombatSimulationUpgradeCost();
    m_TotalSpent += cost;

    m_CombatSimulationLevel++;
    ApplyUpgrades();

    Log::Infof("Upgraded Combat Simulation to level ", m_CombatSimulationLevel, " - Rate now ", GetOfflineCombatRate(), " combats/hour");
}

f64 OfflineUpgrades::GetOfflineCombatRate() const {
    // Level 0: 10 combats/hour
    // Level 1: 14 combats/hour
    // Level 2: 18 combats/hour
    // Level 3: 22 combats/hour
    // Level 4: 26 combats/hour
    // Level 5: 30 combats/hour
    return 10.0 + (m_CombatSimulationLevel * 4.0);
}

f64 OfflineUpgrades::GetCombatSimulationUpgradeCost() const {
    // Exponential cost: 750, 1875, 3750, 7500, 15000
    return 750.0 * std::pow(2.5, m_CombatSimulationLevel);
}

const char* OfflineUpgrades::GetCombatSimulationDescription() const {
    i32 rate = static_cast<i32>(GetOfflineCombatRate());
    static char buffer[128];
    snprintf(buffer, sizeof(buffer), "Simulate %d combats per hour while offline", rate);
    return buffer;
}

// === Loot Drop Upgrades ===

void OfflineUpgrades::SetLootDropLevel(i32 level) {
    if (level < 0) level = 0;
    if (level > 5) level = 5;
    m_LootDropLevel = level;
    ApplyUpgrades();
}

bool OfflineUpgrades::CanUpgradeLootDrop() const {
    return m_LootDropLevel < 5;
}

void OfflineUpgrades::UpgradeLootDrop() {
    if (!CanUpgradeLootDrop()) {
        Log::Warning("Loot Drop already at max level");
        return;
    }

    f64 cost = GetLootDropUpgradeCost();
    m_TotalSpent += cost;

    m_LootDropLevel++;
    ApplyUpgrades();

    Log::Infof("Upgraded Loot Drop to level ", m_LootDropLevel, " - Chance now ", GetOfflineLootChance() * 100.0, "%");
}

f64 OfflineUpgrades::GetOfflineLootChance() const {
    // Level 0: 20% (0.20)
    // Level 1: 26% (0.26)
    // Level 2: 32% (0.32)
    // Level 3: 38% (0.38)
    // Level 4: 44% (0.44)
    // Level 5: 50% (0.50)
    return 0.20 + (m_LootDropLevel * 0.06);
}

f64 OfflineUpgrades::GetLootDropUpgradeCost() const {
    // Exponential cost: 600, 1500, 3000, 6000, 12000
    return 600.0 * std::pow(2.5, m_LootDropLevel);
}

const char* OfflineUpgrades::GetLootDropDescription() const {
    i32 percent = static_cast<i32>(GetOfflineLootChance() * 100.0);
    static char buffer[128];
    snprintf(buffer, sizeof(buffer), "%d%% chance for loot per offline combat", percent);
    return buffer;
}

// === Statistics ===

i32 OfflineUpgrades::GetTotalUpgradeLevels() const {
    return m_CryoStasisLevel + m_OfflineEfficiencyLevel + m_CombatSimulationLevel + m_LootDropLevel;
}

// === Apply Upgrades ===

void OfflineUpgrades::ApplyUpgrades() {
    if (!m_TimeManager) return;

    m_TimeManager->SetOfflineCapHours(GetOfflineCapHours());
    m_TimeManager->SetOfflineMultiplier(GetOfflineMultiplier());
    m_TimeManager->SetOfflineCombatRate(GetOfflineCombatRate());
    m_TimeManager->SetOfflineLootChance(GetOfflineLootChance());

    Log::Infof("Applied offline upgrades - Cap: ", GetOfflineCapHours(), "h, Mult: ", GetOfflineMultiplier() * 100.0, "%, Combat: ", GetOfflineCombatRate(), "/h, Loot: ", GetOfflineLootChance() * 100.0, "%");
}

// === Save/Load ===

void OfflineUpgrades::SaveToJson(std::ofstream& file) const {
    file << "    \"offlineUpgrades\": {\n";
    file << "        \"cryoStasisLevel\": " << m_CryoStasisLevel << ",\n";
    file << "        \"offlineEfficiencyLevel\": " << m_OfflineEfficiencyLevel << ",\n";
    file << "        \"combatSimulationLevel\": " << m_CombatSimulationLevel << ",\n";
    file << "        \"lootDropLevel\": " << m_LootDropLevel << ",\n";
    file << "        \"totalSpent\": " << m_TotalSpent << "\n";
    file << "    }";
}

void OfflineUpgrades::LoadFromJson(const std::string& line) {
    // Simple manual JSON parsing (to be replaced with nlohmann/json later)
    if (line.find("\"cryoStasisLevel\"") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            m_CryoStasisLevel = std::stoi(value);
        }
    }
    else if (line.find("\"offlineEfficiencyLevel\"") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            m_OfflineEfficiencyLevel = std::stoi(value);
        }
    }
    else if (line.find("\"combatSimulationLevel\"") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            m_CombatSimulationLevel = std::stoi(value);
        }
    }
    else if (line.find("\"lootDropLevel\"") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            m_LootDropLevel = std::stoi(value);
        }
    }
    else if (line.find("\"totalSpent\"") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            m_TotalSpent = std::stod(value);
        }
    }

    // Reapply upgrades after loading
    ApplyUpgrades();
}
