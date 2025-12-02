#pragma once

#include "Types.h"
#include "GridSystem.h"
#include <string>
#include <vector>

// Crew member rarity
enum class CrewRarity {
    Common,      // Gray - Basic crew
    Uncommon,    // Green - Improved stats
    Rare,        // Blue - Strong stats
    Epic,        // Purple - Very strong
    Legendary    // Gold - Exceptional
};

// Crew roles/classes
enum class CrewRole {
    Engineer,    // Boosts resource generation, ship systems
    Scientist,   // Boosts research, quantum mechanics
    Soldier,     // Boosts combat damage, defense
    Pilot,       // Boosts engines, speed, dodge
    Medic,       // Boosts healing, HP
    Commander    // Boosts all stats, leadership synergies
};

// Crew tags for synergies
enum class CrewTag {
    // Species/Origin
    Human,
    Alien,
    Synthetic,
    Quantum,

    // Specialization
    Veteran,
    Rookie,
    Elite,
    Experimental,

    // Personality
    Leader,
    Lone_Wolf,
    Team_Player,
    Unstable,

    COUNT
};

// Synergy bonus between two crew members
struct SynergyBonus {
    CrewTag tag1;
    CrewTag tag2;
    f64 damageBonus;
    f64 defenseBonus;
    f64 resourceBonus;
    std::string description;

    SynergyBonus()
        : tag1(CrewTag::Human), tag2(CrewTag::Human),
          damageBonus(0.0), defenseBonus(0.0), resourceBonus(0.0),
          description("") {}

    SynergyBonus(CrewTag _tag1, CrewTag _tag2,
                 f64 _damage, f64 _defense, f64 _resource,
                 const std::string& _desc)
        : tag1(_tag1), tag2(_tag2),
          damageBonus(_damage), defenseBonus(_defense), resourceBonus(_resource),
          description(_desc) {}
};

// Crew member definition
class Crew {
public:
    Crew();
    Crew(i32 _id, const std::string& _name, CrewRarity _rarity, CrewRole _role);

    // === Identity ===
    i32 GetID() const { return m_ID; }
    const std::string& GetName() const { return m_Name; }
    CrewRarity GetRarity() const { return m_Rarity; }
    CrewRole GetRole() const { return m_Role; }

    // === Stats ===
    i32 GetLevel() const { return m_Level; }
    void SetLevel(i32 level) { m_Level = level; }
    void LevelUp();

    f64 GetDamage() const { return m_Damage; }
    f64 GetDefense() const { return m_Defense; }
    f64 GetMaxHP() const { return m_MaxHP; }
    f64 GetCurrentHP() const { return m_CurrentHP; }
    f64 GetResourceBonus() const { return m_ResourceBonus; }

    void SetDamage(f64 damage) { m_Damage = damage; }
    void SetDefense(f64 defense) { m_Defense = defense; }
    void SetMaxHP(f64 hp) { m_MaxHP = hp; }
    void SetCurrentHP(f64 hp) { m_CurrentHP = hp; }
    void SetResourceBonus(f64 bonus) { m_ResourceBonus = bonus; }

    // === Tags ===
    void AddTag(CrewTag tag);
    void RemoveTag(CrewTag tag);
    bool HasTag(CrewTag tag) const;
    const std::vector<CrewTag>& GetTags() const { return m_Tags; }

    // === Synergies ===
    bool HasSynergyWith(const Crew* other) const;
    f64 GetSynergyDamageBonus(const Crew* other) const;
    f64 GetSynergyDefenseBonus(const Crew* other) const;
    f64 GetSynergyResourceBonus(const Crew* other) const;

    // === Experience ===
    void GainXP(i32 amount);
    i32 GetXP() const { return m_XP; }
    i32 GetXPToNextLevel() const;

    // === Display ===
    Color GetRarityColor() const;
    const char* GetRarityName() const;
    const char* GetRoleName() const;

    // === Save/Load ===
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    // Identity
    i32 m_ID;
    std::string m_Name;
    CrewRarity m_Rarity;
    CrewRole m_Role;

    // Stats
    i32 m_Level;
    f64 m_Damage;
    f64 m_Defense;
    f64 m_MaxHP;
    f64 m_CurrentHP;
    f64 m_ResourceBonus; // % bonus to resource generation

    // Progression
    i32 m_XP;

    // Tags for synergies
    std::vector<CrewTag> m_Tags;

    // Calculate base stats based on rarity and level
    void RecalculateStats();
};

// Crew Manager - Handles crew roster and synergies
class CrewManager {
public:
    CrewManager();

    void Initialize();
    void Update(f64 deltaTime);

    // === Crew Roster ===
    void RegisterCrew(Crew* crew);
    Crew* GetCrew(i32 crewID);
    const Crew* GetCrew(i32 crewID) const;
    std::vector<Crew*> GetAllCrew();
    const std::vector<Crew*>& GetAllCrew() const;
    i32 GetCrewCount() const;

    // === Crew Acquisition ===
    Crew* RecruitRandomCrew(CrewRarity minRarity = CrewRarity::Common);
    Crew* RecruitSpecificCrew(i32 crewID);
    void AssignStarterCrew(); // Give player starting crew

    // === Grid Integration ===
    void SetGridSystem(GridSystem* grid) { m_GridSystem = grid; }
    GridSystem* GetGridSystem() { return m_GridSystem; }

    // Calculate total synergy bonuses based on grid placement
    f64 CalculateTotalDamageBonus() const;
    f64 CalculateTotalDefenseBonus() const;
    f64 CalculateTotalResourceBonus() const;

    // === Synergy Database ===
    void RegisterSynergy(const SynergyBonus& synergy);
    SynergyBonus GetSynergyBonus(CrewTag tag1, CrewTag tag2) const;
    std::vector<SynergyBonus> GetActiveSynergies() const; // Based on current grid

    // === Statistics ===
    i32 GetCrewByRarity(CrewRarity rarity) const;
    i32 GetCrewByRole(CrewRole role) const;
    f64 GetAverageLevel() const;

    // === Save/Load ===
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    std::vector<Crew*> m_Roster;
    i32 m_NextCrewID;

    GridSystem* m_GridSystem; // Reference to grid for synergy calculations

    // Synergy bonuses database
    std::vector<SynergyBonus> m_SynergyDatabase;

    void InitializeSynergies();
};
