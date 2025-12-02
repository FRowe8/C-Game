#include "ResourceManager.h"
#include "GameState.h" // For QuantumTimeline definition
#include <algorithm>
#include <cmath>

ResourceManager::ResourceManager()
    : m_Coherence(100.0)
    , m_MaxCoherence(100.0)
    , m_CoherenceDecayRate(1.0)
    , m_PlayerCredits(0)
    , m_Timeline(nullptr)
{
    // Initialize resources to 0
    for (int i = 0; i < 3; i++) {
        m_Resources[i] = 0.0;
        m_ResourceCaps[i] = -1.0; // No cap by default
    }
}

void ResourceManager::Initialize() {
    // Set starting values
    m_Resources[static_cast<int>(QuantumResource::Qubits)] = 10.0; // Start with 10 qubits
    m_Resources[static_cast<int>(QuantumResource::Coherence)] = 0.0;
    m_Resources[static_cast<int>(QuantumResource::Entanglement)] = 0.0;

    m_Coherence = 100.0;
    m_MaxCoherence = 100.0;
    m_CoherenceDecayRate = 1.0;

    m_PlayerCredits = 0;
}

void ResourceManager::Update(f64 deltaTime) {
    // Coherence decay
    if (m_Coherence > 0.0) {
        m_Coherence -= m_CoherenceDecayRate * deltaTime;
        if (m_Coherence < 0.0) {
            m_Coherence = 0.0;
        }
    }
}

// === Basic Resource Operations ===

f64 ResourceManager::GetResource(QuantumResource type) const {
    return m_Resources[static_cast<int>(type)];
}

f64 ResourceManager::AddResource(QuantumResource type, f64 amount) {
    if (amount <= 0.0) return 0.0;

    int idx = static_cast<int>(type);
    f64 oldValue = m_Resources[idx];
    f64 newValue = oldValue + amount;

    // Apply cap if it exists
    newValue = ApplyCap(type, newValue);

    m_Resources[idx] = newValue;

    // Return actual amount added
    return newValue - oldValue;
}

bool ResourceManager::SpendResource(QuantumResource type, f64 amount) {
    if (amount <= 0.0) return true; // Nothing to spend

    int idx = static_cast<int>(type);

    if (m_Resources[idx] < amount) {
        return false; // Not enough resources
    }

    m_Resources[idx] -= amount;
    return true;
}

bool ResourceManager::CanAfford(QuantumResource type, f64 amount) const {
    return GetResource(type) >= amount;
}

// === Coherence System ===

void ResourceManager::SetCoherence(f64 value) {
    m_Coherence = std::clamp(value, 0.0, m_MaxCoherence);
}

void ResourceManager::AddCoherence(f64 amount) {
    m_Coherence += amount;
    if (m_Coherence > m_MaxCoherence) {
        m_Coherence = m_MaxCoherence;
    }
}

bool ResourceManager::SpendCoherence(f64 amount) {
    if (m_Coherence < amount) {
        return false;
    }
    m_Coherence -= amount;
    return true;
}

// === Prestige Currencies ===

f64 ResourceManager::GetPhotons() const {
    return m_Timeline ? m_Timeline->photons : 0.0;
}

f64 ResourceManager::GetSingularities() const {
    return m_Timeline ? m_Timeline->singularities : 0.0;
}

i32 ResourceManager::GetCompletedResets() const {
    return m_Timeline ? m_Timeline->completedResets : 0;
}

i32 ResourceManager::GetCompletedCollapses() const {
    return m_Timeline ? m_Timeline->completedCollapses : 0;
}

void ResourceManager::AddPhotons(f64 amount) {
    if (m_Timeline) {
        m_Timeline->photons += amount;
    }
}

void ResourceManager::AddSingularities(f64 amount) {
    if (m_Timeline) {
        m_Timeline->singularities += amount;
    }
}

// NEW: Added these implementations here to resolve the header error
void ResourceManager::IncrementResets() {
    if (m_Timeline) {
        m_Timeline->completedResets++;
    }
}

// NEW: Added these implementations here to resolve the header error
void ResourceManager::IncrementCollapses() {
    if (m_Timeline) {
        m_Timeline->completedCollapses++;
    }
}

// === Player Credits ===

bool ResourceManager::SpendCredits(i32 amount) {
    if (m_PlayerCredits < amount) {
        return false;
    }
    m_PlayerCredits -= amount;
    return true;
}

// === Resource Caps ===

void ResourceManager::SetResourceCap(QuantumResource type, f64 cap) {
    m_ResourceCaps[static_cast<int>(type)] = cap;
}

f64 ResourceManager::GetResourceCap(QuantumResource type) const {
    return m_ResourceCaps[static_cast<int>(type)];
}

bool ResourceManager::HasResourceCap(QuantumResource type) const {
    return m_ResourceCaps[static_cast<int>(type)] >= 0.0;
}

f64 ResourceManager::ApplyCap(QuantumResource type, f64 value) const {
    f64 cap = m_ResourceCaps[static_cast<int>(type)];
    if (cap >= 0.0 && value > cap) {
        return cap;
    }
    return value;
}