#pragma once

#include "Types.h"
#include <string>
#include <vector>

// Milestone System - Clear progression markers with big rewards

enum class MilestoneID {
    // Qubit Milestones
    FirstThousand,        // 1,000 qubits
    FirstMillion,         // 1,000,000 qubits
    FirstBillion,         // 1,000,000,000 qubits
    FirstTrillion,        // 1,000,000,000,000 qubits

    // Station Milestones
    FiveStations,         // Own 5 stations
    TenStations,          // Own 10 stations
    MaxedStation,         // Max out a station (level 100)
    AllStationsMaxed,     // Max all stations

    // Prestige Milestones
    FirstPrestige,        // Complete first prestige
    TenPrestiges,         // Complete 10 prestiges
    FiftyPrestiges,       // Complete 50 prestiges
    HundredPhotons,       // Earn 100 total photons

    // Time Milestones
    OneHourPlayed,        // Play for 1 hour
    OneDayPlayed,         // Play for 24 hours (total)
    OneWeekPlayed,        // Play for 1 week (total)

    // Achievement Milestones
    HalfAchievements,     // Unlock 50% of achievements
    AllAchievements,      // Unlock all achievements

    // Research Milestones
    FirstResearch,        // Complete first research
    TenResearch,          // Complete 10 research
    AllResearch,          // Complete all research

    // Special Milestones
    PerfectObservation,   // Get 100% observation 100 times
    QuantumMaster,        // Reach the endgame
    TrueEnding,           // Complete everything

    COUNT
};

struct Milestone {
    MilestoneID id;
    std::string name;
    std::string description;
    std::string rewardDescription;

    // Progress
    f64 progress;
    f64 target;
    bool completed;

    // Rewards
    f64 qubitReward;
    f64 photonReward;
    f64 permanentProductionBonus;  // Permanent +X% production
    bool unlocksFeature;
    std::string featureName;

    Milestone();
};

class MilestoneSystem {
public:
    MilestoneSystem();

    void Initialize();
    void CheckMilestones(class GameState* state);
    void CompleteMilestone(MilestoneID id, class GameState* state);

    bool IsCompleted(MilestoneID id) const;
    Milestone* GetMilestone(MilestoneID id);

    std::vector<Milestone*> GetActiveMilestones();      // In progress
    std::vector<Milestone*> GetCompletedMilestones();   // Completed
    std::vector<Milestone*> GetRecentCompletions();     // Recently completed (for UI)

    f64 GetTotalProductionBonus() const;

    // For UI notifications
    void ClearRecentCompletions();

private:
    std::vector<Milestone> m_Milestones;
    std::vector<MilestoneID> m_RecentCompletions;

    void AddMilestone(MilestoneID id, const std::string& name, const std::string& desc,
                      f64 target, f64 qReward, f64 pReward, f64 prodBonus = 0.0);
};
