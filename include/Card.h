#pragma once

#include "Types.h"
#include <string>
#include <vector>

// Card System - Quantum Deck building and strategic gameplay

enum class CardRarity {
    Common,      // Gray - Easy to find
    Uncommon,    // Green - Moderate chance
    Rare,        // Blue - Low chance
    Epic,        // Purple - Very low chance
    Legendary    // Gold - Extremely rare
};

enum class CardType {
    Attack,         // Deal damage in combat
    Defense,        // Boost shields/defense
    Utility,        // Special effects (draw cards, gain resources, etc.)
    ResourceGen,    // Generate quantum resources (Qubits, Coherence)
    Buff,           // Temporary stat boost
    Debuff,         // Weaken enemies
    Synergy         // Combo with other cards/crew
};

enum class CardTarget {
    Self,           // Affects player
    Enemy,          // Affects current enemy
    AllEnemies,     // Affects all enemies (boss fights)
    Random,         // Random target
    LowestHP,       // Target with lowest HP
    HighestHP       // Target with highest HP
};

// Card effect definition
struct CardEffect {
    CardType type;
    f64 value;              // Damage, heal amount, resource gain, etc.
    f64 duration;           // For buffs/debuffs (in seconds, 0 = instant)
    CardTarget target;

    CardEffect()
        : type(CardType::Attack), value(0.0), duration(0.0), target(CardTarget::Enemy) {}

    CardEffect(CardType _type, f64 _value, f64 _duration = 0.0, CardTarget _target = CardTarget::Enemy)
        : type(_type), value(_value), duration(_duration), target(_target) {}
};

// Card definition
struct Card {
    i32 id;
    std::string name;
    std::string description;
    CardRarity rarity;

    // Cost to play
    f64 qubitCost;
    f64 coherenceCost;
    i32 energyCost;         // Energy is gained each turn in combat

    // Effects (a card can have multiple effects)
    std::vector<CardEffect> effects;

    // Upgrade tracking
    i32 upgradeLevel;       // 0 = base, 1-3 = upgraded
    bool canUpgrade;

    // Unlock requirements
    i32 minPrestigeLevel;
    bool unlocked;

    Card();
    Card(i32 _id, const std::string& _name, const std::string& _desc,
         CardRarity _rarity, f64 _qubitCost, f64 _coherenceCost, i32 _energyCost,
         i32 _minPrestige = 0);

    // Get display color based on rarity
    Color GetRarityColor() const;

    // Get rarity name
    const char* GetRarityName() const;

    // Add effect to card
    void AddEffect(const CardEffect& effect);

    // Upgrade card (increases effect values)
    void Upgrade();

    // Can afford to play this card?
    bool CanAfford(f64 qubits, f64 coherence, i32 energy) const;
};

// Card collection/inventory
struct CardInstance {
    i32 cardID;             // References Card definition
    i32 instanceID;         // Unique instance ID
    i32 upgradeLevel;       // This instance's upgrade level
    bool inDeck;            // Is this card in the active deck?

    CardInstance(i32 _cardID, i32 _instanceID)
        : cardID(_cardID), instanceID(_instanceID), upgradeLevel(0), inDeck(false) {}
};
