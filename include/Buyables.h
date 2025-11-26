#pragma once

#include "Types.h"
#include <string>
#include <functional>
#include <vector>

// Forward declaration
class GameState;

// Buyable upgrade - repeatable purchases that provide permanent bonuses
struct BuyableUpgrade {
    std::string id;              // Unique identifier
    std::string name;            // Display name
    std::string description;     // What it does

    f64 baseCost;                // Initial cost
    f64 costMultiplier;          // How much cost increases per purchase (e.g., 1.15 = +15% each time)

    i32 timesPurchased = 0;      // How many times player has bought this
    i32 maxPurchases = -1;       // Maximum times can be purchased (-1 = infinite)

    // Callback when purchased (use this to apply effects to GameState)
    std::function<void(GameState*)> onPurchase;

    // Helper: Calculate current cost based on times purchased
    f64 GetCurrentCost() const {
        if (timesPurchased >= maxPurchases && maxPurchases != -1) {
            return -1.0; // Already maxed out
        }
        return baseCost * std::pow(costMultiplier, timesPurchased);
    }

    // Helper: Check if can afford
    bool CanAfford(f64 currentResources) const {
        if (timesPurchased >= maxPurchases && maxPurchases != -1) {
            return false; // Already maxed out
        }
        return currentResources >= GetCurrentCost();
    }

    // Helper: Check if maxed out
    bool IsMaxed() const {
        return maxPurchases != -1 && timesPurchased >= maxPurchases;
    }

    // Helper: Get progress string (e.g., "5/10" or "12/∞")
    std::string GetProgressString() const {
        if (maxPurchases == -1) {
            return std::to_string(timesPurchased) + "/∞";
        }
        return std::to_string(timesPurchased) + "/" + std::to_string(maxPurchases);
    }
};

// Buyable manager - handles all buyables in the game
class BuyableManager {
public:
    BuyableManager();
    ~BuyableManager();

    // Initialize buyables (create all available buyables)
    void Initialize(GameState* gameState);

    // Purchase a buyable (returns true if successful)
    bool Purchase(const std::string& id, GameState* gameState);

    // Get all buyables
    const std::vector<BuyableUpgrade>& GetBuyables() const { return m_Buyables; }

    // Get specific buyable by ID
    BuyableUpgrade* GetBuyable(const std::string& id);

private:
    std::vector<BuyableUpgrade> m_Buyables;

    // Helper to create specific buyables
    void CreateProductionMultipliers(GameState* gameState);
};
