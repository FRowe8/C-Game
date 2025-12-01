#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <functional>
#include <cmath>

// Forward declaration
class GameState;

// Singularity upgrade (permanent, purchased with singularities)
struct SingularityUpgrade {
    std::string id;
    std::string name;
    std::string description;
    f64 baseCost;
    f64 costMultiplier;     // For repeatable upgrades
    i32 maxPurchases;       // -1 for infinite
    i32 timesPurchased;
    std::function<void(GameState*)> onPurchase;

    bool IsMaxed() const {
        return maxPurchases != -1 && timesPurchased >= maxPurchases;
    }

    f64 GetCurrentCost() const {
        if (maxPurchases == 1) return baseCost; // One-time purchase
        return baseCost * std::pow(costMultiplier, timesPurchased);
    }

    bool CanAfford(f64 singularities) const {
        return !IsMaxed() && singularities >= GetCurrentCost();
    }

    std::string GetProgressString() const {
        if (maxPurchases == -1) {
            return "Purchased: " + std::to_string(timesPurchased) + "x";
        } else if (maxPurchases == 1) {
            return timesPurchased > 0 ? "PURCHASED" : "Not purchased";
        } else {
            return std::to_string(timesPurchased) + "/" + std::to_string(maxPurchases);
        }
    }
};

class SingularityShopManager {
public:
    SingularityShopManager();
    ~SingularityShopManager();

    void Initialize(GameState* gameState);
    bool Purchase(const std::string& id, GameState* gameState);

    SingularityUpgrade* GetUpgrade(const std::string& id);
    const std::vector<SingularityUpgrade>& GetUpgrades() const { return m_Upgrades; }

    // Get total bonuses from all purchases
    f64 GetPhotonGenerationRate() const;      // Photons per second
    f64 GetResearchCostMultiplier() const;    // Cost multiplier for research
    f64 GetChallengeRewardMultiplier() const; // Bonus to challenge rewards

private:
    std::vector<SingularityUpgrade> m_Upgrades;
};
