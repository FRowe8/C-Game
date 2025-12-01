#pragma once
#include "Types.h"
#include <string>
#include <vector>
#include <functional>
#include <cmath>

// Forward declaration
class GameState;

// Essence upgrades provide permanent bonuses that persist through prestiges
struct EssenceUpgrade {
    std::string id;
    std::string name;
    std::string description;
    f64 baseCost;           // Base cost in Quantum Essence
    f64 costMultiplier;     // Cost multiplier for repeatable upgrades
    i32 maxPurchases;       // -1 for infinite
    i32 timesPurchased;     // How many times purchased
    std::function<void(GameState*)> onPurchase; // Effect when purchased

    bool IsMaxed() const {
        return maxPurchases > 0 && timesPurchased >= maxPurchases;
    }

    f64 GetCurrentCost() const {
        if (costMultiplier > 1.0 && timesPurchased > 0) {
            return baseCost * std::pow(costMultiplier, timesPurchased);
        }
        return baseCost;
    }

    bool CanAfford(f64 essence) const {
        if (IsMaxed()) return false;
        return essence >= GetCurrentCost();
    }

    std::string GetProgressString() const {
        if (maxPurchases <= 0) {
            return std::to_string(timesPurchased) + "x";
        }
        return std::to_string(timesPurchased) + " / " + std::to_string(maxPurchases);
    }
};

class EssenceShopManager {
public:
    EssenceShopManager();
    ~EssenceShopManager();

    void Initialize(GameState* gameState);
    bool Purchase(const std::string& id, GameState* gameState);

    std::vector<EssenceUpgrade>& GetUpgrades() { return m_Upgrades; }
    const std::vector<EssenceUpgrade>& GetUpgrades() const { return m_Upgrades; }

    EssenceUpgrade* GetUpgrade(const std::string& id);

    // Get total bonus multipliers from purchased upgrades
    f64 GetProductionMultiplier() const;
    f64 GetStartingQubits() const;
    f64 GetOfflineProgressMultiplier() const;

    f64 GetGlobalProductionBonus() const;


private:
    std::vector<EssenceUpgrade> m_Upgrades;
};
