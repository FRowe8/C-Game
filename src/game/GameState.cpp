#include "GameState.h"
#include "Renderer.h"
#include "Input.h"
#include "ImGuiUtils.h"
#include "Logger.h"
#include "Platform.h"
#include "GameUtils.h"
#include "SteamIntegration.h"
#include "imgui.h"
#include <fstream>
#include <cstring>
#include <cstdio>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <functional>
#include <iomanip>

#include "Research.h"
#include "UIManager.h"
#include "GuiLayer.h" // Phase 1.1: Decoupled UI layer
#include "TutorialOverlay.h" // Phase 2.1: Tutorial system
#include "FloatingTextManager.h" // Phase 2.2: Visual feedback

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace {

// Simple salted hash to protect save files from tampering/corruption.
std::string ComputeSaveHash(const std::string& content) {
    static const std::string SAVE_HASH_SALT = "QuantumIdle::SaveSalt::v1";
    std::hash<std::string> hasher;
    size_t value = hasher(SAVE_HASH_SALT + content);

    std::ostringstream oss;
    oss << std::hex << value;
    return oss.str();
}

}


// Achievement implementation
Achievement::Achievement()
    : id(AchievementID::FirstQubit), unlocked(false),
      progress(0), target(1), rewardQubits(0), rewardPhotons(0) {
}

// GameStatistics implementation
GameStatistics::GameStatistics()
    : totalQubitsEarned(0), totalCoherenceEarned(0), totalEntanglementEarned(0),
      totalObservations(0), totalUpgrades(0), totalPrestigesPerformed(0),
      sessionQubits(0), sessionTime(0), sessionObservations(0),
      highestQubits(0), fastestPrestige(99999.0), longestStreak(0),
      currentStreak(0), lastLoginTimestamp(0) {
}

void GameStatistics::Reset() {
    sessionQubits = 0;
    sessionTime = 0;
    sessionObservations = 0;
}

void GameStatistics::UpdateSession(f64 deltaTime) {
    sessionTime += deltaTime;
}

// QuantumEvent implementation
QuantumEvent::QuantumEvent()
    : type(QuantumEventType::WaveCollapse), duration(30.0),
      timeRemaining(0), multiplier(2.0), active(false) {
}

// QuantumTimeline implementation
QuantumTimeline::QuantumTimeline()
    : completedResets(0), photons(0), photonBonus(1.0),
      completedCollapses(0), singularities(0) {
}

// ResearchStation implementation
ResearchStation::ResearchStation()
    : baseProduction(0), currentProduction(0), level(0),
      upgradeCost(0), upgradeCostMultiplier(1.15f),
      superpositionValue(0), superpositionProbability(0.5f),
      passiveCollapseRate(0.0),  // Phase 3.1: Default to 0 (no passive income)
      unlocked(false), unlockCost(0), autoUpgrade(false) {
}

void ResearchStation::Upgrade() {
    if (level == 0) {
        // First purchase - unlock
        unlocked = true;
        level = 1;
        currentProduction = baseProduction;
    } else {
        level++;
        currentProduction = baseProduction * level;
    }

    upgradeCost = upgradeCost * upgradeCostMultiplier;
}

void ResearchStation::Observe(GameState* state) {
    if (superpositionValue <= 0) return;

    // Quantum observation - collapse the wave function!
    f64 roll = static_cast<f64>(rand()) / RAND_MAX;
    f64 collapsedValue = superpositionValue;

    // Check for lucky observation event
    QuantumEvent* activeEvent = state->GetActiveEvent();
    f64 observeBonus = 1.0;
    if (activeEvent && activeEvent->type == QuantumEventType::LuckyObservation) {
        superpositionProbability = 1.0; // Guaranteed success
    } else if (activeEvent && activeEvent->type == QuantumEventType::WaveCollapse) {
        observeBonus = activeEvent->multiplier;
    }

    // Phase 4.1: Apply particle collection observation bonus
    f64 particleObservationBonus = state->GetParticleCollection().GetTotalObservationBonus();
    observeBonus *= (1.0 + particleObservationBonus);
    observeBonus *= (1.0 + state->GetParticleCollection().GetDiscoveryObservationBonus());

    if (roll < superpositionProbability) {
        // Success - full value
        collapsedValue *= 1.0 * observeBonus;
    } else {
        // Partial collapse
        collapsedValue *= (0.5 + roll * 0.5) * observeBonus;
    }

    // Critical Observation chance! (10% chance for 2-5x multiplier)
    f64 criticalRoll = static_cast<f64>(rand()) / RAND_MAX;
    bool isCritical = criticalRoll < 0.10;

    if (isCritical) {
        // Critical success! 2x to 5x multiplier
        f64 critMultiplier = 2.0 + (static_cast<f64>(rand() % 4)); // 2, 3, 4, or 5x
        collapsedValue *= critMultiplier;
        Log::Infof("CRITICAL OBSERVATION! ", critMultiplier, "x reward!");
        state->GetSoundManager().PlaySound(SoundEffect::ObserveCritical);
    } else {
        state->GetSoundManager().PlaySound(SoundEffect::ObserveSuccess);
    }

    state->AddResource(resourceType, collapsedValue);
    superpositionValue = 0;

    // Update statistics
    auto& stats = state->GetStatistics();
    stats.totalObservations++;
    stats.sessionObservations++;

    // Add combo point for observation
    state->AddComboPoint();

    // Award Observation skill XP
    state->GetSpecializedSkills().AddExperience(SkillCategory::Observation, SkillXP::OBSERVE_STATION);

    // Phase 4.1: Particle discovery chance! (0.5% chance on each observation)
    if (state->GetParticleCollection().TryFindParticle()) {
        // Particle discovered! TryFindParticle() handles logging and notifications
        state->GetSoundManager().PlaySound(SoundEffect::AchievementUnlock); // Celebratory sound
    }

    // Ship part drop chance! (20% base chance + bonus from ship's drop rate bonus)
    f64 partDropChance = 0.20; // 20% base chance
    f64 shipDropBonus = state->GetSpaceship().GetTotalDropRateBonus() / 100.0; // Convert % to decimal
    partDropChance += shipDropBonus;

    // Higher level stations have slightly better drop chances
    partDropChance += (level / 100.0) * 0.05; // +5% per 100 levels

    f64 partDropRoll = static_cast<f64>(rand()) / RAND_MAX;
    if (partDropRoll < partDropChance) {
        // Drop a ship part!
        ShipPart droppedPart = ShipPartGenerator::GenerateRandomPart();
        state->GetSpaceship().AddPart(droppedPart);

        Log::Infof("Ship part dropped: ", droppedPart.GetRarityName(), " ", droppedPart.name);
    }

    // Spawn particles for visual feedback (flying to resource counter)
    // Determine particle color based on resource type
    Color particleColor;
    if (resourceType == QuantumResource::Qubits) {
        particleColor = Color::QuantumBlue();
    } else if (resourceType == QuantumResource::Coherence) {
        particleColor = Color::CoherenceGreen();
    } else {
        particleColor = Color::EntanglementOrange();
    }

    // Spawn more particles for critical observations
    i32 particleCount = isCritical ? 20 : 5;

    // Note: Particles will be spawned from station position to resource counter
    // This requires access to station position, which we'll handle in the button onClick
    (void)particleColor; // Suppress unused warning - will be used when we have position
    (void)particleCount;
}

void ResearchStation::Update(f64 deltaTime) {
    if (!unlocked || level == 0) return;

    // Accumulate in superposition
    superpositionValue += currentProduction * deltaTime;

    // Phase 3.1: Passive collapse for early game smoothing
    // Automatically collapse a percentage of superposition every second
    if (passiveCollapseRate > 0.0 && superpositionValue > 0.0) {
        f64 amountToCollapse = superpositionValue * passiveCollapseRate * deltaTime;
        if (amountToCollapse > 0.0) {
            // Note: passive collapse is guaranteed (no RNG), but at reduced efficiency
            // This provides steady income without frustrating failures
            superpositionValue -= amountToCollapse;

            // The passive collapse will be added to resources via GameState
            // We'll need to pass this back to GameState, so we'll handle it there
            // For now, just reduce the superposition
        }
    }
}

// GameState implementation
GameState::GameState()
    : m_PlayerCredits(0),
      m_CreditProductionMultiplier(1.0),  // Phase 3.2: No bonus by default
      m_CreditConversionRate(100.0),      // Phase 3.2: 100 credits = 1% production
      m_MatterConverterBuffer(0.0),
      m_ExoticMaterials(0),               // Phase 3.3: Start with 0 exotic materials
      m_ResearchData(0),
      m_CurrentEvent(nullptr), m_TimeSinceLastEvent(0), m_EventCooldown(120.0),
      m_QuantumEssence(0),
      m_PlayerLevel(1),
      m_PlayerXP(0.0),
    m_LastSaveTimestamp(0),
    m_ActiveModal(ActiveModal::None),
    m_ShowAchievements(false), m_ShowStats(false), m_ShowResearch(false), m_ShowMilestones(false), m_ShowBuyables(false), m_ShowChallenges(false), m_ShowEssenceShop(false), m_ShowSingularityShop(false), m_ShowSpaceship(false), m_ShowCombat(false), m_ShowGatcha(false), m_ShowSkills(false), m_ShowEnhancement(false), m_ShowMoreMenu(false),
    m_NumberFormat(GameUtils::NumberFormat::Suffix),
    m_TotalTimePlayed(0), m_TimeSinceLastSave(0), m_TimeSinceLastPrestige(0),
      m_Coherence(100), m_MaxCoherence(100), m_CoherenceDecayRate(1.0),
      m_BoostActive(false), m_BoostTimeRemaining(0), m_BoostCooldownRemaining(0),
      m_BoostDuration(30.0), m_BoostCooldown(120.0), m_BoostMultiplier(2.0),
      m_AutoPrestigeThreshold(10.0),
      m_TimeSinceLastAnomaly(0), m_AnomalySpawnInterval(45.0),
      m_ComboCount(0), m_ComboTimeRemaining(0), m_ComboWindow(5.0),
      m_PrestigeFlashTimer(0), m_PrestigeFlashActive(false),
      m_LastThemeUnlocked(0) {

    for (int i = 0; i < 3; i++) {
        m_Resources[i] = 0;
    }

    m_Telemetry.StartSession();
    m_Particles.reserve(m_MaxActiveParticles);
    m_ParticlePool.reserve(m_MaxActiveParticles);
}

GameState::~GameState() {
}

void GameState::Initialize() {
    Log::Info("Initializing game state...");

    // GameState::Initialize
    m_UIManager = std::make_unique<UIManager>(this);
    m_UIManager->Initialize();

    // --- FIX START: Allocate ResearchTree object ---
    if (!m_ResearchTree) {
        // Allocate the ResearchTree object before using it
        m_ResearchTree = CreateScope<ResearchTree>();
    }

    InitializeStations();
    InitializeUI();

    // Initialize Achievements
    struct AchievementDef {
        AchievementID id;
        const char* name;
        const char* desc;
        f64 target;
        f64 rewardQ;
        f64 rewardP;
    };

    AchievementDef achDefs[] = {
        {AchievementID::FirstQubit, "First Steps", "Earn your first qubit", 1, 10, 0},
        {AchievementID::Observe100Times, "Observer", "Observe 100 times", 100, 100, 1},
        {AchievementID::Reach1Million, "Millionaire", "Reach 1 million qubits", 1000000, 5000, 5},
        {AchievementID::FirstPrestige, "Quantum Leap", "Perform your first prestige", 1, 0, 10},
        {AchievementID::TenStations, "Industrialist", "Own 10 research stations", 10, 1000, 2},
        {AchievementID::MaxCoherence, "Perfect Stability", "Reach maximum coherence", 1, 500, 1},
        {AchievementID::Entangle5Pairs, "Entangled", "Create 5 entanglement pairs", 5, 2000, 3},
        {AchievementID::OfflineMillionaire, "Passive Income", "Earn 1M qubits offline", 1000000, 10000, 5},
        {AchievementID::SpeedRunner, "Speed Runner", "Prestige within 10 minutes", 1, 5000, 10},
        {AchievementID::Hoarder, "Hoarder", "Save 100M qubits", 100000000, 50000, 20},
        {AchievementID::QuantumMaster, "Quantum Master", "Reach prestige level 10", 10, 100000, 50},
        {AchievementID::Collector, "Collector", "Unlock all station types", 5, 10000, 10},
        {AchievementID::EventHunter, "Event Hunter", "Experience 50 events", 50, 5000, 5},
        {AchievementID::WeekStreak, "Dedicated", "Play 7 days in a row", 7, 20000, 15},
        // Spaceship achievements
        {AchievementID::FirstShipPart, "Salvage Crew", "Acquire your first ship part", 1, 500, 1},
        {AchievementID::ShipOperational, "Flight Ready", "Repair ship to 25%", 1, 2000, 3},
        {AchievementID::ShipFullyRepaired, "Master Engineer", "Fully repair the ship", 1, 10000, 10},
        {AchievementID::FirstLegendaryPart, "Legendary Find", "Discover a legendary part", 1, 5000, 5},
        {AchievementID::PartCollector, "Junkyard King", "Collect 50 ship parts", 50, 20000, 15},
    };

    m_Achievements.clear();
    for (const auto& def : achDefs) {
        Achievement ach;
        ach.id = def.id;
        ach.name = def.name;
        ach.description = def.desc;
        ach.target = def.target;
        ach.rewardQubits = def.rewardQ;
        ach.rewardPhotons = def.rewardP;
        ach.unlocked = false;
        ach.progress = 0;
        m_Achievements.push_back(ach);
    }

    // Give starting resources
    m_Resources[static_cast<int>(QuantumResource::Qubits)] = 10.0;
    m_Resources[static_cast<int>(QuantumResource::Coherence)] = 50.0;
    m_Resources[static_cast<int>(QuantumResource::Entanglement)] = 0.0;

    // Initialize Research Tree
    m_ResearchTree->Initialize();
    Log::Info("Research tree initialized");

    // Initialize Milestone System
    m_MilestoneSystem.Initialize();
    Log::Info("Milestone system initialized");

    // Initialize Buyables System
    m_BuyableManager.Initialize(this);
    Log::Info("Buyables system initialized");

    // Initialize Challenge System
    m_ChallengeManager.Initialize();
    Log::Info("Challenge system initialized");

    // Initialize Essence Shop System
    m_EssenceShopManager.Initialize(this);
    Log::Info("Essence shop initialized");

    // Initialize Singularity Shop System
    m_SingularityShopManager.Initialize(this);
    Log::Info("Singularity shop initialized");

    // Initialize Spaceship System
    m_Spaceship.Initialize(this);
    Log::Info("Spaceship system initialized");

    // Try to load save file
    std::string savePath = Platform::GetSaveDirectory() + "quantum_save.json";
    if (Platform::FileExists(savePath)) {
        if (Load(savePath)) {
            Log::Info("Save file loaded successfully");
            CalculateOfflineProgress(); // Calculate what happened while away
        }
    }

    // Give starting currency for gatcha system (for testing)
    m_GatchaSystem.AddStellarShards(50); // Start with 50 shards for testing
    m_GatchaSystem.AddSummonTickets(5);  // Start with 5 tickets

    // Initialize skill tree system
    m_SkillTree.Initialize();

    // Initialize enhancement system
    m_EnhancementSystem.Initialize();

    // Initialize specialized skills system
    m_SpecializedSkills.Initialize();

    // Phase 4.1: Initialize particle collection system
    m_ParticleCollection.Initialize();
    Log::Info("Particle collection system initialized");

    // Initialize sound manager
    m_SoundManager.Initialize();
    if (m_SoundManager.IsAudioAvailable()) {
        m_SoundManager.PlayMusic(MusicTrack::MainTheme, true);
    }

    // Initialize feature unlock manager
    m_UnlockManager.Initialize();
    m_UnlockManager.CheckUnlocks(m_PlayerLevel);

    Log::Info("Game state initialized");
}

void GameState::InitializeStations() {
    // Station 1: Basic Qubit Generator
    // Rebalanced for gradual progression like Shark Incremental
    ResearchStation station1;
    station1.name = "Qubit Generator";
    station1.description = "Generates qubits in superposition";
    station1.resourceType = QuantumResource::Qubits;
    station1.baseProduction = 1.0;
    station1.currentProduction = 1.0; // Start producing immediately
    station1.level = 1; // Start at level 1 for immediate passive income
    station1.upgradeCost = 10.0;
    station1.upgradeCostMultiplier = 1.5; // Increased from 1.15 for better balance
    station1.superpositionProbability = 0.7;
    station1.passiveCollapseRate = 0.05; // Phase 3.1: 5% passive income per second (smooths early game)
    station1.unlocked = true; // First one is unlocked
    m_Stations.push_back(station1);

    // Station 2: Coherence Stabilizer
    ResearchStation station2;
    station2.name = "Coherence Stabilizer";
    station2.description = "Maintains quantum coherence";
    station2.resourceType = QuantumResource::Coherence;
    station2.baseProduction = 0.5;
    station2.level = 0;
    station2.upgradeCost = 50.0; // Increased from 25
    station2.upgradeCostMultiplier = 1.6; // Increased from 1.18
    station2.superpositionProbability = 0.8;
    station2.unlocked = false;
    station2.unlockCost = 100.0; // Increased from 50
    m_Stations.push_back(station2);

    // Station 3: Entanglement Chamber
    ResearchStation station3;
    station3.name = "Entanglement Chamber";
    station3.description = "Creates quantum entanglement";
    station3.resourceType = QuantumResource::Entanglement;
    station3.baseProduction = 0.2;
    station3.level = 0;
    station3.upgradeCost = 250.0; // Increased from 100
    station3.upgradeCostMultiplier = 1.7; // Increased from 1.2
    station3.superpositionProbability = 0.6;
    station3.unlocked = false;
    station3.unlockCost = 500.0; // Increased from 150
    m_Stations.push_back(station3);

    // Station 4: Advanced Qubit Synthesizer
    ResearchStation station4;
    station4.name = "Qubit Synthesizer";
    station4.description = "Advanced qubit generation";
    station4.resourceType = QuantumResource::Qubits;
    station4.baseProduction = 5.0;
    station4.level = 0;
    station4.upgradeCost = 1500.0; // Increased from 500
    station4.upgradeCostMultiplier = 1.8; // Increased from 1.25
    station4.superpositionProbability = 0.5;
    station4.unlocked = false;
    station4.unlockCost = 2000.0; // Increased from 300
    m_Stations.push_back(station4);

    // Station 5: Quantum Supercomputer
    ResearchStation station5;
    station5.name = "Quantum Supercomputer";
    station5.description = "Massive parallel processing";
    station5.resourceType = QuantumResource::Qubits;
    station5.baseProduction = 20.0;
    station5.level = 0;
    station5.upgradeCost = 10000.0; // Increased from 2000
    station5.upgradeCostMultiplier = 2.0; // Increased from 1.3
    station5.superpositionProbability = 0.4;
    station5.unlocked = false;
    station5.unlockCost = 15000.0; // Increased from 1000
    m_Stations.push_back(station5);

    // Phase 3.2: Station 6: Matter Converter (Combat Integration)
    // Special station that converts Combat Credits into production bonuses
    ResearchStation station6;
    station6.name = "Matter Converter";
    station6.description = "Converts combat spoils into quantum energy";
    station6.resourceType = QuantumResource::Qubits;  // Produces qubits from credits
    station6.baseProduction = 0.0;  // No automatic production
    station6.level = 0;
    station6.upgradeCost = 5000.0;
    station6.upgradeCostMultiplier = 2.2;
    station6.superpositionProbability = 1.0;  // Always succeeds
    station6.unlocked = false;
    station6.unlockCost = 7500.0;  // Unlocks at Tier 2
    m_Stations.push_back(station6);
}

void GameState::InitializeUI() {
    m_ScrollOffset = Vec2(0, 0);
    // Note: Button creation removed - buttons are now rendered inline in RenderStationsContent()
}

void GameState::Update(f64 deltaTime, Input* input, Renderer* renderer) {
    (void)renderer; // Unused parameter - reserved for future use

    m_TotalTimePlayed += deltaTime;
    m_TimeSinceLastSave += deltaTime;
    m_TimeSinceLastEvent += deltaTime;

    // Update feature unlock manager (handles notifications)
    m_UnlockManager.Update(deltaTime);

    // Phase 4.2: Update music system (handle cross-fading)
    m_SoundManager.UpdateMusic(deltaTime);

    // Phase 4.2: Dynamic music based on active modal
    UpdateDynamicMusic();

    // Update boost timers
    if (m_BoostActive) {
        m_BoostTimeRemaining -= deltaTime;
        if (m_BoostTimeRemaining <= 0) {
            m_BoostActive = false;
            m_BoostTimeRemaining = 0;
            m_BoostCooldownRemaining = m_BoostCooldown;
            Log::Info("Boost ended! Starting cooldown...");
        }
    } else if (m_BoostCooldownRemaining > 0) {
        m_BoostCooldownRemaining -= deltaTime;
        if (m_BoostCooldownRemaining < 0) {
            m_BoostCooldownRemaining = 0;
        }
    }

    // Update statistics
    m_Statistics.UpdateSession(deltaTime);
    m_Telemetry.AddSessionTime(deltaTime);

    // Track time for fastest prestige achievement
    m_TimeSinceLastPrestige += deltaTime;

    // Update stations
    UpdateStations(deltaTime);

    // Update spaceship expeditions
    m_Spaceship.Update(deltaTime);

    // Auto-research if enabled (automatically purchase research when affordable)
    auto availableResearch = m_ResearchTree->GetAvailableResearch(m_Timeline.completedResets); // CORRECT
    for (const ResearchNode* node : availableResearch) {
        if (node && node->autoResearch && !node->researched) {
            // Try to purchase this research (PurchaseResearch handles all checks)
            PurchaseResearch(node->id);
            // Note: Only one research per frame to avoid spending all resources at once
            break;
        }
    }

    // Passive photon generation from singularities
    f64 photonGenRate = m_SingularityShopManager.GetPhotonGenerationRate();
    if (photonGenRate > 0 && m_Timeline.singularities > 0) {
        // photonGenRate is the multiplier per singularity
        // Total generation = photonGenRate * singularities * deltaTime
        f64 photonsGained = photonGenRate * m_Timeline.singularities * deltaTime;
        AddPhotons(photonsGained);
    }

    // Auto-prestige if enabled and threshold reached
    if (m_ResearchTree->IsResearched(ResearchID::AutoPrestige)) {
        f64 photonsOnPrestige = CalculatePhotonsOnPrestige();
        if (photonsOnPrestige >= m_AutoPrestigeThreshold) {
            PerformPrestige();
            Log::Infof("Auto-prestige triggered at ", photonsOnPrestige, " photons");
        }
    }

    // Update coherence
    UpdateCoherence(deltaTime);

    // Update quantum events
    UpdateEvents(deltaTime);

    // Random event chance (every 2 minutes on average)
    if (m_TimeSinceLastEvent >= m_EventCooldown) {
        f64 eventChance = 0.3; // 30% chance when cooldown expires
        if (static_cast<f64>(rand()) / RAND_MAX < eventChance) {
            TriggerRandomEvent();
            m_TimeSinceLastEvent = 0;
        } else {
            m_TimeSinceLastEvent = m_EventCooldown * 0.8; // Retry sooner
        }
    }

    // Check achievements
    CheckAchievements();

    // Check milestones
    CheckMilestones();

    // Check for challenge completion
    m_ChallengeManager.CompleteChallenge(this);

    // Update visual effects systems
    UpdateQuantumAnomalies(deltaTime);
    UpdateParticles(deltaTime);

    // Update combat system
    if (m_CombatSystem.IsInCombat()) {
        m_CombatSystem.Update(deltaTime);

        // Check if combat ended (victory or defeat)
        if (m_CombatSystem.GetState() == CombatState::Victory ||
            m_CombatSystem.GetState() == CombatState::Defeat) {
            // Give time to see the result before ending
            static f64 combatEndTimer = 0.0;
            combatEndTimer += deltaTime;
            if (combatEndTimer >= 2.0) {
                EndCombat();
                combatEndTimer = 0.0;
            }
        }
    }

    // Update gatcha system
    m_GatchaSystem.Update(deltaTime);

    // Update combo timer
    if (m_ComboTimeRemaining > 0) {
        m_ComboTimeRemaining -= deltaTime;
        if (m_ComboTimeRemaining <= 0) {
            ResetCombo();
        }
    }

    // Update prestige flash effect
    if (m_PrestigeFlashActive) {
        m_PrestigeFlashTimer -= deltaTime;
        if (m_PrestigeFlashTimer <= 0) {
            m_PrestigeFlashActive = false;
        }
    }

    // Update GuiLayer (tutorial system, etc.)
    if (m_GuiLayer) {
        m_GuiLayer->Update(this);
    }

    // Update UI
    UpdateUI(input);

    // Auto-save every 10 seconds (reduced from 30 for more frequent saves)
    if (m_TimeSinceLastSave >= 10.0) {
        std::string savePath = Platform::GetSaveDirectory() + "quantum_save.json";
        if (Save(savePath)) {
            Log::Debug("Auto-save successful");
        }
        m_TimeSinceLastSave = 0;

#ifdef __EMSCRIPTEN__
        // For Emscripten, trigger IndexedDB sync after each save
        EM_ASM(
            FS.syncfs(false, function(err) {
                if (err) {
                    console.error('Error syncing to IndexedDB:', err);
                }
            });
        );
#endif
    }
}

void GameState::UpdateStations(f64 deltaTime) {
    // Apply prestige bonus
    f64 globalMultiplier = m_Timeline.photonBonus;

    // Apply boost multiplier if active
    if (m_BoostActive) {
        globalMultiplier *= m_BoostMultiplier;
    }

    // Apply challenge modifiers
    if (m_ChallengeManager.HasModifier(ChallengeModifier::HalfProduction)) {
        globalMultiplier *= 0.5; // Half production rate
    }
    if (m_ChallengeManager.HasModifier(ChallengeModifier::SlowTime)) {
        globalMultiplier *= 0.5; // Time runs at half speed (same effect)
    }

    // Apply skill tree production bonus
    globalMultiplier *= m_SkillTree.GetProductionMultiplier();

    // Phase 3.2: Apply credit conversion production bonus
    globalMultiplier *= m_CreditProductionMultiplier;

    // Check if Auto-Observer research is unlocked
    bool hasAutoObserver = m_ResearchTree->IsResearched(ResearchID::AutoObserver);

    f64 currentQubits = GetResource(QuantumResource::Qubits);
    bool canUpgrade = !m_ChallengeManager.HasModifier(ChallengeModifier::NoUpgrades);

    for (auto& station : m_Stations) {
        // Store superposition before update for passive collapse calculation
        f64 superpositionBefore = station.superpositionValue;

        station.Update(deltaTime * globalMultiplier);

        // Phase 3.1: Handle passive collapse rewards
        if (station.passiveCollapseRate > 0.0 && station.unlocked) {
            f64 superpositionAfter = station.superpositionValue;
            f64 collapsed = superpositionBefore - superpositionAfter;

            // If superposition decreased due to passive collapse (not just normal growth)
            if (collapsed > 0.0 && superpositionAfter < superpositionBefore) {
                // Award resources from passive collapse (no floating text to avoid spam)
                AddResource(station.resourceType, collapsed, false);
            }
        }

        // Matter Converter: automatically trade combat credits into production multipliers
        if (station.name == "Matter Converter" && station.level > 0) {
            m_MatterConverterBuffer += deltaTime * static_cast<f64>(station.level * 10);
            i32 creditsToConvert = std::min(static_cast<i32>(m_MatterConverterBuffer), m_PlayerCredits);
            if (creditsToConvert > 0) {
                m_MatterConverterBuffer -= creditsToConvert;
                ConvertCreditsToProduction(creditsToConvert);
            }
        }

        // Auto-observe if research is unlocked and superposition is high enough
        // Auto-observe still works even in NoObserve challenge (only manual is disabled)
        if (hasAutoObserver && station.unlocked && station.superpositionValue >= 10.0) {
            station.Observe(this);
        }

        // Auto-upgrade if enabled and can afford 10x the cost (prevents spending all resources)
        if (station.autoUpgrade && station.unlocked && canUpgrade) {
            // Calculate effective upgrade cost (with challenge modifiers)
            f64 effectiveCost = station.upgradeCost;
            if (m_ChallengeManager.HasModifier(ChallengeModifier::ExpensiveUpgrades)) {
                effectiveCost *= 3.0;
            }

            // Only auto-upgrade if we can afford 10x the cost (safety buffer)
            f64 safeThreshold = effectiveCost * 10.0;
            if (currentQubits >= safeThreshold) {
                if (SpendResource(QuantumResource::Qubits, effectiveCost)) {
                    station.Upgrade();
                    m_SpecializedSkills.AddExperience(SkillCategory::Engineering, SkillXP::UPGRADE_STATION);
                    currentQubits = GetResource(QuantumResource::Qubits); // Update current amount
                }
            }
        }
    }

    // Recalculate production if any auto-upgrades happened
    UpdateResearchBonuses();
}

void GameState::UpdateCoherence(f64 deltaTime) {
    // Check if coherence is disabled by challenge
    if (m_ChallengeManager.HasModifier(ChallengeModifier::NoCoherence)) {
        m_Coherence = 0;
        return;
    }

    // Coherence slowly decays
    m_Coherence -= m_CoherenceDecayRate * deltaTime;
    if (m_Coherence < 0) m_Coherence = 0;

    // Coherence affects production (applied in UpdateStations)
}

// Phase 4.2: Dynamic music system - change music based on active modal
void GameState::UpdateDynamicMusic() {
    if (!m_SoundManager.IsAudioAvailable()) return;

    // Don't switch music if we're currently fading
    if (m_SoundManager.IsFading()) return;

    // Determine which music track should be playing based on current modal
    MusicTrack targetTrack = MusicTrack::MainTheme;  // Default

    switch (m_ActiveModal) {
        case ActiveModal::None:
            // Idle/stations gameplay
            targetTrack = MusicTrack::MainTheme;
            break;

        case ActiveModal::Combat:
            // Combat mode
            targetTrack = MusicTrack::CombatTheme;
            break;

        case ActiveModal::Research:
        case ActiveModal::Skills:
        case ActiveModal::SpecializedSkills:
            // Research/skill trees
            targetTrack = MusicTrack::ResearchTheme;
            break;

        case ActiveModal::EssenceShop:
        case ActiveModal::SingularityShop:
        case ActiveModal::Buyables:
            // Shops
            targetTrack = MusicTrack::ShopTheme;
            break;

        case ActiveModal::Statistics:
        case ActiveModal::Achievements:
        case ActiveModal::Milestones:
        case ActiveModal::Collection:
            // Calm menus
            targetTrack = MusicTrack::AmbientCalm;
            break;

        default:
            // Keep current track for other modals
            targetTrack = m_SoundManager.GetCurrentTrack();
            break;
    }

    // Fade to new track if different from current
    if (targetTrack != m_SoundManager.GetCurrentTrack()) {
        m_SoundManager.FadeMusicTo(targetTrack, 2.0f);  // 2-second cross-fade
    }
}

void GameState::UpdateUI(Input* input) {
    Vec2 mousePos = input->GetMousePosition();
    bool mousePressed = input->IsMouseButtonPressed(MouseButton::Left);

    // ESC key to close any open modal (Highest priority)
    const int KEY_ESC = 41; // SDL_SCANCODE_ESCAPE
    if (input->IsKeyPressed(KEY_ESC)) {
        // Close any open modal and return to base state
        if (m_ActiveModal != ActiveModal::None) {
            SetActiveModal(ActiveModal::None);
        }
    }

    // --- REMOVED: Custom scrolling logic (m_ScrollOffset) is removed as RenderStations now uses native ImGui scrolling. ---

    // Phase 1.3: Get ImGui input capture state early
    ImGuiIO& io = ImGui::GetIO();
    bool uiCapturingMouse = io.WantCaptureMouse;

    // Handle unlock notification clicks (click to dismiss)
    // Note: These are rendered as ImGui windows, so this check is redundant but kept for clarity
    if (mousePressed && m_UnlockManager.HasActiveNotification()) {
        f32 screenWidth = static_cast<f32>(input->GetWindowWidth());
        f32 notifWidth = 400.0f;
        f32 notifHeight = 100.0f;
        f32 notifX = (screenWidth - notifWidth) * 0.5f;
        f32 notifY = 100.0f;

        Rect notifRect(notifX, notifY, notifWidth, notifHeight);
        if (notifRect.Contains(Vec2(mousePos.x, mousePos.y))) {
            m_UnlockManager.DismissNotification(0); // Dismiss the first (topmost) notification
            return; // Don't process other clicks this frame
        }
    }

    // --- Keyboard shortcuts - exclusive modal activation ---
    const int KEY_A = 4;
    const int KEY_B = 5;
    const int KEY_C = 6;
    const int KEY_E = 8;
    const int KEY_F = 9;
    const int KEY_H = 11;
    const int KEY_M = 13;
    const int KEY_R = 15;
    const int KEY_S = 16;

    if (input->IsKeyPressed(KEY_A)) {
        SetActiveModal(m_ActiveModal == ActiveModal::Achievements ? ActiveModal::None : ActiveModal::Achievements);
    }
    if (input->IsKeyPressed(KEY_S)) {
        SetActiveModal(m_ActiveModal == ActiveModal::Statistics ? ActiveModal::None : ActiveModal::Statistics);
    }
    if (input->IsKeyPressed(KEY_R)) {
        SetActiveModal(m_ActiveModal == ActiveModal::Research ? ActiveModal::None : ActiveModal::Research);
    }
    if (input->IsKeyPressed(KEY_M)) {
        SetActiveModal(m_ActiveModal == ActiveModal::Milestones ? ActiveModal::None : ActiveModal::Milestones);
    }
    if (input->IsKeyPressed(KEY_B)) {
        SetActiveModal(m_ActiveModal == ActiveModal::Buyables ? ActiveModal::None : ActiveModal::Buyables);
    }
    if (input->IsKeyPressed(KEY_C)) {
        SetActiveModal(m_ActiveModal == ActiveModal::Challenges ? ActiveModal::None : ActiveModal::Challenges);
    }
    if (input->IsKeyPressed(KEY_E)) {
        SetActiveModal(m_ActiveModal == ActiveModal::EssenceShop ? ActiveModal::None : ActiveModal::EssenceShop);
    }
    if (input->IsKeyPressed(KEY_H)) {
        SetActiveModal(m_ActiveModal == ActiveModal::Spaceship ? ActiveModal::None : ActiveModal::Spaceship);
    }
    if (input->IsKeyPressed(KEY_F)) {
        // Toggle number format between Suffix and Scientific
        m_NumberFormat = (m_NumberFormat == GameUtils::NumberFormat::Suffix)
            ? GameUtils::NumberFormat::Scientific
            : GameUtils::NumberFormat::Suffix;
        Log::Info("Number format toggled");
    }
    // The ESC check here is redundant due to the high-priority check above.
    /*
    if (input->IsKeyPressed(KEY_ESCAPE)) {
        // ... all flags set to false ...
    }
    */
    // --- REMOVED: Manual close button checks for popups (handled by ImGui) ---

    // Handle Quantum Anomaly clicks (active gameplay)
    // ONLY if UI is not capturing the mouse (prevents click-through on modals)
    if (mousePressed && !uiCapturingMouse) {
        ClickQuantumAnomaly(mousePos);
    }

    // Handle navigation bar button clicks (only if no popup consumed the click)
    // The following logic is left as it handles the non-ImGui button click areas
    // that determine which popup to open.

    if (mousePressed && !uiCapturingMouse) {
        f32 navY = 100.0f;
        f32 navHeight = 80.0f;
        f32 btnWidth = 180.0f;
        f32 btnHeight = 60.0f;
        f32 btnY = navY + (navHeight - btnHeight) * 0.5f;
        f32 spacing = 10.0f;
        f32 startX = 15.0f;

        bool handled = false; // Reset local handled flag for this block

        // Check each navigation button (new order: BUYABLES, CHALLENGES, ESSENCE, RESEARCH, STATS, MILESTONES)
        for (int i = 0; i < 6; i++) {
            f32 x = startX + i * (btnWidth + spacing);

            // Check if button would go off screen
            if (x + btnWidth > 1280.0f - 200.0f) {
                break;
            }

            Rect btnRect(x, btnY, btnWidth, btnHeight);

            if (btnRect.Contains(mousePos)) {
                // Toggle the corresponding panel (new order)
                if (i == 0) m_ShowBuyables = !m_ShowBuyables;
                else if (i == 1) m_ShowChallenges = !m_ShowChallenges;
                else if (i == 2) m_ShowEssenceShop = !m_ShowEssenceShop;
                else if (i == 3) m_ShowResearch = !m_ShowResearch;
                else if (i == 4) m_ShowStats = !m_ShowStats;
                else if (i == 5) m_ShowMilestones = !m_ShowMilestones;
                handled = true;
                break;  // Only handle one click per frame
            }
        }

        // Check MORE menu button
        if (!handled) {
            f32 boostBtnWidth = 200.0f;
            f32 moreBtnWidth = 80.0f;
            f32 moreBtnX = 1280.0f - boostBtnWidth - moreBtnWidth - 35.0f;
            Rect moreBtnRect(moreBtnX, btnY, moreBtnWidth, btnHeight);

            if (moreBtnRect.Contains(mousePos)) {
                m_ShowMoreMenu = !m_ShowMoreMenu;
                handled = true;
            }
        }

        // Handle MORE menu popup clicks
        if (!handled && m_ShowMoreMenu) {
            f32 menuWidth = 250.0f;
            f32 menuHeight = 490.0f;
            f32 moreBtnWidth = 80.0f;
            f32 boostBtnWidth = 200.0f;
            f32 moreBtnX = 1280.0f - boostBtnWidth - moreBtnWidth - 35.0f;
            f32 menuX = moreBtnX;
            f32 menuY = navY + navHeight + 5.0f;

            f32 itemHeight = 60.0f;
            f32 itemY = menuY + 10.0f;

            // Achievements button
            Rect achievementsRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
            if (achievementsRect.Contains(mousePos)) {
                m_ShowAchievements = !m_ShowAchievements;
                m_ShowMoreMenu = false; // Close menu after selection
                handled = true;
            }

            // Singularity button
            itemY += itemHeight + 10.0f;
            Rect singularityRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
            if (singularityRect.Contains(mousePos)) {
                m_ShowSingularityShop = !m_ShowSingularityShop;
                m_ShowMoreMenu = false; // Close menu after selection
                handled = true;
            }

            // Ship button
            itemY += itemHeight + 10.0f;
            Rect shipRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
            if (shipRect.Contains(mousePos)) {
                m_ShowSpaceship = !m_ShowSpaceship;
                m_ShowMoreMenu = false; // Close menu after selection
                handled = true;
            }

            // Battle button
            itemY += itemHeight + 10.0f;
            Rect battleRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
            if (battleRect.Contains(mousePos)) {
                // Start combat and show combat screen
                StartRandomCombat();
                m_ShowCombat = true;
                m_ShowMoreMenu = false; // Close menu after selection
                handled = true;
            }

            // Summon button
            itemY += itemHeight + 10.0f;
            Rect summonRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
            if (summonRect.Contains(mousePos)) {
                m_ShowGatcha = !m_ShowGatcha;
                m_ShowMoreMenu = false; // Close menu after selection
                handled = true;
            }

            // Skills button
            itemY += itemHeight + 10.0f;
            Rect skillsRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
            if (skillsRect.Contains(mousePos)) {
                m_ShowSkills = !m_ShowSkills;
                m_ShowMoreMenu = false; // Close menu after selection
                handled = true;
            }

            // Enhancement button
            itemY += itemHeight + 10.0f;
            Rect enhanceRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
            if (enhanceRect.Contains(mousePos)) {
                m_ShowEnhancement = !m_ShowEnhancement;
                m_ShowMoreMenu = false; // Close menu after selection
                handled = true;
            }

            // Click outside menu closes it
            Rect menuBg(menuX, menuY, menuWidth, menuHeight);
            if (!handled && !menuBg.Contains(mousePos)) {
                m_ShowMoreMenu = false;
            }
        }
    }

    // Handle boost button click
    if (mousePressed) {
        // Boost button (from RenderUI) - Updated to match new sizes
        f32 boostBtnWidth = 200.0f;
        f32 boostBtnHeight = 60.0f;
        f32 navY = 100.0f;
        f32 navHeight = 80.0f;
        f32 boostBtnX = 1280.0f - boostBtnWidth - 25.0f;
        f32 boostBtnY = navY + (navHeight - boostBtnHeight) * 0.5f;
        Rect boostBtnRect(boostBtnX, boostBtnY, boostBtnWidth, boostBtnHeight);

        // Check if boost is allowed (not disabled by challenge)
        bool canBoost = !m_ChallengeManager.HasModifier(ChallengeModifier::NoBoost);
        if (boostBtnRect.Contains(mousePos) && !m_BoostActive && m_BoostCooldownRemaining <= 0 && canBoost) {
            // Activate boost!
            m_BoostActive = true;
            m_BoostTimeRemaining = m_BoostDuration;
            Log::Infof("Boost activated! 2x production for ", m_BoostDuration, " seconds!");
        }
    }

    // Handle auto-upgrade toggle button clicks
    if (mousePressed) {
        f32 startY = 190.0f;
        f32 stationHeight = 180.0f;
        f32 margin = 25.0f;

        for (size_t i = 0; i < m_Stations.size(); i++) {
            auto& station = m_Stations[i];
            // Since we removed m_ScrollOffset, we need to rely on the correct ImGui positioning
            // However, since this logic is outside ImGui::Begin/End, we cannot rely on ImGui for positioning.
            // This is a known architectural mix that requires a full UI migration.
            // We will remove the m_ScrollOffset and keep the rest of the logic as the best effort manual click handler.

            if (!station.unlocked) continue; // Only unlocked stations have auto-upgrade toggle

            f32 y = startY + i * (stationHeight + margin); // Removed m_ScrollOffset.y

            // Skip if off-screen (basic approximation)
            if (y + stationHeight < 100.0f || y > 720.0f) continue;

            // Auto toggle button position (must match render position)
            f32 stationWidth = 1280.0f - 60.0f;
            f32 autoToggleSize = 60.0f;
            f32 autoToggleX = 30.0f + stationWidth - autoToggleSize - 10.0f;
            f32 autoToggleY = y + 10.0f;
            Rect autoToggleRect(autoToggleX, autoToggleY, autoToggleSize, 25.0f);

            if (autoToggleRect.Contains(mousePos)) {
                station.autoUpgrade = !station.autoUpgrade;
                Log::Infof(station.name, " auto-upgrade: ", station.autoUpgrade ? "ON" : "OFF");
                break; // Only handle one click per frame
            }
        }
    }

    // Note: Button updates removed - all buttons now handled by ImGui in RenderStationsContent()

    // Handle auto-prestige threshold adjustment button clicks
    if (mousePressed && m_ResearchTree->IsResearched(ResearchID::AutoPrestige)) {
        // Calculate button positions (must match RenderStations rendering)
        f32 stationHeight = 180.0f;
        f32 margin = 25.0f;
        f32 startY = 190.0f;
        // Removed m_ScrollOffset.y from prestigeY calculation
        f32 prestigeY = startY + m_Stations.size() * (stationHeight + margin) + 20.0f;
        f32 autoPrestigeY = prestigeY + 70.0f;

        f32 btnW = 50.0f;
        f32 btnH = 30.0f;
        f32 btnSpacing = 10.0f;
        f32 startX = 500.0f;

        Rect minusTenRect(startX, autoPrestigeY + 5.0f, btnW, btnH);
        Rect minusOneRect(startX + btnW + btnSpacing, autoPrestigeY + 5.0f, btnW, btnH);
        Rect plusOneRect(startX + (btnW + btnSpacing) * 2, autoPrestigeY + 5.0f, btnW, btnH);
        Rect plusTenRect(startX + (btnW + btnSpacing) * 3, autoPrestigeY + 5.0f, btnW, btnH);

        if (minusTenRect.Contains(mousePos)) {
            m_AutoPrestigeThreshold = std::max(1.0, m_AutoPrestigeThreshold - 10.0);
            Log::Infof("Auto-prestige threshold: ", m_AutoPrestigeThreshold);
        } else if (minusOneRect.Contains(mousePos)) {
            m_AutoPrestigeThreshold = std::max(1.0, m_AutoPrestigeThreshold - 1.0);
            Log::Infof("Auto-prestige threshold: ", m_AutoPrestigeThreshold);
        } else if (plusOneRect.Contains(mousePos)) {
            m_AutoPrestigeThreshold += 1.0;
            Log::Infof("Auto-prestige threshold: ", m_AutoPrestigeThreshold);
        } else if (plusTenRect.Contains(mousePos)) {
            m_AutoPrestigeThreshold += 10.0;
            Log::Infof("Auto-prestige threshold: ", m_AutoPrestigeThreshold);
        }
    }

    // Handle singularity collapse button clicks
    if (mousePressed) {
        // Calculate button position (must match RenderStations rendering)
        f32 stationHeight = 180.0f;
        f32 margin = 25.0f;
        f32 startY = 190.0f;
        // Removed m_ScrollOffset.y from prestigeY calculation
        f32 prestigeY = startY + m_Stations.size() * (stationHeight + margin) + 20.0f;

        f32 collapseY = prestigeY + 115.0f;
        if (!m_ResearchTree->IsResearched(ResearchID::AutoPrestige)) {
            collapseY = prestigeY + 70.0f;
        }

        Rect collapseBtn(30.0f, collapseY, static_cast<f32>(1280.0f) - 60.0f, 60.0f);

        if (collapseBtn.Contains(mousePos)) {
            if (CalculateSingularitiesOnCollapse() > 0) {
                PerformCollapse();
            } else {
                Log::Info("Not enough photons for collapse (need 10,000)");
            }
        }
    }

}


f64 GameState::GetProductionMultiplier(QuantumResource type) const {
    f64 multiplier = 1.0;

    // 1. Permanent Global Multipliers (Photons, Essence Shop)
    multiplier *= m_Timeline.photonBonus;
    multiplier *= m_EssenceShopManager.GetGlobalProductionBonus(); // Assuming this exists

    // 2. Milestone System Bonus (Permanent Production Bonus)
    multiplier *= (1.0 + m_MilestoneSystem.GetTotalProductionBonus());

    // 3. Research Tree (Resource-specific or Global)
    // Assuming m_ResearchTree has a method to get the bonus for a type
    multiplier *= m_ResearchTree->GetResourceBonus(type);

    // 4. Temporary Boost System
    if (m_BoostActive) {
        multiplier *= m_BoostMultiplier;
    }

    // 5. Quantum Event Modifiers
    if (m_CurrentEvent && m_CurrentEvent->active && m_CurrentEvent->type == QuantumEventType::TimeDialation) {
        multiplier *= 2.0; // Assuming TimeDialation gives 2x
    }

    // 6. Challenges (e.g., if a challenge halves production)
    // Example: If challenge manager tracked modifiers
    // if (m_ChallengeManager.IsProductionHalved()) {
    //     multiplier *= 0.5;
    // }

    // Phase 4.1: Particle Collection Production Bonus
    f64 particleBonus = m_ParticleCollection.GetTotalProductionBonus();
    multiplier *= (1.0 + particleBonus);

    // Discovery tree passive bonuses
    multiplier *= (1.0 + m_ParticleCollection.GetDiscoveryProductionBonus());

    // Challenge modifiers
    if (m_ChallengeManager.HasModifier(ChallengeModifier::HalfProduction)) {
        multiplier *= 0.5;
    }

    return multiplier;
}

// In src/game/GameState.cpp

void GameState::Render(Renderer* renderer) {
    (void)renderer; // We still pass the renderer, but rely on ImGui for drawing primitives

    // Get the draw list for background elements (behind all ImGui windows)
    ImDrawList* bg_draw_list = ImGui::GetBackgroundDrawList();

    // NEW IMGUI CODE (Fixes error and uses modern UI sizing):
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;
    f32 screenHeight = ImGui::GetIO().DisplaySize.y;

    // PHASE D: Dynamic background theme based on game progress
    Color bgTint(0.0f, 0.0f, 0.0f, 0.1f); // Default: subtle dark overlay

    if (m_Timeline.singularities >= 50.0) {
        // Cosmic Void theme (50+ singularities)
        bgTint = Color(0.05f, 0.0f, 0.15f, 0.15f); // Deep purple tint
    } else if (m_Timeline.singularities >= 10.0) {
        // Cosmic Observatory theme (10+ singularities)
        bgTint = Color(0.0f, 0.05f, 0.15f, 0.12f); // Deep blue tint
    } else if (m_Timeline.photons >= 100.0) {
        // Quantum Facility theme (100+ photons)
        bgTint = Color(0.0f, 0.1f, 0.1f, 0.1f); // Cyan tint
    }

    // Apply background tint (Replaces renderer->DrawRect)
    if (bgTint.a > 0.0f) {
        bg_draw_list->AddRectFilled(
            ImVec2(0, 0),
            ImVec2(screenWidth, screenHeight),
            ImGui::GetColorU32(ToImVec4(bgTint))
        );
    }

    // Render active gameplay elements (before popups)
    RenderQuantumAnomalies(renderer); // Clickable orbs

    // Render particles (visual feedback)
    for (const auto& particle : m_Particles) {
        f32 size = 3.0f + (1.0f - particle.lifetime / particle.maxLifetime) * 3.0f;
        Color color = particle.color;

        bg_draw_list->AddCircleFilled(
            ToImVec2(particle.position),
            size,
            ToImU32(color)
        );
    }

    // Render combo counter (top-right, above everything)
    if (m_ComboCount > 1) {
        std::string comboText = std::to_string(m_ComboCount) + "x COMBO!";
        ImVec2 comboPos(screenWidth - 150.0f, 180.0f);

        Color comboColor = (m_ComboCount >= 5) ? Color(1.0f, 0.8f, 0.0f, 1.0f) : // Gold for 5+
                          (m_ComboCount >= 3) ? Color(1.0f, 0.0f, 1.0f, 1.0f) : // Magenta for 3-4
                          Color::NeonCyan(); // Cyan for 2

        // Draw Combo Text
        bg_draw_list->AddText(comboPos, ToImU32(comboColor), comboText.c_str());

        // Time remaining bar
        f32 barWidth = 100.0f;
        f32 barHeight = 6.0f;
        ImVec2 barPos(screenWidth - 140.0f, 200.0f);
        ImVec2 barEnd(barPos.x + barWidth, barPos.y + barHeight);

        // Draw Background Rect
        bg_draw_list->AddRectFilled(barPos, barEnd, ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, 0.8f)));

        // Draw Fill Rect
        f32 timeRatio = static_cast<f32>(m_ComboTimeRemaining / m_ComboWindow);
        ImVec2 barFillEnd(barPos.x + barWidth * timeRatio, barEnd.y);
        bg_draw_list->AddRectFilled(barPos, barFillEnd, ToImU32(comboColor));
    }

    // --- RENDER IMGUI WINDOWS ---
    // Phase 1.1: GuiLayer now handles all UI rendering (decoupled from GameState logic)
    if (m_GuiLayer) {
        m_GuiLayer->Render(this, renderer);
    } else if (m_UIManager) {
        // Fallback to UIManager if GuiLayer not initialized
        m_UIManager->Render(renderer);
    } else {
        // Legacy fallback
        RenderResources(renderer);
        RenderUI(renderer);
    }

    // Prestige flash effect (screen overlay, on top of everything)
    if (m_PrestigeFlashActive) {
        f32 alpha = static_cast<f32>(m_PrestigeFlashTimer / 0.5); // Fade over 0.5 seconds
        alpha = std::min(alpha, 1.0f); // Clamp to max 1.0

        Color flashColor(1.0f, 1.0f, 1.0f, alpha * 0.3f); // White flash, max 30% opacity

        bg_draw_list->AddRectFilled(
            ImVec2(0, 0),
            ImVec2(screenWidth, screenHeight),
            ToImU32(flashColor)
        );
    }
}

void GameState::RenderResources(Renderer* renderer) {
    (void)renderer; // We don't use the legacy renderer object here anymore

    // --- ImGui Resources Panel ---

    // Define window properties: fixed top-left corner
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);

    // Use ImGui::GetIO().DisplaySize.x to get the current screen width
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 100.0f), ImGuiCond_Always);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

    // We start a window named "##ResourcesPanel" (## hides the title)
    if (ImGui::Begin("##ResourcesPanel", NULL, flags)) {

        // Define resource values
        f64 qubits = GetResource(QuantumResource::Qubits);
        f64 coherence = m_Coherence;
        f64 entanglement = GetResource(QuantumResource::Entanglement);
        f64 photons = m_Timeline.photons;
        i32 singularities = m_Timeline.singularities;

        // Use ImGui::Columns to align items
        ImGui::Columns(5, "ResourceColumns", false);

        // Use ImGui::GetWindowWidth() to correctly size columns relative to the current window
        f32 columnWidth = ImGui::GetWindowWidth() / 5.0f;

        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::SetColumnWidth(1, columnWidth);
        ImGui::SetColumnWidth(2, columnWidth);
        ImGui::SetColumnWidth(3, columnWidth);
        ImGui::SetColumnWidth(4, columnWidth);

        // 1. Qubits
        ImGui::Text("💰 Qubits");
        ImGui::Text("%.2s", GameUtils::FormatNumber(qubits, m_NumberFormat).c_str());
        ImGui::NextColumn();

        // 2. Coherence (with a progress bar)
        ImGui::Text("🧬 Coherence (%.0f%%)", coherence);
        char overlay[32];
        snprintf(overlay, sizeof(overlay), "%.0f / %.0f", coherence, m_MaxCoherence);
        ImGui::ProgressBar(coherence / m_MaxCoherence, ImVec2(-1, 0), overlay);
        ImGui::NextColumn();

        // 3. Entanglement
        ImGui::Text("🔗 Entanglement");
        ImGui::Text("%.2s", GameUtils::FormatNumber(entanglement, m_NumberFormat).c_str());
        ImGui::NextColumn();

        // 4. Photons
        ImGui::Text("⚡ Photons");
        ImGui::Text("%.2s", GameUtils::FormatNumber(photons, GameUtils::NumberFormat::Suffix).c_str());
        ImGui::NextColumn();

        // 5. Singularities
        ImGui::Text("⚫ Singularities");
        ImGui::Text("%d", singularities);
        ImGui::NextColumn();

        ImGui::Columns(1); // Stop columns
    }
    ImGui::End();
}

// In src/game/GameState.cpp (Around line 870)

// In src/game/GameState.cpp

void GameState::RenderStationsContent() {
    // NOTE: This function is the content rendering module for UIManager,
    // so it does NOT create its own window (ImGui::Begin/End are omitted).

    // We rely on ImGui's built-in scrollbar now.

    f64 currentQubits = GetResource(QuantumResource::Qubits);
    f64 effectiveBonus = GetProductionMultiplier(QuantumResource::Qubits);
    bool canUpgrade = !m_ChallengeManager.HasModifier(ChallengeModifier::NoUpgrades);
    bool canManuallyObserve = !m_ChallengeManager.HasModifier(ChallengeModifier::NoObserve);

    // Iterate through all stations
    for (size_t i = 0; i < m_Stations.size(); i++) {
        auto& station = m_Stations[i];

        // --- Station Display Header ---
        Color tierColor = GetStationTierColor(station.level);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(tierColor.r, tierColor.g, tierColor.b, 1.0f));
        ImGui::Text(">> %s (Level %d)", station.name.c_str(), station.level);
        ImGui::PopStyleColor();

        ImGui::TextWrapped("%s", station.description.c_str());
        ImGui::Separator();

        // --- UNLOCKED STATION UI ---
        if (station.unlocked) {
            f64 productionRate = station.currentProduction * effectiveBonus;

            // Superposition progress bar
            char barOverlay[64];
            f32 progress = 0.0f;
            if (station.upgradeCost > 0.0) {
                progress = static_cast<f32>(station.superpositionValue / (station.upgradeCost * 0.1));
                if (progress > 1.0f) progress = 1.0f;
            }

            snprintf(barOverlay, sizeof(barOverlay), "Superposition: %.2s (%.2s / sec)",
                     GameUtils::FormatNumber(station.superpositionValue, m_NumberFormat).c_str(),
                     GameUtils::FormatNumber(productionRate, m_NumberFormat).c_str());

            ImGui::ProgressBar(progress, ImVec2(-1, 0), barOverlay);

            // Button Row 1 (Observe, Upgrade, Buy Max)

            // OBSERVE Button - Purple
            Color observeColor = Color::QuantumPurple();
            if (!canManuallyObserve) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(observeColor.r * 0.7f, observeColor.g * 0.7f, observeColor.b * 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(observeColor.r, observeColor.g, observeColor.b, 1.0f));
            if (ImGui::Button(("OBSERVE##ObserveBtn" + std::to_string(i)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.30f, 40.0f)) && canManuallyObserve) {
                station.Observe(this);
                Log::Infof("Observed ", station.name);
            }
            ImGui::PopStyleColor(2);
            if (!canManuallyObserve) {
                ImGui::PopStyleVar();
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("Manual observation disabled during this challenge");
                }
            }

            // UPGRADE Button - Orange
            ImGui::SameLine();
            f64 effectiveUpgradeCost = station.upgradeCost * (m_ChallengeManager.HasModifier(ChallengeModifier::ExpensiveUpgrades) ? 3.0 : 1.0);
            bool canAffordUpgrade = currentQubits >= effectiveUpgradeCost && canUpgrade;
            if (!canAffordUpgrade) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

            Color upgradeColor = Color::EntanglementOrange();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(upgradeColor.r * 0.7f, upgradeColor.g * 0.7f, upgradeColor.b * 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(upgradeColor.r, upgradeColor.g, upgradeColor.b, 1.0f));

            std::string upgradeText = "Upgrade (" + GameUtils::FormatNumber(effectiveUpgradeCost, m_NumberFormat) + ")";

            if (ImGui::Button((upgradeText + "##UpgradeBtn" + std::to_string(i)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.45f, 40.0f)) && canAffordUpgrade) {
                f64 effectiveCost = station.upgradeCost;
                if (m_ChallengeManager.HasModifier(ChallengeModifier::ExpensiveUpgrades)) {
                    effectiveCost *= 3.0;
                }
                if (SpendResource(QuantumResource::Qubits, effectiveCost)) {
                    station.Upgrade();
                    UpdateResearchBonuses();
                    m_SpecializedSkills.AddExperience(SkillCategory::Engineering, SkillXP::UPGRADE_STATION);
                    m_SoundManager.PlaySound(SoundEffect::UpgradeComplete);
                    Log::Infof("Upgraded ", station.name, " to level ", station.level);
                }
            }
            ImGui::PopStyleColor(2);
            if (!canAffordUpgrade) ImGui::PopStyleVar();

            // BUY MAX Button - Green
            ImGui::SameLine();
            Color buyMaxColor = Color::CoherenceGreen();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(buyMaxColor.r * 0.7f, buyMaxColor.g * 0.7f, buyMaxColor.b * 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buyMaxColor.r, buyMaxColor.g, buyMaxColor.b, 1.0f));

            if (ImGui::Button(("BUY MAX##BuyMaxBtn" + std::to_string(i)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
                f64 qubits = GetResource(QuantumResource::Qubits);
                i32 upgradesBought = 0;
                bool expensiveUpgrades = m_ChallengeManager.HasModifier(ChallengeModifier::ExpensiveUpgrades);

                while (upgradesBought < 1000) {
                    f64 effectiveCost = station.upgradeCost;
                    if (expensiveUpgrades) effectiveCost *= 3.0;

                    if (qubits >= effectiveCost) {
                        if (SpendResource(QuantumResource::Qubits, effectiveCost)) {
                            station.Upgrade();
                            m_SpecializedSkills.AddExperience(SkillCategory::Engineering, SkillXP::UPGRADE_STATION);
                            qubits = GetResource(QuantumResource::Qubits);
                            upgradesBought++;
                        } else {
                            break;
                        }
                    } else {
                        break;
                    }
                }

                if (upgradesBought > 0) {
                    UpdateResearchBonuses();
                    Log::Infof("Bought ", upgradesBought, " upgrades for ", station.name, " (now level ", station.level, ")");
                }
            }
            ImGui::PopStyleColor(2);

            // Auto-Upgrade Toggle
            ImGui::Checkbox(("Auto-Upgrade##" + std::to_string(i)).c_str(), &station.autoUpgrade);

        }
        // --- LOCKED STATION UI ---
        else {
            // UNLOCK Button - Blue
            bool canAffordUnlock = currentQubits >= station.unlockCost;
            if (!canAffordUnlock) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

            Color unlockColor = Color::QuantumBlue();
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(unlockColor.r * 0.7f, unlockColor.g * 0.7f, unlockColor.b * 0.7f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(unlockColor.r, unlockColor.g, unlockColor.b, 1.0f));

            std::string unlockText = "Unlock for " + GameUtils::FormatNumber(station.unlockCost, m_NumberFormat) + " Qubits";
            if (ImGui::Button((unlockText + "##UnlockBtn" + std::to_string(i)).c_str(), ImVec2(-1, 50.0f)) && canAffordUnlock) {
                if (SpendResource(QuantumResource::Qubits, station.unlockCost)) {
                    station.unlocked = true;
                    station.level = 0;
                    Log::Infof("Unlocked: ", station.name);
                }
            }
            ImGui::PopStyleColor(2);
            if (!canAffordUnlock) ImGui::PopStyleVar();
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

    } // End station loop

    // --- Prestige Button (Last button in the list) ---
    f32 photonsToGain = CalculatePhotonsOnPrestige();
    ImGui::Spacing();
    ImGui::Spacing();

    // Set button color and text for Prestige
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.5f, 0.8f)); // Magenta
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 1.0f, 1.0f));

    std::string prestigeText = "PERFORM PRESTIGE (" + std::to_string(static_cast<i32>(photonsToGain)) + " Photons)";
    if (photonsToGain <= 0.0) {
        prestigeText = "PRESTIGE (Need more progress)";
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f);
    }

    // Prestige button
    if (ImGui::Button(prestigeText.c_str(), ImVec2(-1, 80.0f)) && photonsToGain > 0.0) {
        PerformPrestige();
    }

    if (photonsToGain <= 0.0) {
        ImGui::PopStyleVar();
    }

    ImGui::PopStyleColor(2);
}

void GameState::RenderUI(Renderer* renderer) {
    // 1. Setup the main Navigation Bar ImGui window

    f32 navY = 100.0f;
    f32 navHeight = 80.0f;
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;

    // Set position and size for the fixed navigation bar
    ImGui::SetNextWindowPos(ImVec2(0, navY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenWidth, navHeight), ImGuiCond_Always);

    // Style the navigation bar to match the original look
    Color darkPanel = Color::DarkPanel();
    Color neonCyan = Color::NeonCyan();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(darkPanel));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    // DECLARE moreBtnPos HERE (outside the ImGui::Begin block)
    ImVec2 moreBtnPos = ImVec2(0.0f, 0.0f);

    if (ImGui::Begin("##NavigationBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Draw the Glowing Cyan Bottom Border ---
        ImVec2 barStart = ImGui::GetWindowPos();

        ImVec2 barEnd(
        ImGui::GetWindowPos().x + ImGui::GetWindowSize().x,
        ImGui::GetWindowPos().y + ImGui::GetWindowSize().y
        );
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Draw the glowing cyan bottom border at the bottom of the window
        draw_list->AddRectFilled(
            ImVec2(barStart.x, barEnd.y - 2.0f),
            ImVec2(barEnd.x, barEnd.y),
            ImGui::GetColorU32(ToImVec4(neonCyan * 0.6f))
        );

        // --- Navigation Buttons (Main Row) ---
        f32 btnWidth = 180.0f;
        f32 btnHeight = 60.0f;
        f32 spacing = 10.0f;
        f32 currentX = 15.0f;

        // Calculate Y position to center the buttons vertically within the 80px bar
        f32 btnY = (navHeight - btnHeight) * 0.5f;

        // Use a structure for buttons as in the original code
        struct NavButton {
            const char* label;
            bool* showFlag;
            Color color;
        };

        NavButton navButtons[] = {
            {"BUYABLES", &m_ShowBuyables, Color::ElectricBlue()},
            {"CHALLENGES", &m_ShowChallenges, Color::Red()},
            {"ESSENCE", &m_ShowEssenceShop, Color::Magenta()},
            {"RESEARCH", &m_ShowResearch, Color::QuantumPurple()},
            {"STATS", &m_ShowStats, Color::EntanglementOrange()},
            {"MILESTONES", &m_ShowMilestones, Color::NeonPink()},
        };

        // Reset cursor to the correct starting Y position
        ImGui::SetCursorPos(ImVec2(currentX, btnY));

        for (int i = 0; i < 6; i++) {
            auto& btn = navButtons[i];

            // Check if button fits (matching original logic)
            if (currentX + btnWidth + spacing + 200.0f + 80.0f + 35.0f > screenWidth) {
                break; // Stop if it clashes with MORE or BOOST button area
            }

            bool active = *btn.showFlag;
            Color btnColor = active ? btn.color : btn.color * 0.5f;

            // Apply custom styles for the button
            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.25f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnColor * 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(btnColor * 0.7f));
            ImGui::PushStyleColor(ImGuiCol_Text, ToImVec4(Color::White()));

            // Custom border drawing using ImGui's draw list to replicate glow
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); // Disable default border

            // Draw the button
            if (ImGui::Button(btn.label, ImVec2(btnWidth, btnHeight))) {
                // Simple toggle for demonstration:
                *btn.showFlag = !active;
            }

            // Draw custom border/glow
            if (active) {
                // Active - bright glow
                ImVec2 rectMin(
                ImGui::GetItemRectMin().x - 2.0f,
                ImGui::GetItemRectMin().y - 2.0f
                );

                ImVec2 rectMax(
                    ImGui::GetItemRectMax().x + 2.0f,
                    ImGui::GetItemRectMax().y + 2.0f
                );
                draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(btn.color * 0.9f)), 0.0f, 0, 2.0f); // 2px thickness
            } else {
                // Inactive - subtle border
                ImVec2 rectMin = ImGui::GetItemRectMin();
                ImVec2 rectMax = ImGui::GetItemRectMax();
                draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(Color::DarkBorder())), 0.0f, 0, 1.0f);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);

            // Advance cursor for next button using SameLine with spacing only for the following buttons
            if (i < 5) {
                 ImGui::SameLine(0.0f, spacing);
            }
        }

        // --- MORE Menu Button (Hamburger) ---
        f32 boostBtnWidth = 200.0f;
        f32 moreBtnWidth = 80.0f;
        f32 moreBtnX = screenWidth - boostBtnWidth - moreBtnWidth - 35.0f;

        // Position for the MORE button using SetCursorPosX
        ImGui::SetCursorPosX(moreBtnX);
        ImGui::SetCursorPosY(btnY); // Keep the correct vertical position

        Color moreColor = m_ShowMoreMenu ? Color::NeonCyan() : Color(0.3f, 0.4f, 0.5f, 1.0f);
        Color moreBgColor = m_ShowMoreMenu ? (moreColor * 0.4f) : (moreColor * 0.35f);

        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(moreBgColor));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(moreColor * 0.6f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(moreColor * 0.8f));
        ImGui::PushStyleColor(ImGuiCol_Text, ToImVec4(Color::Transparent()));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); // Disable default border

        if (ImGui::Button("##MoreMenuBtn", ImVec2(moreBtnWidth, btnHeight))) {
            m_ShowMoreMenu = !m_ShowMoreMenu; // Toggle the menu
        }

        // Draw custom border/glow
        if (m_ShowMoreMenu) {
            // Corrected code using component access:
            ImVec2 rectMin(
                ImGui::GetItemRectMin().x - 2.0f,
                ImGui::GetItemRectMin().y - 2.0f
            );

            ImVec2 rectMax(
                ImGui::GetItemRectMax().x + 2.0f,
                ImGui::GetItemRectMax().y + 2.0f
            );

            draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(Color::NeonCyan() * 0.9f)), 0.0f, 0, 2.0f);
        } else {
            ImVec2 rectMin = ImGui::GetItemRectMin();
            ImVec2 rectMax = ImGui::GetItemRectMax();
            draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(Color(0.4f, 0.5f, 0.6f, 0.8f))), 0.0f, 0, 1.0f);
        }

        // Draw Hamburger Icon (three lines)
        ImVec2 btnMin = ImGui::GetItemRectMin();
        f32 lineWidth = 30.0f;
        f32 lineHeight = 3.0f;
        f32 lineSpacing = 8.0f;
        f32 lineStartX = btnMin.x + (moreBtnWidth - lineWidth) * 0.5f;
        f32 lineStartY = btnMin.y + (btnHeight - (lineHeight * 3 + lineSpacing * 2)) * 0.5f;
        Color lineColor = m_ShowMoreMenu ? Color::NeonCyan() : Color(0.8f, 0.9f, 1.0f, 1.0f);

        for (int i = 0; i < 3; i++) {
            draw_list->AddRectFilled(
                ImVec2(lineStartX, lineStartY + i * (lineHeight + lineSpacing)),
                ImVec2(lineStartX + lineWidth, lineStartY + i * (lineHeight + lineSpacing) + lineHeight),
                ImGui::GetColorU32(ToImVec4(lineColor))
            );
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        // Save MORE button position for the popup
        moreBtnPos = ImGui::GetItemRectMin();

        // --- Boost Button ---
        f32 boostBtnX = screenWidth - boostBtnWidth - 25.0f;
        ImGui::SetCursorPosX(boostBtnX); // Absolute X positioning
        ImGui::SetCursorPosY(btnY); // Keep the correct vertical position

        // Determine boost button state and text
        Color boostColor;
        std::string boostText;
        bool boostClickable = false;
        bool boostDisabledByChallenge = m_ChallengeManager.HasModifier(ChallengeModifier::NoBoost);

        if (boostDisabledByChallenge) {
            boostColor = Color(0.3f, 0.3f, 0.3f, 1.0f);
            boostText = "BOOST DISABLED";
        } else if (m_BoostActive) {
            boostColor = Color::CoherenceGreen();
            boostText = "BOOST ACTIVE! " + std::to_string(static_cast<i32>(m_BoostTimeRemaining)) + "s";
        } else if (m_BoostCooldownRemaining > 0) {
            boostColor = Color(0.4f, 0.4f, 0.4f, 1.0f);
            boostText = "COOLDOWN " + std::to_string(static_cast<i32>(m_BoostCooldownRemaining)) + "s";
        } else {
            boostColor = Color::NeonCyan();
            boostText = "BOOST (2x)";
            boostClickable = true;
        }

        // Push styles for the Boost button
        Color boostBgColor = boostColor * 0.3f;
        Color boostBorderColor = m_BoostActive ? boostColor * 0.9f : (boostClickable ? boostColor * 0.7f : Color(0.3f, 0.3f, 0.3f, 1.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(boostBgColor));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(boostColor * 0.5f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(boostColor * 0.7f));
        ImGui::PushStyleColor(ImGuiCol_Text, ToImVec4(Color::White()));

        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f); // Disable default border

        // Draw boost button
        if (ImGui::Button(boostText.c_str(), ImVec2(boostBtnWidth, btnHeight))) {
            if (boostClickable) {
                // Activate the boost
                m_BoostActive = true;
                m_BoostTimeRemaining = m_BoostDuration;
            }
        }

        // ... (Boost button border/glow logic remains) ...
        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectMax = ImGui::GetItemRectMax();

        if (m_BoostActive) {
            // Pulsing glow when active
            ImVec2 glowMin(
                rectMin.x - 2.0f,
                rectMax.y - 2.0f
            );

            ImVec2 glowMax(
                rectMin.x + 2.0f,
                rectMax.y + 2.0f
            );

            draw_list->AddRect(glowMin, glowMax, ImGui::GetColorU32(ToImVec4(boostColor * 0.9f)), 0.0f, 0, 2.0f);
        } else {
            draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(boostBorderColor)), 0.0f, 0, 1.0f);
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        // --- FIX: Use Dummy to force window boundary extension ---
        // CRITICAL FIX: Forces the window to register its full height (80px), resolving the assertion.
        ImGui::SetCursorPosY(navHeight);
        ImGui::Dummy(ImVec2(1.0f, 1.0f));

        // --- Scroll Indicator (Removed from this function for cleanliness, drawing in GameState::Render) ---

    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(1);

    // 2. Render the MORE Menu Popup (Must be called outside the main window)
    // 2. Render the MORE Menu Popup (Must be called outside the main window)
    if (m_ShowMoreMenu) {

        f32 menuWidth = 250.0f;
        f32 menuHeight = 490.0f;
        f32 navY = 100.0f;
        f32 navHeight = 80.0f;

        // --- FIX: Repositioning Logic ---
        // Instead of calculating menuPos using navY + navHeight + 5.0f (which puts it directly
        // over the station list), we position it relative to the 'MORE' button's X coordinate
        // (moreBtnPos.x) but starting vertically immediately below the nav bar.

        // Original menuPos (which caused overlap):
        // ImVec2 menuPos(moreBtnPos.x, navY + navHeight + 5.0f);

        // Reposition the menu to the top right, directly under the nav bar.
        // Since the whole nav bar is fixed at the top, the issue is that the popup
        // content overlaps the main game area below the navigation bar (starting at y=180).

        // We restore the original, correct positioning logic which should be fine
        // as the popup should float above the scrollable area below it.
        // The overlap occurs because the elements below it (stations) start immediately.

        // We will make the menu align its top edge to the bottom of the navigation bar.
        ImVec2 menuPos(moreBtnPos.x, navY + navHeight + 5.0f); // Restore original logic

        ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight), ImGuiCond_Always);

        // Style the popup
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(Color::DarkPanel()));
        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::NeonCyan() * 0.8f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

        if (ImGui::Begin("##MoreMenuPopup", &m_ShowMoreMenu, // Passing flag for close on outside click
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {

            // Button list setup (remains unchanged)
            f32 itemWidth = menuWidth - 20.0f;
            f32 itemHeight = 60.0f;

            struct MoreButton {
                const char* label;
                bool* showFlag;
                Color color;
            };

            MoreButton moreButtons[] = {
                {"ACHIEVEMENTS", &m_ShowAchievements, Color::ElectricBlue()},
                {"SINGULARITY", &m_ShowSingularityShop, Color(0.5f, 0.0f, 1.0f, 1.0f)},
                {"SPACESHIP", &m_ShowSpaceship, Color(1.0f, 0.7f, 0.0f, 1.0f)},
                {"BATTLE", &m_ShowCombat, Color(1.0f, 0.3f, 0.3f, 1.0f)},
                {"SUMMON", &m_ShowGatcha, Color(1.0f, 0.3f, 1.0f, 1.0f)},
                {"SKILLS", &m_ShowSkills, Color(0.0f, 1.0f, 0.5f, 1.0f)},
                {"ENHANCE", &m_ShowEnhancement, Color(0.8f, 0.6f, 0.2f, 1.0f)},
            };

            for (size_t i = 0; i < 7; i++) {
                auto& btn = moreButtons[i];
                bool active = *btn.showFlag;
                Color btnColor = active ? btn.color : Color(0.3f, 0.3f, 0.3f, 1.0f);

                // Apply custom styles for the button
                ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnColor * 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(btnColor * 0.7f));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnColor));

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

                if (ImGui::Button(btn.label, ImVec2(itemWidth, itemHeight))) {
                    // Toggle the flag and close the menu
                    *btn.showFlag = !active;
                    m_ShowMoreMenu = false;
                }

                ImGui::PopStyleVar();
                ImGui::PopStyleColor(4);

                if (i < 6) ImGui::Spacing();
            }

        }
        ImGui::End();

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }
}

void GameState::AddResource(QuantumResource type, f64 amount, bool showFloatingText) {
    m_Resources[static_cast<int>(type)] += amount;

    // Track statistics
    switch (type) {
        case QuantumResource::Qubits:
            m_Statistics.totalQubitsEarned += amount;
            m_Statistics.sessionQubits += amount;
            if (m_Resources[0] > m_Statistics.highestQubits) {
                m_Statistics.highestQubits = m_Resources[0];
            }
            break;
        case QuantumResource::Coherence:
            m_Statistics.totalCoherenceEarned += amount;
            break;
        case QuantumResource::Entanglement:
            m_Statistics.totalEntanglementEarned += amount;
            break;
    }

    // Spawn floating text for visual feedback
    if (showFloatingText && m_GuiLayer && m_GuiLayer->GetFloatingTextManager()) {
        // Get screen center as default position
        ImGuiIO& io = ImGui::GetIO();
        Vec2 position(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.4f);

        // Spawn the text
        m_GuiLayer->GetFloatingTextManager()->SpawnResourceText(type, amount, position, m_NumberFormat);
    }
}

void GameState::RegisterUIButtonFeedback(const std::string& label, const ImVec2& screenPos, const ImVec4& color) {
    // Audio feedback
    m_SoundManager.PlaySound(SoundEffect::ButtonPress, 0.85f);

    // Visual feedback via floating text
    if (m_GuiLayer && m_GuiLayer->GetFloatingTextManager()) {
        Vec2 pos(screenPos.x, screenPos.y);
        m_GuiLayer->GetFloatingTextManager()->SpawnText(label, pos, color, 1.25f);
    }

    // Hook for future particle/button effects - keeps place for particle bursts
    m_ParticleCollection.CheckForDiscoveries(0.0);
}

bool GameState::SpendResource(QuantumResource type, f64 amount) {
    int idx = static_cast<int>(type);
    if (m_Resources[idx] >= amount) {
        m_Resources[idx] -= amount;
        return true;
    }
    return false;
}

f64 GameState::GetResource(QuantumResource type) const {
    return m_Resources[static_cast<int>(type)];
}

i32 GameState::GetPlayerCredits() {
    // Note: Use 'const' if the declaration in GameState.h uses it.
    // Assuming m_PlayerCredits is the private member:
    return m_PlayerCredits;
}

void GameState::AddPlayerCredits(i32 amount) {
    if (amount <= 0) return;
    m_PlayerCredits += amount;

    if (m_GuiLayer && m_GuiLayer->GetFloatingTextManager()) {
        ImGuiIO& io = ImGui::GetIO();
        Vec2 position(io.DisplaySize.x * 0.6f, io.DisplaySize.y * 0.25f);
        ImVec4 color(0.8f, 0.8f, 0.2f, 1.0f);
        std::string text = "+" + std::to_string(amount) + " Credits";
        m_GuiLayer->GetFloatingTextManager()->SpawnText(text, position, color, 1.8f);
    }
}

void GameState::AddEssence(f64 amount) {
    m_QuantumEssence += amount;
    Log::Infof("Gained ", static_cast<i32>(amount), " Quantum Essence! Total: ", static_cast<i32>(m_QuantumEssence));
}

bool GameState::SpendEssence(f64 amount) {
    if (m_QuantumEssence >= amount) {
        m_QuantumEssence -= amount;
        return true;
    }
    return false;
}

// Modal Window Management - ensures only one modal is active at a time
void GameState::SetActiveModal(ActiveModal modal) {
    m_ActiveModal = modal;

    // Synchronize legacy bool flags
    m_ShowAchievements = (modal == ActiveModal::Achievements);
    m_ShowStats = (modal == ActiveModal::Statistics);
    m_ShowResearch = (modal == ActiveModal::Research);
    m_ShowMilestones = (modal == ActiveModal::Milestones);
    m_ShowBuyables = (modal == ActiveModal::Buyables);
    m_ShowChallenges = (modal == ActiveModal::Challenges);
    m_ShowEssenceShop = (modal == ActiveModal::EssenceShop);
    m_ShowSingularityShop = (modal == ActiveModal::SingularityShop);
    m_ShowSpaceship = (modal == ActiveModal::Spaceship);
    m_ShowCombat = (modal == ActiveModal::Combat);
    m_ShowGatcha = (modal == ActiveModal::Gatcha);
    m_ShowSkills = (modal == ActiveModal::Skills);
    m_ShowEnhancement = (modal == ActiveModal::Enhancement);
    m_ShowMoreMenu = (modal == ActiveModal::MoreMenu);
}

f64 GameState::CalculatePhotonsOnPrestige() const {
    // Photons based on total qubits earned
    // Formula adjusted for better progression balance
    f64 totalQubits = m_Resources[0];
    if (totalQubits < 5000.0) return 0; // Increased from 1000 to slow early game

    // Reduced photon gain for more gradual progression
    return std::floor(std::sqrt(totalQubits / 500.0)); // Increased divisor from 100 to 500
}

void GameState::PerformPrestige() {
    f64 photons = CalculatePhotonsOnPrestige();
    if (photons <= 0) {
        Log::Warning("Not enough progress for prestige");
        return;
    }

    // Apply photon doubling if purchased from essence shop
    auto* photonDouble = m_EssenceShopManager.GetUpgrade("photon_double");
    if (photonDouble && photonDouble->timesPurchased > 0) {
        photons *= 2.0;
    }

    Log::Infof("Performing prestige! Gained ", photons, " photons");

    // TRACK FASTEST PRESTIGE: Update achievement tracking
    if (m_TimeSinceLastPrestige < m_Statistics.fastestPrestige) {
        m_Statistics.fastestPrestige = m_TimeSinceLastPrestige;
        Log::Infof("New fastest prestige record: ", GameUtils::FormatTime(m_TimeSinceLastPrestige));
    }
    m_Statistics.totalPrestigesPerformed++;
    m_Telemetry.RecordPrestige(m_TimeSinceLastPrestige, m_Timeline.completedResets + 1);
    m_TimeSinceLastPrestige = 0.0; // Reset timer for next run

    // VISUAL EFFECTS: Screen flash and particle explosion!
    m_PrestigeFlashActive = true;
    m_PrestigeFlashTimer = 0.5; // 0.5 second flash

    // Particle explosion from center of screen
    Vec2 centerPos(640.0f, 360.0f);
    SpawnParticleBurst(centerPos, Color::Magenta(), 50);
    SpawnParticleBurst(centerPos, Color(1.0f, 0.8f, 0.0f, 1.0f), 30); // Gold particles too

    m_Timeline.photons += photons;
    m_Timeline.completedResets++;
    m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1); // 10% per photon

    // Reset resources
    for (int i = 0; i < 3; i++) {
        m_Resources[i] = 0;
    }

    // Start with base qubits plus essence shop bonus
    f64 startingQubits = 10.0 + m_EssenceShopManager.GetStartingQubits();
    m_Resources[0] = startingQubits;

    // Reset stations
    for (auto& station : m_Stations) {
        if (station.name != "Qubit Generator") {
            station.unlocked = false;
        }
        station.level = 0;
        station.currentProduction = 0;
        station.superpositionValue = 0;
        station.upgradeCost = station.upgradeCostMultiplier; // Reset cost
    }

    m_Coherence = m_MaxCoherence;
}

f64 GameState::CalculateSingularitiesOnCollapse() const {
    // Formula: singularities = sqrt(photons) / 100
    // Minimum requirement: 10,000 photons
    if (m_Timeline.photons < 10000.0) {
        return 0.0;
    }

    return std::sqrt(m_Timeline.photons) / 100.0;
}

void GameState::PerformCollapse() {
    f64 singularities = CalculateSingularitiesOnCollapse();
    if (singularities <= 0) {
        Log::Warning("Not enough photons for singularity collapse (need 10,000)");
        return;
    }

    Log::Infof("Performing singularity collapse! Gained ", singularities, " singularities");

    m_Timeline.singularities += singularities;
    m_Timeline.completedCollapses++;

    // Reset photon layer
    m_Timeline.photons = 0;
    m_Timeline.photonBonus = 1.0;
    m_Timeline.completedResets = 0;

    // Reset resources
    for (int i = 0; i < 3; i++) {
        m_Resources[i] = 0;
    }

    // Start with base qubits plus essence shop bonus
    f64 startingQubits = 10.0 + m_EssenceShopManager.GetStartingQubits();
    m_Resources[0] = startingQubits;

    // Reset stations
    for (auto& station : m_Stations) {
        if (station.name != "Qubit Generator") {
            station.unlocked = false;
        }
        station.level = 0;
        station.currentProduction = 0;
        station.superpositionValue = 0;
        station.upgradeCost = station.upgradeCostMultiplier; // Reset cost
    }

    m_Coherence = m_MaxCoherence;
}

bool GameState::Save(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Log::Errorf("Failed to open save file: ", filepath);
        return false;
    }

    // Save timestamp for offline progress
    m_LastSaveTimestamp = static_cast<i64>(Platform::GetTime());

    std::ostringstream body;

    // Tutorial progress
    i32 tutorialStep = 0;
    bool tutorialCompleted = false;
    if (m_GuiLayer && m_GuiLayer->GetTutorialOverlay()) {
        tutorialStep = static_cast<i32>(m_GuiLayer->GetTutorialOverlay()->GetCurrentStep());
        tutorialCompleted = m_GuiLayer->GetTutorialOverlay()->IsCompleted();
    }

    body << "  \"saveTimestamp\": " << m_LastSaveTimestamp << ",\n";
    body << "  \"tutorialStep\": " << tutorialStep << ",\n";
    body << "  \"tutorialCompleted\": " << (tutorialCompleted ? "true" : "false") << ",\n";

    // Resources
    file << "  \"resources\": [" << m_Resources[0] << ", " << m_Resources[1] << ", " << m_Resources[2] << "],\n";
    file << "  \"coherence\": " << m_Coherence << ",\n";
    file << "  \"playerCredits\": " << m_PlayerCredits << ",\n";

    // Prestige
    body << "  \"photons\": " << m_Timeline.photons << ",\n";
    body << "  \"resets\": " << m_Timeline.completedResets << ",\n";

    // Phase 3.3: Exotic Materials
    file << "  \"exoticMaterials\": " << m_ExoticMaterials << ",\n";
    file << "  \"researchData\": " << m_ResearchData << ",\n";

    // Time
    body << "  \"timePlayed\": " << m_TotalTimePlayed << ",\n";

    // Stations
    body << "  \"stations\": [\n";
    for (size_t i = 0; i < m_Stations.size(); i++) {
        const auto& s = m_Stations[i];
        body << "    {\"level\": " << s.level << ", \"unlocked\": " << (s.unlocked ? "true" : "false") << "}";
        if (i < m_Stations.size() - 1) body << ",";
        body << "\n";
    }
    body << "  ],\n";

    // Statistics
    body << "  \"statistics\": {\n";
    body << "    \"totalQubitsEarned\": " << m_Statistics.totalQubitsEarned << ",\n";
    body << "    \"totalObservations\": " << m_Statistics.totalObservations << ",\n";
    body << "    \"totalUpgrades\": " << m_Statistics.totalUpgrades << ",\n";
    body << "    \"totalPrestiges\": " << m_Statistics.totalPrestigesPerformed << ",\n";
    body << "    \"highestQubits\": " << m_Statistics.highestQubits << ",\n";
    body << "    \"fastestPrestige\": " << m_Statistics.fastestPrestige << ",\n";
    body << "    \"currentStreak\": " << m_Statistics.currentStreak << ",\n";
    body << "    \"lastLogin\": " << m_Statistics.lastLoginTimestamp << "\n";
    body << "  },\n";

    // Achievements
    body << "  \"achievements\": [\n";
    for (size_t i = 0; i < m_Achievements.size(); i++) {
        const auto& ach = m_Achievements[i];
        body << "    {";
        body << "\"id\": " << static_cast<i32>(ach.id) << ", ";
        body << "\"unlocked\": " << (ach.unlocked ? "true" : "false") << ", ";
        body << "\"progress\": " << ach.progress;
        body << "}";
        if (i < m_Achievements.size() - 1) body << ",";
        body << "\n";
    }
    body << "  ],\n";

    // Milestones
    body << "  \"milestones\": [\n";
    const auto& milestones = m_MilestoneSystem.GetMilestones();
    for (size_t i = 0; i < milestones.size(); i++) {
        const auto& milestone = milestones[i];
        body << "    {";
        body << "\"id\": " << static_cast<i32>(milestone.id) << ", ";
        body << "\"progress\": " << milestone.progress << ", ";
        body << "\"completed\": " << (milestone.completed ? "true" : "false") << ", ";
        body << "\"claimed\": " << (milestone.claimed ? "true" : "false");
        body << "}";
        if (i < milestones.size() - 1) body << ",";
        body << "\n";
    }
    body << "  ],\n";

    // Research Tree
    body << "  \"research\": [\n";
    auto researchedNodes = m_ResearchTree->GetResearchedNodes();
    for (size_t i = 0; i < researchedNodes.size(); i++) {
        body << "    " << static_cast<i32>(researchedNodes[i]->id);
        if (i < researchedNodes.size() - 1) body << ",";
        body << "\n";
    }
    body << "  ],\n";

    // Phase 4.1: Particle Collection
    body << "  \"particleCollection\": {\n";
    body << "    \"discovered\": [";
    for (i32 i = 0; i < static_cast<i32>(ParticleType::COUNT); i++) {
        ParticleType type = static_cast<ParticleType>(i);
        bool discovered = m_ParticleCollection.IsDiscovered(type);
        body << (discovered ? "true" : "false");
        if (i < static_cast<i32>(ParticleType::COUNT) - 1) body << ", ";
    }
    body << "],\n";
    body << "    \"equipped\": [";
    auto equippedParticles = m_ParticleCollection.GetEquippedParticles();
    for (size_t i = 0; i < equippedParticles.size(); i++) {
        body << static_cast<i32>(equippedParticles[i]->type);
        if (i < equippedParticles.size() - 1) body << ", ";
    }
    body << "]\n";
    body << "  },\n";

    // Specialized Skills
    m_SpecializedSkills.SaveToJson(body);

    std::string bodyStr = body.str();

    std::ostringstream unsignedContent;
    unsignedContent << "{\n";
    unsignedContent << "  \"version\": 3,\n";
    unsignedContent << bodyStr;
    unsignedContent << "}\n";

    std::string hash = ComputeSaveHash(unsignedContent.str());

    file << "{\n";
    file << "  \"version\": 3,\n";
    file << "  \"hash\": \"" << hash << "\",\n";
    file << bodyStr;
    file << "}\n";

    file.close();
    Log::Debugf("Game saved to ", filepath);

    // Mirror to Steam Cloud when available
    SteamIntegration::MirrorSaveToCloud(filepath, "default");
    return true;
}

bool GameState::Load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContent = buffer.str();
    file.close();

    i32 saveVersion = 1;
    {
        std::istringstream versionStream(fileContent);
        std::string versionLine;
        while (std::getline(versionStream, versionLine)) {
            if (versionLine.find("\"version\"") != std::string::npos) {
                saveVersion = static_cast<i32>(GameUtils::ParseJsonNumber(versionLine, "version"));
                break;
            }
        }
    }

    if (saveVersion >= 3) {
        std::string hashValue;
        std::ostringstream unsignedContent;
        std::istringstream hashStream(fileContent);
        std::string hashLine;

        while (std::getline(hashStream, hashLine)) {
            if (hashLine.find("\"hash\"") != std::string::npos) {
                size_t colonPos = hashLine.find(':');
                size_t valueStart = (colonPos != std::string::npos) ? hashLine.find('"', colonPos) : std::string::npos;
                if (valueStart != std::string::npos) {
                    size_t hashStart = hashLine.find('"', valueStart + 1);
                    size_t hashEnd = hashLine.find('"', hashStart + 1);
                    if (hashStart != std::string::npos && hashEnd != std::string::npos) {
                        hashValue = hashLine.substr(hashStart + 1, hashEnd - hashStart - 1);
                    }
                }
                continue;
            }

            unsignedContent << hashLine << "\n";
        }

        if (hashValue.empty()) {
            Log::Error("Save file missing integrity hash");
            return false;
        }

        std::string computedHash = ComputeSaveHash(unsignedContent.str());
        if (computedHash != hashValue) {
            Log::Warning("Save file hash mismatch - aborting load to prevent corrupted state");
            return false;
        }
    } else {
        Log::Warning("Loading legacy save without integrity hash; validation skipped");
    }

    try {
        // Simple parsing using utility functions
        std::string line;
        bool inStatistics = false;
        bool inAchievements = false;
        bool inResearch = false;
        bool inSpecializedSkills = false;
        bool inParticleCollection = false;  // Phase 4.1
        bool inMilestones = false;

        std::istringstream contentStream(fileContent);
        while (std::getline(contentStream, line)) {
            // Track sections
            if (line.find("\"statistics\"") != std::string::npos) {
                inStatistics = true;
                inAchievements = false;
                inResearch = false;
                inSpecializedSkills = false;
                inMilestones = false;
                continue;
            } else if (line.find("\"achievements\"") != std::string::npos) {
                inStatistics = false;
                inAchievements = true;
                inResearch = false;
                inSpecializedSkills = false;
                inMilestones = false;
                continue;
            } else if (line.find("\"milestones\"") != std::string::npos) {
                inStatistics = false;
                inAchievements = false;
                inResearch = false;
                inSpecializedSkills = false;
                inMilestones = true;
                continue;
            } else if (line.find("\"research\"") != std::string::npos) {
                inStatistics = false;
                inAchievements = false;
                inResearch = true;
                inSpecializedSkills = false;
                inMilestones = false;
                continue;
            } else if (line.find("\"specializedSkills\"") != std::string::npos) {
                inStatistics = false;
                inAchievements = false;
                inResearch = false;
                inParticleCollection = false;  // Phase 4.1
                inSpecializedSkills = true;
                inMilestones = false;
                continue;
            } else if (line.find("\"particleCollection\"") != std::string::npos) {
                // Phase 4.1
                inStatistics = false;
                inAchievements = false;
                inResearch = false;
                inSpecializedSkills = false;
                inParticleCollection = true;
                inMilestones = false;
                continue;
            } else if (line.find("}") != std::string::npos || line.find("]") != std::string::npos) {
                if (line.find("},") == std::string::npos) {
                    inStatistics = false;
                    inAchievements = false;
                    inResearch = false;
                    inSpecializedSkills = false;
                    inParticleCollection = false;  // Phase 4.1
                    inMilestones = false;
                }
            }

            // Parse based on section
            if (inStatistics) {
                m_Statistics.totalQubitsEarned = GameUtils::ParseJsonNumber(line, "totalQubitsEarned");
                m_Statistics.totalObservations = static_cast<i32>(GameUtils::ParseJsonNumber(line, "totalObservations"));
                m_Statistics.totalUpgrades = static_cast<i32>(GameUtils::ParseJsonNumber(line, "totalUpgrades"));
                m_Statistics.totalPrestigesPerformed = static_cast<i32>(GameUtils::ParseJsonNumber(line, "totalPrestiges"));
                m_Statistics.highestQubits = GameUtils::ParseJsonNumber(line, "highestQubits");
                m_Statistics.fastestPrestige = GameUtils::ParseJsonNumber(line, "fastestPrestige");
                m_Statistics.currentStreak = static_cast<i32>(GameUtils::ParseJsonNumber(line, "currentStreak"));
                m_Statistics.lastLoginTimestamp = static_cast<i64>(GameUtils::ParseJsonNumber(line, "lastLogin"));
            } else if (inAchievements) {
                // Parse achievement unlocked status
                i32 achId = static_cast<i32>(GameUtils::ParseJsonNumber(line, "id"));
                if (achId >= 0 && achId < static_cast<i32>(m_Achievements.size())) {
                    m_Achievements[achId].unlocked = GameUtils::ParseJsonBool(line, "unlocked");
                    m_Achievements[achId].progress = GameUtils::ParseJsonNumber(line, "progress");
                }
            } else if (inMilestones) {
                i32 milestoneId = static_cast<i32>(GameUtils::ParseJsonNumber(line, "id"));
                auto& milestones = m_MilestoneSystem.GetMilestones();
                if (milestoneId >= 0 && milestoneId < static_cast<i32>(milestones.size())) {
                    milestones[milestoneId].progress = GameUtils::ParseJsonNumber(line, "progress");
                    milestones[milestoneId].completed = GameUtils::ParseJsonBool(line, "completed");
                    milestones[milestoneId].claimed = GameUtils::ParseJsonBool(line, "claimed");
                }
            } else if (inResearch) {
                // Parse research IDs (simple number per line)
                // Remove whitespace and commas
                std::string trimmed = line;
                trimmed.erase(std::remove_if(trimmed.begin(), trimmed.end(),
                    [](char c) { return std::isspace(c) || c == ','; }), trimmed.end());

                if (!trimmed.empty() && std::isdigit(trimmed[0])) {
                    try {
                        i32 researchId = std::stoi(trimmed);
                        if (researchId >= 0 && researchId < static_cast<i32>(ResearchID::COUNT)) {
                            ResearchID id = static_cast<ResearchID>(researchId);
                            ResearchNode* node = m_ResearchTree->GetNode(id);
                            if (node) {
                                node->unlocked = true;
                                node->researched = true;
                            }
                        }
                    } catch (...) {
                        // Ignore parse errors
                    }
                }
            } else if (inParticleCollection) {
                // Phase 4.1: Parse particle collection data
                if (line.find("\"discovered\"") != std::string::npos) {
                    // Parse discovered array
                    size_t start = line.find('[');
                    size_t end = line.find(']');
                    if (start != std::string::npos && end != std::string::npos) {
                        std::string values = line.substr(start + 1, end - start - 1);
                        size_t pos = 0;
                        i32 index = 0;
                        while (pos < values.length() && index < static_cast<i32>(ParticleType::COUNT)) {
                            // Skip whitespace
                            while (pos < values.length() && std::isspace(values[pos])) pos++;
                            // Check if true/false
                            if (values.substr(pos, 4) == "true") {
                                m_ParticleCollection.DiscoverParticle(static_cast<ParticleType>(index));
                                pos += 4;
                            } else if (values.substr(pos, 5) == "false") {
                                pos += 5;
                            }
                            // Skip comma and whitespace
                            while (pos < values.length() && (values[pos] == ',' || std::isspace(values[pos]))) pos++;
                            index++;
                        }
                    }
                } else if (line.find("\"equipped\"") != std::string::npos) {
                    // Parse equipped array
                    size_t start = line.find('[');
                    size_t end = line.find(']');
                    if (start != std::string::npos && end != std::string::npos) {
                        std::string values = line.substr(start + 1, end - start - 1);
                        if (!values.empty()) {
                            size_t pos = 0;
                            while (pos < values.length()) {
                                // Skip whitespace
                                while (pos < values.length() && std::isspace(values[pos])) pos++;
                                if (pos >= values.length()) break;

                                // Parse number
                                size_t numStart = pos;
                                while (pos < values.length() && std::isdigit(values[pos])) pos++;
                                if (pos > numStart) {
                                    try {
                                        i32 particleId = std::stoi(values.substr(numStart, pos - numStart));
                                        if (particleId >= 0 && particleId < static_cast<i32>(ParticleType::COUNT)) {
                                            m_ParticleCollection.EquipParticle(static_cast<ParticleType>(particleId));
                                        }
                                    } catch (...) {
                                        // Ignore parse errors
                                    }
                                }
                                // Skip comma and whitespace
                                while (pos < values.length() && (values[pos] == ',' || std::isspace(values[pos]))) pos++;
                            }
                        }
                    }
                }
            } else if (inSpecializedSkills) {
                // Parse specialized skills (delegate to LoadFromJson)
                m_SpecializedSkills.LoadFromJson(line);
            } else {
                // Parse main game state
                if (line.find("\"saveTimestamp\"") != std::string::npos) {
                    m_LastSaveTimestamp = static_cast<i64>(GameUtils::ParseJsonNumber(line, "saveTimestamp"));
                } else if (line.find("\"tutorialStep\"") != std::string::npos) {
                    i32 tutorialStep = static_cast<i32>(GameUtils::ParseJsonNumber(line, "tutorialStep"));
                    if (m_GuiLayer && m_GuiLayer->GetTutorialOverlay()) {
                        m_GuiLayer->GetTutorialOverlay()->SetCurrentStep(static_cast<UI::TutorialStep>(tutorialStep));
                    }
                } else if (line.find("\"tutorialCompleted\"") != std::string::npos) {
                    bool tutorialCompleted = GameUtils::ParseJsonBool(line, "tutorialCompleted");
                    if (tutorialCompleted && m_GuiLayer && m_GuiLayer->GetTutorialOverlay()) {
                        m_GuiLayer->GetTutorialOverlay()->SetCurrentStep(UI::TutorialStep::None);
                    }
                } else if (line.find("\"resources\"") != std::string::npos) {
                    // Parse resources array
                    size_t start = line.find('[');
                    size_t end = line.find(']');
                    if (start != std::string::npos && end != std::string::npos) {
                        std::string values = line.substr(start + 1, end - start - 1);
                        std::istringstream iss(values);
                        std::string val;
                        int idx = 0;
                        while (std::getline(iss, val, ',') && idx < 3) {
                            m_Resources[idx++] = std::stod(val);
                        }
                    }
                } else if (line.find("\"coherence\"") != std::string::npos) {
                    m_Coherence = GameUtils::ParseJsonNumber(line, "coherence");
                } else if (line.find("\"playerCredits\"") != std::string::npos) {
                    m_PlayerCredits = static_cast<i32>(GameUtils::ParseJsonNumber(line, "playerCredits"));
                } else if (line.find("\"photons\"") != std::string::npos) {
                    m_Timeline.photons = GameUtils::ParseJsonNumber(line, "photons");
                    m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1);
                } else if (line.find("\"resets\"") != std::string::npos) {
                    m_Timeline.completedResets = static_cast<i32>(GameUtils::ParseJsonNumber(line, "resets"));
                } else if (line.find("\"exoticMaterials\"") != std::string::npos) {
                    // Phase 3.3: Load Exotic Materials
                    m_ExoticMaterials = static_cast<i32>(GameUtils::ParseJsonNumber(line, "exoticMaterials"));
                } else if (line.find("\"researchData\"") != std::string::npos) {
                    m_ResearchData = static_cast<i32>(GameUtils::ParseJsonNumber(line, "researchData"));
                } else if (line.find("\"timePlayed\"") != std::string::npos) {
                    m_TotalTimePlayed = GameUtils::ParseJsonNumber(line, "timePlayed");
                }
            }
        }

        // After loading, unlock available research based on current prestige level
        m_ResearchTree->UnlockAvailableResearch(m_Timeline.completedResets, m_ResearchTree->GetResearchedCount());

        // Apply research bonuses to all stations
        UpdateResearchBonuses();

        Log::Infof("Game loaded from ", filepath);
        return true;
    } catch (const std::exception& e) {
        Log::Errorf("Error loading save file: ", e.what());
        return false;
    }
}

// Achievement System
void GameState::CheckAchievements() {
    // Update achievement progress
    for (auto& ach : m_Achievements) {
        if (ach.unlocked) continue;

        switch (ach.id) {
            case AchievementID::FirstQubit:
                ach.progress = m_Statistics.totalQubitsEarned >= 1 ? 1 : 0;
                break;
            case AchievementID::Observe100Times:
                ach.progress = m_Statistics.totalObservations;
                break;
            case AchievementID::Reach1Million:
                ach.progress = m_Resources[0];
                break;
            case AchievementID::FirstPrestige:
                ach.progress = m_Timeline.completedResets;
                break;
            case AchievementID::TenStations:
                ach.progress = 0;
                for (const auto& s : m_Stations) {
                    if (s.unlocked) ach.progress++;
                }
                break;
            case AchievementID::MaxCoherence:
                ach.progress = (m_Coherence >= m_MaxCoherence) ? 1 : 0;
                break;
            case AchievementID::Hoarder:
                ach.progress = m_Resources[0];
                break;
            case AchievementID::QuantumMaster:
                ach.progress = m_Timeline.completedResets;
                break;
            case AchievementID::Collector:
                ach.progress = 0;
                for (const auto& s : m_Stations) {
                    if (s.unlocked) ach.progress++;
                }
                break;
            // Spaceship achievements
            case AchievementID::FirstShipPart:
                ach.progress = m_Spaceship.GetTotalPartsCollected();
                break;
            case AchievementID::ShipOperational:
                ach.progress = m_Spaceship.GetRepairProgress() >= 25.0 ? 1 : 0;
                break;
            case AchievementID::ShipFullyRepaired:
                ach.progress = m_Spaceship.GetRepairProgress() >= 100.0 ? 1 : 0;
                break;
            case AchievementID::FirstLegendaryPart:
                ach.progress = m_Spaceship.GetLegendaryPartsCollected();
                break;
            case AchievementID::PartCollector:
                ach.progress = m_Spaceship.GetTotalPartsCollected();
                break;
            default:
                break;
        }

        // Check if unlocked
        if (ach.progress >= ach.target && !ach.unlocked) {
            UnlockAchievement(ach.id);
        }
    }
}

void GameState::UnlockAchievement(AchievementID id) {
    Achievement* ach = GetAchievement(id);
    if (!ach || ach->unlocked) return;

    ach->unlocked = true;
    m_RecentUnlocks.push_back(id);

    // Grant rewards
    if (ach->rewardQubits > 0) {
        AddResource(QuantumResource::Qubits, ach->rewardQubits);
    }
    if (ach->rewardPhotons > 0) {
        m_Timeline.photons += ach->rewardPhotons;
        m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1);
    }

    // Award Quantum Essence (based on achievement importance)
    f64 essenceReward = 1.0; // Default: 1 essence
    // Major achievements get more essence
    if (id == AchievementID::QuantumMaster || id == AchievementID::Hoarder) {
        essenceReward = 3.0; // Milestone achievements: 3 essence
    } else if (id == AchievementID::FirstPrestige || id == AchievementID::Collector || id == AchievementID::WeekStreak) {
        essenceReward = 2.0; // Important achievements: 2 essence
    }
    AddEssence(essenceReward);

    Log::Infof("Achievement Unlocked: ", ach->name);

    // Propagate unlock to Steam where available
    SteamIntegration::UnlockAchievement(ach->name);
}

Achievement* GameState::GetAchievement(AchievementID id) {
    for (auto& ach : m_Achievements) {
        if (ach.id == id) return &ach;
    }
    return nullptr;
}

// Quantum Events System
void GameState::TriggerRandomEvent() {
    if (m_CurrentEvent && m_CurrentEvent->active) return; // Already has active event

    // Pick random event type
    int eventType = rand() % static_cast<int>(QuantumEventType::COUNT);

    QuantumEvent event;
    event.type = static_cast<QuantumEventType>(eventType);
    event.active = true;
    event.duration = 30.0 + (rand() % 30); // 30-60 seconds
    event.timeRemaining = event.duration;
    event.multiplier = 1.5 + (static_cast<f64>(rand()) / RAND_MAX); // 1.5-2.5x

    switch (event.type) {
        case QuantumEventType::WaveCollapse:
            event.name = "Wave Collapse Bonus";
            event.description = "Observations yield more resources!";
            break;
        case QuantumEventType::CoherenceBoost:
            event.name = "Coherence Surge";
            event.description = "Coherence decay paused!";
            m_Coherence = m_MaxCoherence;
            break;
        case QuantumEventType::QuantumFluctuation:
            event.name = "Quantum Fluctuation";
            event.description = "Random resource bonus!";
            AddResource(QuantumResource::Qubits, m_Resources[0] * 0.1);
            AddResource(QuantumResource::Coherence, 50);
            break;
        case QuantumEventType::EntanglementSurge:
            event.name = "Entanglement Surge";
            event.description = "Free entanglements!";
            AddResource(QuantumResource::Entanglement, 10);
            break;
        case QuantumEventType::TimeDialation:
            event.name = "Time Dilation";
            event.description = "Production doubled!";
            event.multiplier = 2.0;
            break;
        case QuantumEventType::LuckyObservation:
            event.name = "Lucky Observation";
            event.description = "Next observation guaranteed!";
            break;
        case QuantumEventType::ResourceRain:
            event.name = "Resource Rain";
            event.description = "Resources falling from the sky!";
            for (int i = 0; i < 50; i++) {
                AddResource(QuantumResource::Qubits, 10);
            }
            break;
        default:
            break;
    }

    m_Events.push_back(event);
    m_CurrentEvent = &m_Events.back();

    Log::Infof("Quantum Event: ", event.name);
}

void GameState::UpdateEvents(f64 deltaTime) {
    if (m_CurrentEvent && m_CurrentEvent->active) {
        m_CurrentEvent->timeRemaining -= deltaTime;

        // Apply event effects
        if (m_CurrentEvent->type == QuantumEventType::TimeDialation) {
            // Time dilation handled in UpdateStations
        }
        if (m_CurrentEvent->type == QuantumEventType::CoherenceBoost) {
            // Prevent coherence decay
            if (m_Coherence < m_MaxCoherence) {
                m_Coherence = m_MaxCoherence;
            }
        }

        // End event
        if (m_CurrentEvent->timeRemaining <= 0) {
            m_CurrentEvent->active = false;
            m_CurrentEvent = nullptr;
        }
    }
}

QuantumEvent* GameState::GetActiveEvent() {
    return m_CurrentEvent;
}

// Offline Progress
void GameState::CalculateOfflineProgress() {
    i64 currentTime = static_cast<i64>(Platform::GetTime());
    if (m_LastSaveTimestamp == 0) {
        m_LastSaveTimestamp = currentTime;
        return;
    }

    i64 timeOffline = currentTime - m_LastSaveTimestamp;
    if (timeOffline < 10) return; // Less than 10 seconds, ignore

    f64 secondsOffline = static_cast<f64>(timeOffline);
    f64 maxOfflineTime = 3600.0 * 4; // 4 hours max

    if (secondsOffline > maxOfflineTime) {
        secondsOffline = maxOfflineTime;
    }

    Log::Infof("You were away for ", secondsOffline / 60.0, " minutes");

    // Calculate offline production at reduced rate
    f64 offlineMultiplier = 0.5; // 50% efficiency while offline

    // Apply essence shop offline progress boost
    offlineMultiplier *= m_EssenceShopManager.GetOfflineProgressMultiplier();

    f64 offlineQubits = 0;

    for (const auto& station : m_Stations) {
        if (station.unlocked && station.level > 0) {
            f64 production = station.currentProduction * secondsOffline * offlineMultiplier;
            production *= m_Timeline.photonBonus;
            offlineQubits += production;
        }
    }

    if (offlineQubits > 0) {
        AddResource(QuantumResource::Qubits, offlineQubits);
        Log::Infof("Offline Progress: +", offlineQubits, " qubits");

        // Check offline millionaire achievement
        Achievement* offlineAch = GetAchievement(AchievementID::OfflineMillionaire);
        if (offlineAch && !offlineAch->unlocked && offlineQubits >= 1000000) {
            UnlockAchievement(AchievementID::OfflineMillionaire);
        }
    }

    m_LastSaveTimestamp = currentTime;
}

// Particle System Implementation
void GameState::SpawnParticle(const Vec2& position, const Color& color, f64 lifetime) {
    Particle p;
    if (!m_ParticlePool.empty()) {
        p = std::move(m_ParticlePool.back());
        m_ParticlePool.pop_back();
    }
    p.position = position;
    p.velocity = Vec2(
        static_cast<f32>(GameUtils::RandomRange(-50.0, 50.0)),
        static_cast<f32>(GameUtils::RandomRange(-100.0, -50.0))
    );
    p.color = color;
    p.lifetime = 0.0f;
    p.maxLifetime = static_cast<f32>(lifetime);
    m_Particles.push_back(p);

    // Enforce active particle budget
    while (m_Particles.size() > m_MaxActiveParticles) {
        m_ParticlePool.push_back(std::move(m_Particles.front()));
        m_Particles.front() = std::move(m_Particles.back());
        m_Particles.pop_back();
    }
}

void GameState::SpawnParticleBurst(const Vec2& position, const Color& color, i32 count) {
    for (i32 i = 0; i < count; i++) {
        SpawnParticle(position, color, GameUtils::RandomRange(0.5, 1.5));
    }
}

void GameState::UpdateParticles(f64 deltaTime) {
    // Update and recycle dead or culled particles
    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    const f32 cullPadding = 16.0f; // Prevent rendering just off-screen
    auto it = m_Particles.begin();
    while (it != m_Particles.end()) {
        it->lifetime += static_cast<f32>(deltaTime);
        it->position.x += it->velocity.x * static_cast<f32>(deltaTime);
        it->position.y += it->velocity.y * static_cast<f32>(deltaTime);

        // Apply gravity
        it->velocity.y += 200.0f * static_cast<f32>(deltaTime);

        // Fade out
        f32 alpha = 1.0f - (it->lifetime / it->maxLifetime);
        it->color.a = alpha;

        const f32 maxParticleSize = 6.0f; // Matches rendering size ramp
        const bool expired = it->lifetime >= it->maxLifetime;
        const bool offscreen =
            (it->position.x < -cullPadding - maxParticleSize) ||
            (it->position.x > displaySize.x + cullPadding + maxParticleSize) ||
            (it->position.y < -cullPadding - maxParticleSize) ||
            (it->position.y > displaySize.y + cullPadding + maxParticleSize);

        if (expired || offscreen) {
            m_ParticlePool.push_back(std::move(*it));
            *it = std::move(m_Particles.back());
            m_Particles.pop_back();
        } else {
            ++it;
        }
    }
}

void GameState::RenderParticleEffects(Renderer* renderer, f64 deltaTime) {

    UpdateParticles(deltaTime);

}

// Achievement UI Rendering
void GameState::RenderAchievements(Renderer* renderer) {
    if (!m_ShowAchievements) return;
    (void)renderer; // Renderer pointer is no longer used for UI drawing

    // Set a consistent window size and position (e.g., center of the screen)
    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
    );

    if (ImGui::Begin("Achievements", &m_ShowAchievements)) { // Pass address of bool to allow closing

        ImGui::Text("--- Achievement Progress ---");
        ImGui::Separator();

        for (auto& achievement : m_Achievements) {

            // Determine text color based on status
            ImVec4 statusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Default: locked/in-progress
            const char* statusText = "IN PROGRESS";

            if (achievement.unlocked) {
                statusColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green: Unlocked
                statusText = "UNLOCKED";
            } else if (achievement.progress >= achievement.target) {
                statusColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Gold: Ready to Claim
                statusText = "CLAIMABLE";
            }

            // Achievement Title and Status
            ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
            ImGui::Text("[%s] %s", statusText, achievement.name.c_str());
            ImGui::PopStyleColor();

            ImGui::Indent();
            ImGui::TextWrapped("%s", achievement.description.c_str());

            f32 progress = static_cast<f32>(achievement.progress / achievement.target);

            if (achievement.progress < achievement.target) {
                // Show progress bar if not complete
                char overlay[64];
                snprintf(overlay, sizeof(overlay), "%.0f / %.0f", achievement.progress, achievement.target);
                ImGui::ProgressBar(progress, ImVec2(-1, 0), overlay);
            } else if (achievement.unlocked) {
                 // Show a full bar for completed, claimed achievements
                ImGui::ProgressBar(1.0f, ImVec2(-1, 0), "Completed");
            }


            // Claim Button Logic
            if (achievement.progress >= achievement.target && !achievement.unlocked) {
                ImGui::Spacing();
                std::string rewardText = "Claim: ";
                if (achievement.rewardQubits > 0) {
                    rewardText += GameUtils::FormatNumber(achievement.rewardQubits, m_NumberFormat) + " Qubits ";
                }
                if (achievement.rewardPhotons > 0) {
                    rewardText += std::to_string(achievement.rewardPhotons) + " Photons";
                }

                if (ImGui::Button((rewardText + "##ClaimAch" + std::to_string(static_cast<int>(achievement.id))).c_str(), ImVec2(150, 30))) {
                    achievement.unlocked = true; // Mark as claimed
                    AddResource(QuantumResource::Qubits, achievement.rewardQubits);
                    m_Timeline.photons += achievement.rewardPhotons;
                }
                ImGui::Spacing();
            }

            ImGui::Unindent();
            ImGui::Separator();
        }

        ImGui::End();
    }
}

// Statistics UI Rendering
void GameState::RenderStatistics(Renderer* renderer) {
    if (!m_ShowStats) return;
    (void)renderer; // Renderer pointer is no longer used for UI drawing

    // Set consistent window size and position
    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
    );


    if (ImGui::Begin("Statistics", &m_ShowStats)) { // Pass address of bool to allow closing

        ImGui::Text("--- Total Lifetime Statistics ---");
        ImGui::Separator();

        // Use columns for neat alignment of labels and values
        ImGui::Columns(2, "StatColumns", true);
        ImGui::SetColumnWidth(0, 300.0f);

        auto format = [this](f64 value) { return GameUtils::FormatNumber(value, m_NumberFormat); };

        // --- Total Stats ---
        ImGui::Text("Total Qubits Earned:"); ImGui::NextColumn();
        ImGui::Text("%s", format(m_Statistics.totalQubitsEarned).c_str()); ImGui::NextColumn();

        ImGui::Text("Total Coherence Earned:"); ImGui::NextColumn();
        ImGui::Text("%s", format(m_Statistics.totalCoherenceEarned).c_str()); ImGui::NextColumn();

        ImGui::Text("Total Entanglement Earned:"); ImGui::NextColumn();
        ImGui::Text("%s", format(m_Statistics.totalEntanglementEarned).c_str()); ImGui::NextColumn();

        ImGui::Text("Total Observations:"); ImGui::NextColumn();
        ImGui::Text("%d", m_Statistics.totalObservations); ImGui::NextColumn();

        ImGui::Text("Total Upgrades Purchased:"); ImGui::NextColumn();
        ImGui::Text("%d", m_Statistics.totalUpgrades); ImGui::NextColumn();

        ImGui::Text("Prestiges Performed:"); ImGui::NextColumn();
        ImGui::Text("%d", m_Statistics.totalPrestigesPerformed); ImGui::NextColumn();

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::Text("--- Session Statistics ---");
        ImGui::Separator();

        ImGui::Columns(2, "SessionStatColumns", true);
        ImGui::SetColumnWidth(0, 300.0f);

        ImGui::Text("Session Qubits Earned:"); ImGui::NextColumn();
        ImGui::Text("%s", format(m_Statistics.sessionQubits).c_str()); ImGui::NextColumn();

        ImGui::Text("Session Time (seconds):"); ImGui::NextColumn();
        ImGui::Text("%.1f", m_Statistics.sessionTime); ImGui::NextColumn();

        ImGui::Text("Session Observations:"); ImGui::NextColumn();
        ImGui::Text("%d", m_Statistics.sessionObservations); ImGui::NextColumn();

        ImGui::Columns(1);
        ImGui::Separator();

        // Live telemetry surface: session pacing and prestige health
        ImGui::Text("--- Live Telemetry ---");
        ImGui::Separator();

        const TelemetryManager& telemetry = m_Telemetry;

        ImGui::Columns(2, "TelemetryStatColumns", true);
        ImGui::SetColumnWidth(0, 300.0f);

        ImGui::Text("Current Session Length:"); ImGui::NextColumn();
        ImGui::Text("%s", GameUtils::FormatTime(telemetry.GetSessionLengthSeconds()).c_str()); ImGui::NextColumn();

        ImGui::Text("Prestiges This Session:"); ImGui::NextColumn();
        ImGui::Text("%d", telemetry.GetSessionPrestiges()); ImGui::NextColumn();

        ImGui::Text("Prestiges Per Hour:"); ImGui::NextColumn();
        ImGui::Text("%.2f", telemetry.GetPrestigesPerHour()); ImGui::NextColumn();

        ImGui::Text("Avg. Minutes Between Prestiges:"); ImGui::NextColumn();
        ImGui::Text("%.2f", telemetry.GetAveragePrestigeIntervalMinutes()); ImGui::NextColumn();

        ImGui::Text("Last Prestige Interval (seconds):"); ImGui::NextColumn();
        ImGui::Text("%.1f", telemetry.GetLastPrestigeIntervalSeconds()); ImGui::NextColumn();

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::Text("--- Records ---");
        ImGui::Separator();

        ImGui::Columns(2, "RecordStatColumns", true);
        ImGui::SetColumnWidth(0, 300.0f);

        ImGui::Text("Highest Qubits Achieved:"); ImGui::NextColumn();
        ImGui::Text("%s", format(m_Statistics.highestQubits).c_str()); ImGui::NextColumn();

        ImGui::Text("Fastest Prestige (seconds):"); ImGui::NextColumn();
        if (m_Statistics.fastestPrestige < 99999.0) {
            ImGui::Text("%.1f", m_Statistics.fastestPrestige); ImGui::NextColumn();
        } else {
            ImGui::Text("N/A"); ImGui::NextColumn();
        }

        ImGui::Text("Longest Login Streak:"); ImGui::NextColumn();
        ImGui::Text("%d days", m_Statistics.longestStreak); ImGui::NextColumn();

        ImGui::Columns(1);

        ImGui::End();
    }
}
// Active Event Display
void GameState::RenderActiveEvent(Renderer* renderer) {
    if (!m_CurrentEvent || !m_CurrentEvent->active) return;
    (void)renderer; // Renderer is not used for ImGui drawing

    // --- 1. Define Window Properties (Fixed Top-Center Position) ---
    f32 bannerWidth = 400.0f;
    f32 bannerHeight = 80.0f;
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;
    f32 bannerX = (screenWidth - bannerWidth) / 2.0f;
    f32 bannerY = 120.0f; // Below the main navigation bar

    ImGui::SetNextWindowPos(ImVec2(bannerX, bannerY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(bannerWidth, bannerHeight), ImGuiCond_Always);

    // Style cleanup for the banner
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f)); // Add internal padding

    // --- 2. Animated Background Drawing ---
    // Animated background calculation
    f32 pulse = static_cast<f32>(0.8 + 0.2 * std::sin(m_TotalTimePlayed * 3.0));
    Color bgColor = Color::QuantumBlue() * pulse;
    bgColor.a = 0.9f;
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(bgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::White()));

    if (ImGui::Begin("##ActiveEventBanner", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse)) {

        // Get window draw list for manual drawing (white border is drawn by ImGuiCol_Border)

        // --- 3. Event Name ---
        ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "⚡ %s", m_CurrentEvent->name.c_str());

        // --- 4. Event Description ---
        ImGui::TextWrapped("%s", m_CurrentEvent->description.c_str());

        // Push darker text color for description (matching 0.9f, 0.9f, 1.0f, 1.0f)
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 1.0f, 1.0f));
        ImGui::PopStyleColor(); // Pop the description color

        // Add separator to position the progress bar at the bottom
        ImGui::Dummy(ImVec2(0.0f, 5.0f)); // Add vertical space

        // --- 5. Time Remaining Progress Bar ---
        f32 barWidth = bannerWidth - 20.0f;
        f32 barHeight = 10.0f;
        f32 progress = static_cast<f32>(m_CurrentEvent->timeRemaining / m_CurrentEvent->duration);

        // Calculate time text string
        std::string timeText = GameUtils::FormatTime(m_CurrentEvent->timeRemaining) + " remaining";

        // Draw the progress bar using ImGui::ProgressBar
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ToImVec4(Color::CoherenceGreen())); // Color for the bar fill
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f)); // Background color

        // The overlay text is placed in the center by default by ImGui.
        // We calculate the time text manually for precise positioning if needed,
        // but using ImGui's built-in overlay is cleaner.

        // Option A: Use built-in overlay (simple)
        ImGui::ProgressBar(progress, ImVec2(barWidth, barHeight), timeText.c_str());

        /*
        // Option B: Manual text placement (closer to original code)
        ImGui::ProgressBar(progress, ImVec2(barWidth, barHeight));
        // Manually position and draw time text over the bar:
        ImVec2 barMin = ImGui::GetItemRectMin();
        ImVec2 textPos(barMin.x + barWidth - 80.0f, barMin.y - 2.0f);
        draw_list->AddText(textPos, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), timeText.c_str());
        */

        ImGui::PopStyleColor(2); // Pop PlotHistogram and FrameBg

        ImGui::End();
    }

    ImGui::PopStyleColor(2); // Pop WindowBg and Border
    ImGui::PopStyleVar(3); // Pop WindowRounding, WindowBorderSize, WindowPadding
}

// Achievement Notification Popups
// Achievement Notification Popups
void GameState::RenderAchievementNotifications(Renderer* renderer) {
    if (m_RecentUnlocks.empty()) return;

    // Only show the most recent achievement
    AchievementID recentID = m_RecentUnlocks.back();
    Achievement* ach = GetAchievement(recentID);
    if (!ach) return;

    // We only need the renderer for getting display size, but we use ImGui::GetIO().DisplaySize
    (void)renderer;

    f32 notifWidth = 350.0f;
    f32 notifHeight = 60.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Position: Bottom right corner, 20px margins
    ImVec2 notifPos(displaySize.x - notifWidth - 20.0f, displaySize.y - notifHeight - 20.0f);

    ImGui::SetNextWindowPos(notifPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(notifWidth, notifHeight), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.95f);

    // Setup styles (CoherenceGreen border)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(Color(0.1f, 0.1f, 0.15f, 0.95f)));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::CoherenceGreen()));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 8.0f));

    if (ImGui::Begin("##AchievementNotif", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {

        // Title
        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "🏆 Achievement Unlocked!");

        // Achievement name
        // We use TextUnformatted to simplify alignment since we are using fixed height
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 2.0f); // Adjust position slightly
        ImGui::TextUnformatted(ach->name.c_str());

    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    // Clear old notifications (keep last 3)
    // We use a simple loop as the original code did.
    while (m_RecentUnlocks.size() > 3) {
        m_RecentUnlocks.erase(m_RecentUnlocks.begin());
    }
}

// Research Tree UI Rendering
// Research Tree UI Rendering
void GameState::RenderResearchTree(Renderer* renderer) {
    if (!m_ShowResearch) return;

    (void)renderer;

    // 1. Setup position and size
    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Center the window
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles (Quantum Purple theme)
    Color mainBorderColor = Color::QuantumPurple() * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f; // Approx DarkPanel

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    // 3. Begin the main research window
    if (ImGui::Begin("Research Tree", &m_ShowResearch,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Header (ImGui provides built-in close button) ---
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press R or ESC to close)");

        // Research count (left aligned)
        i32 researched = m_ResearchTree->GetResearchedCount();
        i32 total = static_cast<i32>(ResearchID::COUNT);
        std::string countText = "Researched: " + std::to_string(researched) + "/" + std::to_string(total);

        ImGui::SetCursorPosY(45.0f);
        ImGui::TextColored(ToImVec4(Color(0.9f, 0.9f, 1.0f, 1.0f)), "%s", countText.c_str());

        ImGui::Separator();
        ImGui::Spacing();

        // --- Research Nodes List (Scrollable Child Window) ---
        f32 contentStartY = ImGui::GetCursorPosY();
        // Reserve space for the bonuses summary at the bottom (60.0f)
        f32 nodesContentHeight = panelHeight - contentStartY - 60.0f - 30.0f;

        if (ImGui::BeginChild("##ResearchNodesList", ImVec2(0, nodesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {

            f32 nodeWidth = ImGui::GetContentRegionAvail().x;
            f32 nodeHeight = 100.0f;
            auto availableNodes = m_ResearchTree->GetAvailableResearch(m_Timeline.completedResets);
            auto researchedNodes = m_ResearchTree->GetResearchedNodes();

            // --- 4. Render Available Nodes ---
            if (!availableNodes.empty()) {
                ImGui::TextColored(ToImVec4(Color::White()), "Available Research:");
                ImGui::Spacing();

                for (const ResearchNode* node : availableNodes) {
                    // Check if research limit is hit (original code had maxDisplay, we rely on scroll)
                    bool canAfford = CanAffordResearch(node->id);

                    // Node background
                    Color nodeBg = canAfford ? Color(0.2f, 0.3f, 0.2f, 1.0f) : Color(0.2f, 0.2f, 0.25f, 1.0f);
                    Color nodeBorder = canAfford ? Color::CoherenceGreen() : Color::QuantumBlue();

                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(nodeBg));
                    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(nodeBorder));

                    std::string nodeChildName = "##ResearchNode_" + std::to_string(static_cast<i32>(node->id));

                    if (ImGui::BeginChild(nodeChildName.c_str(), ImVec2(nodeWidth, nodeHeight), true, ImGuiWindowFlags_NoScrollbar)) {

                        // Node name
                        ImGui::TextColored(ToImVec4(Color::White()), "%s", node->name.c_str());

                        // Auto-research toggle button (top-right corner of node)
                        f32 autoToggleW = 60.0f;
                        f32 autoToggleH = 25.0f;
                        ImGui::SameLine(nodeWidth - autoToggleW - 10.0f);
                        ImGui::SetCursorPosY(10.0f); // Relative to child start

                        ResearchNode* mutableNode = m_ResearchTree->GetNode(node->id);
                        bool autoEnabled = (mutableNode && mutableNode->autoResearch);

                        Color autoToggleColor = autoEnabled ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(autoToggleColor * 0.4f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(autoToggleColor * 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(autoToggleColor));

                        if (ImGui::Button("AUTO", ImVec2(autoToggleW, autoToggleH))) {
                            // Toggle auto-research
                            if (mutableNode) mutableNode->autoResearch = !mutableNode->autoResearch;
                        }
                        ImGui::PopStyleColor(3);

                        // Node description
                        ImGui::TextColored(ToImVec4(Color(0.8f, 0.8f, 0.9f, 1.0f)), "%s", node->description.c_str());

                        // Costs
                        std::string costText = "Cost: ";
                        if (node->qubitCost > 0) costText += GameUtils::FormatNumber(node->qubitCost, m_NumberFormat) + " Qubits | ";
                        if (node->coherenceCost > 0) costText += GameUtils::FormatNumber(node->coherenceCost, m_NumberFormat) + " Coherence | ";
                        if (node->entanglementCost > 0) costText += GameUtils::FormatNumber(node->entanglementCost, m_NumberFormat) + " Entanglement | ";
                        if (node->photonCost > 0) costText += std::to_string(node->photonCost) + " Photons";
                        if (node->exoticMaterialsCost > 0) costText += " | " + std::to_string(node->exoticMaterialsCost) + " Exotic Materials";

                        Color costColor = canAfford ? Color::CoherenceGreen() : Color::QuantumPurple();
                        ImGui::TextColored(ToImVec4(costColor), "%s", costText.c_str());

                        // Prerequisites
                        if (!node->prerequisites.empty()) {
                            std::string prereqText = "Requires: ";
                            for (size_t i = 0; i < node->prerequisites.size(); i++) {
                                const ResearchNode* prereq = m_ResearchTree->GetNode(node->prerequisites[i]);
                                if (prereq) {
                                    prereqText += prereq->name;
                                    if (i < node->prerequisites.size() - 1) prereqText += ", ";
                                }
                            }
                            ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.7f, 1.0f)), "%s", prereqText.c_str());
                        }

                        // Research button (Bottom right corner)
                        ImGui::SetCursorPosY(nodeHeight - 35.0f); // Push to bottom
                        ImGui::SameLine(nodeWidth - 120.0f);

                        Color btnColor = canAfford ? Color::QuantumPurple() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                        Color btnHoveredColor = canAfford ? Color::QuantumPurple() * 1.5f : Color(0.4f, 0.4f, 0.4f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                        if (ImGui::Button("RESEARCH", ImVec2(110.0f, 30.0f)) && canAfford) {
                            // Purchase the research
                            PurchaseResearch(node->id);
                        }
                        ImGui::PopStyleColor(3);

                    }
                    ImGui::EndChild();

                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar(2);
                    ImGui::Spacing();
                }
            }

            // --- 5. Render Completed Research if no Available Nodes ---
            if (availableNodes.empty()) {
                ImGui::SetCursorPosX(nodeWidth / 2.0f - 180.0f);
                ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.7f, 1.0f)), "No research available at current prestige level!");
                ImGui::Spacing();

                ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "Completed Research:");

                // Display up to 8 completed nodes in columns
                i32 completedCount = 0;
                for (const ResearchNode* node : researchedNodes) {
                    if (completedCount == 0) ImGui::Separator();

                    std::string completedText = "✓ " + node->name;
                    ImGui::TextColored(ToImVec4(Color(0.8f, 0.9f, 0.8f, 1.0f)), "%s", completedText.c_str());

                    completedCount++;

                    // Display in two columns
                    if (completedCount % 2 == 1 && static_cast<size_t>(completedCount) < researchedNodes.size()) {
                        ImGui::SameLine(nodeWidth / 2.0f);
                    } else if (completedCount % 2 == 0) {
                        // New line for next pair
                    }

                    if (completedCount >= 8) {
                        ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.7f, 1.0f)), "(...and %d more)", (int)researchedNodes.size() - completedCount);
                        break;
                    }
                }
            }

        }
        ImGui::EndChild(); // End ResearchNodesList

        // --- Bonuses Summary (Fixed to the bottom) ---
        ImGui::SetCursorPosY(panelHeight - 60.0f);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ToImVec4(Color::QuantumBlue()), "Active Bonuses:");

        f64 prodMult = m_ResearchTree->GetTotalProductionMultiplier();
        f64 obsMult = m_ResearchTree->GetTotalObservationBonus();
        f64 cohMult = m_ResearchTree->GetTotalCoherenceBonus();

        // Ensure calculation of percentage display is correct (x100 and cast to int)
        std::string bonusText = "Production: +" + std::to_string(static_cast<i32>((prodMult - 1.0) * 100.0)) + "%%  |  ";
        // Assuming observation and coherence are returned as multipliers like 1.25 (25% bonus)
        std::string obsBonusStr = (obsMult >= 1.0) ? "+" + std::to_string(static_cast<i32>((obsMult - 1.0) * 100.0)) : std::to_string(static_cast<i32>((obsMult - 1.0) * 100.0));
        std::string cohBonusStr = (cohMult >= 1.0) ? "+" + std::to_string(static_cast<i32>((cohMult - 1.0) * 100.0)) : std::to_string(static_cast<i32>((cohMult - 1.0) * 100.0));

        bonusText += "Observation: " + obsBonusStr + "%%  |  ";
        bonusText += "Coherence: " + cohBonusStr + "%%";

        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "%s", bonusText.c_str());

    }
    ImGui::End(); // End ResearchPanel

    // 4. Pop styles
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

// Research Tree Methods
bool GameState::CanAffordResearch(ResearchID id) const {
    const ResearchNode* node = m_ResearchTree->GetNode(id);
    if (!node) return false;

    // Apply singularity shop research cost discount
    f64 costMultiplier = m_SingularityShopManager.GetResearchCostMultiplier();

    // Check resource costs (with discount applied)
    if (GetResource(QuantumResource::Qubits) < node->qubitCost * costMultiplier) return false;
    if (GetResource(QuantumResource::Coherence) < node->coherenceCost * costMultiplier) return false;
    if (GetResource(QuantumResource::Entanglement) < node->entanglementCost * costMultiplier) return false;

    // Check photon cost
    if (m_Timeline.photons < node->photonCost * costMultiplier) return false;

    // Phase 3.3: Check exotic materials cost (not affected by discount)
    if (m_ExoticMaterials < node->exoticMaterialsCost) return false;

    // Check if can be researched
    if (!m_ResearchTree->CanResearch(id, m_Timeline.completedResets)) return false;

    return true;
}

bool GameState::PurchaseResearch(ResearchID id) {
    if (!CanAffordResearch(id)) return false;

    ResearchNode* node = m_ResearchTree->GetNode(id);
    if (!node) return false;

    // Apply singularity shop research cost discount
    f64 costMultiplier = m_SingularityShopManager.GetResearchCostMultiplier();

    // Spend resources (with discount applied)
    SpendResource(QuantumResource::Qubits, node->qubitCost * costMultiplier);
    SpendResource(QuantumResource::Coherence, node->coherenceCost * costMultiplier);
    SpendResource(QuantumResource::Entanglement, node->entanglementCost * costMultiplier);
    m_Timeline.photons -= node->photonCost * costMultiplier;

    // Phase 3.3: Spend exotic materials (not affected by discount)
    SpendExoticMaterials(node->exoticMaterialsCost);

    // Research it
    m_ResearchTree->Research(id);

    // Update bonuses
    UpdateResearchBonuses();

    // Unlock new research
    m_ResearchTree->UnlockAvailableResearch(m_Timeline.completedResets, m_ResearchTree->GetResearchedCount());

    // Spawn celebration particles
    SpawnParticleBurst(Vec2(640.0f, 360.0f), Color::QuantumPurple(), 20);

    // Award Engineering skill XP
    m_SpecializedSkills.AddExperience(SkillCategory::Engineering, SkillXP::PURCHASE_RESEARCH);

    // Play research complete sound
    m_SoundManager.PlaySound(SoundEffect::ResearchComplete);

    Log::Info("Researched: " + node->name);

    return true;
}

void GameState::UpdateResearchBonuses() {
    // Apply research bonuses to all stations
    f64 productionMult = m_ResearchTree->GetTotalProductionMultiplier();

    // Apply milestone production bonuses
    f64 milestoneBonus = 1.0 + m_MilestoneSystem.GetTotalProductionBonus();
    productionMult *= milestoneBonus;

    // Apply challenge reward multipliers (permanent bonuses from completed challenges)
    f64 challengeBonus = m_ChallengeManager.GetTotalRewardMultiplier();
    productionMult *= challengeBonus;

    // Apply essence shop production multiplier (permanent meta-upgrades)
    f64 essenceBonus = m_EssenceShopManager.GetProductionMultiplier();
    productionMult *= essenceBonus;

    // Calculate buyable multipliers for each resource type
    // Each purchase doubles production (2^timesPurchased)
    auto* quantumAccelerator = m_BuyableManager.GetBuyable("quantum_accelerator");
    auto* coherenceAmplifier = m_BuyableManager.GetBuyable("coherence_amplifier");
    auto* entanglementBooster = m_BuyableManager.GetBuyable("entanglement_booster");

    f64 qubitMultiplier = quantumAccelerator ? std::pow(2.0, quantumAccelerator->timesPurchased) : 1.0;
    f64 coherenceMultiplier = coherenceAmplifier ? std::pow(2.0, coherenceAmplifier->timesPurchased) : 1.0;
    f64 entanglementMultiplier = entanglementBooster ? std::pow(2.0, entanglementBooster->timesPurchased) : 1.0;

    for (auto& station : m_Stations) {
        if (station.unlocked && station.level > 0) {
            station.currentProduction = station.baseProduction * station.level * productionMult;

            // Apply resource-specific buyable multipliers
            if (station.resourceType == QuantumResource::Qubits) {
                station.currentProduction *= qubitMultiplier;
            } else if (station.resourceType == QuantumResource::Coherence) {
                station.currentProduction *= coherenceMultiplier;
            } else if (station.resourceType == QuantumResource::Entanglement) {
                station.currentProduction *= entanglementMultiplier;
            }
        }
    }

    // Update photon bonus multiplier with research
    m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1);

    // Check for PhotonMultiplier research
    if (m_ResearchTree->IsResearched(ResearchID::PhotonMultiplier)) {
        m_Timeline.photonBonus *= 1.5; // +50% photon effectiveness
    }

    // Apply production multiplier from timeline photons
    for (auto& station : m_Stations) {
        if (station.unlocked && station.level > 0) {
            station.currentProduction *= m_Timeline.photonBonus;
        }
    }
}

// Milestone System Methods
void GameState::AddPhotons(f64 amount) {
    m_Timeline.photons += amount;
    m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1);

    // Apply PhotonMultiplier research bonus
    if (m_ResearchTree->IsResearched(ResearchID::PhotonMultiplier)) {
        m_Timeline.photonBonus *= 1.5;
    }

    // PHASE D: Check for theme unlocks
    if (m_Timeline.photons >= 100.0 && m_LastThemeUnlocked < 1) {
        m_LastThemeUnlocked = 1;
        Log::Info("🎨 NEW THEME UNLOCKED: Quantum Facility! Your lab has a cyan glow.");
    }
    if (m_Timeline.singularities >= 10.0 && m_LastThemeUnlocked < 2) {
        m_LastThemeUnlocked = 2;
        Log::Info("🌌 NEW THEME UNLOCKED: Cosmic Observatory! Deep space blue surrounds you.");
    }
    if (m_Timeline.singularities >= 50.0 && m_LastThemeUnlocked < 3) {
        m_LastThemeUnlocked = 3;
        Log::Info("⚫ NEW THEME UNLOCKED: Cosmic Void! You've entered the purple abyss.");
    }
}

f64 GameState::GetPhotons() const {
    return m_Timeline.photons;
}

void GameState::CheckMilestones() {
    // This is a simplified version - full implementation would be in Milestones.cpp
    // But we need to update progress here since we have access to GameState data
    
    for (auto milestone : m_MilestoneSystem.GetActiveMilestones()) {
        // Update progress based on milestone type
        switch (milestone->id) {
            case MilestoneID::FirstThousand:
            case MilestoneID::FirstMillion:
            case MilestoneID::FirstBillion:
            case MilestoneID::FirstTrillion:
                milestone->progress = m_Statistics.highestQubits;
                break;

            case MilestoneID::FiveStations:
            case MilestoneID::TenStations: {
                i32 count = 0;
                for (const auto& s : m_Stations) {
                    if (s.unlocked && s.level > 0) count++;
                }
                milestone->progress = static_cast<f64>(count);
                break;
            }

            case MilestoneID::MaxedStation: {
                f64 maxLevel = 0;
                for (const auto& s : m_Stations) {
                    if (s.level > maxLevel) maxLevel = static_cast<f64>(s.level);
                }
                milestone->progress = maxLevel;
                break;
            }

            case MilestoneID::AllStationsMaxed: {
                bool allMaxed = true;
                for (const auto& s : m_Stations) {
                    if (s.unlocked && s.level < 100) {
                        allMaxed = false;
                        break;
                    }
                }
                milestone->progress = allMaxed ? 100.0 : 0.0;
                break;
            }

            case MilestoneID::FirstPrestige:
            case MilestoneID::TenPrestiges:
            case MilestoneID::FiftyPrestiges:
                milestone->progress = static_cast<f64>(m_Statistics.totalPrestigesPerformed);
                break;

            case MilestoneID::HundredPhotons:
                milestone->progress = m_Timeline.photons;
                break;

            case MilestoneID::OneHourPlayed:
            case MilestoneID::OneDayPlayed:
            case MilestoneID::OneWeekPlayed:
                milestone->progress = m_TotalTimePlayed;
                break;

            case MilestoneID::HalfAchievements:
            case MilestoneID::AllAchievements: {
                i32 unlocked = 0;
                for (const auto& ach : m_Achievements) {
                    if (ach.unlocked) unlocked++;
                }
                f64 percentage = static_cast<f64>(unlocked) / static_cast<f64>(m_Achievements.size());
                milestone->progress = percentage;
                break;
            }

            case MilestoneID::FirstResearch:
            case MilestoneID::TenResearch:
            case MilestoneID::AllResearch:
                milestone->progress = static_cast<f64>(m_ResearchTree->GetResearchedCount());
                break;

            case MilestoneID::QuantumMaster:
                if (m_Timeline.completedResets >= 50 && m_Timeline.photons >= 500.0) {
                    milestone->progress = 1.0;
                }
                break;

            case MilestoneID::TrueEnding:
                if (m_ResearchTree->IsResearched(ResearchID::QuantumSingularity)) {
                    milestone->progress = 1.0;
                }
                break;

            default:
                break;
        }

        // Check if completed
        if (milestone->progress >= milestone->target && !milestone->completed) {
            m_MilestoneSystem.CompleteMilestone(milestone->id, this);
            
            // Spawn celebration particles
            SpawnParticleBurst(Vec2(640.0f, 200.0f), Color::QuantumBlue(), 30);
        }
    }
}

void GameState::RenderMilestones(Renderer* renderer) {
    if (!m_ShowMilestones) return;
    (void)renderer; // Renderer pointer is no longer used for UI drawing

    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
    ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
    ImGuiCond_Once,
    ImVec2(0.5f, 0.5f)
    );

    if (ImGui::Begin("Timeline Milestones", &m_ShowMilestones)) {

        ImGui::Text("--- Timeline Milestones ---");
        ImGui::Separator();

        for (auto& milestone : m_MilestoneSystem.GetMilestones()) {

            // Determine text color based on status
            ImVec4 statusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Default: locked/in-progress
            const char* statusText = "IN PROGRESS";

            if (milestone.claimed) {
                statusColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green: Unlocked
                statusText = "COMPLETED";
            } else if (milestone.progress >= milestone.target) {
                statusColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Gold: Ready to Claim
                statusText = "CLAIMABLE";
            }

            // Milestone Title and Status
            ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
            ImGui::Text("[%s] %s", statusText, milestone.name.c_str());
            ImGui::PopStyleColor();

            ImGui::Indent();
            ImGui::TextWrapped("Target: %s %s", GameUtils::FormatNumber(milestone.target, m_NumberFormat).c_str(), milestone.featureName.c_str());

            f32 progress = static_cast<f32>(milestone.progress / milestone.target);

            if (!milestone.claimed) {
                // Show progress bar
                char overlay[64];
                snprintf(overlay, sizeof(overlay), "%.0f / %.0f", milestone.progress, milestone.target);
                ImGui::ProgressBar(progress, ImVec2(-1, 0), overlay);
            } else {
                 // Show a full bar for completed, claimed milestones
                ImGui::ProgressBar(1.0f, ImVec2(-1, 0), "Completed");
            }

            // Claim Button Logic
            if (milestone.progress >= milestone.target && !milestone.claimed) {
                ImGui::Spacing();
                std::string rewardText = "Claim: " + std::to_string(milestone.rewardSingularities) + " Singularities";

                if (ImGui::Button((rewardText + "##ClaimMS" + std::to_string(static_cast<int>(milestone.id))).c_str(), ImVec2(200, 30))) {
                    milestone.claimed = true; // Mark as claimed
                    m_Timeline.singularities += milestone.rewardSingularities;
                }
                ImGui::Spacing();
            }

            ImGui::Unindent();
            ImGui::Separator();
        }

        ImGui::End();
    }
}

void GameState::RenderBuyables(Renderer* renderer) {
    if (!m_ShowBuyables) return;

    (void)renderer;

    // 1. Setup position and size
    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Center the window
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles (Electric Blue theme)
    Color mainBorderColor = Color::ElectricBlue() * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f; // Approx DarkPanel

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    // 3. Begin the main buyables window
    if (ImGui::Begin("Buyable Upgrades", &m_ShowBuyables,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Header (ImGui provides built-in close button) ---
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press B or ESC to close)");
        ImGui::Separator();

        // --- Buyables List (Scrollable Child Window) ---
        f32 contentStartY = ImGui::GetCursorPosY();
        f32 buyablesContentHeight = panelHeight - contentStartY - 30.0f;

        if (ImGui::BeginChild("##BuyablesList", ImVec2(0, buyablesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {

            f32 buyableWidth = ImGui::GetContentRegionAvail().x;
            f32 buyableHeight = 100.0f;

            auto& buyables = m_BuyableManager.GetBuyables();

            for (size_t i = 0; i < buyables.size(); i++) {
                const auto& buyable = buyables[i];
                bool maxed = buyable.IsMaxed();
                bool canAfford = buyable.CanAfford(m_Resources[0]);
                bool disabledByChallenge = m_ChallengeManager.HasModifier(ChallengeModifier::NoBuyables);
                bool interactive = canAfford && !disabledByChallenge;

                // --- Single Buyable Panel (Child window for styling) ---
                Color buyableBg = maxed ? Color(0.1f, 0.2f, 0.15f, 1.0f) : Color(0.15f, 0.15f, 0.2f, 1.0f);
                Color buyableBorder = maxed ? Color::CoherenceGreen() * 0.6f : Color::ElectricBlue() * 0.6f;

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(buyableBg));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(buyableBorder));

                std::string buyableChildName = "##Buyable_" + std::to_string(i);

                if (ImGui::BeginChild(buyableChildName.c_str(), ImVec2(buyableWidth, buyableHeight), true, ImGuiWindowFlags_NoScrollbar)) {

                    // Buyable Name
                    ImGui::TextColored(ToImVec4(Color::White()), "%s", buyable.name.c_str());

                    // Purchase count
                    ImGui::SameLine(400.0f);
                    ImGui::TextColored(ToImVec4(Color::QuantumPurple()), "Owned: %s", buyable.GetProgressString().c_str());

                    // Description (TextWrapped handles description wrapping)
                    ImGui::TextWrapped("%s", buyable.description.c_str());

                    // Cost and Buy button
                    ImGui::SetCursorPosY(buyableHeight - 35.0f); // Push to bottom of child area

                    if (!maxed) {
                        std::string costStr = GameUtils::FormatNumber(buyable.GetCurrentCost(), m_NumberFormat) + " Qubits";
                        Color costColor = interactive ? Color::CoherenceGreen() : Color(0.7f, 0.5f, 0.5f, 1.0f);
                        ImGui::TextColored(ToImVec4(costColor), "Cost: %s", costStr.c_str());

                        // Buy button
                        ImGui::SameLine(buyableWidth - 120.0f);
                        std::string btnText = disabledByChallenge ? "DISABLED" : "PURCHASE";

                        Color btnColor = interactive ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                        Color btnHoveredColor = interactive ? Color::CoherenceGreen() * 1.5f : Color(0.4f, 0.4f, 0.4f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                        if (ImGui::Button(btnText.c_str(), ImVec2(110.0f, 35.0f)) && interactive) {
                            // Purchase the buyable
                            m_BuyableManager.Purchase(buyable.id, this);
                        }
                        ImGui::PopStyleColor(3);

                    } else {
                        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "MAXED OUT");
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                ImGui::Spacing();
            }
        }
        ImGui::EndChild(); // End BuyablesList
    }
    ImGui::End(); // End BuyablesPanel

    // 4. Pop styles
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void GameState::RenderMilestoneNotifications(Renderer* renderer) {
    auto recentCompletions = m_MilestoneSystem.GetRecentCompletions();
    if (recentCompletions.empty()) return;

    // We only need the renderer for getting display size, but we use ImGui::GetIO().DisplaySize
    (void)renderer;

    // Only show the most recent
    const Milestone* recent = recentCompletions.back();

    f32 notifWidth = 400.0f;
    f32 notifHeight = 90.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Position: Bottom right corner, 20px in from X and 100px up from Y
    ImVec2 notifPos(displaySize.x - notifWidth - 20.0f, displaySize.y - notifHeight - 100.0f);

    ImGui::SetNextWindowPos(notifPos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(notifWidth, notifHeight), ImGuiCond_Always);

    // Set global alpha for the window
    ImGui::SetNextWindowBgAlpha(0.95f);

    // Animated background color (QuantumBlue pulsating)
    f32 pulse = static_cast<f32>(0.9 + 0.1 * std::sin(m_TotalTimePlayed * 4.0));
    Color bgColor = Color::QuantumBlue() * pulse;

    // Setup styles
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(bgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::CoherenceGreen()));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 12.0f));

    if (ImGui::Begin("##MilestoneNotif", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {

        // Title
        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "🎯 Milestone Completed!");

        // Milestone name
        ImGui::TextColored(ToImVec4(Color::White()), "%s", recent->name.c_str());

        // Reward
        ImGui::TextColored(ToImVec4(Color(1.0f, 1.0f, 0.6f, 1.0f)), "%s", recent->rewardDescription.c_str());

    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    // Clear notifications after 5 seconds
    static f64 lastClear = m_TotalTimePlayed;
    if (m_TotalTimePlayed - lastClear > 5.0) {
        m_MilestoneSystem.ClearRecentCompletions();
        lastClear = m_TotalTimePlayed;
    }
}

void GameState::RenderChallenges(Renderer* renderer) {
    if (!m_ShowChallenges) return;

    (void)renderer;

    // 1. Setup position and size
    f32 panelWidth = 900.0f;
    f32 panelHeight = 650.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Center the window
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles (Red theme)
    Color mainBorderColor = Color::Red() * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f; // Approx DarkPanel

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    // 3. Begin the main challenges window
    if (ImGui::Begin("Quantum Challenges", &m_ShowChallenges,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Header (ImGui provides built-in close button) ---
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press C or ESC to close)");
        ImGui::Separator();

        // Current challenge info (if in challenge)
        const Challenge* currentChallenge = m_ChallengeManager.GetCurrentChallenge();
        f32 contentStartY = ImGui::GetCursorPosY();

        if (currentChallenge) {
            ImGui::TextColored(ToImVec4(Color::Red()), "⚠ ACTIVE CHALLENGE: %s", currentChallenge->name.c_str());

            f64 currentQubits = GetResource(QuantumResource::Qubits);
            std::string goalText = "Goal: " + GameUtils::FormatNumber(currentQubits, m_NumberFormat) +
                                   " / " + GameUtils::FormatNumber(currentChallenge->goalQubits, m_NumberFormat) + " Qubits";
            ImGui::TextColored(ToImVec4(Color::Yellow()), "%s", goalText.c_str());

            ImGui::Spacing();
            contentStartY = ImGui::GetCursorPosY();
        }

        ImGui::Separator();

        // --- Challenges List (Scrollable Child Window) ---
        f32 challengesContentHeight = panelHeight - contentStartY - 30.0f; // Calculate remaining space

        if (ImGui::BeginChild("##ChallengesList", ImVec2(0, challengesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {

            f32 challengeWidth = ImGui::GetContentRegionAvail().x;
            f32 challengeHeight = 120.0f;

            auto& challenges = m_ChallengeManager.GetChallenges();
            i32 currentPrestige = m_Statistics.totalPrestigesPerformed;

            for (size_t i = 0; i < challenges.size(); i++) {
                const auto& challenge = challenges[i];
                bool completed = challenge.completed;
                bool active = challenge.active;
                // Only allow entering if no challenge is active OR the current one is being checked
                bool canEnter = challenge.CanEnter(currentPrestige, currentChallenge != nullptr && currentChallenge != &challenge);

                Color challengeBg;
                Color challengeBorder;
                if (active) {
                    challengeBg = Color(0.2f, 0.1f, 0.1f, 1.0f);
                    challengeBorder = Color::Red();
                } else if (completed) {
                    challengeBg = Color(0.1f, 0.2f, 0.15f, 1.0f);
                    challengeBorder = Color::CoherenceGreen() * 0.6f;
                } else {
                    challengeBg = Color(0.15f, 0.15f, 0.2f, 1.0f);
                    challengeBorder = Color::Red() * 0.6f;
                }

                // --- Single Challenge Panel (Uses Child for styled background) ---
                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(challengeBg));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(challengeBorder));

                std::string challengeChildName = "##Challenge_" + std::to_string(i);

                if (ImGui::BeginChild(challengeChildName.c_str(), ImVec2(challengeWidth, challengeHeight), true, ImGuiWindowFlags_NoScrollbar)) {

                    // Challenge name
                    std::string nameStr = challenge.name;
                    if (active) nameStr += " [ACTIVE]";
                    if (completed) nameStr += " [COMPLETED]";
                    Color nameColor = completed ? Color::CoherenceGreen() : (active ? Color::Red() : Color::White());
                    ImGui::TextColored(ToImVec4(nameColor), "%s", nameStr.c_str());

                    // Description
                    ImGui::TextWrapped("%s", challenge.description.c_str());

                    // Requirements
                    std::string reqText = "Requires: " + std::to_string(challenge.minPrestigeLevel) + " prestiges";
                    Color reqColor = currentPrestige >= challenge.minPrestigeLevel ? Color::CoherenceGreen() : Color(0.7f, 0.5f, 0.5f, 1.0f);
                    ImGui::TextColored(ToImVec4(reqColor), "%s", reqText.c_str());

                    // Goal
                    std::string goalText = "Goal: " + GameUtils::FormatNumber(challenge.goalQubits, m_NumberFormat) + " Qubits";
                    ImGui::TextColored(ToImVec4(Color::Yellow()), "%s", goalText.c_str());

                    // Reward
                    ImGui::TextColored(ToImVec4(Color::QuantumPurple()), "Reward: %s", challenge.rewardDescription.c_str());

                    // Enter/Exit button
                    if (!completed) {
                        ImGui::SetCursorPosY(challengeHeight - 40.0f);
                        ImGui::SameLine(challengeWidth - 120.0f);

                        if (active) {
                            // Exit button
                            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(Color::Red() * 0.3f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(Color::Red() * 0.5f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::Red()));
                            if (ImGui::Button("EXIT##ChallengeBtn", ImVec2(110.0f, 30.0f))) {
                                // Exit the current challenge
                                m_ChallengeManager.ExitChallenge(this);
                            }
                            ImGui::PopStyleColor(3);
                        } else {
                            // Enter button
                            Color btnColor = canEnter ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                            Color btnHoveredColor = canEnter ? Color::CoherenceGreen() * 1.5f : Color(0.4f, 0.4f, 0.4f, 1.0f);

                            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                            if (ImGui::Button("ENTER##ChallengeBtn", ImVec2(110.0f, 30.0f)) && canEnter) {
                                // Enter the challenge
                                m_ChallengeManager.EnterChallenge(challenge.id, this);
                            }
                            ImGui::PopStyleColor(3);
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                ImGui::Spacing();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    // 4. Pop styles
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void GameState::RenderEssenceShop(Renderer* renderer) {
    if (!m_ShowEssenceShop) return;

    // The renderer is not needed for the ImGui UI structure
    (void)renderer;

    // 1. Setup position and size
    f32 panelWidth = 900.0f;
    f32 panelHeight = 650.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Center the window
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles (Magenta theme)
    Color mainBorderColor = Color::Magenta() * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f; // Approx DarkPanel

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    // 3. Begin the main shop window
    if (ImGui::Begin("💎 Essence Shop - Permanent Upgrades", &m_ShowEssenceShop,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Header (ImGui provides built-in close button) ---
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press E or ESC to close)");
        ImGui::Separator();

        // Current Essence display
        std::string essenceText = "Your Quantum Essence: " + GameUtils::FormatNumber(m_QuantumEssence, m_NumberFormat);
        ImGui::TextColored(ToImVec4(Color::Magenta() * 1.3f), "%s", essenceText.c_str());

        ImGui::Spacing();
        ImGui::Separator();

        // --- Upgrades List (Scrollable Child Window) ---
        f32 upgradesContentHeight = panelHeight - ImGui::GetCursorPosY() - 30.0f; // Calculate remaining space

        if (ImGui::BeginChild("##EssenceUpgradesList", ImVec2(0, upgradesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {

            f32 upgradeWidth = ImGui::GetContentRegionAvail().x;
            f32 upgradeHeight = 100.0f;

            auto& upgrades = m_EssenceShopManager.GetUpgrades();

            for (size_t i = 0; i < upgrades.size(); i++) {
                const auto& upgrade = upgrades[i];
                bool maxed = upgrade.IsMaxed();
                bool canAfford = upgrade.CanAfford(m_QuantumEssence);

                // --- Single Upgrade Panel ---
                Color upgradeBg = maxed ? Color(0.1f, 0.2f, 0.15f, 1.0f) : Color(0.15f, 0.15f, 0.2f, 1.0f);
                Color upgradeBorder = maxed ? Color::CoherenceGreen() * 0.6f : Color::Magenta() * 0.6f;

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(upgradeBg));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(upgradeBorder));

                if (ImGui::BeginChild(upgrade.name.c_str(), ImVec2(upgradeWidth, upgradeHeight), true, ImGuiWindowFlags_NoScrollbar)) {

                    // Upgrade Name and Progress
                    ImGui::TextColored(ToImVec4(Color::White()), "%s", upgrade.name.c_str());
                    ImGui::SameLine(400.0f);
                    ImGui::TextColored(ToImVec4(Color::Magenta()), "Owned: %s", upgrade.GetProgressString().c_str());

                    // Description (TextWrapped handles description wrapping)
                    ImGui::TextWrapped("%s", upgrade.description.c_str());

                    // Cost and Buy button
                    ImGui::SetCursorPosY(upgradeHeight - 35.0f); // Push to bottom of child area

                    if (!maxed) {
                        f64 cost = upgrade.GetCurrentCost();
                        std::string costStr = "Cost: " + GameUtils::FormatNumber(cost, m_NumberFormat) + " Essence";
                        Color costColor = canAfford ? Color::Magenta() * 1.3f : Color(0.7f, 0.5f, 0.5f, 1.0f);
                        ImGui::TextColored(ToImVec4(costColor), "%s", costStr.c_str());

                        // Buy button
                        ImGui::SameLine(upgradeWidth - 120.0f);
                        Color btnColor = canAfford ? Color::Magenta() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                        Color btnHoveredColor = canAfford ? Color::Magenta() * 1.5f : Color(0.4f, 0.4f, 0.4f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                        if (ImGui::Button("PURCHASE", ImVec2(110.0f, 35.0f)) && canAfford) {
                            // Placeholder for actual purchase logic
                            // m_EssenceShopManager.BuyUpgrade(i);
                        }
                        ImGui::PopStyleColor(3);

                    } else {
                        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "MAXED OUT");
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                ImGui::Spacing(); // Add spacing between upgrades
            }
        }
        ImGui::EndChild(); // End EssenceUpgradesList
    }
    ImGui::End(); // End EssenceShopPanel

    // 4. Pop styles
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void GameState::RenderSingularityShop(Renderer* renderer) {
    if (!m_ShowSingularityShop) return;

    // The renderer is only used for internal calls in the original code,
    // but not needed for the UI structure itself.
    (void)renderer;

    // 1. Setup position and size
    f32 panelWidth = 900.0f;
    f32 panelHeight = 650.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Center the window
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles (matching old aesthetics)
    Color mainBorderColor = Color(0.5f, 0.0f, 1.0f, 1.0f) * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f; // Approx DarkPanel

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    // 3. Begin the main shop window
    if (ImGui::Begin("⭐ Singularity Shop - Cosmic Upgrades", &m_ShowSingularityShop,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Header (ImGui provides built-in close button) ---
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press ESC to close)");
        ImGui::Separator();

        // Current Singularities display
        std::string singularityText = "Your Singularities: " + GameUtils::FormatNumber(m_Timeline.singularities, m_NumberFormat);
        ImGui::TextColored(ToImVec4(Color(0.8f, 0.0f, 1.0f, 1.0f) * 1.3f), "%s", singularityText.c_str());

        ImGui::Spacing();
        ImGui::Separator();

        // --- Upgrades List (Scrollable Child Window) ---
        f32 upgradesContentHeight = panelHeight - ImGui::GetCursorPosY() - 30.0f; // Calculate remaining space

        if (ImGui::BeginChild("##UpgradesList", ImVec2(0, upgradesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {

            f32 upgradeWidth = ImGui::GetContentRegionAvail().x;
            f32 upgradeHeight = 100.0f;

            auto& upgrades = m_SingularityShopManager.GetUpgrades();

            for (size_t i = 0; i < upgrades.size(); i++) {
                const auto& upgrade = upgrades[i];
                bool maxed = upgrade.IsMaxed();
                bool canAfford = upgrade.CanAfford(m_Timeline.singularities);

                // --- Single Upgrade Panel ---

                // Use a separate child window for each upgrade to simulate the Rect background/border
                Color upgradeBg = maxed ? Color(0.1f, 0.15f, 0.2f, 1.0f) : Color(0.1f, 0.1f, 0.15f, 1.0f);
                Color upgradeBorder = maxed ? Color::CoherenceGreen() * 0.6f : mainBorderColor * 0.6f;

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(upgradeBg));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(upgradeBorder));

                if (ImGui::BeginChild(upgrade.name.c_str(), ImVec2(upgradeWidth, upgradeHeight), true, ImGuiWindowFlags_NoScrollbar)) {

                    // Upgrade Name and Progress
                    ImGui::TextColored(ToImVec4(Color::White()), "%s", upgrade.name.c_str());
                    ImGui::SameLine(400.0f);
                    ImGui::TextColored(ToImVec4(Color(0.8f, 0.0f, 1.0f, 1.0f)), "Owned: %s", upgrade.GetProgressString().c_str());

                    // Description
                    ImGui::TextWrapped("%s", upgrade.description.c_str());

                    // Cost and Buy button
                    ImGui::SetCursorPosY(upgradeHeight - 35.0f); // Push to bottom of child area

                    if (!maxed) {
                        f64 cost = upgrade.GetCurrentCost();
                        std::string costStr = "Cost: " + GameUtils::FormatNumber(cost, m_NumberFormat) + " Singularities";
                        Color costColor = canAfford ? Color(0.8f, 0.0f, 1.0f, 1.0f) * 1.3f : Color(0.7f, 0.5f, 0.5f, 1.0f);
                        ImGui::TextColored(ToImVec4(costColor), "%s", costStr.c_str());

                        // Buy button
                        ImGui::SameLine(upgradeWidth - 120.0f);
                        Color btnColor = canAfford ? Color(0.5f, 0.0f, 1.0f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f);
                        Color btnHoveredColor = canAfford ? Color(0.8f, 0.0f, 1.0f, 1.0f) : Color(0.4f, 0.4f, 0.4f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                        if (ImGui::Button("PURCHASE", ImVec2(110.0f, 35.0f)) && canAfford) {
                            // Placeholder for actual purchase logic
                            // m_SingularityShopManager.BuyUpgrade(i);
                        }
                        ImGui::PopStyleColor(3);

                    } else {
                        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "MAXED OUT");
                    }

                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                ImGui::Spacing(); // Add spacing between upgrades
            }
        }
        ImGui::EndChild(); // End UpgradesList
    }
    ImGui::End(); // End SingularityShopPanel

    // 4. Pop styles
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void GameState::RenderSpaceship(Renderer* renderer) {
    if (!m_ShowSpaceship) return;

    // We still pass the renderer to the m_Spaceship functions, but the main UI relies on ImGui
    (void)renderer;

    // 1. Setup position and size
    f32 panelWidth = 1100.0f;
    f32 panelHeight = 700.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    // Center the window
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles (matching old aesthetics)
    // Background: DarkPanel (using a representative dark color)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f));
    // Border: Gold (Color(1.0f, 0.7f, 0.0f, 1.0f) * 0.8f)
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color(1.0f, 0.7f, 0.0f, 1.0f) * 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    // 3. Begin the main spaceship window
    if (ImGui::Begin("🚀 Spaceship - Repair and Upgrade", &m_ShowSpaceship,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Header (ImGui provides built-in close button) ---
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press H or ESC to close)");
        ImGui::Separator();

        // --- Split Panel Layout ---
        f32 availableHeight = panelHeight - ImGui::GetCursorPosY() - 40.0f; // Height for the split panels
        f32 spacing = 20.0f;
        f32 totalContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - spacing;
        f32 panelContentWidth = totalContentWidth * 0.5f;

        // Left panel: Ship status and installed parts
        // Use a child window with border (true) and calculated size
        ImGui::BeginChild("##ShipStatusPanel", ImVec2(panelContentWidth, availableHeight), true);
        {
            // The coordinates (0, 0) are now relative to the top-left of this child window.
            // Assuming m_Spaceship::RenderShipPanel is now ImGui-friendly, it will render within the current context.
            m_Spaceship.RenderShipPanel();
        }
        ImGui::EndChild();

        ImGui::SameLine(); // Move cursor to the right

        // Right panel: Part inventory
        ImGui::BeginChild("##InventoryPanel", ImVec2(panelContentWidth, availableHeight), true);
        {
            // Assuming m_Spaceship::RenderInventoryPanel is now ImGui-friendly
            m_Spaceship.RenderInventoryPanel();

        }
        ImGui::EndChild();

        // --- Instructions at bottom ---
        // Need to push the cursor down to the bottom of the main window for the instructions
        ImGui::SetCursorPosY(panelHeight - 35.0f);
        ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.7f, 1.0f)),
                           "Ship parts drop from Research Station observations. Install parts to increase production & unlock travel!");
    }
    ImGui::End();

    // 4. Pop styles
    ImGui::PopStyleVar(3); // Pop WindowPadding, WindowBorderSize, WindowRounding
    ImGui::PopStyleColor(2); // Pop Border, WindowBg
}
// ============================================================================
// VISUAL EFFECTS & ACTIVE GAMEPLAY SYSTEMS
// ============================================================================

// Quantum Anomaly System - Clickable orbs for active rewards
void GameState::SpawnQuantumAnomaly() {
    QuantumAnomaly anomaly;

    // Random position (avoid edges and fixed UI areas)
    f32 marginX = 100.0f;
    f32 marginY = 250.0f; // Avoid top resource/nav area
    anomaly.position.x = marginX + (rand() % (1280 - static_cast<i32>(marginX * 2)));
    anomaly.position.y = marginY + (rand() % (720 - static_cast<i32>(marginY * 2)));

    // Size and lifetime
    anomaly.radius = 20.0f + (rand() % 15); // 20-35px radius
    anomaly.lifetime = 0.0f;
    anomaly.maxLifetime = 10.0f; // 10 seconds to click it

    // Reward multiplier (higher = better reward)
    i32 rarity = rand() % 100;
    if (rarity < 60) {
        // Common: 10-30x production
        anomaly.rewardMultiplier = 10.0 + (rand() % 21);
        anomaly.color = Color(0.0f, 0.8f, 1.0f, 1.0f); // Cyan
    } else if (rarity < 85) {
        // Uncommon: 30-60x production
        anomaly.rewardMultiplier = 30.0 + (rand() % 31);
        anomaly.color = Color(0.2f, 1.0f, 0.2f, 1.0f); // Green
    } else if (rarity < 95) {
        // Rare: 60-100x production
        anomaly.rewardMultiplier = 60.0 + (rand() % 41);
        anomaly.color = Color(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
    } else {
        // Legendary: 100-200x production
        anomaly.rewardMultiplier = 100.0 + (rand() % 101);
        anomaly.color = Color(1.0f, 0.8f, 0.0f, 1.0f); // Gold
    }

    anomaly.clicked = false;
    m_Anomalies.push_back(anomaly);

    Log::Infof("Quantum Anomaly spawned! Reward: ", anomaly.rewardMultiplier, "x");
}

void GameState::UpdateQuantumAnomalies(f64 deltaTime) {
    // Update existing anomalies
    const ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    const f32 cullPadding = 32.0f;
    for (auto it = m_Anomalies.begin(); it != m_Anomalies.end();) {
        it->lifetime += deltaTime;

        const bool offscreen =
            (it->position.x + it->radius < -cullPadding) ||
            (it->position.x - it->radius > displaySize.x + cullPadding) ||
            (it->position.y + it->radius < -cullPadding) ||
            (it->position.y - it->radius > displaySize.y + cullPadding);

        // Remove if expired or clicked
        if (it->lifetime >= it->maxLifetime || it->clicked || offscreen) {
            if (it->lifetime >= it->maxLifetime && !it->clicked) {
                Log::Info("Quantum Anomaly expired...");
            }
            it = m_Anomalies.erase(it);
        } else {
            ++it;
        }
    }

    // Spawn new anomalies periodically
    m_TimeSinceLastAnomaly += deltaTime;
    if (m_TimeSinceLastAnomaly >= m_AnomalySpawnInterval) {
        SpawnQuantumAnomaly();
        m_TimeSinceLastAnomaly = 0.0;
        // Randomize next spawn time (30-60 seconds)
        m_AnomalySpawnInterval = 30.0 + (rand() % 31);
    }
}

void GameState::RenderQuantumAnomalies(Renderer* renderer) {
    (void)renderer; // The custom renderer is no longer needed

    // Get the background draw list to draw elements beneath ImGui windows
    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    for (const auto& anomaly : m_Anomalies) {
        f32 lifeRatio = 1.0f - (anomaly.lifetime / anomaly.maxLifetime);

        // Pulsing effect
        f32 pulseScale = 1.0f + 0.2f * sinf(anomaly.lifetime * 5.0f);
        f32 currentRadius = anomaly.radius * pulseScale;

        // Convert anomaly position to ImGui format
        ImVec2 anomalyPos = ToImVec2(anomaly.position);

        // 1. Outer glow (larger, transparent)
        Color glowColor = anomaly.color;
        glowColor.a = 0.3f * lifeRatio;
        draw_list->AddCircleFilled(anomalyPos, currentRadius * 1.5f, ToImU32(glowColor));

        // 2. Inner core (bright, opaque)
        Color coreColor = anomaly.color;
        coreColor.a = 0.9f * lifeRatio;
        draw_list->AddCircleFilled(anomalyPos, currentRadius, ToImU32(coreColor));

        // 3. White center dot
        draw_list->AddCircleFilled(anomalyPos, currentRadius * 0.3f, ToImU32(Color::White()));

        // 4. Show reward multiplier text above it
        std::string rewardText = std::to_string(static_cast<i32>(anomaly.rewardMultiplier)) + "x";

        // Calculate text position
        ImVec2 textPos(anomalyPos.x - 15.0f, anomalyPos.y - currentRadius - 20.0f);
        draw_list->AddText(textPos, ToImU32(Color::White()), rewardText.c_str());

        // 5. Lifetime bar below it
        f32 barWidth = anomaly.radius * 2.0f;
        f32 barHeight = 4.0f;

        // Calculate bar position
        ImVec2 barPosStart(
            anomalyPos.x - barWidth / 2.0f,
            anomalyPos.y + currentRadius + 10.0f
        );
        ImVec2 barPosEnd(
            barPosStart.x + barWidth,
            barPosStart.y + barHeight
        );

        // 5a. Background Rect
        draw_list->AddRectFilled(barPosStart, barPosEnd, ToImU32(Color(0.2f, 0.2f, 0.2f, 0.8f)));

        // 5b. Fill Rect
        ImVec2 barFillEnd(barPosStart.x + barWidth * lifeRatio, barPosStart.y + barHeight);
        draw_list->AddRectFilled(barPosStart, barFillEnd, ToImU32(anomaly.color));
    }
}

void GameState::ClickQuantumAnomaly(const Vec2& clickPos) {
    // Phase 1.3: This function is now only called when ImGui is NOT capturing mouse
    // (see UpdateUI for the WantCaptureMouse check)
    // This prevents clicking anomalies through UI windows

    for (auto& anomaly : m_Anomalies) {
        if (anomaly.clicked) continue;

        // Check if click is within anomaly circle
        f32 dx = clickPos.x - anomaly.position.x;
        f32 dy = clickPos.y - anomaly.position.y;
        f32 distSq = dx * dx + dy * dy;
        f32 radiusSq = anomaly.radius * anomaly.radius;

        if (distSq <= radiusSq) {
            // Clicked! Award bonus resources
            anomaly.clicked = true;

            // Calculate reward based on current total production
            f64 totalProduction = 0.0;
            for (const auto& station : m_Stations) {
                if (station.unlocked) {
                    totalProduction += station.currentProduction;
                }
            }

            // Give reward equal to N seconds of production
            f64 secondsWorth = anomaly.rewardMultiplier;
            f64 qubitReward = totalProduction * secondsWorth * m_Timeline.photonBonus;

            AddResource(QuantumResource::Qubits, qubitReward);

            // Visual feedback: particle burst
            SpawnParticleBurst(anomaly.position, anomaly.color, 30);

            // Add combo point
            AddComboPoint();

            Log::Infof("Anomaly claimed! Gained ", qubitReward, " qubits (", anomaly.rewardMultiplier, "x)");

            return; // Only click one anomaly per click
        }
    }
}

// Combo System - Reward multiple interactions
void GameState::AddComboPoint() {
    m_ComboCount++;
    m_ComboTimeRemaining = m_ComboWindow; // Reset timer (5 seconds)

    if (m_ComboCount >= 3) {
        Log::Infof("COMBO x", m_ComboCount, "!");
    }
}

void GameState::ResetCombo() {
    if (m_ComboCount > 0) {
        Log::Info("Combo broken!");
    }
    m_ComboCount = 0;
    m_ComboTimeRemaining = 0.0;
}

f64 GameState::GetComboMultiplier() const {
    if (m_ComboCount <= 1) return 1.0;

    // 2 combo = 1.2x, 3 combo = 1.5x, 4 combo = 2.0x, 5+ combo = 2.5x
    if (m_ComboCount == 2) return 1.2;
    if (m_ComboCount == 3) return 1.5;
    if (m_ComboCount == 4) return 2.0;
    return 2.5;
}

// Station Visual Tiers - Color changes based on level
Color GameState::GetStationTierColor(i32 level) const {
    if (level < 10) {
        // Tier 1: Quantum Blue (default)
        return Color::QuantumBlue();
    } else if (level < 25) {
        // Tier 2: Neon Cyan
        return Color::NeonCyan();
    } else if (level < 50) {
        // Tier 3: Electric Purple
        return Color(0.5f, 0.0f, 1.0f, 1.0f);
    } else if (level < 100) {
        // Tier 4: Magenta/Pink
        return Color::Magenta();
    } else {
        // Tier 5: Cosmic Gold
        return Color(1.0f, 0.8f, 0.0f, 1.0f);
    }
}

// Flying Resource Particles - Visual feedback for production
void GameState::SpawnResourceParticles(const Vec2& start, const Vec2& end, const Color& color, i32 count) {
    for (i32 i = 0; i < count; i++) {
        Particle p;
        p.position = start;

        // Calculate velocity to reach end point
        Vec2 direction(end.x - start.x, end.y - start.y);
        f32 distance = sqrtf(direction.x * direction.x + direction.y * direction.y);

        if (distance > 0.1f) {
            direction.x /= distance;
            direction.y /= distance;

            // Speed varies slightly for visual variety
            f32 speed = 300.0f + (rand() % 100);
            p.velocity.x = direction.x * speed;
            p.velocity.y = direction.y * speed;
        }

        p.color = color;
        p.maxLifetime = 0.8f + (rand() % 40) / 100.0f; // 0.8-1.2 seconds
        p.lifetime = 0.0f;

        m_Particles.push_back(p);
    }
}

// Update particles (fade out over time)

// ============================================================================
// PHASE C & D: COLLECTION SYSTEM & BIOME/THEMES
// ============================================================================

// These will be implemented at the end of the file to keep code organized

// ============================================================================
// COMBAT SYSTEM INTEGRATION
// ============================================================================

void GameState::StartRandomCombat() {
    // Generate random enemy based on player level
    m_CurrentEnemy = EnemyGenerator::GenerateEnemy(m_PlayerLevel);

    // Start combat with current enemy (pass this to apply skill bonuses)
    m_CombatSystem.StartCombat(&m_CurrentEnemy, m_PlayerLevel, &m_Spaceship, nullptr, this);
    
    Log::Infof("Starting combat with ", m_CurrentEnemy.GetName());
}

void GameState::EndCombat() {
    if (!m_CombatSystem.IsInCombat()) return;
    
    // Check if we won and award rewards
    if (m_CombatSystem.GetState() == CombatState::Victory) {
        // Award credits (as Qubits)
        AddResource(QuantumResource::Qubits, static_cast<f64>(m_CombatSystem.GetCreditsEarned()));

        // Track raw credits for conversion systems
        AddPlayerCredits(m_CombatSystem.GetCreditsEarned());

        // Award research data from combat intel
        i32 researchData = std::max(1, m_PlayerLevel / 5);
        AddResearchData(researchData);

        // Award XP
        AddXP(static_cast<f64>(m_CombatSystem.GetXPEarned()));

        // Award Command skill XP for combat victory
        m_SpecializedSkills.AddExperience(SkillCategory::Command, SkillXP::WIN_COMBAT);

        // Play victory sound
        m_SoundManager.PlaySound(SoundEffect::Victory);

        // Award enhancement materials based on enemy level
        i32 techScraps = 2 + (m_PlayerLevel / 5); // 2-22 scraps
        i32 nanoAlloy = (m_PlayerLevel >= 10) ? (1 + m_PlayerLevel / 10) : 0; // 0-11 alloy
        i32 quantumCore = (m_PlayerLevel >= 30) ? (m_PlayerLevel / 30) : 0; // 0-3 cores

        m_EnhancementSystem.AddMaterial(MaterialType::TechScraps, techScraps);
        if (nanoAlloy > 0) m_EnhancementSystem.AddMaterial(MaterialType::NanoAlloy, nanoAlloy);
        if (quantumCore > 0) m_EnhancementSystem.AddMaterial(MaterialType::QuantumCore, quantumCore);

        // Award ship part if dropped
        if (m_CombatSystem.GetPartDropped()) {
            // Generate part based on enemy's min rarity
            i32 minRarity = m_CurrentEnemy.GetMinPartRarity();
            PartRarity rarity = static_cast<PartRarity>(minRarity);
            
            // Random chance for higher rarity
            i32 roll = rand() % 100;
            if (roll < 20 && minRarity < 4) { // 20% chance for +1 rarity
                rarity = static_cast<PartRarity>(minRarity + 1);
            }
            
            ShipPart droppedPart = ShipPartGenerator::GeneratePart(rarity);
            m_Spaceship.AddPart(droppedPart);

            // Play loot sound based on rarity
            switch (rarity) {
                case PartRarity::Legendary:
                    m_SoundManager.PlaySound(SoundEffect::LootLegendary);
                    break;
                case PartRarity::Epic:
                    m_SoundManager.PlaySound(SoundEffect::LootEpic);
                    break;
                case PartRarity::Rare:
                    m_SoundManager.PlaySound(SoundEffect::LootRare);
                    break;
                default:
                    m_SoundManager.PlaySound(SoundEffect::LootDrop);
                    break;
            }

            Log::Infof("Combat reward: ", droppedPart.GetRarityName(), " ", droppedPart.name);
        }
    }
    
    // End the combat
    m_CombatSystem.EndCombat();
    m_ShowCombat = false;
    
    Log::Info("Combat ended");
}

f64 GameState::GetXPForNextLevel() const {
    // XP required scales exponentially: 100 * (level ^ 1.5)
    return 100.0 * pow(static_cast<f64>(m_PlayerLevel), 1.5);
}

void GameState::AddXP(f64 amount) {
    m_PlayerXP += amount;

    // Check for level up
    f64 xpRequired = GetXPForNextLevel();
    while (m_PlayerXP >= xpRequired && m_PlayerLevel < 100) {
        m_PlayerXP -= xpRequired;
        m_PlayerLevel++;

        Log::Infof("LEVEL UP! You are now level ", m_PlayerLevel);

        // Play level up sound
        m_SoundManager.PlaySound(SoundEffect::LevelUp);

        // Check for unlocked features
        m_UnlockManager.OnLevelUp(m_PlayerLevel);

        // Award Stellar Shards on level up!
        i32 shardsEarned = 1 + (m_PlayerLevel / 10); // 1 shard + bonus every 10 levels
        m_GatchaSystem.AddStellarShards(shardsEarned);
        Log::Infof("Earned ", shardsEarned, " Stellar Shards!");

        // Award Summon Tickets at milestone levels
        if (m_PlayerLevel % 10 == 0) {
            m_GatchaSystem.AddSummonTickets(1);
            Log::Info("Earned 1 Summon Ticket!");
        }

        // Award Skill Points on level up!
        m_SkillTree.AddSkillPoints(1);
        Log::Info("Earned 1 Skill Point!");

        // Spawn celebration particles
        SpawnParticleBurst(Vec2(640.0f, 360.0f), Color(1.0f, 0.9f, 0.0f, 1.0f), 30);

        // Recalculate for next level
        xpRequired = GetXPForNextLevel();
    }
}

// In src/game/GameState.cpp

void GameState::RenderCombat(Renderer* renderer) {
    if (!m_ShowCombat) return;

    // Pass the renderer (even if unused in ImGui logic, we keep the signature)
    // m_CombatSystem.RenderCombatUI(renderer) handles the main window logic
    m_CombatSystem.RenderCombatUI(renderer);

    // Get the ImGui draw list for the current *active* window (which is the one created by RenderCombatUI)
    // NOTE: This must be called *after* RenderCombatUI is run to get the correct context.

    // Define XP bar size (must match the layout chosen in RenderCombatUI if applicable)
    // Since CombatSystem uses a large window, we'll draw the XP bar directly to the screen background
    // for simplicity, or assume a fixed position on the screen.

    // XP Bar Position (Fixed Top Left)
    f32 xpBarWidth = 300.0f;
    f32 xpBarHeight = 25.0f;

    // We will draw to the main background list to ensure visibility over the combat window if needed,
    // positioning it relative to the top-left of the overall screen (0,0).
    ImDrawList* bg_draw_list = ImGui::GetBackgroundDrawList();

    ImVec2 xpBarStart(10.0f, 10.0f);
    ImVec2 xpBarEnd(xpBarStart.x + xpBarWidth, xpBarStart.y + xpBarHeight);

    f64 xpRequired = GetXPForNextLevel();
    f64 xpPercent = xpRequired > 0 ? (m_PlayerXP / xpRequired) : 0.0;

    // 1. Draw Background Rect
    bg_draw_list->AddRectFilled(xpBarStart, xpBarEnd, ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, 0.8f)));

    // 2. Draw Fill Rect
    ImVec2 xpBarFillEnd(xpBarStart.x + xpBarWidth * xpPercent, xpBarEnd.y);
    bg_draw_list->AddRectFilled(xpBarStart, xpBarFillEnd, ImGui::GetColorU32(ImVec4(1.0f, 0.9f, 0.0f, 1.0f))); // Gold

    char levelText[64];
    snprintf(levelText, sizeof(levelText), "Level %d - %.0f / %.0f XP", m_PlayerLevel, m_PlayerXP, xpRequired);

    // 3. Draw Text (Centered over the bar)
    ImVec2 textPos(xpBarStart.x + 5.0f, xpBarStart.y + 5.0f);
    bg_draw_list->AddText(textPos, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), levelText);

    // Crucial step: Ensure the cursor is not left behind. Since we used the BG draw list, the main
    // ImGui window cursor position is fine, but we'll manually advance it just to be safe if CombatSystem
    // didn't push it far enough (if CombatSystem used the cursor, it must reset it).

    // We skip the cursor position adjustment as we drew on the background layer, which doesn't affect the layout.
}

void GameState::RenderGatcha(Renderer* renderer) {
    if (!m_ShowGatcha) return;

    m_GatchaSystem.RenderSummonUI(renderer, this);

}

void GameState::DeductPlayerCredits(i32 amount) { // <-- FIX IS HERE
    if (amount > 0) {
        // Now the compiler knows 'm_PlayerCredits' is a member of 'this' GameState object
        m_PlayerCredits = std::max(0, m_PlayerCredits - amount);
        // Ensure Log::Infof is also available
        // Log::Infof("Deducted %d credits. Remaining: %d", amount, m_PlayerCredits);
    }
}

// Phase 3.3: Exotic Materials Management
void GameState::AddExoticMaterials(i32 amount) {
    if (amount > 0) {
        m_ExoticMaterials += amount;
        Log::Infof("Gained ", amount, " Exotic Materials! Total: ", m_ExoticMaterials);

        // Spawn floating text for visual feedback
        if (m_GuiLayer && m_GuiLayer->GetFloatingTextManager()) {
            ImGuiIO& io = ImGui::GetIO();
            Vec2 position(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.3f);
            ImVec4 color(1.0f, 0.5f, 1.0f, 1.0f); // Purple/magenta for exotic materials
            std::string text = "+" + std::to_string(amount) + " Exotic Materials";
            m_GuiLayer->GetFloatingTextManager()->SpawnText(text, position, color, 2.5f);
        }
    }
}

void GameState::AddResearchData(i32 amount) {
    if (amount <= 0) return;
    m_ResearchData += amount;

    if (m_GuiLayer && m_GuiLayer->GetFloatingTextManager()) {
        ImGuiIO& io = ImGui::GetIO();
        Vec2 position(io.DisplaySize.x * 0.55f, io.DisplaySize.y * 0.28f);
        ImVec4 color(0.3f, 0.9f, 1.0f, 1.0f);
        std::string text = "+" + std::to_string(amount) + " Research Data";
        m_GuiLayer->GetFloatingTextManager()->SpawnText(text, position, color, 1.6f);
    }
}

bool GameState::SpendResearchData(i32 amount) {
    if (amount <= 0) return true;
    if (m_ResearchData < amount) return false;
    m_ResearchData -= amount;
    return true;
}

bool GameState::SpendExoticMaterials(i32 amount) {
    if (amount <= 0) return true;
    if (m_ExoticMaterials >= amount) {
        m_ExoticMaterials -= amount;
        return true;
    }
    return false;
}

// Phase 3.2: Credit Conversion System Implementation
void GameState::ConvertCreditsToProduction(i32 credits) {
    if (credits <= 0 || m_PlayerCredits < credits) {
        return; // Not enough credits
    }

    // Calculate bonus (100 credits = 1% = 0.01 multiplier)
    f64 bonusGained = static_cast<f64>(credits) / m_CreditConversionRate * 0.01;

    // Deduct credits
    DeductPlayerCredits(credits);

    // Add to production multiplier (permanent bonus)
    m_CreditProductionMultiplier += bonusGained;

    Log::Infof("Converted ", credits, " credits to +",
               static_cast<i32>(bonusGained * 100.0), "% production! Total: +",
               static_cast<i32>((m_CreditProductionMultiplier - 1.0) * 100.0), "%");

    // Spawn particle effect
    SpawnParticleBurst(Vec2(640.0f, 360.0f), Color::EntanglementOrange(), 20);
}

f64 GameState::CalculateProductionBonusFromCredits(i32 credits) const {
    // Returns the % bonus that would be gained from converting X credits
    return (static_cast<f64>(credits) / m_CreditConversionRate * 0.01) * 100.0;
}

// Implement the setter for Gatcha UI visibility
void GameState::SetGatchaUIVisible(bool visible) {
    m_ShowGatcha = visible;
    Log::Infof("Gatcha UI visibility set to %s", visible ? "true" : "false");
}

void GameState::RenderSkillTree(Renderer* renderer) {
    if (!m_ShowSkills) return;

    // Render the skill tree UI
    // m_SkillTree.RenderSkillTree handles the ImGui::Begin/End window creation
    m_SkillTree.RenderSkillTree(renderer, this);

    // NOTE: The previous manual mouse click check for the close button in UpdateUI
    // is now handled implicitly by ImGui within the RenderSkillTree function.
}

void GameState::RenderSpecializedSkills(Renderer* renderer) {
    (void)renderer; // Unused in ImGui rendering

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

    // Header
    ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "SPECIALIZED SKILLS");
    ImGui::TextWrapped("Gain experience through gameplay actions. Each skill provides unique bonuses.");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Summary stats
    ImGui::Text("Total Skill Level: %d", m_SpecializedSkills.GetTotalSkillLevel());
    ImGui::Text("Average Skill Level: %d", m_SpecializedSkills.GetAverageSkillLevel());
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // Skill cards in a grid
    float cardWidth = 350.0f;
    float availableWidth = ImGui::GetContentRegionAvail().x;
    i32 cardsPerRow = static_cast<i32>(availableWidth / (cardWidth + 10.0f));
    if (cardsPerRow < 1) cardsPerRow = 1;

    // Define skill data
    struct SkillDisplay {
        SkillCategory category;
        const char* name;
        const char* icon;
        const char* description;
        ImVec4 color;
        const char* bonusDesc;
    };

    SkillDisplay skills[] = {
        {
            SkillCategory::Observation,
            "OBSERVATION",
            "👁",
            "Production & Discovery\nGain XP by observing and unlocking stations.",
            ImVec4(0.3f, 0.7f, 1.0f, 1.0f),
            "+%% Global Production"
        },
        {
            SkillCategory::Engineering,
            "ENGINEERING",
            "⚙",
            "Efficiency & Building\nGain XP by upgrading stations and researching.",
            ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
            "+%% Cost Reduction"
        },
        {
            SkillCategory::Command,
            "COMMAND",
            "⚔",
            "Combat & Management\nGain XP by winning battles and managing crew.",
            ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
            "+%% Combat Power"
        }
    };

    // Render each skill card
    for (i32 i = 0; i < 3; i++) {
        const SkillDisplay& display = skills[i];
        const SpecializedSkill& skill = m_SpecializedSkills.GetSkill(display.category);

        // Card background
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.2f, 0.9f));
        ImGui::PushStyleColor(ImGuiCol_Border, display.color);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

        char cardId[64];
        snprintf(cardId, sizeof(cardId), "SkillCard%d", i);

        if (ImGui::BeginChild(cardId, ImVec2(cardWidth, 220.0f), true)) {
            // Icon and title
            ImGui::PushFont(ImGui::GetFont()); // Use current font (you can customize)
            ImGui::TextColored(display.color, "%s %s", display.icon, display.name);
            ImGui::PopFont();

            ImGui::Spacing();

            // Level display
            ImGui::Text("Level: %d", skill.level);

            // XP Progress bar
            f32 progress = static_cast<f32>(m_SpecializedSkills.GetSkillProgress(display.category));
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, display.color);
            ImGui::ProgressBar(progress, ImVec2(-1, 25));
            ImGui::PopStyleColor();

            // XP text
            ImGui::Text("XP: %.0f / %.0f", skill.experience, skill.experienceToNextLevel);

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Description
            ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cardWidth - 40);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", display.description);
            ImGui::PopTextWrapPos();

            ImGui::Spacing();

            // Bonus display
            f64 bonusPercent = (skill.GetBonusMultiplier() - 1.0) * 100.0;
            ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "Bonus: +%.0f%% (%s)",
                             bonusPercent, display.bonusDesc);
        }
        ImGui::EndChild();

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);

        // Same line for next card if not the last in row
        if ((i + 1) % cardsPerRow != 0 && i < 2) {
            ImGui::SameLine(0, 10.0f);
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // XP Rewards Reference Table
    if (ImGui::CollapsingHeader("XP Rewards Reference", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Columns(3, "XPTable", true);

        // Observation column
        ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "OBSERVATION");
        ImGui::Separator();
        ImGui::Text("Observe Station: +5 XP");
        ImGui::Text("Unlock Station: +25 XP");
        ImGui::NextColumn();

        // Engineering column
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "ENGINEERING");
        ImGui::Separator();
        ImGui::Text("Upgrade Station: +10 XP");
        ImGui::Text("Purchase Research: +20 XP");
        ImGui::Text("Buy Upgrade: +15 XP");
        ImGui::NextColumn();

        // Command column
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "COMMAND");
        ImGui::Separator();
        ImGui::Text("Win Combat: +30 XP");
        ImGui::Text("Install Ship Part: +15 XP");
        ImGui::NextColumn();

        ImGui::Columns(1);
    }

    ImGui::PopStyleVar();
}