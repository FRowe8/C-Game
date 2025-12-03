#include "../../include/ParticleCollection.h"
#include "../../include/Log.h"
#include <algorithm>
#include <random>

// ============================================================================
// Particle
// ============================================================================

Particle::Particle()
    : type(ParticleType::Electron)
    , name("")
    , description("")
    , rarity(ParticleRarity::Common)
    , discovered(false)
    , count(0)
    , dropChance(0.0)
    , productionBonus(0.0)
    , observationBonus(0.0)
    , photonBonus(0.0)
    , eventChanceBonus(0.0)
    , particleColor(Color::White())
{
}

// ============================================================================
// ParticleCollection
// ============================================================================

ParticleCollection::ParticleCollection()
    : m_TimeSinceLastCheck(0.0)
{
}

void ParticleCollection::Initialize() {
    m_Particles.clear();
    m_EquippedParticles.clear();
    m_RecentDiscoveries.clear();

    // ===== COMMON PARTICLES (4) =====
    // Easy to find, small bonuses
    AddParticle(ParticleType::Electron, "Electron",
        "A fundamental particle with negative charge. The building block of atoms.",
        ParticleRarity::Common, 0.10, Color(0.3f, 0.5f, 1.0f, 1.0f),
        0.05, 0.02, 0.0);  // +5% production, +2% observation

    AddParticle(ParticleType::Proton, "Proton",
        "A positively charged particle found in atomic nuclei. Stable and reliable.",
        ParticleRarity::Common, 0.10, Color(1.0f, 0.3f, 0.3f, 1.0f),
        0.05, 0.0, 0.0);  // +5% production

    AddParticle(ParticleType::Neutron, "Neutron",
        "A neutral particle in the nucleus. Provides stability without charge.",
        ParticleRarity::Common, 0.10, Color(0.6f, 0.6f, 0.6f, 1.0f),
        0.03, 0.03, 0.0);  // +3% production, +3% observation

    AddParticle(ParticleType::Photon, "Photon",
        "A quantum of light. Massless and travels at the speed of light.",
        ParticleRarity::Common, 0.10, Color(1.0f, 1.0f, 0.3f, 1.0f),
        0.02, 0.05, 0.0);  // +2% production, +5% observation

    // ===== UNCOMMON PARTICLES (4) =====
    // Moderate rarity, better bonuses
    AddParticle(ParticleType::Muon, "Muon",
        "A heavier cousin of the electron. Unstable but powerful.",
        ParticleRarity::Uncommon, 0.05, Color(0.5f, 0.3f, 0.8f, 1.0f),
        0.10, 0.05, 0.0);  // +10% production, +5% observation

    AddParticle(ParticleType::Tau, "Tau Lepton",
        "The heaviest lepton. Rare and exotic, it decays rapidly.",
        ParticleRarity::Uncommon, 0.05, Color(0.8f, 0.3f, 0.8f, 1.0f),
        0.08, 0.08, 0.0);  // +8% production, +8% observation

    AddParticle(ParticleType::Neutrino, "Neutrino",
        "Nearly massless and rarely interacts. The ghost particle.",
        ParticleRarity::Uncommon, 0.05, Color(0.7f, 0.9f, 1.0f, 1.0f),
        0.05, 0.10, 0.02);  // +5% production, +10% observation, +2% events

    AddParticle(ParticleType::Positron, "Positron",
        "The antimatter counterpart of the electron. Annihilates on contact.",
        ParticleRarity::Uncommon, 0.05, Color(1.0f, 0.7f, 0.3f, 1.0f),
        0.12, 0.03, 0.0);  // +12% production, +3% observation

    // ===== RARE PARTICLES (6) =====
    // The six quarks - harder to find, strong bonuses
    AddParticle(ParticleType::QuarkUp, "Up Quark",
        "The lightest quark with +2/3 charge. Found in protons and neutrons.",
        ParticleRarity::Rare, 0.02, Color(1.0f, 0.4f, 0.4f, 1.0f),
        0.15, 0.05, 0.0);  // +15% production, +5% observation

    AddParticle(ParticleType::QuarkDown, "Down Quark",
        "A quark with -1/3 charge. Complements the up quark in matter.",
        ParticleRarity::Rare, 0.02, Color(0.4f, 0.4f, 1.0f, 1.0f),
        0.15, 0.05, 0.0);  // +15% production, +5% observation

    AddParticle(ParticleType::QuarkCharm, "Charm Quark",
        "A heavier quark discovered in 1974. Adds a charming touch to physics.",
        ParticleRarity::Rare, 0.02, Color(1.0f, 0.6f, 1.0f, 1.0f),
        0.18, 0.07, 0.0);  // +18% production, +7% observation

    AddParticle(ParticleType::QuarkStrange, "Strange Quark",
        "An unusual quark with unexpected properties. Lives up to its name.",
        ParticleRarity::Rare, 0.02, Color(0.6f, 1.0f, 0.6f, 1.0f),
        0.18, 0.07, 0.03);  // +18% production, +7% observation, +3% events

    AddParticle(ParticleType::QuarkTop, "Top Quark",
        "The heaviest known elementary particle. Extremely short-lived.",
        ParticleRarity::Rare, 0.01, Color(1.0f, 0.8f, 0.2f, 1.0f),
        0.25, 0.10, 0.0);  // +25% production, +10% observation

    AddParticle(ParticleType::QuarkBottom, "Bottom Quark",
        "Also called the beauty quark. Heavy and unstable.",
        ParticleRarity::Rare, 0.01, Color(0.8f, 0.4f, 0.8f, 1.0f),
        0.25, 0.10, 0.0);  // +25% production, +10% observation

    // ===== EXOTIC PARTICLES (4) =====
    // Force carriers and special particles
    AddParticle(ParticleType::Gluon, "Gluon",
        "Mediates the strong force. Binds quarks together with incredible strength.",
        ParticleRarity::Exotic, 0.01, Color(0.2f, 1.0f, 0.2f, 1.0f),
        0.30, 0.15, 0.05);  // +30% production, +15% observation, +5% events

    AddParticle(ParticleType::WBoson, "W Boson",
        "Mediates weak nuclear force. Causes radioactive decay.",
        ParticleRarity::Exotic, 0.008, Color(0.9f, 0.5f, 0.2f, 1.0f),
        0.35, 0.15, 0.05);  // +35% production, +15% observation, +5% events

    AddParticle(ParticleType::ZBoson, "Z Boson",
        "Neutral carrier of the weak force. Partner to the W boson.",
        ParticleRarity::Exotic, 0.008, Color(0.5f, 0.5f, 1.0f, 1.0f),
        0.35, 0.15, 0.05);  // +35% production, +15% observation, +5% events

    AddParticle(ParticleType::HiggsBoson, "Higgs Boson",
        "The God Particle. Gives mass to other particles through the Higgs field.",
        ParticleRarity::Exotic, 0.005, Color(1.0f, 0.9f, 0.0f, 1.0f),
        0.50, 0.25, 0.10);  // +50% production, +25% observation, +10% events

    // ===== LEGENDARY PARTICLES (4) =====
    // Theoretical and extremely rare
    AddParticle(ParticleType::Graviton, "Graviton",
        "Theoretical particle that mediates gravity. Never observed.",
        ParticleRarity::Legendary, 0.003, Color(0.6f, 0.3f, 0.9f, 1.0f),
        0.75, 0.35, 0.15);  // +75% production, +35% observation, +15% events

    AddParticle(ParticleType::Axion, "Axion",
        "Dark matter candidate. Could solve the strong CP problem.",
        ParticleRarity::Legendary, 0.003, Color(0.2f, 0.2f, 0.3f, 1.0f),
        0.60, 0.40, 0.20);  // +60% production, +40% observation, +20% events

    AddParticle(ParticleType::Tachyon, "Tachyon",
        "Hypothetical faster-than-light particle. Violates causality.",
        ParticleRarity::Legendary, 0.002, Color(0.0f, 1.0f, 1.0f, 1.0f),
        1.00, 0.50, 0.25);  // +100% production, +50% observation, +25% events

    AddParticle(ParticleType::Monopole, "Magnetic Monopole",
        "A magnet with only one pole. Predicted but never found.",
        ParticleRarity::Legendary, 0.002, Color(1.0f, 0.3f, 0.3f, 1.0f),
        0.80, 0.30, 0.30);  // +80% production, +30% observation, +30% events

    // ===== MYTHICAL PARTICLES (4) =====
    // Ultimate discoveries with game-changing bonuses
    AddParticle(ParticleType::QuantumSingularity, "Quantum Singularity",
        "A point of infinite density in quantum space. Reality bends around it.",
        ParticleRarity::Mythical, 0.001, Color(0.5f, 0.0f, 0.5f, 1.0f),
        2.00, 1.00, 0.50);  // +200% production, +100% observation, +50% events

    AddParticle(ParticleType::GodParticle, "God Particle",
        "The true fundamental particle. The source of all quantum phenomena.",
        ParticleRarity::Mythical, 0.0005, Color(1.0f, 1.0f, 1.0f, 1.0f),
        3.00, 1.50, 1.00);  // +300% production, +150% observation, +100% events

    AddParticle(ParticleType::DarkMatter, "Dark Matter",
        "The invisible scaffold of the universe. Makes up 85% of all matter.",
        ParticleRarity::Mythical, 0.0005, Color(0.1f, 0.1f, 0.2f, 1.0f),
        2.50, 1.25, 0.75);  // +250% production, +125% observation, +75% events

    AddParticle(ParticleType::DarkEnergy, "Dark Energy",
        "The mysterious force accelerating the universe's expansion.",
        ParticleRarity::Mythical, 0.0003, Color(0.2f, 0.0f, 0.3f, 1.0f),
        4.00, 2.00, 1.50);  // +400% production, +200% observation, +150% events

    Log::Info("ParticleCollection initialized with 28 particles");
}

void ParticleCollection::AddParticle(ParticleType type, const std::string& name, const std::string& desc,
                                     ParticleRarity rarity, f64 dropChance, const Color& color,
                                     f64 prodBonus, f64 obsBonus, f64 photonBonus) {
    Particle p;
    p.type = type;
    p.name = name;
    p.description = desc;
    p.rarity = rarity;
    p.discovered = false;
    p.count = 0;
    p.dropChance = dropChance;
    p.productionBonus = prodBonus;
    p.observationBonus = obsBonus;
    p.photonBonus = photonBonus;
    p.eventChanceBonus = 0.0;
    p.particleColor = color;

    m_Particles.push_back(p);
}

void ParticleCollection::CheckForDiscoveries(f64 deltaTime) {
    m_TimeSinceLastCheck += deltaTime;

    // Check every second for potential discoveries
    if (m_TimeSinceLastCheck >= 1.0) {
        m_TimeSinceLastCheck = 0.0;
        // Passive discovery happens in TryFindParticle() which is called from Observe()
    }
}

bool ParticleCollection::TryFindParticle() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<f64> dis(0.0, 1.0);

    // Phase 4.1: 0.5% chance on each observation to discover a particle
    f64 discoveryChance = 0.005;
    if (dis(gen) > discoveryChance) {
        return false;  // No discovery this time
    }

    // Build a weighted list of undiscovered particles
    std::vector<ParticleType> undiscovered;
    std::vector<f64> weights;

    for (auto& particle : m_Particles) {
        if (!particle.discovered) {
            undiscovered.push_back(particle.type);
            weights.push_back(particle.dropChance);
        }
    }

    if (undiscovered.empty()) {
        return false;  // All particles discovered!
    }

    // Weighted random selection
    std::discrete_distribution<> weightedDis(weights.begin(), weights.end());
    i32 selectedIndex = weightedDis(gen);

    DiscoverParticle(undiscovered[selectedIndex]);
    return true;
}

void ParticleCollection::DiscoverParticle(ParticleType type) {
    Particle* particle = GetParticle(type);
    if (!particle) return;

    if (!particle->discovered) {
        particle->discovered = true;
        particle->count = 1;
        m_RecentDiscoveries.push_back(type);

        Log::Infof("*** NEW PARTICLE DISCOVERED: ", particle->name, " (",
                   static_cast<i32>(particle->rarity), ") ***");
    } else {
        particle->count++;
        Log::Infof("Found another ", particle->name, "! Total: ", particle->count);
    }
}

bool ParticleCollection::IsDiscovered(ParticleType type) const {
    for (const auto& particle : m_Particles) {
        if (particle.type == type) {
            return particle.discovered;
        }
    }
    return false;
}

i32 ParticleCollection::GetCount(ParticleType type) const {
    for (const auto& particle : m_Particles) {
        if (particle.type == type) {
            return particle.count;
        }
    }
    return 0;
}

Particle* ParticleCollection::GetParticle(ParticleType type) {
    for (auto& particle : m_Particles) {
        if (particle.type == type) {
            return &particle;
        }
    }
    return nullptr;
}

void ParticleCollection::EquipParticle(ParticleType type) {
    if (!IsDiscovered(type)) return;

    // Check if already equipped
    if (IsEquipped(type)) return;

    // Limit to 3 equipped particles at once
    if (m_EquippedParticles.size() >= 3) {
        Log::Warn("Cannot equip more than 3 particles! Unequip one first.");
        return;
    }

    m_EquippedParticles.push_back(type);
    Particle* p = GetParticle(type);
    if (p) {
        Log::Infof("Equipped: ", p->name);
    }
}

void ParticleCollection::UnequipParticle(ParticleType type) {
    auto it = std::find(m_EquippedParticles.begin(), m_EquippedParticles.end(), type);
    if (it != m_EquippedParticles.end()) {
        m_EquippedParticles.erase(it);
        Particle* p = GetParticle(type);
        if (p) {
            Log::Infof("Unequipped: ", p->name);
        }
    }
}

bool ParticleCollection::IsEquipped(ParticleType type) const {
    return std::find(m_EquippedParticles.begin(), m_EquippedParticles.end(), type) != m_EquippedParticles.end();
}

f64 ParticleCollection::GetTotalProductionBonus() const {
    f64 total = 0.0;
    for (ParticleType type : m_EquippedParticles) {
        for (const auto& particle : m_Particles) {
            if (particle.type == type) {
                total += particle.productionBonus;
                break;
            }
        }
    }
    return total;
}

f64 ParticleCollection::GetTotalObservationBonus() const {
    f64 total = 0.0;
    for (ParticleType type : m_EquippedParticles) {
        for (const auto& particle : m_Particles) {
            if (particle.type == type) {
                total += particle.observationBonus;
                break;
            }
        }
    }
    return total;
}

f64 ParticleCollection::GetTotalPhotonBonus() const {
    f64 total = 0.0;
    for (ParticleType type : m_EquippedParticles) {
        for (const auto& particle : m_Particles) {
            if (particle.type == type) {
                total += particle.photonBonus;
                break;
            }
        }
    }
    return total;
}

f64 ParticleCollection::GetTotalEventChanceBonus() const {
    f64 total = 0.0;
    for (ParticleType type : m_EquippedParticles) {
        for (const auto& particle : m_Particles) {
            if (particle.type == type) {
                total += particle.eventChanceBonus;
                break;
            }
        }
    }
    return total;
}

i32 ParticleCollection::GetDiscoveredCount() const {
    i32 count = 0;
    for (const auto& particle : m_Particles) {
        if (particle.discovered) count++;
    }
    return count;
}

i32 ParticleCollection::GetTotalParticles() const {
    return static_cast<i32>(ParticleType::COUNT);
}

f64 ParticleCollection::GetCompletionPercentage() const {
    return (static_cast<f64>(GetDiscoveredCount()) / static_cast<f64>(GetTotalParticles())) * 100.0;
}

std::vector<Particle*> ParticleCollection::GetDiscoveredParticles() {
    std::vector<Particle*> discovered;
    for (auto& particle : m_Particles) {
        if (particle.discovered) {
            discovered.push_back(&particle);
        }
    }
    return discovered;
}

std::vector<Particle*> ParticleCollection::GetEquippedParticles() {
    std::vector<Particle*> equipped;
    for (ParticleType type : m_EquippedParticles) {
        Particle* p = GetParticle(type);
        if (p) {
            equipped.push_back(p);
        }
    }
    return equipped;
}

std::vector<ParticleType> ParticleCollection::GetRecentDiscoveries() {
    return m_RecentDiscoveries;
}

void ParticleCollection::ClearRecentDiscoveries() {
    m_RecentDiscoveries.clear();
}
