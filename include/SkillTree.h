#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <unordered_map>

// Forward declarations
class Renderer;
class GameState;

// Skill categories (three main branches)
enum class SkillBranch {
    Combat,         // Offensive abilities, damage, critical hits
    Engineering,    // Defense, repair, production bonuses
    Exploration     // Loot, discovery, efficiency
};

// Individual skill ID
enum class SkillID {
    // Combat Branch (offensive)
    CombatBasics,           // +10% damage
    PowerStrike,            // +20% damage, requires CombatBasics
    CriticalHit,            // +5% crit chance
    DeadlyPrecision,        // +10% crit chance, requires CriticalHit
    Execution,              // +50% damage to enemies below 25% HP
    MultiTarget,            // 25% chance to hit twice
    LifeSteal,              // Heal 10% of damage dealt
    BattleMastery,          // +30% damage, requires PowerStrike

    // Engineering Branch (defensive/production)
    HullReinforcement,      // +15% max HP
    AdvancedArmor,          // +30% max HP, requires HullReinforcement
    AutoRepair,             // Heal 2% HP per turn
    Overcharge,             // +50% damage but costs 5% HP per attack
    EmergencyShield,        // Block one fatal hit per combat
    PartEfficiency,         // +20% bonus from all installed parts
    ProductionBoost,        // +10% global production
    MasterEngineer,         // +50% bonus from parts, requires PartEfficiency

    // Exploration Branch (utility/loot)
    Scavenger,              // +10% drop rates
    TreasureHunter,         // +25% drop rates, requires Scavenger
    AdvancedScanning,       // See enemy stats before battle
    FastTravel,             // Reduce combat animation time
    LuckyFind,              // 5% chance for double loot
    SafePassage,            // 20% chance to avoid losing on defeat
    RareDiscovery,          // +1 min rarity on drops
    LegendSeeker,           // +10% legendary drop rate, requires TreasureHunter

    COUNT
};

// Skill node in the tree
struct Skill {
    SkillID id;
    std::string name;
    std::string description;
    SkillBranch branch;

    i32 cost;                           // Skill points required
    i32 level;                          // Current level (0 = not learned)
    i32 maxLevel;                       // Maximum level (usually 1 or 5)

    std::vector<SkillID> prerequisites; // Must have these first
    i32 minPlayerLevel;                 // Minimum player level required

    // Position in skill tree UI (for rendering)
    i32 treeRow;
    i32 treeColumn;

    // Effect values (scale with level)
    f64 effectValue;                    // Primary effect (damage %, HP %, etc.)

    Skill();
    Skill(SkillID id, const char* name, const char* desc, SkillBranch branch,
          i32 cost, i32 maxLvl, i32 minLvl, i32 row, i32 col);

    bool IsLearned() const { return level > 0; }
    bool IsMaxed() const { return level >= maxLevel; }
    f64 GetCurrentEffect() const { return effectValue * level; }
};

// Skill tree system
class SkillTreeSystem {
public:
    SkillTreeSystem();

    // Initialization
    void Initialize();

    // Skill points
    void AddSkillPoints(i32 amount) { m_SkillPoints += amount; }
    i32 GetSkillPoints() const { return m_SkillPoints; }
    i32 GetTotalSkillPointsSpent() const { return m_TotalSpent; }

    // Learning skills
    bool CanLearnSkill(SkillID id, GameState* state) const;
    bool LearnSkill(SkillID id, GameState* state);
    bool CanResetSkills(GameState* state) const; // Check if can afford reset
    bool ResetSkills(GameState* state); // Reset all skills (costs Photons)

    // Query skills
    Skill* GetSkill(SkillID id);
    const Skill* GetSkill(SkillID id) const;
    bool HasSkill(SkillID id) const;
    i32 GetSkillLevel(SkillID id) const;

    // Get all skills in a branch
    std::vector<Skill*> GetSkillsInBranch(SkillBranch branch);

    // Combat effects (query these during combat)
    f64 GetDamageMultiplier() const;
    f64 GetMaxHPMultiplier() const;
    f64 GetCritChanceBonus() const;
    f64 GetDropRateMultiplier() const;
    f64 GetPartBonusMultiplier() const;
    f64 GetProductionMultiplier() const;
    bool HasAutoRepair() const;
    bool HasEmergencyShield() const;
    bool HasLifeSteal() const;
    bool HasMultiTarget() const;
    bool HasAdvancedScanning() const;
    f64 GetLifeStealPercent() const;
    f64 GetAutoRepairPercent() const;

    // Rendering
    void RenderSkillTree(Renderer* renderer, GameState* state);

    // Input handling
    void HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state);

    // Save/Load
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    // Skill data
    std::unordered_map<SkillID, Skill> m_Skills;

    // Skill points
    i32 m_SkillPoints;
    i32 m_TotalSpent;

    // UI state
    SkillBranch m_SelectedBranch;
    f32 m_ScrollOffset;

    // Helper methods
    void InitializeSkills();
    void RenderBranchTabs(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth);
    void RenderSkillNodes(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth);
    void RenderSkillInfo(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth, f32 panelHeight);
    void RenderSkillConnections(Renderer* renderer, f32 nodeStartX, f32 nodeStartY, f32 nodeWidth, f32 nodeHeight);

    Color GetBranchColor(SkillBranch branch) const;
    bool CheckPrerequisites(const Skill& skill, GameState* state) const;
};
