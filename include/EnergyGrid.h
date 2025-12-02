#pragma once

#include "Types.h"
#include <string>

/**
 * EnergyGrid - Bridge between Quantum Resources and Spaceship Systems
 *
 * As mentioned in IMPROVEMENTS.md Section 2.1, this creates the "thematic bridge"
 * between the Quantum layer and Spaceship layer:
 *
 * "Research Stations should not just generate Qubits; they should power the
 * Spaceship's shields and weapons. You must route Coherence (Quantum) to specific
 * Ship Systems (Shields, Engines, Weapons)."
 *
 * This forces the player to balance the idle layer (Quantum) to succeed in the
 * active layer (Combat).
 */

// Ship systems that can be powered
enum class ShipSystem {
    Shields,
    Engines,
    Weapons,
    LifeSupport,
    COUNT
};

// Energy Grid manages power routing to ship systems
class EnergyGrid {
public:
    EnergyGrid();
    ~EnergyGrid() = default;

    // Initialize
    void Initialize();

    // Update (handles automatic power drain)
    void Update(f64 deltaTime);

    // === Power Management ===

    // Total power capacity (based on research stations)
    f64 GetMaxPower() const { return m_MaxPower; }
    void SetMaxPower(f64 power) { m_MaxPower = power; }

    // Current available power (from Coherence)
    f64 GetAvailablePower() const { return m_AvailablePower; }
    void SetAvailablePower(f64 power);

    // Power allocated to a specific system
    f64 GetPowerAllocation(ShipSystem system) const;
    void SetPowerAllocation(ShipSystem system, f64 power);

    // Total power allocated across all systems
    f64 GetTotalAllocated() const;

    // Remaining unallocated power
    f64 GetRemainingPower() const { return m_AvailablePower - GetTotalAllocated(); }

    // === Power Routing ===

    // Allocate power to a system (returns actual amount allocated)
    f64 AllocatePower(ShipSystem system, f64 amount);

    // Deallocate power from a system (returns amount freed)
    f64 DeallocatePower(ShipSystem system, f64 amount);

    // Rebalance power equally across all systems
    void RebalancePower();

    // Emergency: Cut all power (for critical situations)
    void CutAllPower();

    // === System Efficiency ===

    // Get efficiency percentage for a system (0-100%)
    f64 GetSystemEfficiency(ShipSystem system) const;

    // Recommended power for optimal operation
    f64 GetRecommendedPower(ShipSystem system) const;

    // Is system at optimal power?
    bool IsOptimallyPowered(ShipSystem system) const;

    // === Combat Bonuses ===

    // Shield strength based on power allocation (multiplier)
    f64 GetShieldMultiplier() const;

    // Engine speed based on power allocation (multiplier)
    f64 GetEngineMultiplier() const;

    // Weapon damage based on power allocation (multiplier)
    f64 GetWeaponMultiplier() const;

    // Life support affects resource generation (multiplier)
    f64 GetLifeSupportMultiplier() const;

    // === Power Drain ===

    // Enable/disable automatic power drain
    void EnablePowerDrain(bool enabled) { m_PowerDrainEnabled = enabled; }
    bool IsPowerDrainEnabled() const { return m_PowerDrainEnabled; }

    // Power drain rate (power per second)
    void SetPowerDrainRate(f64 rate) { m_PowerDrainRate = rate; }
    f64 GetPowerDrainRate() const { return m_PowerDrainRate; }

    // === Upgrades ===

    // Increase max power capacity
    void UpgradeCapacity(f64 amount) { m_MaxPower += amount; }

    // Improve system efficiency (reduces power requirements)
    void UpgradeEfficiency(ShipSystem system, f64 amount);

    // === Utility ===

    const char* GetSystemName(ShipSystem system) const;
    Color GetSystemColor(ShipSystem system) const;

    // === Save/Load ===
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    // Power capacity and availability
    f64 m_MaxPower;           // Maximum power capacity
    f64 m_AvailablePower;     // Current available power (from Coherence)

    // Power allocated to each system
    f64 m_PowerAllocations[static_cast<int>(ShipSystem::COUNT)];

    // System efficiency upgrades (reduces power requirements)
    f64 m_SystemEfficiency[static_cast<int>(ShipSystem::COUNT)];

    // Recommended power levels for optimal operation
    f64 m_RecommendedPower[static_cast<int>(ShipSystem::COUNT)];

    // Power drain system
    bool m_PowerDrainEnabled;
    f64 m_PowerDrainRate;     // Power lost per second

    // Helper to clamp power allocation
    void ClampPowerAllocation(ShipSystem system);
};
