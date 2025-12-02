#pragma once

#include "Types.h"
#include <string>
#include <vector>

// Forward declarations
class Renderer;
class GameState;

// Ship part slot types
enum class PartSlot {
    Hull,
    Engine,
    Weapons,
    Shields,
    COUNT
};

// Rarity tiers for ship parts (affects bonus strength)
enum class PartRarity {
    Common,      // 60% drop chance, 1.0-1.5x bonus
    Uncommon,    // 25% drop chance, 1.5-2.5x bonus
    Rare,        // 10% drop chance, 2.5-4.0x bonus
    Epic,        // 4% drop chance, 4.0-7.0x bonus
    Legendary    // 1% drop chance, 7.0-15.0x bonus
};

// Individual ship part
struct ShipPart {
    std::string name;
    std::string description;
    PartSlot slot;
    PartRarity rarity;

    // Bonuses granted by this part
    f64 powerBonus;        // +% to global production
    f64 combatBonus;       // +% to damage (for future combat)
    f64 dropRateBonus;     // +% to part drop rate
    f64 repairContribution; // How much this part contributes to ship repair (1-10)

    bool installed;        // Is this part currently installed?
    i32 tier;              // Visual tier (1-5 based on rarity)

    // Enhancement system
    i32 enhancementLevel;  // +0 to +15 (adds 10% per level to bonuses)
    i32 stars;             // 1 to 6 stars (major stat boosts)

    ShipPart();
    ShipPart(PartSlot slotType, PartRarity rarityTier);

    Color GetRarityColor() const;
    const char* GetRarityName() const;
    const char* GetSlotName() const;

    // Enhancement helpers
    f64 GetEnhancementMultiplier() const; // 1.0 + (enhancementLevel * 0.10)
    f64 GetStarMultiplier() const;        // 1.0 + (stars - 1) * 0.25
    f64 GetTotalMultiplier() const;       // Enhancement * Star multipliers
};

// Main spaceship progression system
class Spaceship {
public:
    Spaceship();

    // Initialization
    void Initialize(GameState* gameState = nullptr);

    // Part management
    void AddPart(const ShipPart& part);
    bool InstallPart(i32 inventoryIndex);
    bool UninstallPart(PartSlot slot);
    ShipPart* GetInstalledPart(PartSlot slot);

    // Inventory
    const std::vector<ShipPart>& GetInventory() const { return m_Inventory; }
    i32 GetInventoryCount() const { return static_cast<i32>(m_Inventory.size()); }

    // Repair progress
    f64 GetRepairProgress() const { return m_RepairProgress; }
    void RecalculateRepairProgress();
    bool IsOperational() const { return m_RepairProgress >= 25.0; } // Can travel at 25%
    bool IsFullyRepaired() const { return m_RepairProgress >= 100.0; }

    // Bonuses
    f64 GetTotalPowerBonus() const { return m_TotalPowerBonus; }
    f64 GetTotalCombatBonus() const { return m_TotalCombatBonus; }
    f64 GetTotalDropRateBonus() const { return m_TotalDropRateBonus; }
    void RecalculateBonuses();

    // Statistics
    i32 GetTotalPartsCollected() const { return m_TotalPartsCollected; }
    i32 GetLegendaryPartsCollected() const { return m_LegendaryPartsCollected; }

    // Rendering
    void RenderShipPanel();
    void RenderInventoryPanel();

    // Save/Load
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    // Installed parts (one per slot)
    ShipPart* m_InstalledParts[static_cast<i32>(PartSlot::COUNT)];

    // Part inventory (uninstalled parts)
    std::vector<ShipPart> m_Inventory;

    // Ship status
    f64 m_RepairProgress;      // 0-100%
    f64 m_TotalPowerBonus;     // Sum of all installed part bonuses
    f64 m_TotalCombatBonus;
    f64 m_TotalDropRateBonus;

    // Statistics
    i32 m_TotalPartsCollected;
    i32 m_LegendaryPartsCollected;

    // UI state
    i32 m_SelectedInventoryIndex;
    f32 m_ScrollOffset;

    // Game state reference (for awarding skill XP)
    GameState* m_GameState;

    // Helper methods
    void UpdateStatistics(const ShipPart& part);
};

// Part generation utilities
namespace ShipPartGenerator {
    // Generate a random ship part with weighted rarity
    ShipPart GenerateRandomPart();

    // Generate specific rarity (for guaranteed drops)
    ShipPart GeneratePart(PartRarity rarity);

    // Get drop chance for each rarity
    f64 GetRarityDropChance(PartRarity rarity);
}
