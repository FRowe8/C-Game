#pragma once

#include "Types.h"
#include <string>
#include <fstream>
#include <cmath>


/**
 * SpecializedSkills - Activity-based skill progression system
 *
 * Three skill categories that level up through gameplay:
 * - Observation: Levels by observing stations, grants production bonuses
 * - Engineering: Levels by upgrading/building, grants efficiency bonuses
 * - Command: Levels by combat/management, grants combat and crew bonuses
 */

enum class SkillCategory {
    Observation,    // Production & Discovery
    Engineering,    // Efficiency & Building
    Command,        // Combat & Management
    COUNT
};

struct SpecializedSkill {
    SkillCategory category;
    i32 level;
    f64 experience;
    f64 experienceToNextLevel;

    SpecializedSkill()
        : category(SkillCategory::Observation)
        , level(1)
        , experience(0.0)
        , experienceToNextLevel(100.0)
    {}

    // Calculate XP required for next level (exponential scaling)
    f64 CalculateXPForLevel(i32 targetLevel) const {
        return 100.0 * std::pow(1.5, targetLevel - 1);
    }

    // Get bonus multiplier from this skill's level
    f64 GetBonusMultiplier() const {
        return 1.0 + (level - 1) * 0.05; // 5% per level
    }
};

class SpecializedSkillsSystem {
public:
    SpecializedSkillsSystem();
    ~SpecializedSkillsSystem() = default;

    // Initialization
    void Initialize();

    // Add experience to a specific skill
    void AddExperience(SkillCategory category, f64 amount);

    // Get skill info
    const SpecializedSkill& GetSkill(SkillCategory category) const;
    i32 GetSkillLevel(SkillCategory category) const;
    f64 GetSkillExperience(SkillCategory category) const;
    f64 GetSkillProgress(SkillCategory category) const; // 0.0 to 1.0

    // Skill bonuses
    f64 GetObservationBonus() const;    // Production multiplier
    f64 GetEngineeringBonus() const;    // Cost reduction & efficiency
    f64 GetCommandBonus() const;        // Combat & crew effectiveness

    // Combined skill level (for overall progression display)
    i32 GetTotalSkillLevel() const;
    i32 GetAverageSkillLevel() const;

    // Skill names
    const char* GetSkillName(SkillCategory category) const;
    const char* GetSkillDescription(SkillCategory category) const;

    // Save/Load
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    SpecializedSkill m_Skills[static_cast<i32>(SkillCategory::COUNT)];

    // Helper to level up a skill
    void LevelUpSkill(SkillCategory category);
};

// Activity-based XP rewards
namespace SkillXP {
    const f64 OBSERVE_STATION = 5.0;        // Per observation (Observation)
    const f64 UNLOCK_STATION = 25.0;        // Per unlock (Observation)
    const f64 UPGRADE_STATION = 10.0;       // Per upgrade (Engineering)
    const f64 PURCHASE_RESEARCH = 20.0;     // Per research (Engineering)
    const f64 BUY_UPGRADE = 15.0;           // Per buyable (Engineering)
    const f64 WIN_COMBAT = 30.0;            // Per victory (Command)
    const f64 INSTALL_PART = 15.0;          // Per installation (Command)
    const f64 RECRUIT_CREW = 20.0;          // Per crew member (Command)
    const f64 COMPLETE_CHALLENGE = 50.0;    // Per challenge (Command)
}
