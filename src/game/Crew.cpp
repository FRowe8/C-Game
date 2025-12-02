#include "Crew.h"
#include "logger.h"
#include <algorithm>
#include <fstream>
#include <cmath>

// === Crew Implementation ===

Crew::Crew()
    : m_ID(0), m_Name("Unknown"), m_Rarity(CrewRarity::Common), m_Role(CrewRole::Engineer),
      m_Level(1), m_Damage(10.0), m_Defense(5.0), m_MaxHP(100.0), m_CurrentHP(100.0),
      m_ResourceBonus(0.0), m_XP(0) {
}

Crew::Crew(i32 _id, const std::string& _name, CrewRarity _rarity, CrewRole _role)
    : m_ID(_id), m_Name(_name), m_Rarity(_rarity), m_Role(_role),
      m_Level(1), m_Damage(10.0), m_Defense(5.0), m_MaxHP(100.0), m_CurrentHP(100.0),
      m_ResourceBonus(0.0), m_XP(0) {
    RecalculateStats();
}

void Crew::LevelUp() {
    m_Level++;
    RecalculateStats();
    m_CurrentHP = m_MaxHP; // Heal on level up
    Log::Infof(m_Name, " leveled up to level ", m_Level, "!");
}

void Crew::AddTag(CrewTag tag) {
    if (!HasTag(tag)) {
        m_Tags.push_back(tag);
    }
}

void Crew::RemoveTag(CrewTag tag) {
    m_Tags.erase(
        std::remove(m_Tags.begin(), m_Tags.end(), tag),
        m_Tags.end()
    );
}

bool Crew::HasTag(CrewTag tag) const {
    return std::find(m_Tags.begin(), m_Tags.end(), tag) != m_Tags.end();
}

bool Crew::HasSynergyWith(const Crew* other) const {
    if (!other) return false;

    // Check if any tags match
    for (const auto& myTag : m_Tags) {
        for (const auto& theirTag : other->GetTags()) {
            if (myTag == theirTag) {
                return true;
            }
        }
    }

    return false;
}

f64 Crew::GetSynergyDamageBonus(const Crew* other) const {
    // This will be calculated by CrewManager using the synergy database
    // For now, simple tag matching gives 10% per matching tag
    if (!other) return 0.0;

    f64 bonus = 0.0;
    for (const auto& myTag : m_Tags) {
        for (const auto& theirTag : other->GetTags()) {
            if (myTag == theirTag) {
                bonus += 0.1; // 10% per shared tag
            }
        }
    }

    return bonus;
}

f64 Crew::GetSynergyDefenseBonus(const Crew* other) const {
    return GetSynergyDamageBonus(other); // Same for now
}

f64 Crew::GetSynergyResourceBonus(const Crew* other) const {
    return GetSynergyDamageBonus(other); // Same for now
}

void Crew::GainXP(i32 amount) {
    m_XP += amount;

    // Check for level up
    while (m_XP >= GetXPToNextLevel()) {
        m_XP -= GetXPToNextLevel();
        LevelUp();
    }
}

i32 Crew::GetXPToNextLevel() const {
    // XP required scales: 100, 200, 300, ...
    return m_Level * 100;
}

Color Crew::GetRarityColor() const {
    switch (m_Rarity) {
        case CrewRarity::Common:
            return Color(0.6f, 0.6f, 0.6f, 1.0f);      // Gray
        case CrewRarity::Uncommon:
            return Color(0.2f, 1.0f, 0.2f, 1.0f);      // Green
        case CrewRarity::Rare:
            return Color(0.2f, 0.6f, 1.0f, 1.0f);      // Blue
        case CrewRarity::Epic:
            return Color(0.8f, 0.3f, 1.0f, 1.0f);      // Purple
        case CrewRarity::Legendary:
            return Color(1.0f, 0.9f, 0.2f, 1.0f);      // Gold
        default:
            return Color::White();
    }
}

const char* Crew::GetRarityName() const {
    switch (m_Rarity) {
        case CrewRarity::Common: return "Common";
        case CrewRarity::Uncommon: return "Uncommon";
        case CrewRarity::Rare: return "Rare";
        case CrewRarity::Epic: return "Epic";
        case CrewRarity::Legendary: return "Legendary";
        default: return "Unknown";
    }
}

const char* Crew::GetRoleName() const {
    switch (m_Role) {
        case CrewRole::Engineer: return "Engineer";
        case CrewRole::Scientist: return "Scientist";
        case CrewRole::Soldier: return "Soldier";
        case CrewRole::Pilot: return "Pilot";
        case CrewRole::Medic: return "Medic";
        case CrewRole::Commander: return "Commander";
        default: return "Unknown";
    }
}

void Crew::SaveToJson(std::ofstream& file) const {
    file << "        {\"id\": " << m_ID
         << ", \"name\": \"" << m_Name << "\""
         << ", \"rarity\": " << static_cast<i32>(m_Rarity)
         << ", \"role\": " << static_cast<i32>(m_Role)
         << ", \"level\": " << m_Level
         << ", \"xp\": " << m_XP
         << ", \"currentHP\": " << m_CurrentHP
         << "}";
}

void Crew::LoadFromJson(const std::string& line) {
    // Simple manual JSON parsing (to be replaced with nlohmann/json later)
    (void)line;
}

void Crew::RecalculateStats() {
    // Base stats depend on rarity
    f64 rarityMultiplier = 1.0;
    switch (m_Rarity) {
        case CrewRarity::Common:    rarityMultiplier = 1.0; break;
        case CrewRarity::Uncommon:  rarityMultiplier = 1.3; break;
        case CrewRarity::Rare:      rarityMultiplier = 1.6; break;
        case CrewRarity::Epic:      rarityMultiplier = 2.0; break;
        case CrewRarity::Legendary: rarityMultiplier = 2.5; break;
    }

    // Level scaling
    f64 levelMultiplier = 1.0 + (m_Level - 1) * 0.15; // 15% per level

    // Base stats
    f64 baseDamage = 10.0;
    f64 baseDefense = 5.0;
    f64 baseHP = 100.0;
    f64 baseResourceBonus = 0.05; // 5% base

    // Role modifiers
    switch (m_Role) {
        case CrewRole::Engineer:
            baseResourceBonus = 0.15; // 15% resource bonus
            baseDamage *= 0.8;
            break;
        case CrewRole::Scientist:
            baseResourceBonus = 0.12;
            baseDamage *= 0.9;
            baseDefense *= 0.9;
            break;
        case CrewRole::Soldier:
            baseDamage *= 1.5;
            baseDefense *= 1.2;
            baseResourceBonus *= 0.5;
            break;
        case CrewRole::Pilot:
            baseDamage *= 1.1;
            baseDefense *= 1.1;
            baseHP *= 0.9;
            break;
        case CrewRole::Medic:
            baseHP *= 1.5;
            baseDefense *= 1.3;
            baseDamage *= 0.7;
            break;
        case CrewRole::Commander:
            baseDamage *= 1.2;
            baseDefense *= 1.2;
            baseResourceBonus *= 1.5;
            baseHP *= 1.1;
            break;
    }

    // Apply multipliers
    m_Damage = baseDamage * rarityMultiplier * levelMultiplier;
    m_Defense = baseDefense * rarityMultiplier * levelMultiplier;
    m_MaxHP = baseHP * rarityMultiplier * levelMultiplier;
    m_ResourceBonus = baseResourceBonus * rarityMultiplier;

    // Ensure current HP doesn't exceed max
    if (m_CurrentHP > m_MaxHP) {
        m_CurrentHP = m_MaxHP;
    }
}

// === CrewManager Implementation ===

CrewManager::CrewManager()
    : m_NextCrewID(1), m_GridSystem(nullptr) {
}

void CrewManager::Initialize() {
    // Clear existing roster
    for (Crew* crew : m_Roster) {
        delete crew;
    }
    m_Roster.clear();

    m_NextCrewID = 1;
    m_GridSystem = nullptr;

    // Initialize synergies
    InitializeSynergies();

    Log::Info("CrewManager initialized");
}

void CrewManager::Update(f64 deltaTime) {
    // Future: Handle time-based crew effects, morale, etc.
    (void)deltaTime;
}

// === Crew Roster ===

void CrewManager::RegisterCrew(Crew* crew) {
    if (!crew) return;

    m_Roster.push_back(crew);
    Log::Infof("Registered crew: ", crew->GetName(), " (ID: ", crew->GetID(), ")");
}

Crew* CrewManager::GetCrew(i32 crewID) {
    for (Crew* crew : m_Roster) {
        if (crew->GetID() == crewID) {
            return crew;
        }
    }
    return nullptr;
}

const Crew* CrewManager::GetCrew(i32 crewID) const {
    for (const Crew* crew : m_Roster) {
        if (crew->GetID() == crewID) {
            return crew;
        }
    }
    return nullptr;
}

std::vector<Crew*> CrewManager::GetAllCrew() {
    return m_Roster;
}

const std::vector<Crew*>& CrewManager::GetAllCrew() const {
    return m_Roster;
}

i32 CrewManager::GetCrewCount() const {
    return static_cast<i32>(m_Roster.size());
}

// === Crew Acquisition ===

Crew* CrewManager::RecruitRandomCrew(CrewRarity minRarity) {
    // Random rarity roll
    i32 rarityRoll = rand() % 100;
    CrewRarity rarity = CrewRarity::Common;

    if (rarityRoll >= 95) {
        rarity = CrewRarity::Legendary; // 5%
    } else if (rarityRoll >= 85) {
        rarity = CrewRarity::Epic; // 10%
    } else if (rarityRoll >= 65) {
        rarity = CrewRarity::Rare; // 20%
    } else if (rarityRoll >= 35) {
        rarity = CrewRarity::Uncommon; // 30%
    } else {
        rarity = CrewRarity::Common; // 35%
    }

    // Ensure meets minimum rarity
    if (rarity < minRarity) {
        rarity = minRarity;
    }

    // Random role
    CrewRole role = static_cast<CrewRole>(rand() % 6);

    // Generate name (placeholder)
    char nameBuf[32];
    snprintf(nameBuf, sizeof(nameBuf), "Crew_%d", m_NextCrewID);
    std::string name = nameBuf;

    // Create crew
    Crew* crew = new Crew(m_NextCrewID++, name, rarity, role);

    // Add random tags based on rarity
    i32 tagCount = static_cast<i32>(rarity) + 1; // Common=1, Legendary=5
    for (i32 i = 0; i < tagCount && i < 3; i++) {
        CrewTag tag = static_cast<CrewTag>(rand() % static_cast<i32>(CrewTag::COUNT));
        crew->AddTag(tag);
    }

    RegisterCrew(crew);

    Log::Infof("Recruited new crew: ", crew->GetName(), " - ", crew->GetRarityName(), " ", crew->GetRoleName());
    return crew;
}

Crew* CrewManager::RecruitSpecificCrew(i32 crewID) {
    // This would load from data/crew.json
    // For now, just stub it out
    (void)crewID;
    Log::Warning("RecruitSpecificCrew not yet implemented (needs crew.json)");
    return nullptr;
}

void CrewManager::AssignStarterCrew() {
    // Give player 3 starting crew members
    for (i32 i = 0; i < 3; i++) {
        RecruitRandomCrew(CrewRarity::Common);
    }
    Log::Info("Assigned starter crew");
}

// === Grid Integration ===

f64 CrewManager::CalculateTotalDamageBonus() const {
    if (!m_GridSystem) return 0.0;

    f64 totalBonus = 0.0;

    // Check all filled grid slots
    for (i32 i = 0; i < 9; i++) {
        GridPos pos = GridPos::FromIndex(i);
        i32 crewID = m_GridSystem->GetCrewAt(pos);

        if (crewID == -1) continue; // Empty slot

        const Crew* crew = GetCrew(crewID);
        if (!crew) continue;

        // Add crew's base damage
        totalBonus += crew->GetDamage();

        // Check for synergies with adjacent crew
        std::vector<i32> adjacentCrewIDs = m_GridSystem->GetAdjacentCrew(pos, true);
        for (i32 adjCrewID : adjacentCrewIDs) {
            const Crew* adjCrew = GetCrew(adjCrewID);
            if (adjCrew) {
                totalBonus += crew->GetSynergyDamageBonus(adjCrew);
            }
        }

        // Apply position bonus from grid
        totalBonus *= m_GridSystem->GetSlotBonus(pos);
    }

    return totalBonus;
}

f64 CrewManager::CalculateTotalDefenseBonus() const {
    if (!m_GridSystem) return 0.0;

    f64 totalBonus = 0.0;

    for (i32 i = 0; i < 9; i++) {
        GridPos pos = GridPos::FromIndex(i);
        i32 crewID = m_GridSystem->GetCrewAt(pos);

        if (crewID == -1) continue;

        const Crew* crew = GetCrew(crewID);
        if (!crew) continue;

        totalBonus += crew->GetDefense();

        // Synergies
        std::vector<i32> adjacentCrewIDs = m_GridSystem->GetAdjacentCrew(pos, true);
        for (i32 adjCrewID : adjacentCrewIDs) {
            const Crew* adjCrew = GetCrew(adjCrewID);
            if (adjCrew) {
                totalBonus += crew->GetSynergyDefenseBonus(adjCrew);
            }
        }

        totalBonus *= m_GridSystem->GetSlotBonus(pos);
    }

    return totalBonus;
}

f64 CrewManager::CalculateTotalResourceBonus() const {
    if (!m_GridSystem) return 0.0;

    f64 totalBonus = 0.0;

    for (i32 i = 0; i < 9; i++) {
        GridPos pos = GridPos::FromIndex(i);
        i32 crewID = m_GridSystem->GetCrewAt(pos);

        if (crewID == -1) continue;

        const Crew* crew = GetCrew(crewID);
        if (!crew) continue;

        totalBonus += crew->GetResourceBonus();

        // Synergies
        std::vector<i32> adjacentCrewIDs = m_GridSystem->GetAdjacentCrew(pos, true);
        for (i32 adjCrewID : adjacentCrewIDs) {
            const Crew* adjCrew = GetCrew(adjCrewID);
            if (adjCrew) {
                totalBonus += crew->GetSynergyResourceBonus(adjCrew);
            }
        }
    }

    return totalBonus;
}

// === Synergy Database ===

void CrewManager::RegisterSynergy(const SynergyBonus& synergy) {
    m_SynergyDatabase.push_back(synergy);
}

SynergyBonus CrewManager::GetSynergyBonus(CrewTag tag1, CrewTag tag2) const {
    for (const auto& synergy : m_SynergyDatabase) {
        if ((synergy.tag1 == tag1 && synergy.tag2 == tag2) ||
            (synergy.tag1 == tag2 && synergy.tag2 == tag1)) {
            return synergy;
        }
    }
    return SynergyBonus(); // No synergy
}

std::vector<SynergyBonus> CrewManager::GetActiveSynergies() const {
    std::vector<SynergyBonus> activeSynergies;

    if (!m_GridSystem) return activeSynergies;

    // Check all pairs of adjacent crew
    for (i32 i = 0; i < 9; i++) {
        GridPos pos = GridPos::FromIndex(i);
        i32 crewID = m_GridSystem->GetCrewAt(pos);

        if (crewID == -1) continue;

        const Crew* crew = GetCrew(crewID);
        if (!crew) continue;

        std::vector<i32> adjacentCrewIDs = m_GridSystem->GetAdjacentCrew(pos, true);
        for (i32 adjCrewID : adjacentCrewIDs) {
            const Crew* adjCrew = GetCrew(adjCrewID);
            if (!adjCrew) continue;

            // Check all tag combinations
            for (const auto& tag1 : crew->GetTags()) {
                for (const auto& tag2 : adjCrew->GetTags()) {
                    SynergyBonus bonus = GetSynergyBonus(tag1, tag2);
                    if (bonus.damageBonus > 0.0 || bonus.defenseBonus > 0.0 || bonus.resourceBonus > 0.0) {
                        activeSynergies.push_back(bonus);
                    }
                }
            }
        }
    }

    return activeSynergies;
}

// === Statistics ===

i32 CrewManager::GetCrewByRarity(CrewRarity rarity) const {
    i32 count = 0;
    for (const Crew* crew : m_Roster) {
        if (crew->GetRarity() == rarity) {
            count++;
        }
    }
    return count;
}

i32 CrewManager::GetCrewByRole(CrewRole role) const {
    i32 count = 0;
    for (const Crew* crew : m_Roster) {
        if (crew->GetRole() == role) {
            count++;
        }
    }
    return count;
}

f64 CrewManager::GetAverageLevel() const {
    if (m_Roster.empty()) return 0.0;

    f64 totalLevel = 0.0;
    for (const Crew* crew : m_Roster) {
        totalLevel += crew->GetLevel();
    }
    return totalLevel / m_Roster.size();
}

// === Save/Load ===

void CrewManager::SaveToJson(std::ofstream& file) const {
    file << "    \"crewManager\": {\n";
    file << "        \"nextCrewID\": " << m_NextCrewID << ",\n";
    file << "        \"roster\": [\n";

    for (size_t i = 0; i < m_Roster.size(); i++) {
        file << "        ";
        m_Roster[i]->SaveToJson(file);
        if (i < m_Roster.size() - 1) file << ",";
        file << "\n";
    }

    file << "        ]\n";
    file << "    }";
}

void CrewManager::LoadFromJson(const std::string& line) {
    // Simple manual JSON parsing (to be replaced with nlohmann/json later)
    (void)line;
}

void CrewManager::InitializeSynergies() {
    m_SynergyDatabase.clear();

    // Define synergies between crew tags
    RegisterSynergy(SynergyBonus(
        CrewTag::Leader, CrewTag::Team_Player,
        0.2, 0.1, 0.0,
        "Leader + Team Player: +20% damage, +10% defense"
    ));

    RegisterSynergy(SynergyBonus(
        CrewTag::Veteran, CrewTag::Rookie,
        0.15, 0.15, 0.0,
        "Veteran + Rookie: +15% damage and defense (mentorship)"
    ));

    RegisterSynergy(SynergyBonus(
        CrewTag::Elite, CrewTag::Elite,
        0.25, 0.0, 0.0,
        "Elite + Elite: +25% damage (synchronized)"
    ));

    RegisterSynergy(SynergyBonus(
        CrewTag::Human, CrewTag::Synthetic,
        0.0, 0.0, 0.15,
        "Human + Synthetic: +15% resources (efficiency)"
    ));

    RegisterSynergy(SynergyBonus(
        CrewTag::Quantum, CrewTag::Experimental,
        0.0, 0.0, 0.25,
        "Quantum + Experimental: +25% resources (breakthrough)"
    ));

    Log::Infof("Initialized ", static_cast<i32>(m_SynergyDatabase.size()), " crew synergies");
}
