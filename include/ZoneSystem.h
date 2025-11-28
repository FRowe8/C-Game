#pragma once

#include "Types.h"
#include "Enemy.h"
#include <string>
#include <vector>

// Forward declarations
class Renderer;
class GameState;

// Zone progression stages
enum class ZoneID {
    AsteroidBelt,      // Zone 1: Levels 1-20
    NebulaFrontier,    // Zone 2: Levels 20-40
    DerelictFleet,     // Zone 3: Levels 40-60
    BlackHoleHorizon,  // Zone 4: Levels 60-80
    AncientArmada,     // Zone 5: Levels 80-100
    COUNT
};

// Individual combat stage within a zone
struct Stage {
    i32 stageNumber;      // 1-15
    i32 recommendedLevel;
    bool isBossStage;     // Boss stages are tougher
    bool completed;       // Has this stage been completed?
    bool firstClearClaimed; // Has first-clear reward been claimed?

    // Rewards
    i32 creditsReward;
    i32 xpReward;
    i32 materialReward;   // Amount of zone-specific materials

    Stage();
    Stage(i32 num, i32 level, bool boss);
};

// Zone/Area definition
class Zone {
public:
    Zone();
    Zone(ZoneID id, const char* name, const char* desc, i32 minLvl, i32 unlockCost);

    // Zone info
    ZoneID GetID() const { return m_ID; }
    const char* GetName() const { return m_Name.c_str(); }
    const char* GetDescription() const { return m_Description.c_str(); }
    i32 GetMinLevel() const { return m_MinLevel; }
    i32 GetUnlockCost() const { return m_UnlockCost; }
    bool IsUnlocked() const { return m_Unlocked; }
    bool IsCompleted() const { return m_Completed; }

    // Stages
    i32 GetStageCount() const { return static_cast<i32>(m_Stages.size()); }
    Stage* GetStage(i32 index);
    i32 GetCurrentStage() const { return m_CurrentStage; }
    i32 GetCompletedStages() const;

    // Unlocking
    bool CanUnlock(GameState* state) const;
    void Unlock();

    // Stage progression
    bool CanEnterStage(i32 stageIndex, GameState* state) const;
    void StartStage(i32 stageIndex, GameState* state);
    void CompleteStage(i32 stageIndex);
    Enemy GenerateEnemyForStage(i32 stageIndex) const;

    // Rewards
    void ClaimFirstClearReward(i32 stageIndex, GameState* state);

    // Visual
    Color GetZoneColor() const;
    Color GetThemeColor() const;

private:
    ZoneID m_ID;
    std::string m_Name;
    std::string m_Description;
    i32 m_MinLevel;      // Minimum player level required
    i32 m_UnlockCost;    // Credit cost to unlock
    bool m_Unlocked;
    bool m_Completed;    // All stages completed
    i32 m_CurrentStage;  // Highest unlocked stage (0-based)

    std::vector<Stage> m_Stages;

    void InitializeStages();
};

// Zone management system
class ZoneSystem {
public:
    ZoneSystem();

    void Initialize();

    // Zone management
    Zone* GetZone(ZoneID id);
    Zone* GetCurrentZone();
    i32 GetUnlockedZoneCount() const;
    bool UnlockZone(ZoneID id, GameState* state);

    // Combat integration
    void StartZoneStage(ZoneID zoneID, i32 stageIndex, GameState* state);
    void CompleteCurrentStage();

    // UI
    void RenderZoneSelectionUI(Renderer* renderer, GameState* state);
    void RenderStageSelectionUI(Renderer* renderer, GameState* state);
    void HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state);

    // State
    ZoneID GetSelectedZone() const { return m_SelectedZone; }
    void SetSelectedZone(ZoneID id) { m_SelectedZone = id; }
    bool IsShowingStages() const { return m_ShowStages; }

    // Serialization
    std::string SaveToJson() const;
    void LoadFromJson(const std::string& line);

private:
    std::vector<Zone> m_Zones;
    ZoneID m_SelectedZone;  // Which zone is selected in UI
    ZoneID m_CurrentZone;   // Which zone player is in for combat
    i32 m_CurrentStageIndex; // Which stage in current zone
    bool m_ShowStages;      // Show stage selection vs zone selection

    void InitializeZones();
};
