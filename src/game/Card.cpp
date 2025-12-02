#include "Card.h"

Card::Card()
    : id(0), name(""), description(""), rarity(CardRarity::Common),
      qubitCost(0.0), coherenceCost(0.0), energyCost(0),
      upgradeLevel(0), canUpgrade(true),
      minPrestigeLevel(0), unlocked(true) {
}

Card::Card(i32 _id, const std::string& _name, const std::string& _desc,
           CardRarity _rarity, f64 _qubitCost, f64 _coherenceCost, i32 _energyCost,
           i32 _minPrestige)
    : id(_id), name(_name), description(_desc), rarity(_rarity),
      qubitCost(_qubitCost), coherenceCost(_coherenceCost), energyCost(_energyCost),
      upgradeLevel(0), canUpgrade(true),
      minPrestigeLevel(_minPrestige), unlocked(_minPrestige == 0) {
}

Color Card::GetRarityColor() const {
    switch (rarity) {
        case CardRarity::Common:
            return Color(0.6f, 0.6f, 0.6f, 1.0f);      // Gray
        case CardRarity::Uncommon:
            return Color(0.2f, 1.0f, 0.2f, 1.0f);      // Green
        case CardRarity::Rare:
            return Color(0.2f, 0.6f, 1.0f, 1.0f);      // Blue
        case CardRarity::Epic:
            return Color(0.8f, 0.3f, 1.0f, 1.0f);      // Purple
        case CardRarity::Legendary:
            return Color(1.0f, 0.9f, 0.2f, 1.0f);      // Gold
        default:
            return Color::White();
    }
}

const char* Card::GetRarityName() const {
    switch (rarity) {
        case CardRarity::Common: return "Common";
        case CardRarity::Uncommon: return "Uncommon";
        case CardRarity::Rare: return "Rare";
        case CardRarity::Epic: return "Epic";
        case CardRarity::Legendary: return "Legendary";
        default: return "Unknown";
    }
}

void Card::AddEffect(const CardEffect& effect) {
    effects.push_back(effect);
}

void Card::Upgrade() {
    if (!canUpgrade || upgradeLevel >= 3) return;

    upgradeLevel++;

    // Boost all effect values by 25% per upgrade
    for (auto& effect : effects) {
        effect.value *= 1.25;
    }

    // Reduce costs slightly (10% per upgrade)
    qubitCost *= 0.9;
    coherenceCost *= 0.9;
    if (energyCost > 0) {
        energyCost = static_cast<i32>(energyCost * 0.9);
    }
}

bool Card::CanAfford(f64 qubits, f64 coherence, i32 energy) const {
    return qubits >= qubitCost &&
           coherence >= coherenceCost &&
           energy >= energyCost;
}
