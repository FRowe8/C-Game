#pragma once

#include "Types.h"
#include <string>
#include <vector>

struct DiscoveryTier {
    i32 requiredDiscoveries;
    f64 productionBonus;
    f64 observationBonus;
    std::string name;
};

// Particle Collection System - Discover exotic quantum particles

enum class ParticleType {
    // Common Particles (Easy to find)
    Electron,
    Proton,
    Neutron,
    Photon,

    // Uncommon Particles
    Muon,
    Tau,
    Neutrino,
    Positron,

    // Rare Particles
    QuarkUp,
    QuarkDown,
    QuarkCharm,
    QuarkStrange,
    QuarkTop,
    QuarkBottom,

    // Exotic Particles
    Gluon,
    WBoson,
    ZBoson,
    HiggsBoson,

    // Legendary Particles
    Graviton,           // Theoretical
    Axion,              // Dark matter candidate
    Tachyon,            // Faster than light
    Monopole,           // Magnetic monopole

    // Mythical Particles
    QuantumSingularity, // The ultimate particle
    GodParticle,        // Higgs nickname
    DarkMatter,         // Unknown composition
    DarkEnergy,         // Mysterious force

    COUNT
};

enum class ParticleRarity {
    Common,
    Uncommon,
    Rare,
    Exotic,
    Legendary,
    Mythical
};

struct Particle {
    ParticleType type;
    std::string name;
    std::string description;
    ParticleRarity rarity;

    // Collection status
    bool discovered;
    i32 count;              // How many collected
    f64 dropChance;         // Base drop chance

    // Effects when equipped
    f64 productionBonus;
    f64 observationBonus;
    f64 photonBonus;
    f64 eventChanceBonus;

    // Visual
    Color particleColor;

    Particle();
};

class ParticleCollection {
public:
    ParticleCollection();

    void Initialize();

    // Discovery mechanics
    void CheckForDiscoveries(f64 deltaTime);
    void DiscoverParticle(ParticleType type);
    bool TryFindParticle();  // Random chance based on current game state

    // Collection management
    bool IsDiscovered(ParticleType type) const;
    i32 GetCount(ParticleType type) const;
    Particle* GetParticle(ParticleType type);

    // Equipped particles (active bonuses)
    void EquipParticle(ParticleType type);
    void UnequipParticle(ParticleType type);
    bool IsEquipped(ParticleType type) const;

    // Get total bonuses from equipped particles
    f64 GetTotalProductionBonus() const;
    f64 GetTotalObservationBonus() const;
    f64 GetTotalPhotonBonus() const;
    f64 GetTotalEventChanceBonus() const;

    // Collection stats
    i32 GetDiscoveredCount() const;
    i32 GetTotalParticles() const;
    f64 GetCompletionPercentage() const;

    std::vector<Particle*> GetDiscoveredParticles();
    std::vector<Particle*> GetEquippedParticles();
    std::vector<ParticleType> GetRecentDiscoveries();  // For UI notifications

    void ClearRecentDiscoveries();

    // Discovery tiers
    f64 GetDiscoveryProductionBonus() const;
    f64 GetDiscoveryObservationBonus() const;
    const std::vector<DiscoveryTier>& GetDiscoveryTiers() const { return m_DiscoveryTiers; }
    i32 GetMaxEquipmentSlots() const { return m_MaxEquipmentSlots; }

private:
    std::vector<Particle> m_Particles;
    std::vector<ParticleType> m_EquippedParticles;
    std::vector<ParticleType> m_RecentDiscoveries;
    std::vector<DiscoveryTier> m_DiscoveryTiers;
    f64 m_TimeSinceLastCheck;
    i32 m_MaxEquipmentSlots;

    void AddParticle(ParticleType type, const std::string& name, const std::string& desc,
                     ParticleRarity rarity, f64 dropChance, const Color& color,
                     f64 prodBonus = 0.0, f64 obsBonus = 0.0, f64 photonBonus = 0.0);
    void UpdateEquipmentSlots();
};

// Quantum Experiments - Challenge mode
enum class ExperimentType {
    DoubleSlitExperiment,     // Observation affects outcome
    SchrodingersBox,          // Quantum superposition test
    QuantumTeleportation,     // Transfer qubits instantly
    EntanglementTest,         // Create maximum entanglement
    TimeReversal,             // Run simulation backwards
    WaveCollapseRace,         // Speed challenge
    CoherencePreservation,    // Keep coherence at 100%
    PhotonBlast,              // Generate photons rapidly

    COUNT
};

struct QuantumExperiment {
    ExperimentType type;
    std::string name;
    std::string description;
    std::string objective;

    // Requirements
    i32 minPrestigeLevel;
    f64 entryCost;

    // Challenge parameters
    f64 timeLimit;
    f64 targetValue;
    bool completed;
    bool available;

    // Rewards
    ParticleType rewardParticle;
    f64 rewardQubits;
    f64 rewardPhotons;

    QuantumExperiment();
};

class ExperimentSystem {
public:
    ExperimentSystem();

    void Initialize();

    bool CanStart(ExperimentType type, class GameState* state) const;
    void StartExperiment(ExperimentType type, class GameState* state);
    void CompleteExperiment(ExperimentType type, class GameState* state);

    QuantumExperiment* GetExperiment(ExperimentType type);
    std::vector<QuantumExperiment*> GetAvailableExperiments();
    std::vector<QuantumExperiment*> GetCompletedExperiments();

private:
    std::vector<QuantumExperiment> m_Experiments;

    void AddExperiment(ExperimentType type, const std::string& name, const std::string& desc,
                       const std::string& objective, i32 minPrestige, f64 cost,
                       f64 timeLimit, ParticleType reward);
};
