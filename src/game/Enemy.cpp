#include "Enemy.h"
#include "Renderer.h"
#include "Logger.h"
#include <cstdlib>
#include <cmath>

// Enemy Implementation
Enemy::Enemy()
    : m_Name("Unknown Enemy"), m_Description("A mysterious foe"),
      m_Type(EnemyType::Scout), m_Tier(EnemyTier::Tier1), m_Level(1),
      m_MaxHealth(100.0), m_CurrentHealth(100.0),
      m_Attack(10.0), m_Defense(5.0), m_Speed(10.0),
      m_CreditReward(10), m_XPReward(5), m_PartDropChance(0.1), m_MinPartRarity(0) {
}

Enemy::Enemy(EnemyType type, EnemyTier tier, i32 level)
    : m_Type(type), m_Tier(tier), m_Level(level),
      m_CurrentHealth(0.0), m_CreditReward(0), m_XPReward(0),
      m_PartDropChance(0.0), m_MinPartRarity(0) {

    GenerateStats();
    GenerateName();
    GenerateRewards();
    m_CurrentHealth = m_MaxHealth; // Start at full health
}

void Enemy::GenerateStats() {
    // Base stats scale with level
    f64 levelScale = 1.0 + (m_Level * 0.15); // 15% per level

    // Tier multipliers (higher tiers = stronger)
    f64 tierMultiplier = 1.0;
    switch (m_Tier) {
        case EnemyTier::Tier1: tierMultiplier = 1.0; break;
        case EnemyTier::Tier2: tierMultiplier = 2.5; break;
        case EnemyTier::Tier3: tierMultiplier = 6.0; break;
        case EnemyTier::Tier4: tierMultiplier = 15.0; break;
        case EnemyTier::Tier5: tierMultiplier = 40.0; break;
    }

    // Type-based stat profiles
    switch (m_Type) {
        case EnemyType::Scout:
            m_MaxHealth = 50.0 * levelScale * tierMultiplier;
            m_Attack = 8.0 * levelScale * tierMultiplier;
            m_Defense = 3.0 * levelScale * tierMultiplier;
            m_Speed = 20.0 + (m_Level * 0.5); // Fast
            m_Description = "A nimble scout ship";
            break;

        case EnemyType::Fighter:
            m_MaxHealth = 100.0 * levelScale * tierMultiplier;
            m_Attack = 15.0 * levelScale * tierMultiplier;
            m_Defense = 8.0 * levelScale * tierMultiplier;
            m_Speed = 15.0 + (m_Level * 0.3);
            m_Description = "A balanced combat vessel";
            break;

        case EnemyType::Cruiser:
            m_MaxHealth = 200.0 * levelScale * tierMultiplier;
            m_Attack = 12.0 * levelScale * tierMultiplier;
            m_Defense = 15.0 * levelScale * tierMultiplier;
            m_Speed = 10.0 + (m_Level * 0.2); // Slow
            m_Description = "A heavily armored cruiser";
            break;

        case EnemyType::Battleship:
            m_MaxHealth = 350.0 * levelScale * tierMultiplier;
            m_Attack = 25.0 * levelScale * tierMultiplier;
            m_Defense = 20.0 * levelScale * tierMultiplier;
            m_Speed = 8.0 + (m_Level * 0.15); // Very slow
            m_Description = "A massive warship";
            break;

        case EnemyType::Boss:
            m_MaxHealth = 1000.0 * levelScale * tierMultiplier;
            m_Attack = 40.0 * levelScale * tierMultiplier;
            m_Defense = 30.0 * levelScale * tierMultiplier;
            m_Speed = 12.0 + (m_Level * 0.2);
            m_Description = "A legendary commander";
            break;
    }

    // Add random variance (±10%)
    f64 variance = 0.9 + (rand() % 20) / 100.0;
    m_MaxHealth *= variance;
    m_Attack *= variance;
    m_Defense *= variance;
}

void Enemy::GenerateName() {
    const char* prefixes[] = {
        "Rogue", "Hostile", "Corrupted", "Ancient", "Void",
        "Quantum", "Plasma", "Dark", "Crimson", "Shadow"
    };

    const char* suffixes[] = {
        "Raider", "Marauder", "Destroyer", "Annihilator", "Reaver",
        "Hunter", "Enforcer", "Sentinel", "Guardian", "Warlord"
    };

    i32 prefixIndex = rand() % 10;
    i32 suffixIndex = rand() % 10;

    // Boss gets special title
    if (m_Type == EnemyType::Boss) {
        const char* bossTitles[] = {
            "Admiral", "Commander", "Overlord", "Tyrant", "Archon",
            "Lord", "Emperor", "Master", "Supreme Leader", "Dreadnought"
        };
        i32 titleIndex = rand() % 10;
        m_Name = std::string(bossTitles[titleIndex]) + " " +
                 std::string(prefixes[prefixIndex]);
    } else {
        m_Name = std::string(prefixes[prefixIndex]) + " " +
                 std::string(suffixes[suffixIndex]);
    }
}

void Enemy::GenerateRewards() {
    // Base rewards scale with level and tier
    f64 levelScale = m_Level;
    f64 tierScale = static_cast<f64>(static_cast<i32>(m_Tier) + 1);

    // Credit rewards
    switch (m_Type) {
        case EnemyType::Scout:
            m_CreditReward = static_cast<i32>(10.0 * levelScale * tierScale);
            m_XPReward = static_cast<i32>(5.0 * levelScale);
            m_PartDropChance = 0.15; // 15%
            m_MinPartRarity = 0; // Common
            break;

        case EnemyType::Fighter:
            m_CreditReward = static_cast<i32>(25.0 * levelScale * tierScale);
            m_XPReward = static_cast<i32>(12.0 * levelScale);
            m_PartDropChance = 0.25; // 25%
            m_MinPartRarity = 0;
            break;

        case EnemyType::Cruiser:
            m_CreditReward = static_cast<i32>(50.0 * levelScale * tierScale);
            m_XPReward = static_cast<i32>(25.0 * levelScale);
            m_PartDropChance = 0.40; // 40%
            m_MinPartRarity = 1; // At least Uncommon
            break;

        case EnemyType::Battleship:
            m_CreditReward = static_cast<i32>(100.0 * levelScale * tierScale);
            m_XPReward = static_cast<i32>(50.0 * levelScale);
            m_PartDropChance = 0.60; // 60%
            m_MinPartRarity = 2; // At least Rare
            break;

        case EnemyType::Boss:
            m_CreditReward = static_cast<i32>(500.0 * levelScale * tierScale);
            m_XPReward = static_cast<i32>(200.0 * levelScale);
            m_PartDropChance = 1.0; // 100% guaranteed drop
            m_MinPartRarity = 3; // At least Epic
            break;
    }

    // Add random variance (±20%)
    f64 variance = 0.8 + (rand() % 40) / 100.0;
    m_CreditReward = static_cast<i32>(m_CreditReward * variance);
    m_XPReward = static_cast<i32>(m_XPReward * variance);
}

void Enemy::TakeDamage(f64 damage) {
    if (!IsAlive()) return;

    // Apply damage reduction from defense
    // Formula: damage * (100 / (100 + defense))
    f64 damageReduction = 100.0 / (100.0 + m_Defense);
    f64 actualDamage = damage * damageReduction;

    m_CurrentHealth -= actualDamage;
    if (m_CurrentHealth < 0.0) {
        m_CurrentHealth = 0.0;
    }

    Log::Infof(m_Name.c_str(), " takes ", actualDamage, " damage! (", m_CurrentHealth, "/", m_MaxHealth, " HP)");
}

void Enemy::Heal(f64 amount) {
    if (!IsAlive()) return;

    m_CurrentHealth += amount;
    if (m_CurrentHealth > m_MaxHealth) {
        m_CurrentHealth = m_MaxHealth;
    }
}

Color Enemy::GetTypeColor() const {
    switch (m_Type) {
        case EnemyType::Scout: return Color(0.7f, 0.9f, 0.7f, 1.0f); // Light green
        case EnemyType::Fighter: return Color(0.9f, 0.9f, 0.5f, 1.0f); // Yellow
        case EnemyType::Cruiser: return Color(0.9f, 0.6f, 0.3f, 1.0f); // Orange
        case EnemyType::Battleship: return Color(0.9f, 0.3f, 0.3f, 1.0f); // Red
        case EnemyType::Boss: return Color(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
    }
    return Color::White();
}

const char* Enemy::GetTypeName() const {
    switch (m_Type) {
        case EnemyType::Scout: return "Scout";
        case EnemyType::Fighter: return "Fighter";
        case EnemyType::Cruiser: return "Cruiser";
        case EnemyType::Battleship: return "Battleship";
        case EnemyType::Boss: return "BOSS";
    }
    return "Unknown";
}

const char* Enemy::GetTierName() const {
    switch (m_Tier) {
        case EnemyTier::Tier1: return "Tier I";
        case EnemyTier::Tier2: return "Tier II";
        case EnemyTier::Tier3: return "Tier III";
        case EnemyTier::Tier4: return "Tier IV";
        case EnemyTier::Tier5: return "Tier V";
    }
    return "Unknown";
}

void Enemy::Render(Renderer* renderer, f32 x, f32 y, f32 width, f32 height) {
    // Background panel
    Rect bgRect(x, y, width, height);
    Color bgColor = GetTypeColor() * 0.15f;
    bgColor.a = 0.9f;
    renderer->DrawRect(bgRect, bgColor, true);

    // Border (brighter for bosses)
    Color borderColor = IsBoss() ? GetTypeColor() : (GetTypeColor() * 0.6f);
    renderer->DrawRect(bgRect, borderColor, false);

    f32 yOffset = y + 10.0f;

    // Enemy name
    Color nameColor = IsBoss() ? Color(1.0f, 0.5f, 1.0f, 1.0f) : GetTypeColor();
    renderer->DrawText(m_Name.c_str(), Vec2(x + 10.0f, yOffset), nameColor, IsBoss() ? 20.0f : 18.0f);
    yOffset += IsBoss() ? 25.0f : 22.0f;

    // Type and level
    char typeLevel[64];
    snprintf(typeLevel, sizeof(typeLevel), "%s | Level %d | %s", GetTypeName(), m_Level, GetTierName());
    renderer->DrawText(typeLevel, Vec2(x + 10.0f, yOffset), Color(0.8f, 0.8f, 0.8f, 1.0f), 12.0f);
    yOffset += 20.0f;

    // Health bar
    f32 barWidth = width - 20.0f;
    f32 barHeight = 25.0f;
    Rect hpBarBg(x + 10.0f, yOffset, barWidth, barHeight);
    Rect hpBarFill(x + 10.0f, yOffset, barWidth * (m_CurrentHealth / m_MaxHealth), barHeight);

    renderer->DrawRect(hpBarBg, Color(0.2f, 0.2f, 0.2f, 1.0f), true);

    Color hpColor = GetHealthPercent() > 50.0 ? Color(0.2f, 1.0f, 0.2f, 1.0f) :
                    GetHealthPercent() > 25.0 ? Color(1.0f, 0.8f, 0.0f, 1.0f) :
                    Color(1.0f, 0.2f, 0.2f, 1.0f);
    renderer->DrawRect(hpBarFill, hpColor, true);

    char hpText[64];
    snprintf(hpText, sizeof(hpText), "%.0f / %.0f HP", m_CurrentHealth, m_MaxHealth);
    renderer->DrawText(hpText, Vec2(x + width * 0.5f - 40.0f, yOffset + 6.0f), Color::White(), 14.0f);
    yOffset += 35.0f;

    // Stats
    char stats[256];
    snprintf(stats, sizeof(stats), "ATK: %.0f | DEF: %.0f | SPD: %.0f", m_Attack, m_Defense, m_Speed);
    renderer->DrawText(stats, Vec2(x + 10.0f, yOffset), Color(0.7f, 0.7f, 0.9f, 1.0f), 12.0f);
    yOffset += 20.0f;

    // Rewards preview
    char rewards[256];
    snprintf(rewards, sizeof(rewards), "Rewards: %d Credits | %d XP | %.0f%% Part Drop",
             m_CreditReward, m_XPReward, m_PartDropChance * 100.0);
    renderer->DrawText(rewards, Vec2(x + 10.0f, yOffset), Color(1.0f, 0.9f, 0.3f, 1.0f), 11.0f);
}

// EnemyGenerator Implementation
namespace EnemyGenerator {
    EnemyTier GetTierFromLevel(i32 level) {
        if (level <= 20) return EnemyTier::Tier1;
        if (level <= 40) return EnemyTier::Tier2;
        if (level <= 60) return EnemyTier::Tier3;
        if (level <= 80) return EnemyTier::Tier4;
        return EnemyTier::Tier5;
    }

    Enemy GenerateEnemy(i32 playerLevel) {
        // Random enemy type (weighted towards weaker types)
        i32 roll = rand() % 100;
        EnemyType type;

        if (roll < 40) {
            type = EnemyType::Scout;
        } else if (roll < 70) {
            type = EnemyType::Fighter;
        } else if (roll < 90) {
            type = EnemyType::Cruiser;
        } else {
            type = EnemyType::Battleship;
        }

        // Generate level close to player level (±2 levels)
        i32 levelVariance = (rand() % 5) - 2; // -2 to +2
        i32 enemyLevel = playerLevel + levelVariance;
        if (enemyLevel < 1) enemyLevel = 1;
        if (enemyLevel > 100) enemyLevel = 100;

        EnemyTier tier = GetTierFromLevel(enemyLevel);

        return Enemy(type, tier, enemyLevel);
    }

    Enemy GenerateEnemyOfType(EnemyType type, i32 level) {
        if (level < 1) level = 1;
        if (level > 100) level = 100;

        EnemyTier tier = GetTierFromLevel(level);
        return Enemy(type, tier, level);
    }

    Enemy GenerateBoss(EnemyTier tier) {
        // Boss level is at the top of its tier range
        i32 level;
        switch (tier) {
            case EnemyTier::Tier1: level = 20; break;
            case EnemyTier::Tier2: level = 40; break;
            case EnemyTier::Tier3: level = 60; break;
            case EnemyTier::Tier4: level = 80; break;
            case EnemyTier::Tier5: level = 100; break;
        }

        return Enemy(EnemyType::Boss, tier, level);
    }
}
