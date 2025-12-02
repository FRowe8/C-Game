#include "TimeManager.h"
#include "Platform.h"
#include "Log.h"
#include <string>
#include <cstdlib>

TimeManager::TimeManager()
    : m_TotalTimePlayed(0.0)
    , m_TimeSinceLastSave(0.0)
    , m_TimeSinceLastPrestige(0.0)
    , m_OfflineCapHours(4.0)
    , m_OfflineMultiplier(0.5)
    , m_OfflineLootChance(0.2)
    , m_OfflineCombatRate(10.0)
    , m_BoostActive(false)
    , m_BoostTimeRemaining(0.0)
    , m_BoostCooldownRemaining(0.0)
    , m_BoostDuration(30.0)
    , m_BoostCooldown(60.0)
    , m_TimeSinceLastEvent(0.0)
    , m_EventCooldown(120.0)
    , m_TimeSinceLastAnomaly(0.0)
    , m_AnomalySpawnInterval(45.0)
    , m_ComboCount(0)
    , m_ComboTimeRemaining(0.0)
    , m_ComboWindow(5.0)
    , m_PrestigeFlashTimer(0.0)
    , m_PrestigeFlashActive(false)
    , m_TimeScale(1.0)
{
}

void TimeManager::Initialize() {
    m_TotalTimePlayed = 0.0;
    m_TimeSinceLastSave = 0.0;
    m_TimeSinceLastPrestige = 0.0;
    m_TimeScale = 1.0;
}

void TimeManager::Update(f64 deltaTime) {
    // Apply time scale
    f64 effectiveDelta = deltaTime * m_TimeScale;

    // Update play time tracking
    m_TotalTimePlayed += effectiveDelta;
    m_TimeSinceLastSave += effectiveDelta;
    m_TimeSinceLastPrestige += effectiveDelta;

    // Update boost system
    if (m_BoostActive) {
        m_BoostTimeRemaining -= deltaTime;
        if (m_BoostTimeRemaining <= 0.0) {
            m_BoostActive = false;
            m_BoostTimeRemaining = 0.0;
            m_BoostCooldownRemaining = m_BoostCooldown;
        }
    } else if (m_BoostCooldownRemaining > 0.0) {
        m_BoostCooldownRemaining -= deltaTime;
        if (m_BoostCooldownRemaining < 0.0) {
            m_BoostCooldownRemaining = 0.0;
        }
    }

    // Update event timer
    m_TimeSinceLastEvent += effectiveDelta;

    // Update anomaly timer
    m_TimeSinceLastAnomaly += effectiveDelta;

    // Update combo timer
    if (m_ComboCount > 0) {
        m_ComboTimeRemaining -= deltaTime;
        if (m_ComboTimeRemaining <= 0.0) {
            ResetCombo();
        }
    }

    // Update prestige flash
    if (m_PrestigeFlashActive) {
        m_PrestigeFlashTimer -= deltaTime;
        if (m_PrestigeFlashTimer <= 0.0) {
            m_PrestigeFlashActive = false;
            m_PrestigeFlashTimer = 0.0;
        }
    }
}

// === Offline Progress ===

TimeManager::OfflineProgressData TimeManager::CalculateOfflineProgress(i64 lastSaveTimestamp, GameState* gameState) {
    OfflineProgressData result;

    i64 currentTime = static_cast<i64>(Platform::GetTime());
    if (lastSaveTimestamp == 0) {
        return result;
    }

    i64 timeOffline = currentTime - lastSaveTimestamp;
    if (timeOffline < 10) {
        return result; // Less than 10 seconds, ignore
    }

    f64 secondsOffline = static_cast<f64>(timeOffline);
    f64 maxOfflineTime = 3600.0 * m_OfflineCapHours;

    if (secondsOffline > maxOfflineTime) {
        secondsOffline = maxOfflineTime;
    }

    result.secondsOffline = secondsOffline;
    result.hadProgress = true;

    f64 hoursOffline = secondsOffline / 3600.0;

    Log::Infof("You were away for ", secondsOffline / 60.0, " minutes (capped at ", m_OfflineCapHours, " hours)");

    // === 1. Resource Generation ===
    // Generate resources at reduced rate (50% by default)
    f64 resourceRate = 10.0; // Base rate per second (would be calculated from game state)
    result.qubitsGenerated = resourceRate * secondsOffline * m_OfflineMultiplier;
    result.coherenceGenerated = resourceRate * 0.1 * secondsOffline * m_OfflineMultiplier;
    result.entanglementGenerated = resourceRate * 0.01 * secondsOffline * m_OfflineMultiplier;

    Log::Infof("Generated ", result.qubitsGenerated, " Qubits while offline");

    // === 2. Combat Simulation ===
    // Simulate auto-combats at specified rate (10 per hour by default)
    i32 totalCombats = static_cast<i32>(hoursOffline * m_OfflineCombatRate);
    if (totalCombats > 0) {
        // Assume 80% win rate for offline combats
        result.combatsWon = static_cast<i32>(totalCombats * 0.8);

        // Credits and XP per combat (would be calculated from player level)
        i32 creditsPerCombat = 50;
        i32 xpPerCombat = 25;

        result.creditsEarned = result.combatsWon * creditsPerCombat;
        result.xpEarned = result.combatsWon * xpPerCombat;

        Log::Infof("Won ", result.combatsWon, " / ", totalCombats, " offline combats");
    }

    // === 3. Loot Drop Simulation ===
    // Roll for loot drops from combat victories
    for (i32 i = 0; i < result.combatsWon; i++) {
        f64 lootRoll = static_cast<f64>(rand() % 100) / 100.0;

        if (lootRoll < m_OfflineLootChance) {
            // Determine loot type
            i32 lootTypeRoll = rand() % 100;

            LootDrop loot;
            if (lootTypeRoll < 40) {
                // 40% - Card drop
                loot.type = LootDrop::Type::Card;
                loot.rarity = RollRarity();
                loot.itemID = 1 + (rand() % 25); // Random card ID
                loot.itemName = "Card #" + std::to_string(loot.itemID);
                result.cardsDropped++;
            }
            else if (lootTypeRoll < 65) {
                // 25% - Crew recruit
                loot.type = LootDrop::Type::Crew;
                loot.rarity = RollRarity();
                loot.itemID = 1 + (rand() % 30); // Random crew ID
                loot.itemName = "Crew #" + std::to_string(loot.itemID);
                result.crewRecruited++;
            }
            else if (lootTypeRoll < 85) {
                // 20% - Ship part
                loot.type = LootDrop::Type::ShipPart;
                loot.rarity = RollRarity();
                loot.itemID = rand() % 100;
                loot.itemName = "Part #" + std::to_string(loot.itemID);
                result.partsDropped++;
            }
            else {
                // 15% - Bonus resources
                loot.type = LootDrop::Type::Resource;
                loot.rarity = RollRarity();
                loot.itemID = 0;
                loot.itemName = "Resource Cache";

                // Add bonus resources based on rarity
                result.qubitsGenerated += 100.0 * (loot.rarity + 1);
                result.photonsEarned += loot.rarity;
            }

            result.lootDrops.push_back(loot);
        }
    }

    if (!result.lootDrops.empty()) {
        Log::Infof("Dropped ", result.lootDrops.size(), " loot items while offline");
    }

    // === 4. Research Progress ===
    // Simulate automatic research completion (if auto-research is enabled)
    // This would check game state for auto-research settings
    // For now, just estimate based on time
    result.researchCompleted = static_cast<i32>(hoursOffline / 2.0); // 1 research every 2 hours

    // === 5. Photon Generation ===
    // Award photons based on time offline (prestige currency trickle)
    result.photonsEarned += static_cast<i32>(hoursOffline * 0.5); // 0.5 photons per hour

    Log::Info("Offline progress calculated successfully");

    return result;
}

// Helper function for rarity rolls
i32 TimeManager::RollRarity() {
    i32 roll = rand() % 100;
    if (roll >= 95) return 4; // 5% Legendary
    if (roll >= 85) return 3; // 10% Epic
    if (roll >= 65) return 2; // 20% Rare
    if (roll >= 35) return 1; // 30% Uncommon
    return 0; // 35% Common
}

// === Time-Based Cooldowns ===

void TimeManager::ActivateBoost() {
    if (m_BoostActive || m_BoostCooldownRemaining > 0.0) {
        return; // Already active or on cooldown
    }

    m_BoostActive = true;
    m_BoostTimeRemaining = m_BoostDuration;
    Log::Infof("Boost activated! 2x production for ", m_BoostDuration, " seconds!");
}

void TimeManager::IncrementCombo() {
    m_ComboCount++;
    m_ComboTimeRemaining = m_ComboWindow;
}

void TimeManager::TriggerPrestigeFlash() {
    m_PrestigeFlashActive = true;
    m_PrestigeFlashTimer = 0.5; // 0.5 second flash
}
