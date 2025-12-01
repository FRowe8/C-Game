#include "EssenceShop.h"
#include "GameState.h"
#include "Logger.h"
#include <cmath>

EssenceShopManager::EssenceShopManager() {
}

EssenceShopManager::~EssenceShopManager() {
}

void EssenceShopManager::Initialize(GameState* gameState) {
    (void)gameState; // May be used in future
    Log::Info("Initializing Essence Shop...");
    m_Upgrades.clear();

    // Upgrade 1: All Production +10% (Repeatable)
    EssenceUpgrade productionBoost;
    productionBoost.id = "production_boost";
    productionBoost.name = "Quantum Amplifier";
    productionBoost.description = "All production +10% (stacks multiplicatively)";
    productionBoost.baseCost = 10.0;
    productionBoost.costMultiplier = 1.5; // Gets more expensive each time
    productionBoost.maxPurchases = -1; // Infinite
    productionBoost.timesPurchased = 0;
    productionBoost.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in GetProductionMultiplier()
        Log::Info("Production boost purchased!");
    };
    m_Upgrades.push_back(productionBoost);

    // Upgrade 2: Offline Progress +25% (One-time)
    EssenceUpgrade offlineBoost;
    offlineBoost.id = "offline_boost";
    offlineBoost.name = "Temporal Persistence";
    offlineBoost.description = "Offline progress +25%";
    offlineBoost.baseCost = 50.0;
    offlineBoost.costMultiplier = 1.0; // Not repeatable
    offlineBoost.maxPurchases = 1;
    offlineBoost.timesPurchased = 0;
    offlineBoost.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in GetOfflineProgressMultiplier()
        Log::Info("Offline progress boost purchased!");
    };
    m_Upgrades.push_back(offlineBoost);

    // Upgrade 3: Starting Qubits +1000 (Repeatable, limited)
    EssenceUpgrade startingQubits;
    startingQubits.id = "starting_qubits";
    startingQubits.name = "Quantum Foundation";
    startingQubits.description = "Start each prestige with +1000 qubits";
    startingQubits.baseCost = 20.0;
    startingQubits.costMultiplier = 2.0; // Doubles each time
    startingQubits.maxPurchases = 5; // Max 5 purchases (5000 starting qubits)
    startingQubits.timesPurchased = 0;
    startingQubits.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in GetStartingQubits()
        Log::Info("Starting qubits boost purchased!");
    };
    m_Upgrades.push_back(startingQubits);

    // Upgrade 4: Double Photon Gain (One-time)
    EssenceUpgrade photonDouble;
    photonDouble.id = "photon_double";
    photonDouble.name = "Photonic Resonance";
    photonDouble.description = "Double all photon gains";
    photonDouble.baseCost = 100.0;
    photonDouble.costMultiplier = 1.0; // Not repeatable
    photonDouble.maxPurchases = 1;
    photonDouble.timesPurchased = 0;
    photonDouble.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in PerformPrestige()
        Log::Info("Photon doubling purchased!");
    };
    m_Upgrades.push_back(photonDouble);

    // Upgrade 5: Auto-Observer (One-time) - Makes Auto-Observer research free
    EssenceUpgrade freeAutoObserver;
    freeAutoObserver.id = "free_auto_observer";
    freeAutoObserver.name = "Automated Observation";
    freeAutoObserver.description = "Auto-Observer research is unlocked from start";
    freeAutoObserver.baseCost = 75.0;
    freeAutoObserver.costMultiplier = 1.0; // Not repeatable
    freeAutoObserver.maxPurchases = 1;
    freeAutoObserver.timesPurchased = 0;
    freeAutoObserver.onPurchase = [](GameState* state) {
        (void)state; // Effect applied at prestige
        Log::Info("Auto-Observer unlock purchased!");
    };
    m_Upgrades.push_back(freeAutoObserver);

    Log::Infof("Initialized essence shop count: ", m_Upgrades.size());
}

bool EssenceShopManager::Purchase(const std::string& id, GameState* gameState) {
    EssenceUpgrade* upgrade = GetUpgrade(id);
    if (!upgrade) {
        Log::Warningf("Essence upgrade not found: ", id);
        return false;
    }

    // Check if maxed
    if (upgrade->IsMaxed()) {
        Log::Info("This upgrade is already maxed out");
        return false;
    }

    // Check if can afford
    f64 cost = upgrade->GetCurrentCost();
    if (!gameState->SpendEssence(cost)) {
        Log::Info("Not enough Quantum Essence");
        return false;
    }

    // Purchase upgrade
    upgrade->timesPurchased++;
    if (upgrade->onPurchase) {
        upgrade->onPurchase(gameState);
    }

    Log::Infof("Purchased essence upgrade: ", upgrade->name);
    return true;
}

EssenceUpgrade* EssenceShopManager::GetUpgrade(const std::string& id) {
    for (auto& upgrade : m_Upgrades) {
        if (upgrade.id == id) {
            return &upgrade;
        }
    }
    return nullptr;
}

f64 EssenceShopManager::GetGlobalProductionBonus() const {
    // This assumes the intention of this function is identical
    // to the already declared GetProductionMultiplier().
    return GetProductionMultiplier();
}

f64 EssenceShopManager::GetProductionMultiplier() const {
    const EssenceUpgrade* productionBoost = nullptr;
    for (const auto& upgrade : m_Upgrades) {
        if (upgrade.id == "production_boost") {
            productionBoost = &upgrade;
            break;
        }
    }

    if (!productionBoost || productionBoost->timesPurchased == 0) {
        return 1.0;
    }

    // Each purchase gives +10% (multiplicative)
    // 1 purchase: 1.1x, 2 purchases: 1.21x, 3 purchases: 1.331x, etc.
    return std::pow(1.1, productionBoost->timesPurchased);
}

f64 EssenceShopManager::GetStartingQubits() const {
    const EssenceUpgrade* startingQubits = nullptr;
    for (const auto& upgrade : m_Upgrades) {
        if (upgrade.id == "starting_qubits") {
            startingQubits = &upgrade;
            break;
        }
    }

    if (!startingQubits) {
        return 0.0;
    }

    return static_cast<f64>(startingQubits->timesPurchased) * 1000.0;
}

f64 EssenceShopManager::GetOfflineProgressMultiplier() const {
    const EssenceUpgrade* offlineBoost = nullptr;
    for (const auto& upgrade : m_Upgrades) {
        if (upgrade.id == "offline_boost") {
            offlineBoost = &upgrade;
            break;
        }
    }

    if (!offlineBoost || offlineBoost->timesPurchased == 0) {
        return 1.0;
    }

    return 1.25; // +25% offline progress
}


