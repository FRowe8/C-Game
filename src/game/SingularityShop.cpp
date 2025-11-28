#include "SingularityShop.h"
#include "GameState.h"
#include "Logger.h"
#include <cmath>

SingularityShopManager::SingularityShopManager() {
}

SingularityShopManager::~SingularityShopManager() {
}

void SingularityShopManager::Initialize(GameState* gameState) {
    (void)gameState; // May be used in future
    Log::Info("Initializing Singularity Shop...");
    m_Upgrades.clear();

    // Upgrade 1: Photon Generator (Repeatable)
    SingularityUpgrade photonGen;
    photonGen.id = "photon_generator";
    photonGen.name = "Photon Singularity Generator";
    photonGen.description = "Generate 1 photon/sec per singularity owned (stacks)";
    photonGen.baseCost = 1.0;
    photonGen.costMultiplier = 2.0; // Doubles each time
    photonGen.maxPurchases = -1; // Infinite
    photonGen.timesPurchased = 0;
    photonGen.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in GetPhotonGenerationRate()
        Log::Info("Photon generation boost purchased!");
    };
    m_Upgrades.push_back(photonGen);

    // Upgrade 2: Research Cost Reduction (Repeatable)
    SingularityUpgrade researchDiscount;
    researchDiscount.id = "research_discount";
    researchDiscount.name = "Quantum Research Efficiency";
    researchDiscount.description = "All research costs -10% (stacks multiplicatively)";
    researchDiscount.baseCost = 2.0;
    researchDiscount.costMultiplier = 3.0; // Gets expensive fast
    researchDiscount.maxPurchases = 10; // Max 10 purchases (65% discount)
    researchDiscount.timesPurchased = 0;
    researchDiscount.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in GetResearchCostMultiplier()
        Log::Info("Research cost reduction purchased!");
    };
    m_Upgrades.push_back(researchDiscount);

    // Upgrade 3: Challenge Reward Bonus (Repeatable, limited)
    SingularityUpgrade challengeBonus;
    challengeBonus.id = "challenge_bonus";
    challengeBonus.name = "Challenge Mastery";
    challengeBonus.description = "All challenge rewards +25%";
    challengeBonus.baseCost = 5.0;
    challengeBonus.costMultiplier = 4.0; // Very expensive
    challengeBonus.maxPurchases = 5; // Max 5 purchases (2.4x rewards)
    challengeBonus.timesPurchased = 0;
    challengeBonus.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in GetChallengeRewardMultiplier()
        Log::Info("Challenge reward boost purchased!");
    };
    m_Upgrades.push_back(challengeBonus);

    // Upgrade 4: Starting Photons (One-time)
    SingularityUpgrade startingPhotons;
    startingPhotons.id = "starting_photons";
    startingPhotons.name = "Photon Reservoir";
    startingPhotons.description = "Start each collapse with 100 photons";
    startingPhotons.baseCost = 10.0;
    startingPhotons.costMultiplier = 1.0; // Not repeatable
    startingPhotons.maxPurchases = 1;
    startingPhotons.timesPurchased = 0;
    startingPhotons.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in PerformCollapse()
        Log::Info("Starting photons purchased!");
    };
    m_Upgrades.push_back(startingPhotons);

    // Upgrade 5: Auto-Collapse (One-time)
    SingularityUpgrade autoCollapse;
    autoCollapse.id = "auto_collapse";
    autoCollapse.name = "Automated Collapse";
    autoCollapse.description = "Automatically collapse at configurable threshold";
    autoCollapse.baseCost = 20.0;
    autoCollapse.costMultiplier = 1.0; // Not repeatable
    autoCollapse.maxPurchases = 1;
    autoCollapse.timesPurchased = 0;
    autoCollapse.onPurchase = [](GameState* state) {
        (void)state; // Effect applied in Update()
        Log::Info("Auto-collapse purchased!");
    };
    m_Upgrades.push_back(autoCollapse);

    Log::Infof("Initialized singularity shop count: ", m_Upgrades.size());
}

bool SingularityShopManager::Purchase(const std::string& id, GameState* gameState) {
    SingularityUpgrade* upgrade = GetUpgrade(id);
    if (!upgrade) {
        Log::Warningf("Singularity upgrade not found: ", id);
        return false;
    }

    // Check if maxed
    if (upgrade->IsMaxed()) {
        Log::Info("This upgrade is already maxed out");
        return false;
    }

    // Check if can afford
    f64 cost = upgrade->GetCurrentCost();
    f64 currentSingularities = gameState->GetTimeline().singularities;
    if (currentSingularities < cost) {
        Log::Info("Not enough singularities");
        return false;
    }

    // Spend singularities
    gameState->GetTimeline().singularities -= cost;

    // Purchase upgrade
    upgrade->timesPurchased++;
    if (upgrade->onPurchase) {
        upgrade->onPurchase(gameState);
    }

    Log::Infof("Purchased singularity upgrade: ", upgrade->name);
    return true;
}

SingularityUpgrade* SingularityShopManager::GetUpgrade(const std::string& id) {
    for (auto& upgrade : m_Upgrades) {
        if (upgrade.id == id) {
            return &upgrade;
        }
    }
    return nullptr;
}

f64 SingularityShopManager::GetPhotonGenerationRate() const {
    const SingularityUpgrade* photonGen = nullptr;
    for (const auto& upgrade : m_Upgrades) {
        if (upgrade.id == "photon_generator") {
            photonGen = &upgrade;
            break;
        }
    }

    if (!photonGen || photonGen->timesPurchased == 0) {
        return 0.0;
    }

    // Each purchase enables 1 photon/sec per singularity
    // If you have 10 singularities and bought this 2x, you get 20 photons/sec
    return static_cast<f64>(photonGen->timesPurchased);
}

f64 SingularityShopManager::GetResearchCostMultiplier() const {
    const SingularityUpgrade* researchDiscount = nullptr;
    for (const auto& upgrade : m_Upgrades) {
        if (upgrade.id == "research_discount") {
            researchDiscount = &upgrade;
            break;
        }
    }

    if (!researchDiscount || researchDiscount->timesPurchased == 0) {
        return 1.0;
    }

    // Each purchase gives -10% (multiplicative)
    // 1 purchase: 0.9x, 2 purchases: 0.81x, 3 purchases: 0.729x, etc.
    return std::pow(0.9, researchDiscount->timesPurchased);
}

f64 SingularityShopManager::GetChallengeRewardMultiplier() const {
    const SingularityUpgrade* challengeBonus = nullptr;
    for (const auto& upgrade : m_Upgrades) {
        if (upgrade.id == "challenge_bonus") {
            challengeBonus = &upgrade;
            break;
        }
    }

    if (!challengeBonus || challengeBonus->timesPurchased == 0) {
        return 1.0;
    }

    // Each purchase gives +25% (multiplicative)
    // 1 purchase: 1.25x, 2 purchases: 1.5625x, etc.
    return std::pow(1.25, challengeBonus->timesPurchased);
}
