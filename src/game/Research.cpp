#include "Research.h"
#include "Logger.h"
#include <algorithm>
#include "GameState.h"

// ResearchNode implementation
ResearchNode::ResearchNode()
    : id(ResearchID::COUNT)
    , category(ResearchCategory::Production)
    , qubitCost(0.0)
    , coherenceCost(0.0)
    , entanglementCost(0.0)
    , photonCost(0)
    , exoticMaterialsCost(0)  // Phase 3.3
    , minPrestigeLevel(0)
    , productionMultiplier(1.0)
    , observationBonus(0.0)
    , coherenceBonus(0.0)
    , unlocksFeature(false)
    , unlocked(false)
    , researched(false)
    , autoResearch(false)
{
}

ResearchNode::ResearchNode(
    ResearchID _id,
    const std::string& _name,
    const std::string& _desc,
    ResearchCategory _category,
    f64 _qubitCost,
    f64 _coherenceCost,
    f64 _entanglementCost,
    i32 _photonCost,
    i32 _exoticMaterialsCost,  // Phase 3.3
    const std::vector<ResearchID>& _prereqs,
    i32 _minPrestige,
    f64 _prodMult,
    f64 _obsMult,
    f64 _cohMult,
    bool _unlocksFeature
) : id(_id)
  , name(_name)
  , description(_desc)
  , category(_category)
  , qubitCost(_qubitCost)
  , coherenceCost(_coherenceCost)
  , entanglementCost(_entanglementCost)
  , photonCost(_photonCost)
  , exoticMaterialsCost(_exoticMaterialsCost)  // Phase 3.3
  , prerequisites(_prereqs)
  , minPrestigeLevel(_minPrestige)
  , productionMultiplier(_prodMult)
  , observationBonus(_obsMult)
  , coherenceBonus(_cohMult)
  , unlocksFeature(_unlocksFeature)
  , unlocked(false)
  , researched(false)
  , autoResearch(false)
{
}

// ResearchTree implementation
ResearchTree::ResearchTree()
{
}

void ResearchTree::Initialize()
{
    m_Nodes.clear();

    // Tier 1 - Basic Research (Always available)
    m_Nodes.push_back(ResearchNode(
        ResearchID::QuantumMechanics101,
        "Quantum Mechanics 101",
        "Basic understanding of quantum physics. +50% production.",
        ResearchCategory::Production,
        1000.0, 500.0, 0.0, 0, 0,  // Phase 3.3: 0 exotic materials for Tier 1
        {}, 0,
        1.5, 0.0, 0.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::WaveFunctionTheory,
        "Wave Function Theory",
        "Master wave functions. +25% observation bonus.",
        ResearchCategory::Observation,
        2000.0, 1000.0, 0.0, 0, 0,  // Phase 3.3: 0 exotic materials for Tier 1
        {}, 0,
        1.0, 0.25, 0.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::CoherenceStabilization,
        "Coherence Stabilization",
        "Stabilize quantum states. +100% coherence generation.",
        ResearchCategory::Coherence,
        1500.0, 750.0, 0.0, 0, 0,  // Phase 3.3: 0 exotic materials for Tier 1
        {}, 0,
        1.0, 0.0, 1.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::EntanglementBasics,
        "Entanglement Basics",
        "Learn quantum entanglement. +50% entanglement generation.",
        ResearchCategory::Entanglement,
        2500.0, 1250.0, 500.0, 0, 0,  // Phase 3.3: 0 exotic materials for Tier 1
        {}, 0,
        1.0, 0.0, 0.0, false
    ));

    // Tier 2 - Intermediate (Requires Tier 1)
    m_Nodes.push_back(ResearchNode(
        ResearchID::QuantumComputing,
        "Quantum Computing",
        "Build quantum computers. +100% production, unlocks automation.",
        ResearchCategory::Production,
        50000.0, 25000.0, 10000.0, 1, 0,  // Phase 3.3: 0 exotic materials for Tier 2
        {ResearchID::QuantumMechanics101}, 1,
        2.0, 0.0, 0.0, true
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::SuperpositionMastery,
        "Superposition Mastery",
        "Perfect superposition control. +50% observation bonus.",
        ResearchCategory::Observation,
        60000.0, 30000.0, 12000.0, 1, 0,  // Phase 3.3: 0 exotic materials for Tier 2
        {ResearchID::WaveFunctionTheory}, 1,
        1.0, 0.5, 0.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::ParticlePhysics,
        "Particle Physics",
        "Study fundamental particles. +150% production.",
        ResearchCategory::Production,
        75000.0, 37500.0, 15000.0, 1, 0,  // Phase 3.3: 0 exotic materials for Tier 2
        {ResearchID::QuantumMechanics101}, 1,
        2.5, 0.0, 0.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::QuantumTunneling,
        "Quantum Tunneling",
        "Master quantum tunneling. +75% observation speed.",
        ResearchCategory::Observation,
        80000.0, 40000.0, 16000.0, 1, 0,  // Phase 3.3: 0 exotic materials for Tier 2
        {ResearchID::WaveFunctionTheory, ResearchID::QuantumMechanics101}, 1,
        1.0, 0.75, 0.0, false
    ));

    // Tier 3 - Advanced (Requires Tier 2 + More Prestiges)
    // Phase 3.3: Tier 3 research requires Exotic Materials!
    m_Nodes.push_back(ResearchNode(
        ResearchID::QuantumFieldTheory,
        "Quantum Field Theory",
        "Understand quantum fields. +200% production. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Production,
        500000.0, 250000.0, 100000.0, 5, 2,  // Phase 3.3: Requires 2 Exotic Materials
        {ResearchID::ParticlePhysics, ResearchID::QuantumComputing}, 3,
        3.0, 0.0, 0.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::StringTheory,
        "String Theory",
        "Vibrating strings create reality. +100% all resources. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Special,
        750000.0, 375000.0, 150000.0, 7, 3,  // Phase 3.3: Requires 3 Exotic Materials
        {ResearchID::QuantumFieldTheory}, 5,
        2.0, 1.0, 1.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::QuantumGravity,
        "Quantum Gravity",
        "Unify quantum and gravity. +250% production. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Production,
        1000000.0, 500000.0, 200000.0, 10, 4,  // Phase 3.3: Requires 4 Exotic Materials
        {ResearchID::QuantumFieldTheory}, 7,
        3.5, 0.0, 0.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::TimeDialation,
        "Time Dilation",
        "Manipulate spacetime. +100% offline progress rate. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Special,
        900000.0, 450000.0, 180000.0, 8, 3,  // Phase 3.3: Requires 3 Exotic Materials
        {ResearchID::StringTheory}, 5,
        1.0, 0.0, 0.0, true
    ));

    // Tier 4 - Exotic (Requires Tier 3 + Many Prestiges)
    // Phase 3.3: Tier 4 research requires even more Exotic Materials!
    m_Nodes.push_back(ResearchNode(
        ResearchID::MultiverseTheory,
        "Multiverse Theory",
        "Access parallel universes. +500% production. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Production,
        5000000.0, 2500000.0, 1000000.0, 20, 5,  // Phase 3.3: Requires 5 Exotic Materials
        {ResearchID::StringTheory, ResearchID::QuantumGravity}, 10,
        6.0, 0.0, 0.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::QuantumImmortality,
        "Quantum Immortality",
        "Consciousness spans universes. +200% all bonuses. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Special,
        10000000.0, 5000000.0, 2000000.0, 50, 10,  // Phase 3.3: Requires 10 Exotic Materials
        {ResearchID::MultiverseTheory}, 20,
        3.0, 2.0, 2.0, true
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::ZeroPointEnergy,
        "Zero Point Energy",
        "Tap vacuum energy. +1000% production. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Production,
        25000000.0, 12500000.0, 5000000.0, 100, 15,  // Phase 3.3: Requires 15 Exotic Materials
        {ResearchID::QuantumGravity}, 25,
        11.0, 0.0, 0.0, false
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::QuantumSingularity,
        "Quantum Singularity",
        "Create quantum singularity. +10000% ALL bonuses. THE TRUE ENDING. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Special,
        100000000.0, 50000000.0, 20000000.0, 500, 20,  // Phase 3.3: Requires 20 Exotic Materials
        {ResearchID::ZeroPointEnergy, ResearchID::QuantumImmortality, ResearchID::MultiverseTheory}, 50,
        101.0, 100.0, 100.0, true
    ));

    // Automation Research
    m_Nodes.push_back(ResearchNode(
        ResearchID::AutoObserver,
        "Auto Observer",
        "Automatically observe when superposition is full.",
        ResearchCategory::Automation,
        100000.0, 50000.0, 20000.0, 3, 0,  // Phase 3.3: 0 exotic materials (not Tier 3)
        {ResearchID::QuantumComputing}, 2,
        1.0, 0.0, 0.0, true
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::AutoUpgrade,
        "Auto Upgrade",
        "Automatically upgrade stations when possible.",
        ResearchCategory::Automation,
        250000.0, 125000.0, 50000.0, 5, 0,  // Phase 3.3: 0 exotic materials (not Tier 3)
        {ResearchID::AutoObserver}, 3,
        1.0, 0.0, 0.0, true
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::AutoPrestige,
        "Auto Prestige",
        "Automatically prestige when photon threshold is reached.",
        ResearchCategory::Automation,
        350000.0, 175000.0, 75000.0, 7, 0,  // Phase 3.3: 0 exotic materials (not Tier 3)
        {ResearchID::AutoUpgrade}, 4,
        1.0, 0.0, 0.0, true
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::SmartInvestor,
        "Smart Investor",
        "AI optimizes resource investment automatically.",
        ResearchCategory::Automation,
        500000.0, 250000.0, 100000.0, 10, 0,  // Phase 3.3: 0 exotic materials (not Tier 3)
        {ResearchID::AutoPrestige}, 5,
        1.0, 0.0, 0.0, true
    ));

    // Special Bonuses
    m_Nodes.push_back(ResearchNode(
        ResearchID::QuantumLuck,
        "Quantum Luck",
        "Bend probability in your favor. +50% event frequency, +25% event bonuses.",
        ResearchCategory::Special,
        150000.0, 75000.0, 30000.0, 3, 0,  // Phase 3.3: 0 exotic materials (Tier 2 prereq)
        {ResearchID::SuperpositionMastery}, 2,
        1.0, 0.0, 0.0, true
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::PhotonMultiplier,
        "Photon Multiplier",
        "Generate more photons on prestige. +50% photon gain.",
        ResearchCategory::Special,
        200000.0, 100000.0, 40000.0, 5, 0,  // Phase 3.3: 0 exotic materials (Tier 2 prereq)
        {ResearchID::QuantumComputing}, 2,
        1.0, 0.0, 0.0, true
    ));

    m_Nodes.push_back(ResearchNode(
        ResearchID::OfflineBoost,
        "Offline Boost",
        "Better offline progress. Increases offline cap to 8 hours, 75% efficiency. [REQUIRES EXOTIC MATERIALS]",
        ResearchCategory::Special,
        300000.0, 150000.0, 60000.0, 7, 1,  // Phase 3.3: 1 exotic material (requires Tier 3 TimeDialation)
        {ResearchID::TimeDialation}, 4,
        1.0, 0.0, 0.0, true
    ));

    // Mark all as locked initially
    for (auto& node : m_Nodes) {
        node.unlocked = false;
        node.researched = false;
    }

    // Unlock Tier 1 research (no prerequisites)
    UnlockAvailableResearch(0, 0);

    Log::Info("Research Tree initialized with " + std::to_string(m_Nodes.size()) + " nodes");
}

ResearchNode* ResearchTree::GetNode(ResearchID id)
{
    for (auto& node : m_Nodes) {
        if (node.id == id) {
            return &node;
        }
    }
    return nullptr;
}

const ResearchNode* ResearchTree::GetNode(ResearchID id) const
{
    for (const auto& node : m_Nodes) {
        if (node.id == id) {
            return &node;
        }
    }
    return nullptr;
}

bool ResearchTree::CanResearch(ResearchID id, i32 prestigeLevel) const
{
    const ResearchNode* node = GetNode(id);
    if (!node) return false;

    // Must be unlocked
    if (!node->unlocked) return false;

    // Must not be already researched
    if (node->researched) return false;

    // Check prestige requirement
    if (prestigeLevel < node->minPrestigeLevel) return false;

    // Check prerequisites
    for (ResearchID prereqID : node->prerequisites) {
        const ResearchNode* prereq = GetNode(prereqID);
        if (!prereq || !prereq->researched) {
            return false;
        }
    }

    return true;
}

bool ResearchTree::Research(ResearchID id)
{
    ResearchNode* node = GetNode(id);
    if (!node) return false;

    if (!node->unlocked || node->researched) return false;

    // Mark as researched
    node->researched = true;

    Log::Info("Research completed: " + node->name);

    return true;
}

void ResearchTree::UnlockAvailableResearch(i32 prestigeLevel, i32 researchedCount)
{
    for (auto& node : m_Nodes) {
        if (node.unlocked) continue;

        // Check if prestige level is high enough
        if (prestigeLevel < node.minPrestigeLevel) continue;

        // Check if all prerequisites are met
        bool allPrereqsMet = true;
        for (ResearchID prereqID : node.prerequisites) {
            const ResearchNode* prereq = GetNode(prereqID);
            if (!prereq || !prereq->researched) {
                allPrereqsMet = false;
                break;
            }
        }

        // If no prerequisites or all met, unlock
        if (node.prerequisites.empty() || allPrereqsMet) {
            node.unlocked = true;
        }
    }
}

f64 ResearchTree::GetTotalProductionMultiplier() const
{
    f64 multiplier = 1.0;
    for (const auto& node : m_Nodes) {
        if (node.researched) {
            multiplier *= node.productionMultiplier;
        }
    }
    return multiplier;
}

f64 ResearchTree::GetTotalObservationBonus() const
{
    f64 bonus = 0.0;
    for (const auto& node : m_Nodes) {
        if (node.researched) {
            bonus += node.observationBonus;
        }
    }
    return bonus;
}

f64 ResearchTree::GetTotalCoherenceBonus() const
{
    f64 bonus = 0.0;
    for (const auto& node : m_Nodes) {
        if (node.researched) {
            bonus += node.coherenceBonus;
        }
    }
    return bonus;
}

bool ResearchTree::IsResearched(ResearchID id) const
{
    const ResearchNode* node = GetNode(id);
    return node && node->researched;
}

i32 ResearchTree::GetResearchedCount() const
{
    i32 count = 0;
    for (const auto& node : m_Nodes) {
        if (node.researched) {
            count++;
        }
    }
    return count;
}

i32 ResearchTree::GetUnlockedCount() const
{
    i32 count = 0;
    for (const auto& node : m_Nodes) {
        if (node.unlocked) {
            count++;
        }
    }
    return count;
}

std::vector<const ResearchNode*> ResearchTree::GetAvailableResearch(i32 prestigeLevel) const
{
    std::vector<const ResearchNode*> available;
    for (const auto& node : m_Nodes) {
        if (CanResearch(node.id, prestigeLevel)) {
            available.push_back(&node);
        }
    }
    return available;
}

std::vector<const ResearchNode*> ResearchTree::GetResearchedNodes() const
{
    std::vector<const ResearchNode*> researched;
    for (const auto& node : m_Nodes) {
        if (node.researched) {
            researched.push_back(&node);
        }
    }
    return researched;
}

std::vector<const ResearchNode*> ResearchTree::GetNodesByCategory(ResearchCategory category) const
{
    std::vector<const ResearchNode*> nodes;
    for (const auto& node : m_Nodes) {
        if (node.category == category && node.unlocked) {
            nodes.push_back(&node);
        }
    }
    return nodes;
}

f64 ResearchTree::GetResourceBonus(QuantumResource type) const
{
    f64 cumulativeMultiplier = 1.0;
    f64 additiveBonus = 0.0;

    for (const auto& node : m_Nodes) {
        if (node.researched) {

            // Qubits and Entanglement: Apply the cumulative production multiplier (e.g., 1.5x, 2.0x)
            if (type == QuantumResource::Qubits || type == QuantumResource::Entanglement) {
                cumulativeMultiplier *= node.productionMultiplier;
            }

            // Coherence: Apply the additive bonus (e.g., 1.0 for +100% rate)
            else if (type == QuantumResource::Coherence) {
                additiveBonus += node.coherenceBonus;
            }
        }
    }

    if (type == QuantumResource::Coherence) {
        // Return 1.0 (base) plus the total additive bonus
        return 1.0 + additiveBonus;
    }

    // Return the cumulative multiplier for Qubits and Entanglement
    return cumulativeMultiplier;
}