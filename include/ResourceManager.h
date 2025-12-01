#pragma once

#include "Types.h"

// Forward declarations
struct QuantumTimeline;

/**
 * ResourceManager - Centralized resource management system
 *
 * Handles all game resources including:
 * - Basic quantum resources (Qubits, Coherence, Entanglement)
 * - Prestige currencies (Photons, Singularities)
 * - Player credits (for Gatcha/Shop systems)
 * - Coherence decay mechanics
 *
 * This class extracts resource management from GameState to establish
 * proper separation of concerns (MVC pattern).
 */
class ResourceManager {
public:
    ResourceManager();
    ~ResourceManager() = default;

    // Initialize with starting values
    void Initialize();

    // Update (handles coherence decay, etc.)
    void Update(f64 deltaTime);

    // === Basic Resource Operations ===

    // Get resource amount
    f64 GetResource(QuantumResource type) const;

    // Add resource (returns actual amount added after caps)
    f64 AddResource(QuantumResource type, f64 amount);

    // Spend resource (returns true if successful)
    bool SpendResource(QuantumResource type, f64 amount);

    // Check if can afford
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
    void IncrementResets() { m_Timeline->completedResets++; }
    void IncrementCollapses() { m_Timeline->completedCollapses++; }

    // Set timeline reference (called by GameState during initialization)
    void SetTimeline(QuantumTimeline* timeline) { m_Timeline = timeline; }

    // === Player Credits (Gatcha/Shop Currency) ===

    i32 GetCredits() const { return m_PlayerCredits; }
    void AddCredits(i32 amount) { m_PlayerCredits += amount; }
    bool SpendCredits(i32 amount);

    // === Resource Caps ===

    void SetResourceCap(QuantumResource type, f64 cap);
    f64 GetResourceCap(QuantumResource type) const;
    bool HasResourceCap(QuantumResource type) const;

private:
    // Basic resources (indexed by QuantumResource enum)
    f64 m_Resources[3]; // Qubits, Coherence, Entanglement
    f64 m_ResourceCaps[3]; // Optional caps (-1 = no cap)

    // Coherence system
    f64 m_Coherence;
    f64 m_MaxCoherence;
    f64 m_CoherenceDecayRate;

    // Player credits (for Gatcha/Shop)
    i32 m_PlayerCredits;

    // Timeline reference (for prestige currencies)
    // Note: We don't own this, GameState does
    QuantumTimeline* m_Timeline;

    // Helper to clamp resource to cap
    f64 ApplyCap(QuantumResource type, f64 value) const;
};
