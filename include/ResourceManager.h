#pragma once

#include "Types.h"

// Forward declarations
struct QuantumTimeline;

class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager() = default;

    void Initialize();
    void Update(f64 deltaTime);

    // === Basic Resource Operations ===
    f64 GetResource(QuantumResource type) const;
    f64 AddResource(QuantumResource type, f64 amount);
    bool SpendResource(QuantumResource type, f64 amount);
    bool CanAfford(QuantumResource type, f64 amount) const;

    // === Coherence System ===
    f64 GetCoherence() const { return m_Coherence; }
    f64 GetMaxCoherence() const { return m_MaxCoherence; }
    f64 GetCoherenceDecayRate() const { return m_CoherenceDecayRate; }

    void SetCoherence(f64 value);
    void SetMaxCoherence(f64 value) { m_MaxCoherence = value; }
    void SetCoherenceDecayRate(f64 rate) { m_CoherenceDecayRate = rate; }

    void AddCoherence(f64 amount);
    bool SpendCoherence(f64 amount);

    // === Prestige Currencies ===
    f64 GetPhotons() const;
    f64 GetSingularities() const;
    i32 GetCompletedResets() const;
    i32 GetCompletedCollapses() const;

    void AddPhotons(f64 amount);
    void AddSingularities(f64 amount);

    // FIX: Changed from inline implementation to declaration only
    void IncrementResets();
    void IncrementCollapses();

    // Set timeline reference (This is safe to keep inline because it only assigns the pointer)
    void SetTimeline(QuantumTimeline* timeline) { m_Timeline = timeline; }

    // === Player Credits ===
    i32 GetCredits() const { return m_PlayerCredits; }
    void AddCredits(i32 amount) { m_PlayerCredits += amount; }
    bool SpendCredits(i32 amount);

    // === Resource Caps ===
    void SetResourceCap(QuantumResource type, f64 cap);
    f64 GetResourceCap(QuantumResource type) const;
    bool HasResourceCap(QuantumResource type) const;

private:
    f64 m_Resources[3];
    f64 m_ResourceCaps[3];

    f64 m_Coherence;
    f64 m_MaxCoherence;
    f64 m_CoherenceDecayRate;

    i32 m_PlayerCredits;

    QuantumTimeline* m_Timeline;

    f64 ApplyCap(QuantumResource type, f64 value) const;
};