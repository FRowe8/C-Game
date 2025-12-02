#include "ZoneSystem.h"
#include "Renderer.h"
#include "Logger.h"
#include "GameState.h"
#include <cstdlib>

// ===== Stage Implementation =====

Stage::Stage()
    : stageNumber(1), recommendedLevel(1), isBossStage(false),
      completed(false), firstClearClaimed(false),
      creditsReward(100), xpReward(50), materialReward(5) {
}

Stage::Stage(i32 num, i32 level, bool boss)
    : stageNumber(num), recommendedLevel(level), isBossStage(boss),
      completed(false), firstClearClaimed(false) {

    // Scale rewards with level
    creditsReward = 100 * level;
    xpReward = 50 * level;
    materialReward = 5 + (level / 10);

    // Boss stages give 3x rewards
    if (boss) {
        creditsReward *= 3;
        xpReward *= 3;
        materialReward *= 3;
    }
}

// ===== Zone Implementation =====

Zone::Zone()
    : m_ID(ZoneID::AsteroidBelt), m_Name("Unknown"), m_Description(""),
      m_MinLevel(1), m_UnlockCost(0), m_Unlocked(false),
      m_Completed(false), m_CurrentStage(0) {
}

Zone::Zone(ZoneID id, const char* name, const char* desc, i32 minLvl, i32 unlockCost)
    : m_ID(id), m_Name(name), m_Description(desc),
      m_MinLevel(minLvl), m_UnlockCost(unlockCost),
      m_Unlocked(false), m_Completed(false), m_CurrentStage(0) {

    InitializeStages();
}

void Zone::InitializeStages() {
    m_Stages.clear();

    // Calculate level range for this zone
    i32 baseLevelPerStage = 0;
    i32 startLevel = m_MinLevel;

    switch (m_ID) {
        case ZoneID::AsteroidBelt:
            baseLevelPerStage = 1; // Levels 1-20
            startLevel = 1;
            break;
        case ZoneID::NebulaFrontier:
            baseLevelPerStage = 2; // Levels 20-40
            startLevel = 20;
            break;
        case ZoneID::DerelictFleet:
            baseLevelPerStage = 2; // Levels 40-60
            startLevel = 40;
            break;
        case ZoneID::BlackHoleHorizon:
            baseLevelPerStage = 2; // Levels 60-80
            startLevel = 60;
            break;
        case ZoneID::AncientArmada:
            baseLevelPerStage = 2; // Levels 80-100
            startLevel = 80;
            break;
        default:
            baseLevelPerStage = 1;
            startLevel = 1;
            break;
    }

    // Create 12 normal stages + 3 boss stages = 15 total
    for (i32 i = 0; i < 15; i++) {
        bool isBoss = (i == 4 || i == 9 || i == 14); // Boss at stages 5, 10, 15
        i32 stageLevel = startLevel + (i * baseLevelPerStage);

        Stage stage(i + 1, stageLevel, isBoss);
        m_Stages.push_back(stage);
    }
}

Stage* Zone::GetStage(i32 index) {
    if (index < 0 || index >= static_cast<i32>(m_Stages.size())) {
        return nullptr;
    }
    return &m_Stages[index];
}

i32 Zone::GetCompletedStages() const {
    i32 count = 0;
    for (const auto& stage : m_Stages) {
        if (stage.completed) count++;
    }
    return count;
}

bool Zone::CanUnlock(GameState* state) const {
    if (m_Unlocked) return false;

    // Check level requirement
    if (state->GetPlayerLevel() < m_MinLevel) return false;

    // Check credits
    if (state->GetResource(QuantumResource::Qubits) < m_UnlockCost) return false;

    // First zone is always unlockable
    if (m_ID == ZoneID::AsteroidBelt) return true;

    // Other zones require previous zone to be completed
    // (This will be checked by ZoneSystem)

    return true;
}

void Zone::Unlock() {
    m_Unlocked = true;
    m_CurrentStage = 0; // Start at first stage
    Log::Infof("Zone unlocked: ", m_Name);
}

bool Zone::CanEnterStage(i32 stageIndex, GameState* state) const {
    if (!m_Unlocked) return false;
    if (stageIndex < 0 || stageIndex >= static_cast<i32>(m_Stages.size())) return false;

    // Must complete previous stage first (except stage 0)
    if (stageIndex > 0 && !m_Stages[stageIndex - 1].completed) {
        return false;
    }

    // Check level requirement
    if (state->GetPlayerLevel() < m_Stages[stageIndex].recommendedLevel) {
        return false;
    }

    return true;
}

void Zone::StartStage(i32 stageIndex, GameState* state) {
    (void)state;
    if (stageIndex >= 0 && stageIndex < static_cast<i32>(m_Stages.size())) {
        m_CurrentStage = stageIndex;
        Log::Infof("Starting zone ", m_Name, " stage ", stageIndex + 1);
    }
}

void Zone::CompleteStage(i32 stageIndex) {
    if (stageIndex >= 0 && stageIndex < static_cast<i32>(m_Stages.size())) {
        m_Stages[stageIndex].completed = true;

        // Check if zone is fully completed
        if (GetCompletedStages() == GetStageCount()) {
            m_Completed = true;
            Log::Infof("Zone completed: ", m_Name);
        }
    }
}

Enemy Zone::GenerateEnemyForStage(i32 stageIndex) const {
    if (stageIndex < 0 || stageIndex >= static_cast<i32>(m_Stages.size())) {
        return EnemyGenerator::GenerateEnemy(1); // Fallback
    }

    const Stage& stage = m_Stages[stageIndex];

    // Boss stages generate boss enemies
    if (stage.isBossStage) {
        EnemyTier tier = EnemyGenerator::GetTierFromLevel(stage.recommendedLevel);
        return EnemyGenerator::GenerateBoss(tier);
    }

    // Normal stages generate random enemies for the level
    return EnemyGenerator::GenerateEnemy(stage.recommendedLevel);
}

void Zone::ClaimFirstClearReward(i32 stageIndex, GameState* state) {
    if (stageIndex < 0 || stageIndex >= static_cast<i32>(m_Stages.size())) return;

    Stage& stage = m_Stages[stageIndex];

    if (!stage.completed || stage.firstClearClaimed) return;

    // Award rewards
    state->AddResource(QuantumResource::Qubits, static_cast<f64>(stage.creditsReward * 2)); // 2x for first clear
    state->AddXP(static_cast<f64>(stage.xpReward * 2));

    // Award materials
    state->GetEnhancementSystem().AddMaterial(MaterialType::TechScraps, stage.materialReward);
    if (stage.isBossStage) {
        state->GetEnhancementSystem().AddMaterial(MaterialType::NanoAlloy, stage.materialReward / 2);
        state->GetEnhancementSystem().AddMaterial(MaterialType::UniversalShards, 1);
    }

    stage.firstClearClaimed = true;
    Log::Infof("First clear reward claimed for ", m_Name, " stage ", stageIndex + 1);
}

Color Zone::GetZoneColor() const {
    switch (m_ID) {
        case ZoneID::AsteroidBelt:
            return Color(0.6f, 0.6f, 0.6f, 1.0f); // Gray
        case ZoneID::NebulaFrontier:
            return Color(0.4f, 0.6f, 1.0f, 1.0f); // Blue
        case ZoneID::DerelictFleet:
            return Color(0.8f, 0.4f, 0.2f, 1.0f); // Orange/Rust
        case ZoneID::BlackHoleHorizon:
            return Color(0.6f, 0.3f, 0.8f, 1.0f); // Purple
        case ZoneID::AncientArmada:
            return Color(1.0f, 0.8f, 0.3f, 1.0f); // Gold
        default:
            return Color::White();
    }
}

Color Zone::GetThemeColor() const {
    return GetZoneColor();
}

// ===== ZoneSystem Implementation =====

ZoneSystem::ZoneSystem()
    : m_SelectedZone(ZoneID::AsteroidBelt),
      m_CurrentZone(ZoneID::AsteroidBelt),
      m_CurrentStageIndex(0),
      m_ShowStages(false) {
}

void ZoneSystem::Initialize() {
    Log::Info("Initializing zone system...");

    InitializeZones();

    // Unlock first zone by default
    m_Zones[0].Unlock();

    Log::Info("Zone system initialized");
}

void ZoneSystem::InitializeZones() {
    m_Zones.clear();

    // Zone 1: Asteroid Belt (Levels 1-20)
    Zone zone1(ZoneID::AsteroidBelt,
               "Asteroid Belt",
               "Mining drones and pirates lurk in the asteroid field",
               1,     // Min level 1
               0);    // Free to unlock
    m_Zones.push_back(zone1);

    // Zone 2: Nebula Frontier (Levels 20-40)
    Zone zone2(ZoneID::NebulaFrontier,
               "Nebula Frontier",
               "Alien scouts patrol the colorful cosmic clouds",
               20,    // Min level 20
               10000); // 10k credits to unlock
    m_Zones.push_back(zone2);

    // Zone 3: Derelict Fleet (Levels 40-60)
    Zone zone3(ZoneID::DerelictFleet,
               "Derelict Fleet",
               "Corrupted AI ships guard the abandoned armada",
               40,    // Min level 40
               50000); // 50k credits to unlock
    m_Zones.push_back(zone3);

    // Zone 4: Black Hole Horizon (Levels 60-80)
    Zone zone4(ZoneID::BlackHoleHorizon,
               "Black Hole Horizon",
               "Void entities warp space at the event horizon",
               60,    // Min level 60
               200000); // 200k credits to unlock
    m_Zones.push_back(zone4);

    // Zone 5: Ancient Armada (Levels 80-100)
    Zone zone5(ZoneID::AncientArmada,
               "Ancient Armada",
               "Legendary warships from a forgotten civilization",
               80,    // Min level 80
               1000000); // 1M credits to unlock
    m_Zones.push_back(zone5);
}

Zone* ZoneSystem::GetZone(ZoneID id) {
    i32 index = static_cast<i32>(id);
    if (index < 0 || index >= static_cast<i32>(m_Zones.size())) {
        return nullptr;
    }
    return &m_Zones[index];
}

Zone* ZoneSystem::GetCurrentZone() {
    return GetZone(m_CurrentZone);
}

i32 ZoneSystem::GetUnlockedZoneCount() const {
    i32 count = 0;
    for (const auto& zone : m_Zones) {
        if (zone.IsUnlocked()) count++;
    }
    return count;
}

bool ZoneSystem::UnlockZone(ZoneID id, GameState* state) {
    Zone* zone = GetZone(id);
    if (!zone) return false;

    if (!zone->CanUnlock(state)) return false;

    // Check if previous zone is completed (except first zone)
    if (id != ZoneID::AsteroidBelt) {
        i32 prevZoneIndex = static_cast<i32>(id) - 1;
        if (prevZoneIndex >= 0 && prevZoneIndex < static_cast<i32>(m_Zones.size())) {
            if (!m_Zones[prevZoneIndex].IsCompleted()) {
                Log::Info("Must complete previous zone first");
                return false;
            }
        }
    }

    // Spend credits
    if (!state->SpendResource(QuantumResource::Qubits, zone->GetUnlockCost())) {
        return false;
    }

    zone->Unlock();
    return true;
}

void ZoneSystem::StartZoneStage(ZoneID zoneID, i32 stageIndex, GameState* state) {
    Zone* zone = GetZone(zoneID);
    if (!zone) return;

    if (!zone->CanEnterStage(stageIndex, state)) {
        Log::Info("Cannot enter this stage yet");
        return;
    }

    m_CurrentZone = zoneID;
    m_CurrentStageIndex = stageIndex;

    zone->StartStage(stageIndex, state);

    // Generate enemy for this stage and start combat
    Enemy enemy = zone->GenerateEnemyForStage(stageIndex);
    // FIX: Added nullptr for EnergyGrid* (4th arg) and passed state as 5th arg
    state->GetCombatSystem().StartCombat(&enemy, state->GetPlayerLevel(), &state->GetSpaceship(), nullptr, state);
}

void ZoneSystem::CompleteCurrentStage() {
    Zone* zone = GetCurrentZone();
    if (zone) {
        zone->CompleteStage(m_CurrentStageIndex);
    }
}

void ZoneSystem::RenderZoneSelectionUI(Renderer* renderer, GameState* state) {
    (void)renderer;
    (void)state;
    // TODO: Render zone selection screen
}

void ZoneSystem::RenderStageSelectionUI(Renderer* renderer, GameState* state) {
    (void)renderer;
    (void)state;
    // TODO: Render stage selection for selected zone
}

void ZoneSystem::HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state) {
    (void)mouseX;
    (void)mouseY;
    (void)mousePressed;
    (void)state;
    // TODO: Handle zone/stage selection clicks
}

std::string ZoneSystem::SaveToJson() const {
    // TODO: Implement save
    return "{}";
}

void ZoneSystem::LoadFromJson(const std::string& line) {
    (void)line;
    // TODO: Implement load
}
