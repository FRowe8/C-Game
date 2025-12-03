#include "Milestones.h"
#include "GameState.h"
#include "Logger.h"
#include "Research.h"

// Milestone implementation
Milestone::Milestone()
    : id(MilestoneID::FirstThousand)
    , progress(0.0)
    , target(1.0)
    , completed(false)
    , claimed(false)
    , qubitReward(0.0)
    , photonReward(0.0)
    , permanentProductionBonus(0.0)
    , unlocksFeature(false)
{
}

// MilestoneSystem implementation
MilestoneSystem::MilestoneSystem()
{
}

void MilestoneSystem::Initialize()
{
    m_Milestones.clear();
    m_RecentCompletions.clear();

    // Qubit Milestones - Progressive wealth goals
    AddMilestone(MilestoneID::FirstThousand, "First Thousand",
        "Accumulate 1,000 qubits",
        1000.0,
        500.0,      // 500 qubits reward
        1.0,        // 1 photon
        5.0);       // +5% permanent production

    AddMilestone(MilestoneID::FirstMillion, "Quantum Millionaire",
        "Accumulate 1,000,000 qubits",
        1000000.0,
        50000.0,    // 50K qubits
        5.0,        // 5 photons
        10.0);      // +10% permanent production

    AddMilestone(MilestoneID::FirstBillion, "Billion Qubit Club",
        "Accumulate 1,000,000,000 qubits",
        1000000000.0,
        5000000.0,  // 5M qubits
        25.0,       // 25 photons
        25.0);      // +25% permanent production

    AddMilestone(MilestoneID::FirstTrillion, "Quantum Trillionaire",
        "Accumulate 1,000,000,000,000 qubits - you're unstoppable!",
        1000000000000.0,
        100000000.0, // 100M qubits
        100.0,       // 100 photons
        50.0);       // +50% permanent production

    // Station Milestones - Infrastructure goals
    AddMilestone(MilestoneID::FiveStations, "Research Network",
        "Own 5 research stations",
        5.0,
        1000.0,     // 1K qubits
        2.0,        // 2 photons
        5.0);       // +5% production

    AddMilestone(MilestoneID::TenStations, "Quantum Empire",
        "Own 10 research stations",
        10.0,
        10000.0,    // 10K qubits
        10.0,       // 10 photons
        15.0);      // +15% production

    AddMilestone(MilestoneID::MaxedStation, "Perfection Achieved",
        "Upgrade a station to level 100",
        100.0,
        50000.0,    // 50K qubits
        20.0,       // 20 photons
        20.0);      // +20% production

    AddMilestone(MilestoneID::AllStationsMaxed, "Ultimate Infrastructure",
        "Max out ALL stations to level 100",
        100.0,      // Will check all stations
        1000000.0,  // 1M qubits
        100.0,      // 100 photons
        100.0);     // +100% production!

    // Prestige Milestones - Dedication goals
    AddMilestone(MilestoneID::FirstPrestige, "Quantum Leap",
        "Perform your first prestige",
        1.0,
        0.0,        // No qubit reward (already got photons)
        5.0,        // 5 bonus photons
        10.0);      // +10% production

    AddMilestone(MilestoneID::TenPrestiges, "Dimension Hopper",
        "Complete 10 prestiges",
        10.0,
        0.0,
        25.0,       // 25 photons
        20.0);      // +20% production

    AddMilestone(MilestoneID::FiftyPrestiges, "Multiverse Veteran",
        "Complete 50 prestiges",
        50.0,
        0.0,
        100.0,      // 100 photons
        50.0);      // +50% production

    AddMilestone(MilestoneID::HundredPhotons, "Photon Hoarder",
        "Accumulate 100 total photons",
        100.0,
        0.0,
        50.0,       // 50 bonus photons
        30.0);      // +30% production

    // Time Milestones - Commitment goals
    AddMilestone(MilestoneID::OneHourPlayed, "Time Traveler",
        "Play for 1 hour (total playtime)",
        3600.0,     // 1 hour in seconds
        5000.0,     // 5K qubits
        5.0,        // 5 photons
        5.0);       // +5% production

    AddMilestone(MilestoneID::OneDayPlayed, "Daily Researcher",
        "Play for 24 hours (total playtime)",
        86400.0,    // 24 hours
        100000.0,   // 100K qubits
        50.0,       // 50 photons
        25.0);      // +25% production

    AddMilestone(MilestoneID::OneWeekPlayed, "Quantum Devotee",
        "Play for 1 week (total playtime)",
        604800.0,   // 1 week
        10000000.0, // 10M qubits
        200.0,      // 200 photons
        75.0);      // +75% production

    // Achievement Milestones
    AddMilestone(MilestoneID::HalfAchievements, "Achievement Hunter",
        "Unlock 50% of all achievements",
        0.5,        // 50% (will multiply by total count)
        25000.0,    // 25K qubits
        15.0,       // 15 photons
        15.0);      // +15% production

    AddMilestone(MilestoneID::AllAchievements, "Completionist",
        "Unlock ALL achievements",
        1.0,        // 100%
        500000.0,   // 500K qubits
        100.0,      // 100 photons
        50.0);      // +50% production

    // Research Milestones
    AddMilestone(MilestoneID::FirstResearch, "Quantum Scholar",
        "Complete your first research",
        1.0,
        1000.0,     // 1K qubits
        2.0,        // 2 photons
        5.0);       // +5% production

    AddMilestone(MilestoneID::TenResearch, "Research Master",
        "Complete 10 research projects",
        10.0,
        50000.0,    // 50K qubits
        25.0,       // 25 photons
        20.0);      // +20% production

    AddMilestone(MilestoneID::AllResearch, "Omniscient",
        "Complete ALL research in the tech tree",
        23.0,       // Total research count
        10000000.0, // 10M qubits
        250.0,      // 250 photons
        100.0);     // +100% production!

    // Special Milestones
    AddMilestone(MilestoneID::PerfectObservation, "Perfect Observer",
        "Get 100% observation success 100 times",
        100.0,
        100000.0,   // 100K qubits
        50.0,       // 50 photons
        25.0);      // +25% production

    AddMilestone(MilestoneID::QuantumMaster, "Quantum Master",
        "Reach prestige level 50 with 500+ photons",
        1.0,        // Binary check
        50000000.0, // 50M qubits
        500.0,      // 500 photons
        200.0);     // +200% production

    AddMilestone(MilestoneID::TrueEnding, "The True Ending",
        "Unlock Quantum Singularity research and max everything",
        1.0,        // Binary check
        1000000000.0, // 1B qubits
        1000.0,     // 1000 photons
        500.0);     // +500% production!!!

    Log::Info("Milestone system initialized with " + std::to_string(m_Milestones.size()) + " milestones");
}

void MilestoneSystem::AddMilestone(MilestoneID id, const std::string& name, const std::string& desc,
                                   f64 target, f64 qReward, f64 pReward, f64 prodBonus)
{
    Milestone m;
    m.id = id;
    m.name = name;
    m.description = desc;
    m.target = target;
    m.progress = 0.0;
    m.completed = false;
    m.qubitReward = qReward;
    m.photonReward = pReward;
    m.permanentProductionBonus = prodBonus;

    // Generate reward description
    std::string rewards = "Rewards: ";
    bool hasReward = false;

    if (qReward > 0) {
        rewards += std::to_string(static_cast<i32>(qReward)) + " qubits";
        hasReward = true;
    }

    if (pReward > 0) {
        if (hasReward) rewards += ", ";
        rewards += std::to_string(static_cast<i32>(pReward)) + " photons";
        hasReward = true;
    }

    if (prodBonus > 0) {
        if (hasReward) rewards += ", ";
        rewards += "+" + std::to_string(static_cast<i32>(prodBonus)) + "% permanent production";
        hasReward = true;
    }

    m.rewardDescription = rewards;
    m_Milestones.push_back(m);
}

void MilestoneSystem::CheckMilestones(GameState* state)
{
    if (!state) return;

    for (auto& milestone : m_Milestones) {
        if (milestone.completed) continue;

        // Update progress based on milestone type
        switch (milestone.id) {
            // Qubit milestones
            case MilestoneID::FirstThousand:
            case MilestoneID::FirstMillion:
            case MilestoneID::FirstBillion:
            case MilestoneID::FirstTrillion:
                milestone.progress = state->GetStatistics().highestQubits;
                break;

            // Station milestones
            case MilestoneID::FiveStations:
            case MilestoneID::TenStations: {
                i32 stationCount = 0;
                // Count unlocked stations (this would need GameState API access)
                milestone.progress = static_cast<f64>(stationCount);
                break;
            }

            case MilestoneID::MaxedStation: {
                // Find highest station level
                f64 maxLevel = 0;
                // Would need to check all station levels
                milestone.progress = maxLevel;
                break;
            }

            case MilestoneID::AllStationsMaxed: {
                // Check if all stations are level 100
                // This is a binary check
                break;
            }

            // Prestige milestones
            case MilestoneID::FirstPrestige:
            case MilestoneID::TenPrestiges:
            case MilestoneID::FiftyPrestiges:
                milestone.progress = static_cast<f64>(state->GetStatistics().totalPrestigesPerformed);
                break;

            case MilestoneID::HundredPhotons:
                // Track total photons earned (would need new stat)
                break;

            // Time milestones
            case MilestoneID::OneHourPlayed:
            case MilestoneID::OneDayPlayed:
            case MilestoneID::OneWeekPlayed:
                // Use total time played from statistics
                break;

            // Achievement milestones
            case MilestoneID::HalfAchievements:
            case MilestoneID::AllAchievements: {
                // Count unlocked achievements
                i32 unlockedCount = 0;
                i32 totalCount = static_cast<i32>(AchievementID::TotalCount);
                f64 percentage = static_cast<f64>(unlockedCount) / static_cast<f64>(totalCount);
                milestone.progress = percentage;
                break;
            }

            // Research milestones
            case MilestoneID::FirstResearch:
            case MilestoneID::TenResearch:
            case MilestoneID::AllResearch:
                milestone.progress = static_cast<f64>(state->GetResearchTree().GetResearchedCount());
                break;

            // Special milestones
            case MilestoneID::PerfectObservation:
                // Would need to track perfect observations
                break;

            case MilestoneID::QuantumMaster:
                // Check prestige level >= 50 AND photons >= 500
                break;

            case MilestoneID::TrueEnding:
                // Check if Quantum Singularity is researched AND everything maxed
                break;

            default:
                break;
        }

        // Check if milestone is completed
        if (milestone.progress >= milestone.target && !milestone.completed) {
            CompleteMilestone(milestone.id, state);
        }
    }
}

void MilestoneSystem::CompleteMilestone(MilestoneID id, GameState* state)
{
    Milestone* milestone = GetMilestone(id);
    if (!milestone || milestone->completed) return;

    milestone->completed = true;
    m_RecentCompletions.push_back(id);

    // Grant rewards
    if (milestone->qubitReward > 0) {
        state->AddResource(QuantumResource::Qubits, milestone->qubitReward);
    }

    if (milestone->photonReward > 0) {
        // Add to photon count (would need GameState API)
        // state->AddPhotons(milestone->photonReward);
    }

    Log::Info("Milestone completed: " + milestone->name);

    // Note: Permanent production bonus is applied via GetTotalProductionBonus()
}

bool MilestoneSystem::IsCompleted(MilestoneID id) const
{
    for (const auto& milestone : m_Milestones) {
        if (milestone.id == id) {
            return milestone.completed;
        }
    }
    return false;
}


// ADD THIS: Definition of the non-const getter
std::vector<Milestone>& MilestoneSystem::GetMilestones() {
    return m_Milestones;
}

// --- Add this function to the MilestoneSystem implementation ---
const std::vector<Milestone>& MilestoneSystem::GetMilestones() const
{
    return m_Milestones;
}

Milestone* MilestoneSystem::GetMilestone(MilestoneID id)
{
    for (auto& milestone : m_Milestones) {
        if (milestone.id == id) {
            return &milestone;
        }
    }
    return nullptr;
}

std::vector<Milestone*> MilestoneSystem::GetActiveMilestones()
{
    std::vector<Milestone*> active;
    for (auto& milestone : m_Milestones) {
        if (!milestone.completed) {
            active.push_back(&milestone);
        }
    }
    return active;
}

std::vector<Milestone*> MilestoneSystem::GetCompletedMilestones()
{
    std::vector<Milestone*> completed;
    for (auto& milestone : m_Milestones) {
        if (milestone.completed) {
            completed.push_back(&milestone);
        }
    }
    return completed;
}

std::vector<Milestone*> MilestoneSystem::GetRecentCompletions()
{
    std::vector<Milestone*> recent;
    for (MilestoneID id : m_RecentCompletions) {
        Milestone* m = GetMilestone(id);
        if (m) {
            recent.push_back(m);
        }
    }
    return recent;
}

f64 MilestoneSystem::GetTotalProductionBonus() const
{
    f64 bonus = 0.0;
    for (const auto& milestone : m_Milestones) {
        if (milestone.completed) {
            bonus += milestone.permanentProductionBonus;
        }
    }
    return bonus / 100.0; // Convert percentage to multiplier (e.g., 50% = 0.5)
}

void MilestoneSystem::ClearRecentCompletions()
{
    m_RecentCompletions.clear();
}
