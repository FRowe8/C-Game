#include "Challenges.h"
#include "GameState.h"
#include "Logger.h"

ChallengeManager::ChallengeManager() {
}

ChallengeManager::~ChallengeManager() {
}

void ChallengeManager::Initialize() {
    Log::Info("Initializing challenge system...");
    m_Challenges.clear();
    m_CurrentChallengeId = "";

    // Challenge 1: Half Production
    Challenge halfProd;
    halfProd.id = "half_production";
    halfProd.name = "Efficiency Test";
    halfProd.description = "Production runs at 50% rate";
    halfProd.modifier = ChallengeModifier::HalfProduction;
    halfProd.minPrestigeLevel = 1;
    halfProd.goalQubits = 10000.0;
    halfProd.rewardMultiplier = 1.25;  // +25% permanent production
    halfProd.rewardDescription = "+25% production forever";
    halfProd.unlocked = true;
    halfProd.completed = false;
    halfProd.active = false;
    m_Challenges.push_back(halfProd);

    // Challenge 2: No Manual Upgrades
    Challenge noUpgrades;
    noUpgrades.id = "no_upgrades";
    noUpgrades.name = "Automation Only";
    noUpgrades.description = "Cannot manually upgrade stations";
    noUpgrades.modifier = ChallengeModifier::NoUpgrades;
    noUpgrades.minPrestigeLevel = 2;
    noUpgrades.goalQubits = 50000.0;
    noUpgrades.rewardMultiplier = 1.5;  // +50% permanent production
    noUpgrades.rewardDescription = "+50% production forever";
    noUpgrades.unlocked = true;
    noUpgrades.completed = false;
    noUpgrades.active = false;
    m_Challenges.push_back(noUpgrades);

    // Challenge 3: No Manual Observe
    Challenge noObserve;
    noObserve.id = "no_observe";
    noObserve.name = "Patience Test";
    noObserve.description = "Cannot manually observe stations";
    noObserve.modifier = ChallengeModifier::NoObserve;
    noObserve.minPrestigeLevel = 3;
    noObserve.goalQubits = 100000.0;
    noObserve.rewardMultiplier = 1.3;  // +30% permanent production
    noObserve.rewardDescription = "+30% production forever";
    noObserve.unlocked = true;
    noObserve.completed = false;
    noObserve.active = false;
    m_Challenges.push_back(noObserve);

    // Challenge 4: Expensive Upgrades
    Challenge expensive;
    expensive.id = "expensive_upgrades";
    expensive.name = "Economic Crisis";
    expensive.description = "All upgrades cost 3x more";
    expensive.modifier = ChallengeModifier::ExpensiveUpgrades;
    expensive.minPrestigeLevel = 4;
    expensive.goalQubits = 250000.0;
    expensive.rewardMultiplier = 2.0;  // +100% permanent production (2x)
    expensive.rewardDescription = "+100% production forever (2x)";
    expensive.unlocked = true;
    expensive.completed = false;
    expensive.active = false;
    m_Challenges.push_back(expensive);

    Log::Infof("Initialized challenges count: ", m_Challenges.size());
}

bool ChallengeManager::EnterChallenge(const std::string& id, GameState* gameState) {
    Challenge* challenge = GetChallenge(id);
    if (!challenge) {
        Log::Warningf("Challenge not found: ", id);
        return false;
    }

    // Check if any challenge is already active
    bool anyActive = !m_CurrentChallengeId.empty();
    if (!challenge->CanEnter(gameState->GetStatistics().totalPrestigesPerformed, anyActive)) {
        Log::Infof("Cannot enter challenge: ", challenge->name);
        return false;
    }

    // Enter challenge (this will reset the game like prestige)
    m_CurrentChallengeId = challenge->id;
    challenge->active = true;

    Log::Infof("Entered challenge: ", challenge->name);
    return true;
}

void ChallengeManager::ExitChallenge(GameState* gameState) {
    (void)gameState; // May be used in future for cleanup

    if (m_CurrentChallengeId.empty()) {
        return;
    }

    Challenge* challenge = GetChallenge(m_CurrentChallengeId);
    if (challenge) {
        challenge->active = false;
    }

    Log::Info("Exited challenge");
    m_CurrentChallengeId = "";
}

bool ChallengeManager::CompleteChallenge(GameState* gameState) {
    if (m_CurrentChallengeId.empty()) {
        return false;
    }

    Challenge* challenge = GetChallenge(m_CurrentChallengeId);
    if (!challenge) {
        return false;
    }

    // Check if goal reached
    f64 currentQubits = gameState->GetResource(QuantumResource::Qubits);
    if (currentQubits < challenge->goalQubits) {
        return false;
    }

    // Complete challenge!
    challenge->completed = true;
    challenge->active = false;
    m_CurrentChallengeId = "";

    Log::Infof("Completed challenge: ", challenge->name, " - ", challenge->rewardDescription);
    return true;
}

bool ChallengeManager::HasModifier(ChallengeModifier modifier) const {
    if (m_CurrentChallengeId.empty()) {
        return false;
    }

    const Challenge* challenge = GetCurrentChallenge();
    return challenge && challenge->modifier == modifier;
}

Challenge* ChallengeManager::GetCurrentChallenge() {
    if (m_CurrentChallengeId.empty()) {
        return nullptr;
    }

    for (auto& challenge : m_Challenges) {
        if (challenge.id == m_CurrentChallengeId) {
            return &challenge;
        }
    }
    return nullptr;
}

const Challenge* ChallengeManager::GetCurrentChallenge() const {
    if (m_CurrentChallengeId.empty()) {
        return nullptr;
    }

    for (const auto& challenge : m_Challenges) {
        if (challenge.id == m_CurrentChallengeId) {
            return &challenge;
        }
    }
    return nullptr;
}

Challenge* ChallengeManager::GetChallenge(const std::string& id) {
    for (auto& challenge : m_Challenges) {
        if (challenge.id == id) {
            return &challenge;
        }
    }
    return nullptr;
}

f64 ChallengeManager::GetTotalRewardMultiplier() const {
    f64 totalMultiplier = 1.0;
    for (const auto& challenge : m_Challenges) {
        if (challenge.completed) {
            totalMultiplier *= challenge.rewardMultiplier;
        }
    }
    return totalMultiplier;
}
