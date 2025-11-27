#pragma once

#include "Types.h"
#include <string>
#include <vector>
#include <functional>

// Forward declaration
class GameState;

// Challenge modifiers - restrictions that make the game harder
enum class ChallengeModifier {
    None,
    HalfProduction,        // 50% production rate
    NoUpgrades,            // Cannot upgrade stations
    NoObserve,             // Cannot manually observe
    NoCoherence,           // Coherence always at 0
    ExpensiveUpgrades,     // Upgrades cost 3x more
    SlowTime,              // Time runs at 50% speed
    NoBoost,               // Cannot use boost button
    NoBuyables             // Cannot purchase buyables during challenge
};

// Challenge - impose restrictions for rewards
struct Challenge {
    std::string id;
    std::string name;
    std::string description;
    ChallengeModifier modifier;

    // Requirements
    i32 minPrestigeLevel;       // Minimum prestige level to attempt

    // Goal (reach this many qubits to complete)
    f64 goalQubits;

    // Reward
    f64 rewardMultiplier;       // Permanent production multiplier after completion
    std::string rewardDescription;

    // State
    bool unlocked;
    bool completed;
    bool active;                // Currently in this challenge

    // Helper: Check if can enter
    bool CanEnter(i32 currentPrestige, bool anyActive) const {
        return unlocked && !completed && !anyActive && currentPrestige >= minPrestigeLevel;
    }
};

// Challenge manager
class ChallengeManager {
public:
    ChallengeManager();
    ~ChallengeManager();

    // Initialize challenges
    void Initialize();

    // Enter a challenge (resets game state)
    bool EnterChallenge(const std::string& id, GameState* gameState);

    // Exit current challenge (without completing)
    void ExitChallenge(GameState* gameState);

    // Complete current challenge (if goal reached)
    bool CompleteChallenge(GameState* gameState);

    // Check if a challenge modifier is active
    bool HasModifier(ChallengeModifier modifier) const;

    // Get current challenge
    Challenge* GetCurrentChallenge();
    const Challenge* GetCurrentChallenge() const;

    // Get all challenges
    const std::vector<Challenge>& GetChallenges() const { return m_Challenges; }

    // Get specific challenge by ID
    Challenge* GetChallenge(const std::string& id);

    // Get total reward multiplier from completed challenges
    f64 GetTotalRewardMultiplier() const;

private:
    std::vector<Challenge> m_Challenges;
    std::string m_CurrentChallengeId;  // Empty if no challenge active
};
