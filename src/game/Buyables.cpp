#include "Buyables.h"
#include "GameState.h"
#include "Logger.h"

BuyableManager::BuyableManager() {
}

BuyableManager::~BuyableManager() {
}

void BuyableManager::Initialize(GameState* gameState) {
    Log::Info("Initializing buyables system...");

    // Clear existing buyables
    m_Buyables.clear();

    // Create production multipliers (Phase 2.2)
    CreateProductionMultipliers(gameState);

    Log::Infof("Initialized buyables count: ", m_Buyables.size());
}

void BuyableManager::CreateProductionMultipliers(GameState* gameState) {
    (void)gameState; // Unused parameter - reserved for future use

    // Quantum Accelerator - 2x qubit production
    // Rebalanced for gradual progression
    BuyableUpgrade quantumAccelerator;
    quantumAccelerator.id = "quantum_accelerator";
    quantumAccelerator.name = "Quantum Accelerator";
    quantumAccelerator.description = "Calibrate the tachyon emitters. It hums aggressively. (Production x2)";
    quantumAccelerator.baseCost = 25000.0; // Increased from 5000
    quantumAccelerator.costMultiplier = 2.5; // Gets expensive quickly
    quantumAccelerator.maxPurchases = -1; // Infinite
    quantumAccelerator.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in UpdateStations() by checking timesPurchased
        Log::Info("Purchased Quantum Accelerator!");
    };
    m_Buyables.push_back(quantumAccelerator);

    // Coherence Amplifier - 2x coherence stability
    BuyableUpgrade coherenceAmplifier;
    coherenceAmplifier.id = "coherence_amplifier";
    coherenceAmplifier.name = "Coherence Amplifier";
    coherenceAmplifier.description = "Phase-lock the resonance coils so coherence spills out of every monitor.";
    coherenceAmplifier.baseCost = 500.0; // Increased from 100
    coherenceAmplifier.costMultiplier = 3.0; // Very expensive scaling
    coherenceAmplifier.maxPurchases = -1; // Infinite
    coherenceAmplifier.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in UpdateStations() by checking timesPurchased
        Log::Info("Purchased Coherence Amplifier!");
    };
    m_Buyables.push_back(coherenceAmplifier);

    // Entanglement Booster - 2x entanglement generation
    BuyableUpgrade entanglementBooster;
    entanglementBooster.id = "entanglement_booster";
    entanglementBooster.name = "Entanglement Booster";
    entanglementBooster.description = "Weave a lattice of paired qubits—tug one node, the whole network answers (Entanglement x2).";
    entanglementBooster.baseCost = 1000.0; // Increased from 50
    entanglementBooster.costMultiplier = 2.8;
    entanglementBooster.maxPurchases = -1; // Infinite
    entanglementBooster.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in UpdateStations() by checking timesPurchased
        Log::Info("Purchased Entanglement Booster!");
    };
    m_Buyables.push_back(entanglementBooster);
}

bool BuyableManager::Purchase(const std::string& id, GameState* gameState) {
    BuyableUpgrade* buyable = GetBuyable(id);
    if (!buyable) {
        Log::Warningf("Buyable not found: ", id);
        return false;
    }

    // Check if maxed out
    if (buyable->IsMaxed()) {
        Log::Infof("Buyable already maxed: ", buyable->name);
        return false;
    }

    // Get current cost
    f64 cost = buyable->GetCurrentCost();
    if (cost < 0) {
        return false; // Maxed out
    }

    // Check if can afford (for now, assume qubits - later can make resource-specific)
    f64 currentQubits = gameState->GetResource(QuantumResource::Qubits);
    if (currentQubits < cost) {
        Log::Infof("Cannot afford buyable: ", buyable->name);
        return false;
    }

    // Spend resources
    if (!gameState->SpendResource(QuantumResource::Qubits, cost)) {
        return false;
    }

    // Increment purchase count
    buyable->timesPurchased++;

    // Call onPurchase callback
    if (buyable->onPurchase) {
        buyable->onPurchase(gameState);
    }

    // Award Engineering skill XP
    gameState->GetSpecializedSkills().AddExperience(SkillCategory::Engineering, SkillXP::BUY_UPGRADE);

    Log::Infof("Purchased: ", buyable->name, " (", buyable->timesPurchased, " times)");
    return true;
}

BuyableUpgrade* BuyableManager::GetBuyable(const std::string& id) {
    for (auto& buyable : m_Buyables) {
        if (buyable.id == id) {
            return &buyable;
        }
    }
    return nullptr;
}
