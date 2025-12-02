#include "EnergyGrid.h"
#include "Logger.h"
#include <algorithm>
#include <sstream>
#include <fstream>

EnergyGrid::EnergyGrid()
    : m_MaxPower(100.0)
    , m_AvailablePower(0.0)
    , m_PowerDrainEnabled(false)
    , m_PowerDrainRate(0.0)
{
    // Initialize power allocations
    for (int i = 0; i < static_cast<int>(ShipSystem::COUNT); i++) {
        m_PowerAllocations[i] = 0.0;
        m_SystemEfficiency[i] = 1.0; // 100% efficiency by default
    }

    // Set recommended power levels for optimal operation
    m_RecommendedPower[static_cast<int>(ShipSystem::Shields)] = 30.0;
    m_RecommendedPower[static_cast<int>(ShipSystem::Engines)] = 25.0;
    m_RecommendedPower[static_cast<int>(ShipSystem::Weapons)] = 35.0;
    m_RecommendedPower[static_cast<int>(ShipSystem::LifeSupport)] = 10.0;
}

void EnergyGrid::Initialize() {
    // Reset all power allocations
    for (int i = 0; i < static_cast<int>(ShipSystem::COUNT); i++) {
        m_PowerAllocations[i] = 0.0;
    }
    m_AvailablePower = 0.0;
    m_PowerDrainEnabled = false;
    m_PowerDrainRate = 0.0;

    Log::Info("EnergyGrid initialized");
}

void EnergyGrid::Update(f64 deltaTime) {
    // Handle automatic power drain (for advanced gameplay)
    if (m_PowerDrainEnabled && m_PowerDrainRate > 0.0) {
        f64 drainAmount = m_PowerDrainRate * deltaTime;

        // Reduce available power
        m_AvailablePower -= drainAmount;
        if (m_AvailablePower < 0.0) {
            m_AvailablePower = 0.0;
        }

        // If available power is zero, start reducing allocations
        if (m_AvailablePower <= 0.0) {
            // Reduce allocations proportionally
            f64 totalAllocated = GetTotalAllocated();
            if (totalAllocated > 0.0) {
                for (int i = 0; i < static_cast<int>(ShipSystem::COUNT); i++) {
                    if (m_PowerAllocations[i] > 0.0) {
                        f64 proportion = m_PowerAllocations[i] / totalAllocated;
                        m_PowerAllocations[i] -= drainAmount * proportion;
                        if (m_PowerAllocations[i] < 0.0) {
                            m_PowerAllocations[i] = 0.0;
                        }
                    }
                }
            }
        }
    }
}

// === Power Management ===

void EnergyGrid::SetAvailablePower(f64 power) {
    m_AvailablePower = std::max(0.0, power);

    // If available power is less than total allocated, reduce allocations proportionally
    f64 totalAllocated = GetTotalAllocated();
    if (totalAllocated > m_AvailablePower) {
        f64 ratio = m_AvailablePower / totalAllocated;
        for (int i = 0; i < static_cast<int>(ShipSystem::COUNT); i++) {
            m_PowerAllocations[i] *= ratio;
        }
    }
}

f64 EnergyGrid::GetPowerAllocation(ShipSystem system) const {
    int idx = static_cast<int>(system);
    return m_PowerAllocations[idx];
}

void EnergyGrid::SetPowerAllocation(ShipSystem system, f64 power) {
    int idx = static_cast<int>(system);
    m_PowerAllocations[idx] = std::max(0.0, power);
    ClampPowerAllocation(system);
}

f64 EnergyGrid::GetTotalAllocated() const {
    f64 total = 0.0;
    for (int i = 0; i < static_cast<int>(ShipSystem::COUNT); i++) {
        total += m_PowerAllocations[i];
    }
    return total;
}

// === Power Routing ===

f64 EnergyGrid::AllocatePower(ShipSystem system, f64 amount) {
    if (amount <= 0.0) return 0.0;

    int idx = static_cast<int>(system);
    f64 remaining = GetRemainingPower();

    // Can only allocate what's available
    f64 actualAmount = std::min(amount, remaining);

    m_PowerAllocations[idx] += actualAmount;
    ClampPowerAllocation(system);

    if (actualAmount > 0.0) {
        Log::Infof("Allocated ", actualAmount, " power to ", GetSystemName(system));
    }

    return actualAmount;
}

f64 EnergyGrid::DeallocatePower(ShipSystem system, f64 amount) {
    if (amount <= 0.0) return 0.0;

    int idx = static_cast<int>(system);
    f64 current = m_PowerAllocations[idx];
    f64 actualAmount = std::min(amount, current);

    m_PowerAllocations[idx] -= actualAmount;

    if (actualAmount > 0.0) {
        Log::Infof("Deallocated ", actualAmount, " power from ", GetSystemName(system));
    }

    return actualAmount;
}

void EnergyGrid::RebalancePower() {
    // Distribute available power equally across all systems
    f64 powerPerSystem = m_AvailablePower / static_cast<f64>(static_cast<int>(ShipSystem::COUNT));

    for (int i = 0; i < static_cast<int>(ShipSystem::COUNT); i++) {
        m_PowerAllocations[i] = powerPerSystem;
    }

    Log::Info("Power rebalanced across all systems");
}

void EnergyGrid::CutAllPower() {
    for (int i = 0; i < static_cast<int>(ShipSystem::COUNT); i++) {
        m_PowerAllocations[i] = 0.0;
    }
    Log::Warning("All power cut!");
}

// === System Efficiency ===

f64 EnergyGrid::GetSystemEfficiency(ShipSystem system) const {
    int idx = static_cast<int>(system);
    f64 allocation = m_PowerAllocations[idx];
    f64 recommended = m_RecommendedPower[idx];

    if (recommended <= 0.0) return 1.0; // If no recommendation, assume 100%

    // Efficiency ranges from 0% to 100% based on power allocation
    // Optimal at recommended power, reduced below and above
    f64 ratio = allocation / recommended;

    // Apply efficiency modifier
    f64 baseEfficiency = std::min(1.0, ratio);
    f64 modifiedEfficiency = baseEfficiency * m_SystemEfficiency[idx];

    return std::min(1.0, modifiedEfficiency);
}

f64 EnergyGrid::GetRecommendedPower(ShipSystem system) const {
    int idx = static_cast<int>(system);
    return m_RecommendedPower[idx] / m_SystemEfficiency[idx];
}

bool EnergyGrid::IsOptimallyPowered(ShipSystem system) const {
    int idx = static_cast<int>(system);
    f64 allocation = m_PowerAllocations[idx];
    f64 recommended = GetRecommendedPower(system);

    // Within 5% of recommended is considered optimal
    f64 diff = std::abs(allocation - recommended);
    return diff <= (recommended * 0.05);
}

// === Combat Bonuses ===

f64 EnergyGrid::GetShieldMultiplier() const {
    // Shields scale linearly with power allocation
    // Base: 1.0x at 0 power
    // Max: 3.0x at recommended power
    f64 efficiency = GetSystemEfficiency(ShipSystem::Shields);
    return 1.0 + (efficiency * 2.0);
}

f64 EnergyGrid::GetEngineMultiplier() const {
    // Engines affect dodge chance and movement speed
    // Base: 1.0x at 0 power
    // Max: 2.5x at recommended power
    f64 efficiency = GetSystemEfficiency(ShipSystem::Engines);
    return 1.0 + (efficiency * 1.5);
}

f64 EnergyGrid::GetWeaponMultiplier() const {
    // Weapons affect damage output
    // Base: 0.5x at 0 power (minimal damage)
    // Max: 3.0x at recommended power
    f64 efficiency = GetSystemEfficiency(ShipSystem::Weapons);
    return 0.5 + (efficiency * 2.5);
}

f64 EnergyGrid::GetLifeSupportMultiplier() const {
    // Life support affects resource generation and crew effectiveness
    // Base: 0.5x at 0 power (reduced generation)
    // Max: 2.0x at recommended power
    f64 efficiency = GetSystemEfficiency(ShipSystem::LifeSupport);
    return 0.5 + (efficiency * 1.5);
}

// === Upgrades ===

void EnergyGrid::UpgradeEfficiency(ShipSystem system, f64 amount) {
    int idx = static_cast<int>(system);
    m_SystemEfficiency[idx] += amount;

    // Cap at 2.0 (200% efficiency = half power requirements)
    m_SystemEfficiency[idx] = std::min(2.0, m_SystemEfficiency[idx]);

    Log::Infof("Upgraded ", GetSystemName(system), " efficiency to ",
               m_SystemEfficiency[idx] * 100.0, "%");
}

// === Utility ===

const char* EnergyGrid::GetSystemName(ShipSystem system) const {
    switch (system) {
        case ShipSystem::Shields: return "Shields";
        case ShipSystem::Engines: return "Engines";
        case ShipSystem::Weapons: return "Weapons";
        case ShipSystem::LifeSupport: return "Life Support";
        default: return "Unknown";
    }
}

Color EnergyGrid::GetSystemColor(ShipSystem system) const {
    switch (system) {
        case ShipSystem::Shields: return Color(0.2f, 0.6f, 1.0f, 1.0f); // Blue
        case ShipSystem::Engines: return Color(1.0f, 0.5f, 0.0f, 1.0f); // Orange
        case ShipSystem::Weapons: return Color(1.0f, 0.2f, 0.2f, 1.0f); // Red
        case ShipSystem::LifeSupport: return Color(0.2f, 1.0f, 0.2f, 1.0f); // Green
        default: return Color::White();
    }
}

// === Save/Load ===

void EnergyGrid::SaveToJson(std::ofstream& file) const {
    file << "    \"energyGrid\": {\n";
    file << "        \"maxPower\": " << m_MaxPower << ",\n";
    file << "        \"availablePower\": " << m_AvailablePower << ",\n";
    file << "        \"powerDrainEnabled\": " << (m_PowerDrainEnabled ? "true" : "false") << ",\n";
    file << "        \"powerDrainRate\": " << m_PowerDrainRate << ",\n";

    // Save power allocations
    file << "        \"powerAllocations\": {\n";
    file << "            \"shields\": " << m_PowerAllocations[static_cast<int>(ShipSystem::Shields)] << ",\n";
    file << "            \"engines\": " << m_PowerAllocations[static_cast<int>(ShipSystem::Engines)] << ",\n";
    file << "            \"weapons\": " << m_PowerAllocations[static_cast<int>(ShipSystem::Weapons)] << ",\n";
    file << "            \"lifeSupport\": " << m_PowerAllocations[static_cast<int>(ShipSystem::LifeSupport)] << "\n";
    file << "        },\n";

    // Save efficiency upgrades
    file << "        \"systemEfficiency\": {\n";
    file << "            \"shields\": " << m_SystemEfficiency[static_cast<int>(ShipSystem::Shields)] << ",\n";
    file << "            \"engines\": " << m_SystemEfficiency[static_cast<int>(ShipSystem::Engines)] << ",\n";
    file << "            \"weapons\": " << m_SystemEfficiency[static_cast<int>(ShipSystem::Weapons)] << ",\n";
    file << "            \"lifeSupport\": " << m_SystemEfficiency[static_cast<int>(ShipSystem::LifeSupport)] << "\n";
    file << "        }\n";
    file << "    }";
}

void EnergyGrid::LoadFromJson(const std::string& line) {
    // Simple manual JSON parsing (to be replaced with nlohmann/json later)

    // Check for main fields
    if (line.find("\"maxPower\"") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            m_MaxPower = std::stod(value);
        }
    }
    else if (line.find("\"availablePower\"") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            m_AvailablePower = std::stod(value);
        }
    }
    else if (line.find("\"powerDrainEnabled\"") != std::string::npos) {
        m_PowerDrainEnabled = (line.find("true") != std::string::npos);
    }
    else if (line.find("\"powerDrainRate\"") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            m_PowerDrainRate = std::stod(value);
        }
    }

    // Load power allocations
    else if (line.find("\"shields\":") != std::string::npos && line.find("powerAllocations") == std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            // Check context - if we're in powerAllocations section
            if (line.find("powerAllocations") != std::string::npos) {
                m_PowerAllocations[static_cast<int>(ShipSystem::Shields)] = std::stod(value);
            } else if (line.find("systemEfficiency") != std::string::npos) {
                m_SystemEfficiency[static_cast<int>(ShipSystem::Shields)] = std::stod(value);
            }
        }
    }
    else if (line.find("\"engines\":") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            if (line.find("powerAllocations") != std::string::npos) {
                m_PowerAllocations[static_cast<int>(ShipSystem::Engines)] = std::stod(value);
            } else if (line.find("systemEfficiency") != std::string::npos) {
                m_SystemEfficiency[static_cast<int>(ShipSystem::Engines)] = std::stod(value);
            }
        }
    }
    else if (line.find("\"weapons\":") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            if (line.find("powerAllocations") != std::string::npos) {
                m_PowerAllocations[static_cast<int>(ShipSystem::Weapons)] = std::stod(value);
            } else if (line.find("systemEfficiency") != std::string::npos) {
                m_SystemEfficiency[static_cast<int>(ShipSystem::Weapons)] = std::stod(value);
            }
        }
    }
    else if (line.find("\"lifeSupport\":") != std::string::npos) {
        size_t pos = line.find(':');
        if (pos != std::string::npos) {
            std::string value = line.substr(pos + 1);
            if (line.find("powerAllocations") != std::string::npos) {
                m_PowerAllocations[static_cast<int>(ShipSystem::LifeSupport)] = std::stod(value);
            } else if (line.find("systemEfficiency") != std::string::npos) {
                m_SystemEfficiency[static_cast<int>(ShipSystem::LifeSupport)] = std::stod(value);
            }
        }
    }
}

// === Private Helpers ===

void EnergyGrid::ClampPowerAllocation(ShipSystem system) {
    int idx = static_cast<int>(system);

    // Can't allocate negative power
    if (m_PowerAllocations[idx] < 0.0) {
        m_PowerAllocations[idx] = 0.0;
    }

    // Can't exceed available power
    f64 totalAllocated = GetTotalAllocated();
    if (totalAllocated > m_AvailablePower) {
        // Reduce this system's allocation to fit
        f64 excess = totalAllocated - m_AvailablePower;
        m_PowerAllocations[idx] -= excess;
        if (m_PowerAllocations[idx] < 0.0) {
            m_PowerAllocations[idx] = 0.0;
        }
    }
}
