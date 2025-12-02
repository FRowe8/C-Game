#include "SkillTree.h"
#include "ImGuiUtils.h"
#include "Renderer.h"
#include "Logger.h"
#include "GameState.h"
#include "imgui.h"
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

bool SkillTreeSystem::CanResetSkills(GameState* state) const {
    // Check if player has any skills to reset
    if (m_TotalSpent == 0) {
        return false;
    }

    // Check if player can afford reset cost (10 Photons base + 5 per skill point spent)
    i32 resetCost = 10 + (m_TotalSpent * 5);
    return state->GetPhotons() >= resetCost;
}

bool SkillTreeSystem::ResetSkills(GameState* state) {
    // Check if any skills are learned
    if (m_TotalSpent == 0) {
        Log::Warn("No skills to reset!");
        return false;
    }

    // Calculate reset cost (10 Photons base + 5 per skill point spent)
    i32 resetCost = 10 + (m_TotalSpent * 5);

    // Check if player can afford
    if (state->GetPhotons() < resetCost) {
        Log::Warnf("Not enough Photons! Need ", resetCost, " Photons to reset skills.");
        return false;
    }

    // Spend Photons
    state->AddPhotons(-resetCost);

    // Refund all spent points
    m_SkillPoints += m_TotalSpent;
    m_TotalSpent = 0;

    // Reset all skill levels
    for (auto& pair : m_Skills) {
        pair.second.level = 0;
    }

    Log::Infof("All skills reset! Cost: ", resetCost, " Photons. ", m_SkillPoints, " skill points refunded.");
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
    (void)renderer; (void)panelX; (void)panelY; (void)panelWidth;

    ImGui::Text("SELECT BRANCH:");
    ImGui::SameLine();

    const char* branchNames[] = {"COMBAT", "ENGINEERING", "EXPLORATION"};
    SkillBranch branches[] = {SkillBranch::Combat, SkillBranch::Engineering, SkillBranch::Exploration};

    for (i32 i = 0; i < 3; i++) {
        if (i > 0) ImGui::SameLine();

        Color branchColor = GetBranchColor(branches[i]);
        ImVec4 imColor(branchColor.r, branchColor.g, branchColor.b, 1.0f);
        ImVec4 imColorBg(
        imColor.x * 0.3f, // R * 0.3f
        imColor.y * 0.3f, // G * 0.3f
        imColor.z * 0.3f, // B * 0.3f
        imColor.w         // Keep the Alpha (A) component as is, or scale it too if needed
        );

        // Apply styles
        ImGui::PushStyleColor(ImGuiCol_FrameBg, m_SelectedBranch == branches[i] ? imColorBg : ImVec4(0.2f, 0.2f, 0.2f, 0.6f));
        // 1. Define the scalar value
        float brightnessFactor = 1.5f;

        // 2. Create the new ImVec4 by scaling the components of imColorBg
        ImVec4 imColorBgHovered(
            imColorBg.x * brightnessFactor,
            imColorBg.y * brightnessFactor,
            imColorBg.z * brightnessFactor,
            imColorBg.w // Keep alpha as is
        );

        // 3. Use the new scaled color
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, imColorBgHovered);
        ImGui::PushStyleColor(ImGuiCol_Text, imColor);

        if (ImGui::RadioButton(branchNames[i], m_SelectedBranch == branches[i])) {
            m_SelectedBranch = branches[i];
            Log::Infof("Selected skill branch: ", i);
        }

        ImGui::PopStyleColor(3);
    }
}

// Note: RenderSkillConnections is integrated into this function.

void SkillTreeSystem::RenderSkillNodes(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth) {
    (void)renderer; (void)panelX; (void)panelY; (void)panelWidth;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // Get the screen position of the current cursor (top-left of the scrollable child window)
    ImVec2 contentStartPos = ImGui::GetCursorScreenPos();

    f32 nodeWidth = 200.0f;
    f32 nodeHeight = 80.0f;
    f32 columnSpacing = 30.0f;
    f32 rowSpacing = 40.0f;

    auto skills = GetSkillsInBranch(m_SelectedBranch);
    Color branchColor = GetBranchColor(m_SelectedBranch);
    ImU32 connectionColor = ToImU32(branchColor * 0.5f);

    // --- 1. Draw Connections (Behind Nodes) ---
    for (const Skill* skill : skills) {
        if (skill->prerequisites.empty()) continue;

        ImVec2 skillCenter(
            contentStartPos.x + skill->treeColumn * (nodeWidth + columnSpacing) + nodeWidth * 0.5f,
            contentStartPos.y + skill->treeRow * (nodeHeight + rowSpacing) + nodeHeight * 0.5f
        );

        for (SkillID prereqID : skill->prerequisites) {
            const Skill* prereq = GetSkill(prereqID);
            if (!prereq) continue;

            ImVec2 prereqCenter(
                contentStartPos.x + prereq->treeColumn * (nodeWidth + columnSpacing) + nodeWidth * 0.5f,
                contentStartPos.y + prereq->treeRow * (nodeHeight + rowSpacing) + nodeHeight * 0.5f
            );

            // Draw line from prerequisite center to skill center
            draw_list->AddLine(prereqCenter, skillCenter, connectionColor, 2.0f);
        }
    }

    // --- 2. Draw Nodes and Interaction ---
    f32 maxNodeY = 0.0f; // Track max height needed for scroll area calculation

    for (Skill* skill : skills) {
        f32 nodeX = skill->treeColumn * (nodeWidth + columnSpacing);
        f32 nodeY = skill->treeRow * (nodeHeight + rowSpacing);

        ImVec2 nodeStart(contentStartPos.x + nodeX, contentStartPos.y + nodeY);
        ImVec2 nodeEnd(nodeStart.x + nodeWidth, nodeStart.y + nodeHeight);

        maxNodeY = std::max(maxNodeY, nodeY + nodeHeight);

        // --- Node State & Colors ---
        Color bgColor;
        bool canLearn = CanLearnSkill(skill->id, state);

        if (skill->IsMaxed()) {
            bgColor = Color(0.2f, 0.8f, 0.2f, 0.6f); // Green (maxed)
        } else if (skill->IsLearned()) {
            bgColor = Color(0.6f, 0.6f, 0.2f, 0.6f); // Yellow (learned)
        } else if (canLearn) {
            bgColor = Color(0.2f, 0.5f, 0.8f, 0.6f); // Blue (available)
        } else {
            bgColor = Color(0.3f, 0.3f, 0.3f, 0.6f); // Gray (locked)
        }

        ImU32 imBgColor = ToImU32(bgColor);
        ImU32 imBorderColor = ToImU32(branchColor);

        // --- Draw Node Background and Border ---
        draw_list->AddRectFilled(nodeStart, nodeEnd, imBgColor, 5.0f);
        draw_list->AddRect(nodeStart, nodeEnd, imBorderColor, 5.0f);

        // --- Draw Text Content ---
        ImVec2 textStart(nodeStart.x + 5.0f, nodeStart.y + 5.0f);

        draw_list->AddText(textStart, ToImU32(Color::White()), skill->name.c_str());

        char levelText[32];
        snprintf(levelText, sizeof(levelText), "Lvl %d/%d", skill->level, skill->maxLevel);
        draw_list->AddText(ImVec2(textStart.x, textStart.y + 18.0f), ToImU32(Color(0.8f, 0.8f, 0.8f, 1.0f)), levelText);

        char costText[32];
        snprintf(costText, sizeof(costText), "Cost: %d SP", skill->cost);
        draw_list->AddText(ImVec2(textStart.x, textStart.y + 34.0f), ToImU32(Color(1.0f, 0.9f, 0.3f, 1.0f)), costText);

        if (state->GetPlayerLevel() < skill->minPlayerLevel) {
            char reqText[32];
            snprintf(reqText, sizeof(reqText), "Req: Lvl %d", skill->minPlayerLevel);
            draw_list->AddText(ImVec2(textStart.x, textStart.y + 50.0f), ToImU32(Color(1.0f, 0.5f, 0.5f, 1.0f)), reqText);
        }

        if (skill->IsLearned()) {
            char effectText[64];
            snprintf(effectText, sizeof(effectText), "+%.0f%%", skill->GetCurrentEffect());
            draw_list->AddText(ImVec2(nodeEnd.x - 70.0f, nodeEnd.y - 25.0f), ToImU32(Color(0.3f, 1.0f, 0.3f, 1.0f)), effectText);
        }

        // --- Interaction/Button Logic ---
        // Use an invisible button overlay to capture clicks
        ImGui::SetCursorScreenPos(nodeStart);
        ImGui::InvisibleButton((skill->name + "##SkillNode").c_str(), ImVec2(nodeWidth, nodeHeight));

        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s\n\n%s\n\n- Current Bonus: +%.1f%%\n- Next Level Cost: %d SP",
                skill->name.c_str(),
                skill->description.c_str(),
                skill->GetCurrentEffect(),
                skill->cost
            );
            draw_list->AddRect(nodeStart, nodeEnd, ToImU32(Color::White()), 5.0f, 0, 3.0f);
        }

        if (ImGui::IsItemClicked()) {
            if (canLearn && !skill->IsMaxed()) {
                // LearnSkill handles deduction and level increase
                if (LearnSkill(skill->id, state)) {
                    // Success, Log is inside LearnSkill
                } else {
                    Log::Info("Cannot learn this skill (check requirements)");
                }
            }
        }
    }

    // Set the child window height to enable proper scrolling to the bottom-most node
    ImGui::SetCursorScreenPos(ImVec2(contentStartPos.x, contentStartPos.y + maxNodeY + rowSpacing));

    // FIX: Add this line to "stamp" the layout at the new cursor position
    ImGui::Dummy(ImVec2(0.0f, 0.0f));
}


void SkillTreeSystem::RenderSkillInfo(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth, f32 panelHeight) {
    (void)renderer; (void)panelX; (void)panelY; (void)panelWidth; (void)panelHeight;

    // Skill Points Info (Left side)
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "SKILL POINTS:");

    // Align the value to the middle
    ImGui::SameLine(ImGui::GetWindowWidth() * 0.4f);

    // 1. Create your desired color using your existing Color class
    Color skillColor(1.0f, 0.9f, 0.3f, 1.0f);

    // 2. Convert it to ImVec4
    ImVec4 imSkillColor(skillColor.r, skillColor.g, skillColor.b, skillColor.a);

    // 3. Pass the ImVec4 to ImGui::TextColored
    ImGui::TextColored(imSkillColor,
        "%d available | %d spent", m_SkillPoints, m_TotalSpent);

    // Reset button (Right side)
    ImGui::SameLine(ImGui::GetWindowWidth() - 160.0f);

    // Calculate reset cost
    i32 resetCost = 10 + (m_TotalSpent * 5);
    bool canReset = CanResetSkills(state);

    // Button styling
    if (!canReset) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.3f, 0.3f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.2f, 0.2f, 1.0f));
    }

    if (ImGui::Button("RESET SKILLS", ImVec2(150.0f, 40.0f)) && canReset) {
        // Open confirmation popup
        ImGui::OpenPopup("Confirm Reset");
    }

    ImGui::PopStyleColor(3);

    // Tooltip
    if (ImGui::IsItemHovered()) {
        if (m_TotalSpent == 0) {
            ImGui::SetTooltip("No skills to reset");
        } else if (!canReset) {
            ImGui::SetTooltip("Need %d Photons to reset (have %d)", resetCost, static_cast<i32>(state->GetPhotons()));
        } else {
            ImGui::SetTooltip("Reset all skills for %d Photons\nRefunds %d skill points", resetCost, m_TotalSpent);
        }
    }

    // Confirmation modal
    if (ImGui::BeginPopupModal("Confirm Reset", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to reset all skills?");
        ImGui::Spacing();
        ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "Cost: %d Photons", resetCost);
        ImGui::TextColored(ImVec4(0.3f, 1.0f, 0.3f, 1.0f), "Refund: %d skill points", m_TotalSpent);
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Confirm button
        if (ImGui::Button("YES, RESET", ImVec2(120, 0))) {
            ResetSkills(state);
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        // Cancel button
        if (ImGui::Button("CANCEL", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    // Hint text
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Press ESC to close");
}


void SkillTreeSystem::RenderSkillTree(Renderer* renderer, GameState* state) {
    // Renderer pointer is no longer used for drawing UI components
    (void)renderer;

    // Set consistent window size and position (e.g., center of the screen)
    ImGui::SetNextWindowSize(ImVec2(1000, 700), ImGuiCond_Once);
    // Calculate the center point correctly first
    ImVec2 centerPos = ImVec2(
        ImGui::GetIO().DisplaySize.x * 0.5f,
        ImGui::GetIO().DisplaySize.y * 0.5f
    );

    // Use the calculated center position
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Once, ImVec2(0.5f, 0.5f));

    // The main Skill Tree window
    if (ImGui::Begin("Skill Tree", NULL, ImGuiWindowFlags_NoCollapse)) {

        // --- TITLE ---
        ImGui::SetCursorPosX(ImGui::GetWindowWidth() * 0.5f - ImGui::CalcTextSize("SKILL TREE").x * 0.5f);
        ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "SKILL TREE");
        ImGui::Separator();

        // --- BRANCH TABS ---
        RenderBranchTabs(renderer, 0.0f, 0.0f, 0.0f); // Parameters now ignored
        ImGui::Separator();

        // --- SKILL NODE AREA (Scrollable) ---
        // Use ImGui::BeginChild to create the scrollable view for the skill map.
        // The height is set to fill the available space minus the info panel height (approx 120px)
        ImGui::BeginChild("##SkillNodeArea", ImVec2(0, ImGui::GetContentRegionAvail().y - 120.0f), true, ImGuiWindowFlags_HorizontalScrollbar);

        RenderSkillNodes(renderer, state, 0.0f, 0.0f, 0.0f); // Parameters now ignored

        ImGui::EndChild();
        ImGui::Separator();

        // --- INFO AND RESET PANEL ---
        RenderSkillInfo(renderer, state, 0.0f, 0.0f, 0.0f, 0.0f); // Parameters now ignored

        ImGui::End();
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
