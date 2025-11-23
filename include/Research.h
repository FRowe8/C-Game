#pragma once

#include "Types.h"
#include <string>
#include <vector>

// Research Tree System - Unlock powerful quantum discoveries

enum class ResearchID {
    // Tier 1 - Basic Research (Early game)
    QuantumMechanics101,      // Unlock basic concepts
    WaveFunctionTheory,       // +10% observation probability
    CoherenceStabilization,   // Slower coherence decay
    EntanglementBasics,       // Unlock entanglement system

    // Tier 2 - Intermediate (Mid game)
    QuantumComputing,         // +25% production
    SuperpositionMastery,     // Superposition accumulates faster
    ParticlePhysics,          // Unlock particle collection
    QuantumTunneling,         // Resources can "tunnel" between types

    // Tier 3 - Advanced (Late game)
    QuantumFieldTheory,       // Unlock quantum field effects
    StringTheory,             // +50% all production
    QuantumGravity,           // Gravity affects resource generation
    TimeDialation,            // Slow down/speed up time

    // Tier 4 - Exotic (End game)
    MultiverseTheory,         // Run parallel universes
    QuantumImmortality,       // Never lose progress
    ZeroPointEnergy,          // Infinite energy source
    QuantumSingularity,       // The ultimate discovery

    // Automation
    AutoObserver,             // Auto-observe every 10 seconds
    AutoUpgrade,              // Auto-buy cheapest upgrade
    SmartInvestor,            // AI decides best purchases

    // Special
    QuantumLuck,              // Better event chances
    PhotonMultiplier,         // More photons on prestige
    OfflineBoost,             // Better offline production

    COUNT
};

enum class ResearchCategory {
    Production,      // Increase production
    Observation,     // Observation bonuses
    Coherence,       // Coherence bonuses
    Entanglement,    // Entanglement bonuses
    Automation,      // Quality of life
    Special          // Unique effects
};

struct ResearchNode {
    ResearchID id;
    std::string name;
    std::string description;
    ResearchCategory category;

    // Costs
    f64 qubitCost;
    f64 coherenceCost;
    f64 entanglementCost;
    i32 photonCost;

    // Requirements
    std::vector<ResearchID> prerequisites;
    i32 minPrestigeLevel;

    // Effects
    f64 productionMultiplier;
    f64 observationBonus;
    f64 coherenceBonus;
    bool unlocksFeature;

    // State
    bool unlocked;
    bool researched;

    ResearchNode();
    ResearchNode(ResearchID _id, const std::string& _name, const std::string& _desc,
                 ResearchCategory _category, f64 _qubitCost, f64 _coherenceCost,
                 f64 _entanglementCost, i32 _photonCost,
                 const std::vector<ResearchID>& _prereqs, i32 _minPrestige,
                 f64 _prodMult, f64 _obsMult, f64 _cohMult, bool _unlocksFeature);
};

class ResearchTree {
public:
    ResearchTree();

    void Initialize();
    bool CanResearch(ResearchID id, i32 prestigeLevel) const;
    bool Research(ResearchID id);
    bool IsResearched(ResearchID id) const;

    ResearchNode* GetNode(ResearchID id);
    const ResearchNode* GetNode(ResearchID id) const;

    // Unlock research based on prerequisites and prestige
    void UnlockAvailableResearch(i32 prestigeLevel, i32 researchedCount);

    // Get research lists
    std::vector<const ResearchNode*> GetAvailableResearch(i32 prestigeLevel) const;
    std::vector<const ResearchNode*> GetResearchedNodes() const;
    std::vector<const ResearchNode*> GetNodesByCategory(ResearchCategory category) const;

    // Get counts
    i32 GetResearchedCount() const;
    i32 GetUnlockedCount() const;

    // Get total bonuses from all research
    f64 GetTotalProductionMultiplier() const;
    f64 GetTotalObservationBonus() const;
    f64 GetTotalCoherenceBonus() const;

private:
    std::vector<ResearchNode> m_Nodes;
};
