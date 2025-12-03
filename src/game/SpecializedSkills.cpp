#include "SpecializedSkills.h"
#include "Logger.h"
#include <cmath>

SpecializedSkillsSystem::SpecializedSkillsSystem() {
    for (i32 i = 0; i < static_cast<i32>(SkillCategory::COUNT); i++) {
        m_Skills[i].category = static_cast<SkillCategory>(i);
        m_Skills[i].level = 1;
        m_Skills[i].experience = 0.0;
        m_Skills[i].experienceToNextLevel = 100.0;
    }
}

void SpecializedSkillsSystem::Initialize() {
    for (i32 i = 0; i < static_cast<i32>(SkillCategory::COUNT); i++) {
        m_Skills[i].level = 1;
        m_Skills[i].experience = 0.0;
        m_Skills[i].experienceToNextLevel = m_Skills[i].CalculateXPForLevel(2);
    }
}

void SpecializedSkillsSystem::AddExperience(SkillCategory category, f64 amount) {
    i32 index = static_cast<i32>(category);
    SpecializedSkill& skill = m_Skills[index];

    skill.experience += amount;

    // Check for level up
    while (skill.experience >= skill.experienceToNextLevel) {
        skill.experience -= skill.experienceToNextLevel;
        LevelUpSkill(category);
    }
}

const SpecializedSkill& SpecializedSkillsSystem::GetSkill(SkillCategory category) const {
    return m_Skills[static_cast<i32>(category)];
}

i32 SpecializedSkillsSystem::GetSkillLevel(SkillCategory category) const {
    return m_Skills[static_cast<i32>(category)].level;
}

f64 SpecializedSkillsSystem::GetSkillExperience(SkillCategory category) const {
    return m_Skills[static_cast<i32>(category)].experience;
}

f64 SpecializedSkillsSystem::GetSkillProgress(SkillCategory category) const {
    const SpecializedSkill& skill = m_Skills[static_cast<i32>(category)];
    return skill.experience / skill.experienceToNextLevel;
}

f64 SpecializedSkillsSystem::GetObservationBonus() const {
    // Production multiplier from Observation skill
    return GetSkill(SkillCategory::Observation).GetBonusMultiplier();
}

f64 SpecializedSkillsSystem::GetEngineeringBonus() const {
    // Cost reduction & efficiency from Engineering skill
    return GetSkill(SkillCategory::Engineering).GetBonusMultiplier();
}

f64 SpecializedSkillsSystem::GetCommandBonus() const {
    // Combat & crew effectiveness from Command skill
    return GetSkill(SkillCategory::Command).GetBonusMultiplier();
}

i32 SpecializedSkillsSystem::GetTotalSkillLevel() const {
    i32 total = 0;
    for (i32 i = 0; i < static_cast<i32>(SkillCategory::COUNT); i++) {
        total += m_Skills[i].level;
    }
    return total;
}

i32 SpecializedSkillsSystem::GetAverageSkillLevel() const {
    return GetTotalSkillLevel() / static_cast<i32>(SkillCategory::COUNT);
}

const char* SpecializedSkillsSystem::GetSkillName(SkillCategory category) const {
    switch (category) {
        case SkillCategory::Observation:
            return "Observation";
        case SkillCategory::Engineering:
            return "Engineering";
        case SkillCategory::Command:
            return "Command";
        default:
            return "Unknown";
    }
}

const char* SpecializedSkillsSystem::GetSkillDescription(SkillCategory category) const {
    switch (category) {
        case SkillCategory::Observation:
            return "Increases production from research stations\n"
                   "Gained by: Observing, Unlocking, Discovering";
        case SkillCategory::Engineering:
            return "Improves efficiency and reduces costs\n"
                   "Gained by: Upgrading, Researching, Building";
        case SkillCategory::Command:
            return "Enhances combat and crew management\n"
                   "Gained by: Combat, Crew, Leadership";
        default:
            return "";
    }
}

void SpecializedSkillsSystem::LevelUpSkill(SkillCategory category) {
    i32 index = static_cast<i32>(category);
    SpecializedSkill& skill = m_Skills[index];

    skill.level++;
    skill.experienceToNextLevel = skill.CalculateXPForLevel(skill.level + 1);

    Log::Infof(GetSkillName(category), " skill level up! Now level ", skill.level);
}

void SpecializedSkillsSystem::SaveToJson(std::ostream& file) const {
    file << "  \"specializedSkills\":{\n";
    file << "    \"observation\":{\"level\":" << m_Skills[0].level << ",\"xp\":" << m_Skills[0].experience << "},\n";
    file << "    \"engineering\":{\"level\":" << m_Skills[1].level << ",\"xp\":" << m_Skills[1].experience << "},\n";
    file << "    \"command\":{\"level\":" << m_Skills[2].level << ",\"xp\":" << m_Skills[2].experience << "}\n";
    file << "  }\n";
}

void SpecializedSkillsSystem::LoadFromJson(const std::string& line) {
    // Simple JSON parsing (to be replaced with nlohmann/json)

    // Parse Observation
    size_t obsPos = line.find("\"observation\"");
    if (obsPos != std::string::npos) {
        size_t levelPos = line.find("\"level\":", obsPos);
        if (levelPos != std::string::npos) {
            i32 level = std::atoi(line.substr(levelPos + 8).c_str());
            m_Skills[0].level = level;
        }
        size_t xpPos = line.find("\"xp\":", obsPos);
        if (xpPos != std::string::npos) {
            f64 xp = std::atof(line.substr(xpPos + 5).c_str());
            m_Skills[0].experience = xp;
        }
    }

    // Parse Engineering
    size_t engPos = line.find("\"engineering\"");
    if (engPos != std::string::npos) {
        size_t levelPos = line.find("\"level\":", engPos);
        if (levelPos != std::string::npos) {
            i32 level = std::atoi(line.substr(levelPos + 8).c_str());
            m_Skills[1].level = level;
        }
        size_t xpPos = line.find("\"xp\":", engPos);
        if (xpPos != std::string::npos) {
            f64 xp = std::atof(line.substr(xpPos + 5).c_str());
            m_Skills[1].experience = xp;
        }
    }

    // Parse Command
    size_t cmdPos = line.find("\"command\"");
    if (cmdPos != std::string::npos) {
        size_t levelPos = line.find("\"level\":", cmdPos);
        if (levelPos != std::string::npos) {
            i32 level = std::atoi(line.substr(levelPos + 8).c_str());
            m_Skills[2].level = level;
        }
        size_t xpPos = line.find("\"xp\":", cmdPos);
        if (xpPos != std::string::npos) {
            f64 xp = std::atof(line.substr(xpPos + 5).c_str());
            m_Skills[2].experience = xp;
        }
    }

    // Recalculate XP requirements
    for (i32 i = 0; i < static_cast<i32>(SkillCategory::COUNT); i++) {
        m_Skills[i].experienceToNextLevel = m_Skills[i].CalculateXPForLevel(m_Skills[i].level + 1);
    }
}
