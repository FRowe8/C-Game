#include "SkillTree.h"
#include "Renderer.h"
#include "Logger.h"
#include "GameState.h"
#include <fstream>

// Skill Implementation
Skill::Skill()
    : id(SkillID::COUNT), name("Unknown"), description(""),
      branch(SkillBranch::Combat), cost(1), level(0), maxLevel(1),
      minPlayerLevel(1), treeRow(0), treeColumn(0), effectValue(0.0) {
}

Skill::Skill(SkillID id, const char* name, const char* desc, SkillBranch branch,
             i32 cost, i32 maxLvl, i32 minLvl, i32 row, i32 col)
    : id(id), name(name), description(desc), branch(branch),
      cost(cost), level(0), maxLevel(maxLvl), minPlayerLevel(minLvl),
      treeRow(row), treeColumn(col), effectValue(0.0) {
}

// SkillTreeSystem Implementation
SkillTreeSystem::SkillTreeSystem()
    : m_SkillPoints(0), m_TotalSpent(0),
      m_SelectedBranch(SkillBranch::Combat), m_ScrollOffset(0.0f) {
}

void SkillTreeSystem::Initialize() {
    Log::Info("Initializing skill tree system...");
    InitializeSkills();
    Log::Info("Skill tree initialized");
}

void SkillTreeSystem::InitializeSkills() {
    m_Skills.clear();

    // ========== COMBAT BRANCH ==========
    // Row 0 (Starting skills)
    Skill combatBasics(SkillID::CombatBasics, "Combat Basics",
        "+10% damage in combat", SkillBranch::Combat, 1, 5, 1, 0, 0);
    combatBasics.effectValue = 10.0; // 10% per level
    m_Skills[SkillID::CombatBasics] = combatBasics;

    Skill criticalHit(SkillID::CriticalHit,
        "Critical Hit", "+5% critical hit chance", SkillBranch::Combat, 1, 3, 1, 0, 1);
    criticalHit.effectValue = 5.0;
    m_Skills[SkillID::CriticalHit] = criticalHit;

    // Row 1 (Tier 2)
    Skill powerStrike(SkillID::PowerStrike,
        "Power Strike", "+20% damage in combat", SkillBranch::Combat, 2, 5, 10, 1, 0);
    powerStrike.effectValue = 20.0;
    powerStrike.prerequisites.push_back(SkillID::CombatBasics);
    m_Skills[SkillID::PowerStrike] = powerStrike;

    Skill deadlyPrecision(SkillID::DeadlyPrecision,
        "Deadly Precision", "+10% critical hit chance", SkillBranch::Combat, 2, 3, 10, 1, 1);
    deadlyPrecision.effectValue = 10.0;
    deadlyPrecision.prerequisites.push_back(SkillID::CriticalHit);
    m_Skills[SkillID::DeadlyPrecision] = deadlyPrecision;

    Skill lifeSteal(SkillID::LifeSteal,
        "Life Steal", "Heal for 10% of damage dealt", SkillBranch::Combat, 3, 1, 15, 1, 2);
    lifeSteal.effectValue = 10.0;
    m_Skills[SkillID::LifeSteal] = lifeSteal;

    // Row 2 (Tier 3)
    Skill battleMastery(SkillID::BattleMastery,
        "Battle Mastery", "+30% damage in combat", SkillBranch::Combat, 3, 5, 25, 2, 0);
    battleMastery.effectValue = 30.0;
    battleMastery.prerequisites.push_back(SkillID::PowerStrike);
    m_Skills[SkillID::BattleMastery] = battleMastery;

    Skill execution(SkillID::Execution,
        "Execution", "+50% damage vs enemies below 25% HP", SkillBranch::Combat, 3, 1, 20, 2, 1);
    execution.effectValue = 50.0;
    execution.prerequisites.push_back(SkillID::DeadlyPrecision);
    m_Skills[SkillID::Execution] = execution;

    Skill multiTarget(SkillID::MultiTarget,
        "Multi-Target", "25% chance to attack twice", SkillBranch::Combat, 4, 1, 30, 2, 2);
    multiTarget.effectValue = 25.0;
    multiTarget.prerequisites.push_back(SkillID::LifeSteal);
    m_Skills[SkillID::MultiTarget] = multiTarget;

    // ========== ENGINEERING BRANCH ==========
    // Row 0
    Skill hullReinforcement(SkillID::HullReinforcement,
        "Hull Reinforcement", "+15% maximum HP", SkillBranch::Engineering, 1, 5, 1, 0, 0);
    hullReinforcement.effectValue = 15.0;
    m_Skills[SkillID::HullReinforcement] = hullReinforcement;

    Skill productionBoost(SkillID::ProductionBoost,
        "Production Boost", "+10% global production", SkillBranch::Engineering, 1, 5, 1, 0, 1);
    productionBoost.effectValue = 10.0;
    m_Skills[SkillID::ProductionBoost] = productionBoost;

    // Row 1
    Skill advancedArmor(SkillID::AdvancedArmor,
        "Advanced Armor", "+30% maximum HP", SkillBranch::Engineering, 2, 5, 10, 1, 0);
    advancedArmor.effectValue = 30.0;
    advancedArmor.prerequisites.push_back(SkillID::HullReinforcement);
    m_Skills[SkillID::AdvancedArmor] = advancedArmor;

    Skill autoRepair(SkillID::AutoRepair,
        "Auto Repair", "Heal 2% HP per turn in combat", SkillBranch::Engineering, 3, 1, 15, 1, 1);
    autoRepair.effectValue = 2.0;
    autoRepair.prerequisites.push_back(SkillID::HullReinforcement);
    m_Skills[SkillID::AutoRepair] = autoRepair;

    Skill partEfficiency(SkillID::PartEfficiency,
        "Part Efficiency", "+20% bonus from ship parts", SkillBranch::Engineering, 2, 5, 10, 1, 2);
    partEfficiency.effectValue = 20.0;
    partEfficiency.prerequisites.push_back(SkillID::ProductionBoost);
    m_Skills[SkillID::PartEfficiency] = partEfficiency;

    // Row 2
    Skill emergencyShield(SkillID::EmergencyShield,
        "Emergency Shield", "Block one fatal hit per combat", SkillBranch::Engineering, 4, 1, 25, 2, 0);
    emergencyShield.effectValue = 1.0;
    emergencyShield.prerequisites.push_back(SkillID::AdvancedArmor);
    m_Skills[SkillID::EmergencyShield] = emergencyShield;

    Skill overcharge(SkillID::Overcharge,
        "Overcharge", "+50% damage but costs 5% HP", SkillBranch::Engineering, 3, 1, 20, 2, 1);
    overcharge.effectValue = 50.0;
    overcharge.prerequisites.push_back(SkillID::AutoRepair);
    m_Skills[SkillID::Overcharge] = overcharge;

    Skill masterEngineer(SkillID::MasterEngineer,
        "Master Engineer", "+50% bonus from ship parts", SkillBranch::Engineering, 4, 5, 30, 2, 2);
    masterEngineer.effectValue = 50.0;
    masterEngineer.prerequisites.push_back(SkillID::PartEfficiency);
    m_Skills[SkillID::MasterEngineer] = masterEngineer;

    // ========== EXPLORATION BRANCH ==========
    // Row 0
    Skill scavenger(SkillID::Scavenger,
        "Scavenger", "+10% drop rates", SkillBranch::Exploration, 1, 5, 1, 0, 0);
    scavenger.effectValue = 10.0;
    m_Skills[SkillID::Scavenger] = scavenger;

    Skill fastTravel(SkillID::FastTravel,
        "Fast Travel", "Speed up combat animations", SkillBranch::Exploration, 1, 1, 1, 0, 1);
    fastTravel.effectValue = 1.0;
    m_Skills[SkillID::FastTravel] = fastTravel;

    // Row 1
    Skill treasureHunter(SkillID::TreasureHunter,
        "Treasure Hunter", "+25% drop rates", SkillBranch::Exploration, 2, 5, 10, 1, 0);
    treasureHunter.effectValue = 25.0;
    treasureHunter.prerequisites.push_back(SkillID::Scavenger);
    m_Skills[SkillID::TreasureHunter] = treasureHunter;

    Skill advancedScanning(SkillID::AdvancedScanning,
        "Advanced Scanning", "See enemy stats before battle", SkillBranch::Exploration, 2, 1, 10, 1, 1);
    advancedScanning.effectValue = 1.0;
    advancedScanning.prerequisites.push_back(SkillID::FastTravel);
    m_Skills[SkillID::AdvancedScanning] = advancedScanning;

    Skill luckyFind(SkillID::LuckyFind,
        "Lucky Find", "5% chance for double loot", SkillBranch::Exploration, 3, 1, 15, 1, 2);
    luckyFind.effectValue = 5.0;
    m_Skills[SkillID::LuckyFind] = luckyFind;

    // Row 2
    Skill legendSeeker(SkillID::LegendSeeker,
        "Legend Seeker", "+10% legendary drop rate", SkillBranch::Exploration, 4, 5, 25, 2, 0);
    legendSeeker.effectValue = 10.0;
    legendSeeker.prerequisites.push_back(SkillID::TreasureHunter);
    m_Skills[SkillID::LegendSeeker] = legendSeeker;

    Skill safePassage(SkillID::SafePassage,
        "Safe Passage", "20% chance to avoid defeat penalty", SkillBranch::Exploration, 3, 1, 20, 2, 1);
    safePassage.effectValue = 20.0;
    safePassage.prerequisites.push_back(SkillID::AdvancedScanning);
    m_Skills[SkillID::SafePassage] = safePassage;

    Skill rareDiscovery(SkillID::RareDiscovery,
        "Rare Discovery", "+1 minimum rarity on drops", SkillBranch::Exploration, 4, 1, 30, 2, 2);
    rareDiscovery.effectValue = 1.0;
    rareDiscovery.prerequisites.push_back(SkillID::LuckyFind);
    m_Skills[SkillID::RareDiscovery] = rareDiscovery;
}

Skill* SkillTreeSystem::GetSkill(SkillID id) {
    auto it = m_Skills.find(id);
    return (it != m_Skills.end()) ? &it->second : nullptr;
}

const Skill* SkillTreeSystem::GetSkill(SkillID id) const {
    auto it = m_Skills.find(id);
    return (it != m_Skills.end()) ? &it->second : nullptr;
}

bool SkillTreeSystem::HasSkill(SkillID id) const {
    const Skill* skill = GetSkill(id);
    return skill && skill->IsLearned();
}

i32 SkillTreeSystem::GetSkillLevel(SkillID id) const {
    const Skill* skill = GetSkill(id);
    return skill ? skill->level : 0;
}

std::vector<Skill*> SkillTreeSystem::GetSkillsInBranch(SkillBranch branch) {
    std::vector<Skill*> skills;
    for (auto& pair : m_Skills) {
        if (pair.second.branch == branch) {
            skills.push_back(&pair.second);
        }
    }
    return skills;
}

bool SkillTreeSystem::CheckPrerequisites(const Skill& skill, GameState* state) const {
    // Check player level
    if (state->GetPlayerLevel() < skill.minPlayerLevel) {
        return false;
    }

    // Check prerequisite skills
    for (SkillID prereq : skill.prerequisites) {
        if (!HasSkill(prereq)) {
            return false;
        }
    }

    return true;
}

bool SkillTreeSystem::CanLearnSkill(SkillID id, GameState* state) const {
    const Skill* skill = GetSkill(id);
    if (!skill) return false;

    // Already maxed?
    if (skill->IsMaxed()) return false;

    // Enough skill points?
    if (m_SkillPoints < skill->cost) return false;

    // Check prerequisites
    if (!CheckPrerequisites(*skill, state)) return false;

    return true;
}

bool SkillTreeSystem::LearnSkill(SkillID id, GameState* state) {
    if (!CanLearnSkill(id, state)) return false;

    Skill* skill = GetSkill(id);
    if (!skill) return false;

    // Deduct cost
    m_SkillPoints -= skill->cost;
    m_TotalSpent += skill->cost;

    // Increase level
    skill->level++;

    Log::Infof("Learned skill: ", skill->name, " (Level ", skill->level, "/", skill->maxLevel, ")");

    return true;
}

bool SkillTreeSystem::ResetSkills() {
    // TODO: Add credit cost for reset
    // Refund all spent points
    m_SkillPoints += m_TotalSpent;
    m_TotalSpent = 0;

    // Reset all skill levels
    for (auto& pair : m_Skills) {
        pair.second.level = 0;
    }

    Log::Info("All skills reset!");
    return true;
}

// Combat effect getters
f64 SkillTreeSystem::GetDamageMultiplier() const {
    f64 multiplier = 1.0;

    // Combat branch bonuses
    multiplier += GetSkillLevel(SkillID::CombatBasics) * 0.10;
    multiplier += GetSkillLevel(SkillID::PowerStrike) * 0.20;
    multiplier += GetSkillLevel(SkillID::BattleMastery) * 0.30;

    // Engineering: Overcharge
    if (HasSkill(SkillID::Overcharge)) {
        multiplier += 0.50;
    }

    return multiplier;
}

f64 SkillTreeSystem::GetMaxHPMultiplier() const {
    f64 multiplier = 1.0;

    multiplier += GetSkillLevel(SkillID::HullReinforcement) * 0.15;
    multiplier += GetSkillLevel(SkillID::AdvancedArmor) * 0.30;

    return multiplier;
}

f64 SkillTreeSystem::GetCritChanceBonus() const {
    f64 bonus = 0.0;

    bonus += GetSkillLevel(SkillID::CriticalHit) * 5.0;
    bonus += GetSkillLevel(SkillID::DeadlyPrecision) * 10.0;

    return bonus;
}

f64 SkillTreeSystem::GetDropRateMultiplier() const {
    f64 multiplier = 1.0;

    multiplier += GetSkillLevel(SkillID::Scavenger) * 0.10;
    multiplier += GetSkillLevel(SkillID::TreasureHunter) * 0.25;
    multiplier += GetSkillLevel(SkillID::LegendSeeker) * 0.10;

    return multiplier;
}

f64 SkillTreeSystem::GetPartBonusMultiplier() const {
    f64 multiplier = 1.0;

    multiplier += GetSkillLevel(SkillID::PartEfficiency) * 0.20;
    multiplier += GetSkillLevel(SkillID::MasterEngineer) * 0.50;

    return multiplier;
}

f64 SkillTreeSystem::GetProductionMultiplier() const {
    f64 multiplier = 1.0;

    multiplier += GetSkillLevel(SkillID::ProductionBoost) * 0.10;

    return multiplier;
}

bool SkillTreeSystem::HasAutoRepair() const {
    return HasSkill(SkillID::AutoRepair);
}

bool SkillTreeSystem::HasEmergencyShield() const {
    return HasSkill(SkillID::EmergencyShield);
}

bool SkillTreeSystem::HasLifeSteal() const {
    return HasSkill(SkillID::LifeSteal);
}

bool SkillTreeSystem::HasMultiTarget() const {
    return HasSkill(SkillID::MultiTarget);
}

bool SkillTreeSystem::HasAdvancedScanning() const {
    return HasSkill(SkillID::AdvancedScanning);
}

f64 SkillTreeSystem::GetLifeStealPercent() const {
    const Skill* skill = GetSkill(SkillID::LifeSteal);
    return skill ? skill->GetCurrentEffect() : 0.0;
}

f64 SkillTreeSystem::GetAutoRepairPercent() const {
    const Skill* skill = GetSkill(SkillID::AutoRepair);
    return skill ? skill->GetCurrentEffect() : 0.0;
}

Color SkillTreeSystem::GetBranchColor(SkillBranch branch) const {
    switch (branch) {
        case SkillBranch::Combat: return Color(1.0f, 0.3f, 0.3f, 1.0f); // Red
        case SkillBranch::Engineering: return Color(0.3f, 0.7f, 1.0f, 1.0f); // Blue
        case SkillBranch::Exploration: return Color(0.3f, 1.0f, 0.5f, 1.0f); // Green
    }
    return Color::White();
}

void SkillTreeSystem::RenderBranchTabs(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 yOffset = panelY + 60.0f;

    renderer->DrawText("SELECT BRANCH:", Vec2(panelX + 20.0f, yOffset), Color::NeonCyan(), 16.0f);
    yOffset += 28.0f;

    f32 tabWidth = (panelWidth - 60.0f) / 3.0f;
    f32 tabHeight = 50.0f;
    f32 tabSpacing = 10.0f;

    const char* branchNames[] = {"COMBAT", "ENGINEERING", "EXPLORATION"};
    SkillBranch branches[] = {SkillBranch::Combat, SkillBranch::Engineering, SkillBranch::Exploration};

    for (i32 i = 0; i < 3; i++) {
        f32 tabX = panelX + 20.0f + i * (tabWidth + tabSpacing);
        Rect tabRect(tabX, yOffset, tabWidth, tabHeight);

        bool selected = (m_SelectedBranch == branches[i]);
        Color branchColor = GetBranchColor(branches[i]);
        Color bgColor = selected ? (branchColor * 0.4f) : (branchColor * 0.2f);

        renderer->DrawRect(tabRect, bgColor, true);
        renderer->DrawRect(tabRect, branchColor, false);

        if (selected) {
            Rect glowRect(tabX - 2.0f, yOffset - 2.0f, tabWidth + 4.0f, tabHeight + 4.0f);
            renderer->DrawRect(glowRect, branchColor * 0.8f, false);
        }

        f32 textWidth = strlen(branchNames[i]) * 6.5f;
        Vec2 textPos(tabX + (tabWidth - textWidth) * 0.5f, yOffset + (tabHeight - 14.0f) * 0.5f);
        renderer->DrawText(branchNames[i], textPos, Color::White(), 14.0f);
    }
}

void SkillTreeSystem::RenderSkillConnections(Renderer* renderer, f32 nodeStartX, f32 nodeStartY,
                                             f32 nodeWidth, f32 nodeHeight) {
    // Draw connections between skills and their prerequisites
    auto skills = GetSkillsInBranch(m_SelectedBranch);
    Color connectionColor = GetBranchColor(m_SelectedBranch) * 0.5f;

    for (const Skill* skill : skills) {
        if (skill->prerequisites.empty()) continue;

        f32 skillX = nodeStartX + skill->treeColumn * (nodeWidth + 30.0f) + nodeWidth * 0.5f;
        f32 skillY = nodeStartY + skill->treeRow * (nodeHeight + 40.0f) + nodeHeight * 0.5f;

        for (SkillID prereqID : skill->prerequisites) {
            const Skill* prereq = GetSkill(prereqID);
            if (!prereq) continue;

            f32 prereqX = nodeStartX + prereq->treeColumn * (nodeWidth + 30.0f) + nodeWidth * 0.5f;
            f32 prereqY = nodeStartY + prereq->treeRow * (nodeHeight + 40.0f) + nodeHeight * 0.5f;

            // Draw line (simplified - just a colored rect for now)
            // In a real implementation, you'd draw a proper line
            renderer->DrawRect(Rect(prereqX - 1, prereqY, 2, skillY - prereqY), connectionColor, true);
        }
    }
}

void SkillTreeSystem::RenderSkillNodes(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 nodeStartY = panelY + 160.0f;
    f32 nodeStartX = panelX + 80.0f;
    f32 nodeWidth = 200.0f;
    f32 nodeHeight = 80.0f;

    // Draw connections first (behind nodes)
    RenderSkillConnections(renderer, nodeStartX, nodeStartY, nodeWidth, nodeHeight);

    // Get skills for selected branch
    auto skills = GetSkillsInBranch(m_SelectedBranch);

    for (Skill* skill : skills) {
        f32 nodeX = nodeStartX + skill->treeColumn * (nodeWidth + 30.0f);
        f32 nodeY = nodeStartY + skill->treeRow * (nodeHeight + 40.0f);

        Rect nodeRect(nodeX, nodeY, nodeWidth, nodeHeight);

        // Node background color
        Color bgColor;
        if (skill->IsMaxed()) {
            bgColor = Color(0.2f, 0.8f, 0.2f, 0.6f); // Green (maxed)
        } else if (skill->IsLearned()) {
            bgColor = Color(0.6f, 0.6f, 0.2f, 0.6f); // Yellow (learned)
        } else if (CanLearnSkill(skill->id, state)) {
            bgColor = Color(0.2f, 0.5f, 0.8f, 0.6f); // Blue (available)
        } else {
            bgColor = Color(0.3f, 0.3f, 0.3f, 0.6f); // Gray (locked)
        }

        renderer->DrawRect(nodeRect, bgColor, true);

        Color borderColor = GetBranchColor(skill->branch);
        renderer->DrawRect(nodeRect, borderColor, false);

        // Skill name
        renderer->DrawText(skill->name.c_str(), Vec2(nodeX + 5.0f, nodeY + 5.0f),
                         Color::White(), 13.0f);

        // Skill level
        char levelText[32];
        snprintf(levelText, sizeof(levelText), "Lvl %d/%d", skill->level, skill->maxLevel);
        renderer->DrawText(levelText, Vec2(nodeX + 5.0f, nodeY + 22.0f),
                         Color(0.8f, 0.8f, 0.8f, 1.0f), 11.0f);

        // Cost
        snprintf(levelText, sizeof(levelText), "Cost: %d SP", skill->cost);
        renderer->DrawText(levelText, Vec2(nodeX + 5.0f, nodeY + 38.0f),
                         Color(1.0f, 0.9f, 0.3f, 1.0f), 10.0f);

        // Requirements
        if (state->GetPlayerLevel() < skill->minPlayerLevel) {
            snprintf(levelText, sizeof(levelText), "Req: Lvl %d", skill->minPlayerLevel);
            renderer->DrawText(levelText, Vec2(nodeX + 5.0f, nodeY + 52.0f),
                             Color(1.0f, 0.5f, 0.5f, 1.0f), 10.0f);
        }

        // Effect
        char effectText[64];
        if (skill->IsLearned()) {
            snprintf(effectText, sizeof(effectText), "+%.0f%%", skill->GetCurrentEffect());
            renderer->DrawText(effectText, Vec2(nodeX + nodeWidth - 50.0f, nodeY + nodeHeight - 20.0f),
                             Color(0.3f, 1.0f, 0.3f, 1.0f), 12.0f);
        }
    }
}

void SkillTreeSystem::RenderSkillInfo(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 infoY = panelY + panelHeight - 120.0f;

    renderer->DrawText("SKILL POINTS:", Vec2(panelX + 20.0f, infoY), Color::NeonCyan(), 16.0f);

    char pointsText[64];
    snprintf(pointsText, sizeof(pointsText), "%d available | %d spent", m_SkillPoints, m_TotalSpent);
    renderer->DrawText(pointsText, Vec2(panelX + 20.0f, infoY + 22.0f),
                     Color(1.0f, 0.9f, 0.3f, 1.0f), 14.0f);

    // Reset button
    Rect resetBtn(panelX + panelWidth - 170.0f, infoY, 150.0f, 40.0f);
    Color resetColor = Color(0.8f, 0.3f, 0.3f, 0.8f);
    renderer->DrawRect(resetBtn, resetColor, true);
    renderer->DrawRect(resetBtn, Color(1.0f, 0.5f, 0.5f, 1.0f), false);
    renderer->DrawText("RESET SKILLS", Vec2(panelX + panelWidth - 155.0f, infoY + 12.0f),
                     Color::White(), 14.0f);
}

void SkillTreeSystem::RenderSkillTree(Renderer* renderer, GameState* state) {
    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.05f, 0.85f), true);

    // Main panel
    f32 panelWidth = 1000.0f;
    f32 panelHeight = 700.0f;
    f32 panelX = (static_cast<f32>(renderer->GetWidth()) - panelWidth) * 0.5f;
    f32 panelY = (static_cast<f32>(renderer->GetHeight()) - panelHeight) * 0.5f;

    Rect panelRect(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelRect, Color(0.05f, 0.05f, 0.1f, 0.95f), true);
    renderer->DrawRect(panelRect, Color::NeonCyan() * 0.7f, false);

    // Title
    renderer->DrawText("SKILL TREE", Vec2(panelX + panelWidth * 0.5f - 80.0f, panelY + 15.0f),
                     Color::NeonCyan(), 24.0f);

    // Render components
    RenderBranchTabs(renderer, panelX, panelY, panelWidth);
    RenderSkillNodes(renderer, state, panelX, panelY, panelWidth);
    RenderSkillInfo(renderer, panelX, panelY, panelWidth);

    // Close hint
    renderer->DrawText("Press ESC to close", Vec2(panelX + panelWidth - 150.0f, panelY + panelHeight - 30.0f),
                     Color(0.6f, 0.6f, 0.6f, 1.0f), 12.0f);
}

void SkillTreeSystem::HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state) {
    if (!mousePressed) return;

    f32 panelWidth = 1000.0f;
    f32 panelHeight = 700.0f;
    f32 panelX = (1920.0f - panelWidth) * 0.5f;  // TODO: Use actual screen width
    f32 panelY = (1080.0f - panelHeight) * 0.5f;

    // Branch tab clicks
    f32 tabY = panelY + 88.0f;
    f32 tabWidth = (panelWidth - 60.0f) / 3.0f;
    f32 tabHeight = 50.0f;
    f32 tabSpacing = 10.0f;

    for (i32 i = 0; i < 3; i++) {
        f32 tabX = panelX + 20.0f + i * (tabWidth + tabSpacing);
        Rect tabRect(tabX, tabY, tabWidth, tabHeight);

        if (tabRect.Contains(Vec2(mouseX, mouseY))) {
            m_SelectedBranch = static_cast<SkillBranch>(i);
            Log::Infof("Selected skill branch: ", i);
            return;
        }
    }

    // Skill node clicks
    f32 nodeStartY = panelY + 160.0f;
    f32 nodeStartX = panelX + 80.0f;
    f32 nodeWidth = 200.0f;
    f32 nodeHeight = 80.0f;

    auto skills = GetSkillsInBranch(m_SelectedBranch);
    for (Skill* skill : skills) {
        f32 nodeX = nodeStartX + skill->treeColumn * (nodeWidth + 30.0f);
        f32 nodeY = nodeStartY + skill->treeRow * (nodeHeight + 40.0f);

        Rect nodeRect(nodeX, nodeY, nodeWidth, nodeHeight);

        if (nodeRect.Contains(Vec2(mouseX, mouseY))) {
            if (LearnSkill(skill->id, state)) {
                Log::Infof("Learned skill: ", skill->name);
            } else {
                Log::Info("Cannot learn this skill (check requirements)");
            }
            return;
        }
    }

    // Reset button click
    Rect resetBtn(panelX + panelWidth - 170.0f, panelY + panelHeight - 120.0f, 150.0f, 40.0f);
    if (resetBtn.Contains(Vec2(mouseX, mouseY))) {
        ResetSkills();
    }
}

void SkillTreeSystem::SaveToJson(std::ofstream& file) const {
    file << "\"skillPoints\":" << m_SkillPoints << ",\n";
    file << "\"totalSpent\":" << m_TotalSpent << ",\n";
    file << "\"skills\":{\n";

    bool first = true;
    for (const auto& pair : m_Skills) {
        if (pair.second.level > 0) {
            if (!first) file << ",\n";
            file << "\"" << static_cast<i32>(pair.first) << "\":" << pair.second.level;
            first = false;
        }
    }

    file << "\n}\n";
}

void SkillTreeSystem::LoadFromJson(const std::string& line) {
    // TODO: Implement JSON parsing
    Log::Info("SkillTreeSystem::LoadFromJson called");
}
