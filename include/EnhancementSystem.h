#pragma once

#include "Types.h"
#include "Spaceship.h"
#include <map>

// Forward declarations
class Renderer;
class GameState;

// Enhancement materials
enum class MaterialType {
    TechScraps,    // Common - from dismantling parts
    NanoAlloy,     // Rare - from elite enemies/combat
    QuantumCore,   // Epic - from raids/bosses
    UniversalShards, // Special - for star ascension
    COUNT
};

// Enhancement result
enum class EnhancementResult {
    Success,
    Failure,      // Item not upgraded, materials lost
    MaxLevel      // Already at max level
};

// Material inventory entry
struct MaterialInventory {
    i32 techScraps;
    i32 nanoAlloy;
    i32 quantumCore;
    i32 universalShards;

    MaterialInventory()
        : techScraps(0), nanoAlloy(0), quantumCore(0), universalShards(0) {}
};

// Enhancement cost structure
struct EnhancementCost {
    i32 credits;
    i32 techScraps;
    i32 nanoAlloy;
    i32 quantumCore;
    f64 successRate; // 0.0 to 1.0

    EnhancementCost()
        : credits(0), techScraps(0), nanoAlloy(0), quantumCore(0), successRate(1.0) {}
};

// Star ascension cost
struct AscensionCost {
    i32 universalShards;  // Can use shards instead of duplicates
    i32 credits;
    bool requiresDuplicate; // Need duplicate part?

    AscensionCost()
        : universalShards(0), credits(0), requiresDuplicate(false) {}
};

// Enhancement system - manage part upgrading and materials
class EnhancementSystem {
public:
    EnhancementSystem();

    void Initialize();

    // Material management
    void AddMaterial(MaterialType type, i32 amount);
    bool SpendMaterial(MaterialType type, i32 amount);
    i32 GetMaterial(MaterialType type) const;
    const MaterialInventory& GetInventory() const { return m_Materials; }

    // Part enhancement
    EnhancementResult EnhancePart(ShipPart& part, GameState* state);
    EnhancementCost GetEnhancementCost(const ShipPart& part) const;
    f64 GetSuccessRate(i32 currentLevel) const;
    bool CanEnhance(const ShipPart& part, GameState* state) const;

    // Star ascension
    bool AscendPart(ShipPart& part, GameState* state, bool useDuplicate);
    AscensionCost GetAscensionCost(const ShipPart& part) const;
    bool CanAscend(const ShipPart& part, GameState* state, bool hasDuplicate) const;

    // Part dismantling
    void DismantlePart(const ShipPart& part);
    MaterialInventory GetDismantleReward(const ShipPart& part) const;

    // Rendering
    void RenderEnhancementUI(Renderer* renderer, GameState* state);
    void HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state);

    // Serialization
    std::string SaveToJson() const;
    void LoadFromJson(const std::string& line);

private:
    MaterialInventory m_Materials;

    // Selected part for enhancement
    i32 m_SelectedInventoryIndex;
    bool m_ShowEnhancementUI;
    bool m_ShowConfirmDismantle;

    // Helper methods
    void RenderMaterialDisplay(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth);
    void RenderPartDetails(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth);
    void RenderEnhancementOptions(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth);

    Color GetMaterialColor(MaterialType type) const;
    const char* GetMaterialName(MaterialType type) const;
};