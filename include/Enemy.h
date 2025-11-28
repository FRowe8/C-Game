#pragma once

#include "Types.h"
#include <string>

// Forward declarations
class Renderer;

// Enemy types (different behavior and loot tables)
enum class EnemyType {
    Scout,       // Fast, low HP, common drops
    Fighter,     // Balanced stats, decent drops
    Cruiser,     // High HP, high defense, rare drops
    Battleship,  // Very high stats, epic drops
    Boss         // Extremely powerful, guaranteed rare+ drops
};

// Enemy difficulty tier (affects stats and rewards)
enum class EnemyTier {
    Tier1,  // Levels 1-20
    Tier2,  // Levels 21-40
    Tier3,  // Levels 41-60
    Tier4,  // Levels 61-80
    Tier5   // Levels 81-100
};

// Individual enemy instance
class Enemy {
public:
    Enemy();
    Enemy(EnemyType type, EnemyTier tier, i32 level);

    // Combat stats
    f64 GetMaxHealth() const { return m_MaxHealth; }
    f64 GetCurrentHealth() const { return m_CurrentHealth; }
    f64 GetAttack() const { return m_Attack; }
    f64 GetDefense() const { return m_Defense; }
    f64 GetSpeed() const { return m_Speed; }
    i32 GetLevel() const { return m_Level; }

    // Info
    const char* GetName() const { return m_Name.c_str(); }
    const char* GetDescription() const { return m_Description.c_str(); }
    EnemyType GetType() const { return m_Type; }
    EnemyTier GetTier() const { return m_Tier; }

    // State
    bool IsAlive() const { return m_CurrentHealth > 0.0; }
    bool IsBoss() const { return m_Type == EnemyType::Boss; }
    f64 GetHealthPercent() const { return m_MaxHealth > 0.0 ? (m_CurrentHealth / m_MaxHealth) * 100.0 : 0.0; }

    // Combat actions
    void TakeDamage(f64 damage);
    void Heal(f64 amount);
    void ResetHealth() { m_CurrentHealth = m_MaxHealth; }

    // Loot/Rewards
    i32 GetCreditReward() const { return m_CreditReward; }
    i32 GetXPReward() const { return m_XPReward; }
    f64 GetPartDropChance() const { return m_PartDropChance; }
    i32 GetMinPartRarity() const { return m_MinPartRarity; } // 0=Common, 1=Uncommon, etc.

    // Visual
    Color GetTypeColor() const;
    const char* GetTypeName() const;
    const char* GetTierName() const;

    // Rendering
    void Render(Renderer* renderer, f32 x, f32 y, f32 width, f32 height);

private:
    // Identity
    std::string m_Name;
    std::string m_Description;
    EnemyType m_Type;
    EnemyTier m_Tier;
    i32 m_Level;

    // Combat stats
    f64 m_MaxHealth;
    f64 m_CurrentHealth;
    f64 m_Attack;
    f64 m_Defense;
    f64 m_Speed;

    // Rewards
    i32 m_CreditReward;
    i32 m_XPReward;
    f64 m_PartDropChance;
    i32 m_MinPartRarity;

    // Initialize based on type and tier
    void GenerateStats();
    void GenerateName();
    void GenerateRewards();
};

// Enemy generation utilities
namespace EnemyGenerator {
    // Generate random enemy for a zone/level range
    Enemy GenerateEnemy(i32 playerLevel);

    // Generate specific enemy type
    Enemy GenerateEnemyOfType(EnemyType type, i32 level);

    // Generate boss enemy for a zone
    Enemy GenerateBoss(EnemyTier tier);

    // Get tier from level
    EnemyTier GetTierFromLevel(i32 level);
}
