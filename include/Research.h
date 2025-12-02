#pragma once

#include "GameState.h"
#include "Types.h"
#include <string>
#include <vector>

// Research Tree System - Unlock powerful quantum discoveries

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
    i32 exoticMaterialsCost;  // Phase 3.3: Required for Tier 3+ research

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
    bool autoResearch; // Automatically purchase when affordable

    ResearchNode();
    ResearchNode(ResearchID _id, const std::string& _name, const std::string& _desc,
                 ResearchCategory _category, f64 _qubitCost, f64 _coherenceCost,
                 f64 _entanglementCost, i32 _photonCost, i32 _exoticMaterialsCost,
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

    f64 GetResourceBonus(QuantumResource type) const;

private:
    std::vector<ResearchNode> m_Nodes;
};
