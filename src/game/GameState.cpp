#include "GameState.h"
#include "Renderer.h"
#include "Input.h"
#include "Logger.h"
#include "Platform.h"
#include "GameUtils.h"
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

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
    }

    state->AddResource(resourceType, collapsedValue);
    superpositionValue = 0;

    // Update statistics
    auto& stats = state->GetStatistics();
    stats.totalObservations++;
    stats.sessionObservations++;

    // Add combo point for observation
    state->AddComboPoint();

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
}

// UIButton implementation
void UIButton::Update(const Vec2& mousePos) {
    hovered = enabled && bounds.Contains(mousePos);
}

// In UIButton implementation

void UIButton::Render(Renderer* renderer) {
    // --- 1. Determine Button Colors and State ---
    Color renderColor, borderColor, textColor;
    f32 baseAlpha = 0.85f;

    if (!enabled) {
        // Disabled: Color-code based on affordability (how close to affording)
        if (affordability >= 0.75) {
            // Close to affording (75-99%) - Yellow tint
            renderColor = Color(0.25f, 0.25f, 0.1f, baseAlpha * 0.5f);
            borderColor = Color::Yellow() * 0.4f;
            textColor = Color(0.9f, 0.9f, 0.6f, 1.0f);
        } else if (affordability >= 0.5) {
            // Halfway there (50-74%) - Orange tint
            renderColor = Color(0.25f, 0.15f, 0.1f, baseAlpha * 0.5f);
            borderColor = Color::EntanglementOrange() * 0.4f;
            textColor = Color(0.9f, 0.7f, 0.5f, 1.0f);
        } else {
            // Far from affording (<50%) - Red/gray tint
            renderColor = Color(0.2f, 0.1f, 0.1f, baseAlpha * 0.5f);
            borderColor = Color(0.4f, 0.2f, 0.2f, baseAlpha * 0.5f);
            textColor = Color(0.7f, 0.5f, 0.5f, 1.0f);
        }
    } else if (hovered) {
        // Hovered: Brighter color, strong white border, strong glow
        renderColor = hoverColor * 0.9f;
        renderColor.a = baseAlpha + 0.1f;
        borderColor = Color::White() * 0.8f;
        textColor = Color::White();
    } else {
        // Normal: Standard color
        renderColor = color;
        renderColor.a = baseAlpha;
        borderColor = color * 1.5f; // Slight color-matched border
        borderColor.a = 0.5f;
        textColor = Color::White();
    }

    // --- 2. Draw Background (Subtle flat fill) ---
    // Use a single, slightly transparent fill for a 'glass' effect
    renderer->DrawRect(bounds, renderColor, true);

    // --- 3. Draw Border ---
    renderer->DrawRect(bounds, borderColor, false);
    
    // --- 4. Draw Hover Glow (for modern feedback) ---
    if (hovered && enabled) {
        // Create a distinct glow effect outside the main button
        Rect glowRect(bounds.x - 1.0f, bounds.y - 1.0f,
                      bounds.width + 2.0f, bounds.height + 2.0f);
        Color glowColor = hoverColor;
        glowColor.a = 0.3f; // Less opaque glow
        renderer->DrawRect(glowRect, glowColor, false);
    }

    // --- 5. Draw Text (Perfectly Centered for professionalism) ---
    Vec2 textPos = bounds.Center();
    
    // NOTE: This text rendering part still relies on a rough text width estimate.
    // For true professionalism, this must be replaced with accurate font rendering metrics (e.g., proper TTF text size calculation).
    f32 approxTextWidth = text.length() * 9.0f; // Rough estimate for 16pt font
    
    textPos.x -= approxTextWidth * 0.5f;
    textPos.y -= 8.0f; // Adjust for vertical centering (font-dependent)

    // No text shadow for a flatter, cleaner look
    renderer->DrawText(text, textPos, textColor, 16.0f);
}

bool UIButton::WasClicked(const Vec2& mousePos, bool mousePressed) {
    if (!enabled) return false;
    if (!bounds.Contains(mousePos)) return false;
    return mousePressed;
}

// GameState implementation
GameState::GameState()
    : m_CurrentEvent(nullptr), m_TimeSinceLastEvent(0), m_EventCooldown(120.0),
      m_QuantumEssence(0),
      m_PlayerLevel(1), m_PlayerXP(0.0),
      m_LastSaveTimestamp(0),
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
}

GameState::~GameState() {
}

void GameState::Initialize() {
    Log::Info("Initializing game state...");

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
    m_ResearchTree.Initialize();
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
    m_Spaceship.Initialize();
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
}

void GameState::InitializeUI() {
    m_ScrollOffset = Vec2(0, 0);
    m_StationButtons.clear();

    // Create persistent buttons for each research station
    // For each station, we create: unlock button (4*i), observe button (4*i+1), upgrade button (4*i+2), buy max button (4*i+3)
    for (size_t i = 0; i < m_Stations.size(); i++) {
        // Unlock Button
        UIButton unlockBtn;
        unlockBtn.text = "Unlock";
        unlockBtn.color = Color::QuantumBlue() * 0.7f;
        unlockBtn.hoverColor = Color::QuantumBlue();
        unlockBtn.onClick = [this, i]() {
            if (SpendResource(QuantumResource::Qubits, m_Stations[i].unlockCost)) {
                m_Stations[i].unlocked = true;
                m_Stations[i].level = 0;
                Log::Infof("Unlocked: ", m_Stations[i].name);
            }
        };
        m_StationButtons.push_back(unlockBtn);

        // Observe Button
        UIButton observeBtn;
        observeBtn.text = "OBSERVE";
        observeBtn.color = Color::QuantumPurple() * 0.7f;
        observeBtn.hoverColor = Color::QuantumPurple();
        observeBtn.onClick = [this, i]() {
            auto& st = m_Stations[i];
            st.Observe(this);
            Log::Infof("Observed ", st.name);
        };
        m_StationButtons.push_back(observeBtn);

        // Upgrade Button
        UIButton upgradeBtn;
        upgradeBtn.text = "Upgrade";
        upgradeBtn.color = Color::EntanglementOrange() * 0.7f;
        upgradeBtn.hoverColor = Color::EntanglementOrange();
        upgradeBtn.onClick = [this, i]() {
            // Calculate effective upgrade cost with challenge modifiers
            f64 effectiveCost = m_Stations[i].upgradeCost;
            if (m_ChallengeManager.HasModifier(ChallengeModifier::ExpensiveUpgrades)) {
                effectiveCost *= 3.0;
            }

            if (SpendResource(QuantumResource::Qubits, effectiveCost)) {
                m_Stations[i].Upgrade();
                UpdateResearchBonuses(); // Recalculate production
                Log::Infof("Upgraded ", m_Stations[i].name, " to level ", m_Stations[i].level);
            }
        };
        m_StationButtons.push_back(upgradeBtn);

        // Buy Max Button
        UIButton buyMaxBtn;
        buyMaxBtn.text = "BUY MAX";
        buyMaxBtn.color = Color::CoherenceGreen() * 0.7f;
        buyMaxBtn.hoverColor = Color::CoherenceGreen();
        buyMaxBtn.onClick = [this, i]() {
            auto& station = m_Stations[i];
            f64 currentQubits = GetResource(QuantumResource::Qubits);
            i32 upgradesBought = 0;

            // Check if expensive upgrades modifier is active
            bool expensiveUpgrades = m_ChallengeManager.HasModifier(ChallengeModifier::ExpensiveUpgrades);

            // Keep buying while we can afford it
            while (upgradesBought < 1000) { // Cap at 1000 to prevent infinite loops
                // Calculate effective upgrade cost with challenge modifiers
                f64 effectiveCost = station.upgradeCost;
                if (expensiveUpgrades) {
                    effectiveCost *= 3.0;
                }

                if (currentQubits >= effectiveCost) {
                    if (SpendResource(QuantumResource::Qubits, effectiveCost)) {
                        station.Upgrade();
                        currentQubits = GetResource(QuantumResource::Qubits);
                        upgradesBought++;
                    } else {
                        break;
                    }
                } else {
                    break;
                }
            }

            if (upgradesBought > 0) {
                UpdateResearchBonuses(); // Recalculate production
                Log::Infof("Bought ", upgradesBought, " upgrades for ", station.name, " (now level ", station.level, ")");
            }
        };
        m_StationButtons.push_back(buyMaxBtn);
    }

    // Create Prestige Button (last button in the list)
    UIButton prestigeBtn;
    prestigeBtn.text = "PRESTIGE";
    prestigeBtn.color = Color::Magenta() * 0.5f;
    prestigeBtn.hoverColor = Color::Magenta();
    prestigeBtn.onClick = [this]() {
        PerformPrestige();
    };
    m_StationButtons.push_back(prestigeBtn);
}

void GameState::Update(f64 deltaTime, Input* input, Renderer* renderer) {
    (void)renderer; // Unused parameter - reserved for future use

    m_TotalTimePlayed += deltaTime;
    m_TimeSinceLastSave += deltaTime;
    m_TimeSinceLastEvent += deltaTime;

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

    // Track time for fastest prestige achievement
    m_TimeSinceLastPrestige += deltaTime;

    // Update stations
    UpdateStations(deltaTime);

    // Auto-research if enabled (automatically purchase research when affordable)
    auto availableResearch = m_ResearchTree.GetAvailableResearch(m_Timeline.completedResets);
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
    if (m_ResearchTree.IsResearched(ResearchID::AutoPrestige)) {
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

    // Check if Auto-Observer research is unlocked
    bool hasAutoObserver = m_ResearchTree.IsResearched(ResearchID::AutoObserver);

    // Check if manual observation is disabled by challenge
    bool canManuallyObserve = !m_ChallengeManager.HasModifier(ChallengeModifier::NoObserve);
    (void)canManuallyObserve; // Reserved for future use

    f64 currentQubits = GetResource(QuantumResource::Qubits);
    bool canUpgrade = !m_ChallengeManager.HasModifier(ChallengeModifier::NoUpgrades);

    for (auto& station : m_Stations) {
        station.Update(deltaTime * globalMultiplier);

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

void GameState::UpdateUI(Input* input) {
    Vec2 mousePos = input->GetMousePosition();
    bool mousePressed = input->IsMouseButtonPressed(MouseButton::Left);

    // Scrolling support
    if (!m_ShowAchievements && !m_ShowStats && !m_ShowResearch && !m_ShowMilestones) {
        // Mouse wheel scrolling (desktop)
        f32 mouseWheel = input->GetMouseWheel();
        if (mouseWheel != 0) {
            m_ScrollOffset.y += mouseWheel * 50.0f; // Increased scroll speed
        }

        // Keyboard arrow scrolling (UP/DOWN arrow keys)
        // SDL_SCANCODE_UP = 82, DOWN = 81
        const int KEY_UP = 82;
        const int KEY_DOWN = 81;
        if (input->IsKeyDown(KEY_UP)) {
            m_ScrollOffset.y += 5.0f; // Smooth scroll up
        }
        if (input->IsKeyDown(KEY_DOWN)) {
            m_ScrollOffset.y -= 5.0f; // Smooth scroll down
        }

        // Touch drag scrolling (mobile)
        const auto& touches = input->GetTouches();
        if (!touches.empty()) {
            const auto& touch = touches[0]; // Use first touch
            m_ScrollOffset.y += touch.delta.y;
        }

        // Clamp scroll bounds (updated for new larger UI elements)
        f32 maxScroll = 0.0f; // Can't scroll up past the top

        // Calculate actual content height based on current UI layout
        f32 stationHeight = 180.0f;  // Updated from 150px
        f32 stationMargin = 25.0f;   // Updated from 20px
        f32 stationsContentHeight = m_Stations.size() * (stationHeight + stationMargin);

        // Add space for prestige button (60px) + auto-prestige controls (45px if unlocked)
        // + collapse button (60px) + gaps (20 + 70 + 50 padding)
        f32 bottomControlsHeight = 305.0f; // Generous padding for all bottom controls

        f32 contentHeight = stationsContentHeight + bottomControlsHeight;

        // Viewport = screen height - fixed top area (resources 100px + nav 80px)
        f32 fixedTopArea = 180.0f;
        f32 screenHeight = 720.0f; // Default screen height
        f32 viewportHeight = screenHeight - fixedTopArea;

        f32 minScroll = -(contentHeight - viewportHeight);
        if (minScroll > 0) minScroll = 0; // If content fits on screen, don't allow scrolling

        m_ScrollOffset.y = std::max(minScroll, std::min(maxScroll, m_ScrollOffset.y));
    }

    // Keyboard shortcuts
    // SDL_SCANCODE_A = 4, B = 5, C = 6, E = 8, F = 9, H = 11, M = 13, R = 15, S = 16, ESCAPE = 41
    const int KEY_A = 4;
    const int KEY_B = 5;
    const int KEY_C = 6;
    const int KEY_E = 8;
    const int KEY_F = 9;
    const int KEY_H = 11;
    const int KEY_M = 13;
    const int KEY_R = 15;
    const int KEY_S = 16;
    const int KEY_ESCAPE = 41;

    if (input->IsKeyPressed(KEY_A)) {
        m_ShowAchievements = !m_ShowAchievements;
    }
    if (input->IsKeyPressed(KEY_S)) {
        m_ShowStats = !m_ShowStats;
    }
    if (input->IsKeyPressed(KEY_R)) {
        m_ShowResearch = !m_ShowResearch;
    }
    if (input->IsKeyPressed(KEY_M)) {
        m_ShowMilestones = !m_ShowMilestones;
    }
    if (input->IsKeyPressed(KEY_B)) {
        m_ShowBuyables = !m_ShowBuyables;
    }
    if (input->IsKeyPressed(KEY_C)) {
        m_ShowChallenges = !m_ShowChallenges;
    }
    if (input->IsKeyPressed(KEY_E)) {
        m_ShowEssenceShop = !m_ShowEssenceShop;
    }
    if (input->IsKeyPressed(KEY_H)) {
        m_ShowSpaceship = !m_ShowSpaceship;
    }
    if (input->IsKeyPressed(KEY_F)) {
        // Toggle number format between Suffix and Scientific
        m_NumberFormat = (m_NumberFormat == GameUtils::NumberFormat::Suffix)
            ? GameUtils::NumberFormat::Scientific
            : GameUtils::NumberFormat::Suffix;
        Log::Info("Number format toggled");
    }
    if (input->IsKeyPressed(KEY_ESCAPE)) {
        m_ShowAchievements = false;
        m_ShowStats = false;
        m_ShowResearch = false;
        m_ShowMilestones = false;
        m_ShowBuyables = false;
        m_ShowChallenges = false;
        m_ShowEssenceShop = false;
        m_ShowSingularityShop = false;
        m_ShowSpaceship = false;
    }

    // Handle popup close button clicks (X button in top-right of panels)
    if (mousePressed) {
        f32 closeBtnSize = 30.0f;
        f32 screenWidth = 1280.0f;  // Default window width
        f32 screenHeight = 720.0f;  // Default window height

        // Helper function to check close button for a panel
        auto checkCloseButton = [&](bool isShown, f32 panelWidth, f32 panelHeight, bool* showFlag) {
            if (!isShown) return false;

            f32 panelX = (screenWidth - panelWidth) / 2.0f;
            f32 panelY = (screenHeight - panelHeight) / 2.0f;
            f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
            f32 closeBtnY = panelY + 10.0f;
            Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);

            if (closeBtn.Contains(mousePos)) {
                *showFlag = false;
                return true;  // Click was handled
            }

            // Click outside panel to close
            Rect panel(panelX, panelY, panelWidth, panelHeight);
            if (!panel.Contains(mousePos)) {
                *showFlag = false;
                return true;  // Click was handled
            }

            return false;
        };

        // Check close buttons for each popup (in reverse render order - check top-most first)
        bool handled = false;
        if (!handled) handled = checkCloseButton(m_ShowSingularityShop, 900.0f, 650.0f, &m_ShowSingularityShop);
        if (!handled) handled = checkCloseButton(m_ShowEssenceShop, 900.0f, 650.0f, &m_ShowEssenceShop);
        if (!handled) handled = checkCloseButton(m_ShowChallenges, 900.0f, 650.0f, &m_ShowChallenges);
        if (!handled) handled = checkCloseButton(m_ShowBuyables, 900.0f, 600.0f, &m_ShowBuyables);
        if (!handled) handled = checkCloseButton(m_ShowMilestones, 950.0f, 670.0f, &m_ShowMilestones);
        if (!handled) handled = checkCloseButton(m_ShowResearch, 900.0f, 600.0f, &m_ShowResearch);
        if (!handled) handled = checkCloseButton(m_ShowStats, 900.0f, 600.0f, &m_ShowStats);
        if (!handled) handled = checkCloseButton(m_ShowAchievements, 900.0f, 600.0f, &m_ShowAchievements);

        // Handle research node clicks (if research panel is open)
        if (!handled && m_ShowResearch) {
            f32 panelWidth = 900.0f;
            f32 panelHeight = 600.0f;
            f32 panelX = (screenWidth - panelWidth) / 2.0f;
            f32 panelY = (screenHeight - panelHeight) / 2.0f;

            f32 nodeStartY = panelY + 80.0f;
            f32 nodeX = panelX + 20.0f;
            f32 nodeWidth = panelWidth - 40.0f;
            f32 nodeHeight = 100.0f;
            f32 nodeSpacing = 10.0f;

            auto availableNodes = m_ResearchTree.GetAvailableResearch(m_Timeline.completedResets);
            i32 displayedCount = 0;
            i32 maxDisplay = 5;

            for (const ResearchNode* node : availableNodes) {
                if (displayedCount >= maxDisplay) break;

                f32 nodeY = nodeStartY + (nodeHeight + nodeSpacing) * displayedCount;

                // Check AUTO toggle button first (top-right corner of node)
                f32 autoToggleW = 60.0f;
                f32 autoToggleH = 25.0f;
                f32 autoToggleX = nodeX + nodeWidth - autoToggleW - 10.0f;
                f32 autoToggleY = nodeY + 10.0f;
                Rect autoToggleRect(autoToggleX, autoToggleY, autoToggleW, autoToggleH);

                if (autoToggleRect.Contains(mousePos)) {
                    // Toggle auto-research flag
                    ResearchNode* mutableNode = m_ResearchTree.GetNode(node->id);
                    if (mutableNode) {
                        mutableNode->autoResearch = !mutableNode->autoResearch;
                        Log::Infof(node->name, " auto-research: ", mutableNode->autoResearch ? "ON" : "OFF");
                    }
                    handled = true;
                    break;
                }

                // Check node click for manual purchase
                Rect nodeRect(nodeX, nodeY, nodeWidth, nodeHeight);
                if (nodeRect.Contains(mousePos)) {
                    // Try to research this node
                    if (CanAffordResearch(node->id)) {
                        PurchaseResearch(node->id);
                        Log::Infof("Researched: ", node->name);
                    } else {
                        Log::Info("Cannot afford this research");
                    }
                    handled = true;
                    break;
                }

                displayedCount++;
            }
        }

        // Handle buyable purchase button clicks (if buyables panel is open)
        if (!handled && m_ShowBuyables) {
            f32 panelWidth = 900.0f;
            f32 panelX = (1280.0f - panelWidth) / 2.0f;  // Default screen width
            f32 panelY = (720.0f - 600.0f) / 2.0f;  // Default screen height

            f32 buyableStartY = panelY + 70.0f;
            f32 buyableX = panelX + 20.0f;
            f32 buyableWidth = panelWidth - 40.0f;
            f32 buyableHeight = 100.0f;
            f32 buyableSpacing = 12.0f;

            auto& buyables = m_BuyableManager.GetBuyables();

            for (size_t i = 0; i < buyables.size(); i++) {
                f32 buyableY = buyableStartY + (buyableHeight + buyableSpacing) * i;

                // Purchase button rectangle
                Rect buyBtn(buyableX + buyableWidth - 120.0f, buyableY + 55.0f, 110.0f, 35.0f);

                if (buyBtn.Contains(mousePos)) {
                    const auto& buyable = buyables[i];
                    // Check if buyables are disabled by challenge
                    if (m_ChallengeManager.HasModifier(ChallengeModifier::NoBuyables)) {
                        Log::Info("Buyables are disabled during this challenge!");
                    } else if (m_BuyableManager.Purchase(buyable.id, this)) {
                        Log::Infof("Purchased: ", buyable.name);
                        UpdateResearchBonuses(); // Recalculate production with new multipliers
                    }
                    handled = true;
                    break;
                }
            }
        }

        // Handle challenge Enter/Exit button clicks (if challenges panel is open)
        if (!handled && m_ShowChallenges) {
            f32 panelWidth = 900.0f;
            f32 panelHeight = 650.0f;
            f32 panelX = (1280.0f - panelWidth) / 2.0f;
            f32 panelY = (720.0f - panelHeight) / 2.0f;

            const Challenge* currentChallenge = m_ChallengeManager.GetCurrentChallenge();
            f32 challengeStartY = currentChallenge ? panelY + 100.0f : panelY + 70.0f;
            f32 challengeX = panelX + 20.0f;
            f32 challengeWidth = panelWidth - 40.0f;
            f32 challengeHeight = 120.0f;
            f32 challengeSpacing = 10.0f;

            auto& challenges = m_ChallengeManager.GetChallenges();
            i32 currentPrestige = m_Statistics.totalPrestigesPerformed;

            for (size_t i = 0; i < challenges.size(); i++) {
                const auto& challenge = challenges[i];
                f32 challengeY = challengeStartY + (challengeHeight + challengeSpacing) * i;

                // Skip if challenge is completed
                if (challenge.completed) continue;

                // Action button rectangle
                Rect actionBtn(challengeX + challengeWidth - 120.0f, challengeY + 80.0f, 110.0f, 30.0f);

                if (actionBtn.Contains(mousePos)) {
                    if (challenge.active) {
                        // Exit challenge
                        m_ChallengeManager.ExitChallenge(this);
                        Log::Infof("Exited challenge: ", challenge.name);
                        PerformPrestige(); // Reset game state when exiting challenge
                    } else {
                        // Try to enter challenge
                        bool canEnter = challenge.CanEnter(currentPrestige, currentChallenge != nullptr);
                        if (canEnter) {
                            if (m_ChallengeManager.EnterChallenge(challenge.id, this)) {
                                Log::Infof("Entered challenge: ", challenge.name);
                                PerformPrestige(); // Reset game state when entering challenge
                            }
                        } else {
                            Log::Info("Cannot enter this challenge (check requirements or exit current challenge)");
                        }
                    }
                    handled = true;
                    break;
                }
            }
        }

        // Handle essence shop purchase button clicks (if essence shop panel is open)
        if (!handled && m_ShowEssenceShop) {
            f32 panelWidth = 900.0f;
            f32 panelHeight = 650.0f;
            f32 panelX = (1280.0f - panelWidth) / 2.0f;
            f32 panelY = (720.0f - panelHeight) / 2.0f;

            f32 upgradeStartY = panelY + 85.0f;
            f32 upgradeX = panelX + 20.0f;
            f32 upgradeWidth = panelWidth - 40.0f;
            f32 upgradeHeight = 100.0f;
            f32 upgradeSpacing = 10.0f;

            auto& upgrades = m_EssenceShopManager.GetUpgrades();

            for (size_t i = 0; i < upgrades.size(); i++) {
                const auto& upgrade = upgrades[i];
                f32 upgradeY = upgradeStartY + (upgradeHeight + upgradeSpacing) * i;

                // Purchase button rectangle
                Rect buyBtn(upgradeX + upgradeWidth - 120.0f, upgradeY + 55.0f, 110.0f, 35.0f);

                if (buyBtn.Contains(mousePos) && !upgrade.IsMaxed()) {
                    if (m_EssenceShopManager.Purchase(upgrade.id, this)) {
                        Log::Infof("Purchased essence upgrade: ", upgrade.name);
                        UpdateResearchBonuses(); // Recalculate production with new multipliers
                    }
                    handled = true;
                    break;
                }
            }
        }

        // Handle singularity shop purchase button clicks (if singularity shop panel is open)
        if (!handled && m_ShowSingularityShop) {
            f32 panelWidth = 900.0f;
            f32 panelHeight = 650.0f;
            f32 panelX = (1280.0f - panelWidth) / 2.0f;
            f32 panelY = (720.0f - panelHeight) / 2.0f;

            f32 upgradeStartY = panelY + 85.0f;
            f32 upgradeX = panelX + 20.0f;
            f32 upgradeWidth = panelWidth - 40.0f;
            f32 upgradeHeight = 100.0f;
            f32 upgradeSpacing = 10.0f;

            auto& upgrades = m_SingularityShopManager.GetUpgrades();

            for (size_t i = 0; i < upgrades.size(); i++) {
                const auto& upgrade = upgrades[i];
                f32 upgradeY = upgradeStartY + (upgradeHeight + upgradeSpacing) * i;

                // Purchase button rectangle
                Rect buyBtn(upgradeX + upgradeWidth - 120.0f, upgradeY + 55.0f, 110.0f, 35.0f);

                if (buyBtn.Contains(mousePos) && !upgrade.IsMaxed()) {
                    if (m_SingularityShopManager.Purchase(upgrade.id, this)) {
                        Log::Infof("Purchased singularity upgrade: ", upgrade.name);
                        UpdateResearchBonuses(); // Recalculate production with new multipliers
                    }
                    handled = true;
                    break;
                }
            }
        }

        // Handle Quantum Anomaly clicks (active gameplay - high priority)
        if (!handled) {
            ClickQuantumAnomaly(mousePos);
            // Note: ClickQuantumAnomaly internally checks if a click was successful
            // We don't set handled=true here because we want other UI elements to still work
        }

        // Handle navigation bar button clicks (only if no popup consumed the click)
        if (!handled) {
            f32 navY = 100.0f;
            f32 navHeight = 80.0f; // Updated to match new nav height
            f32 btnWidth = 180.0f; // Updated to match new button width
            f32 btnHeight = 60.0f; // Updated to match new button height
            f32 btnY = navY + (navHeight - btnHeight) * 0.5f;
            f32 spacing = 10.0f; // Updated spacing
            f32 startX = 15.0f; // Updated start position

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
                f32 menuHeight = 490.0f; // Increased from 420 to fit 7 items
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
    }

    // Handle boost button click
    if (mousePressed) {
        // Boost button (from RenderUI) - Updated to match new sizes
        f32 boostBtnWidth = 200.0f;  // Updated from 150px
        f32 boostBtnHeight = 60.0f;  // Updated from 38px
        f32 navY = 100.0f;
        f32 navHeight = 80.0f;        // Updated from 50px
        f32 boostBtnX = 1280.0f - boostBtnWidth - 25.0f;  // Default screen width
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
        f32 startY = 190.0f;          // Updated to match RenderStations
        f32 stationHeight = 180.0f;   // Updated to match RenderStations
        f32 margin = 25.0f;           // Updated to match RenderStations

        for (size_t i = 0; i < m_Stations.size(); i++) {
            auto& station = m_Stations[i];
            if (!station.unlocked) continue; // Only unlocked stations have auto-upgrade toggle

            f32 y = startY + i * (stationHeight + margin) + m_ScrollOffset.y;

            // Skip if off-screen
            if (y + stationHeight < 100.0f || y > 720.0f) continue;

            // Auto toggle button position (must match render position)
            f32 stationWidth = 1280.0f - 60.0f; // Default screen width minus margins
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

    // Update buttons
    for (auto& button : m_StationButtons) {
        button.Update(mousePos);

        if (button.WasClicked(mousePos, mousePressed) && button.onClick) {
            button.onClick();
        }
    }

    // Handle auto-prestige threshold adjustment button clicks
    if (mousePressed && m_ResearchTree.IsResearched(ResearchID::AutoPrestige)) {
        // Calculate button positions (must match RenderStations rendering)
        f32 stationHeight = 180.0f;  // Updated to match RenderStations
        f32 margin = 25.0f;           // Updated to match RenderStations
        f32 startY = 190.0f;          // Updated to match RenderStations
        f32 prestigeY = startY + m_Stations.size() * (stationHeight + margin) + 20.0f + m_ScrollOffset.y;
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
        f32 stationHeight = 180.0f;  // Updated to match RenderStations
        f32 margin = 25.0f;           // Updated to match RenderStations
        f32 startY = 190.0f;          // Updated to match RenderStations
        f32 prestigeY = startY + m_Stations.size() * (stationHeight + margin) + 20.0f + m_ScrollOffset.y;

        f32 collapseY = prestigeY + 115.0f;
        if (!m_ResearchTree.IsResearched(ResearchID::AutoPrestige)) {
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

    // Handle combat UI clicks
    if (m_ShowCombat && m_CombatSystem.IsInCombat()) {
        m_CombatSystem.HandleClick(mousePos.x, mousePos.y, mousePressed);
    }

    // Handle gatcha UI clicks
    if (m_ShowGatcha) {
        m_GatchaSystem.HandleClick(mousePos.x, mousePos.y, mousePressed, this);
    }

    // Handle skill tree UI clicks
    if (m_ShowSkills) {
        m_SkillTree.HandleClick(mousePos.x, mousePos.y, mousePressed, this);
    }

    // Handle enhancement UI clicks
    if (m_ShowEnhancement) {
        m_EnhancementSystem.HandleClick(mousePos.x, mousePos.y, mousePressed, this);
    }
}

void GameState::Render(Renderer* renderer) {
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

    // Apply background tint
    if (bgTint.a > 0.0f) {
        Rect fullScreen(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
        renderer->DrawRect(fullScreen, bgTint, true);
    }

    RenderResources(renderer);  // Fixed at top (0-100px)
    RenderUI(renderer);          // Navigation bar (100-150px) - BEFORE stations so it's on top
    RenderStations(renderer);    // Scrollable area (starts at 150px)

    // Render active gameplay elements (before popups)
    RenderQuantumAnomalies(renderer); // Clickable orbs

    // Render particles (visual feedback)
    for (const auto& particle : m_Particles) {
        f32 size = 3.0f + (1.0f - particle.lifetime / particle.maxLifetime) * 3.0f;
        renderer->DrawCircle(particle.position, size, particle.color, true);
    }

    // Render combo counter (top-right, above everything)
    if (m_ComboCount > 1) {
        std::string comboText = std::to_string(m_ComboCount) + "x COMBO!";
        Vec2 comboPos(static_cast<f32>(renderer->GetWidth()) - 150.0f, 180.0f);

        // Pulsing effect for high combos
        f32 pulseScale = 1.0f + 0.1f * sinf(m_ComboTimeRemaining * 10.0f);
        f32 fontSize = 18.0f * pulseScale;

        // Color based on combo level
        Color comboColor = (m_ComboCount >= 5) ? Color(1.0f, 0.8f, 0.0f, 1.0f) : // Gold for 5+
                          (m_ComboCount >= 3) ? Color(1.0f, 0.0f, 1.0f, 1.0f) : // Magenta for 3-4
                          Color::NeonCyan(); // Cyan for 2

        renderer->DrawText(comboText, comboPos, comboColor, fontSize);

        // Time remaining bar
        f32 barWidth = 100.0f;
        f32 barHeight = 6.0f;
        Vec2 barPos(static_cast<f32>(renderer->GetWidth()) - 140.0f, 200.0f);
        Rect barBg(barPos.x, barPos.y, barWidth, barHeight);
        renderer->DrawRect(barBg, Color(0.2f, 0.2f, 0.2f, 0.8f), true);

        f32 timeRatio = static_cast<f32>(m_ComboTimeRemaining / m_ComboWindow);
        Rect barFill(barPos.x, barPos.y, barWidth * timeRatio, barHeight);
        renderer->DrawRect(barFill, comboColor, true);
    }

    // Render panels/popups (on top of gameplay)
    RenderActiveEvent(renderer);
    RenderAchievements(renderer);
    RenderStatistics(renderer);
    RenderResearchTree(renderer);
    RenderMilestones(renderer);
    RenderBuyables(renderer);
    RenderChallenges(renderer);
    RenderEssenceShop(renderer);
    RenderSingularityShop(renderer);
    RenderSpaceship(renderer);
    RenderCombat(renderer); // Combat overlay renders on top
    RenderGatcha(renderer); // Gatcha overlay renders on top
    RenderSkillTree(renderer); // Skill tree overlay renders on top

    // Render enhancement UI
    if (m_ShowEnhancement) {
        m_EnhancementSystem.RenderEnhancementUI(renderer, this);
    }

    RenderParticleEffects(renderer, 1.0/60.0); // Assume 60 FPS for particles
    RenderAchievementNotifications(renderer);
    RenderMilestoneNotifications(renderer);

    // Prestige flash effect (screen overlay, on top of everything)
    if (m_PrestigeFlashActive) {
        f32 alpha = static_cast<f32>(m_PrestigeFlashTimer / 0.5); // Fade over 0.5 seconds
        Color flashColor(1.0f, 1.0f, 1.0f, alpha * 0.3f); // White flash, max 30% opacity
        Rect fullScreen(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
        renderer->DrawRect(fullScreen, flashColor, true);
    }
}

void GameState::RenderResources(Renderer* renderer) {
    // Draw resource panel at top
    f32 panelHeight = 100.0f;
    Rect panel(0, 0, static_cast<f32>(renderer->GetWidth()), panelHeight);
    renderer->DrawRect(panel, Color(0.1f, 0.1f, 0.15f, 0.9f), true);

    const char* resourceNames[] = {"Qubits", "Coherence", "Entanglement"};
    Color resourceColors[] = {
        Color::QuantumBlue(),
        Color::CoherenceGreen(),
        Color::EntanglementOrange()
    };

    f32 xOffset = 20.0f;
    for (int i = 0; i < 3; i++) {
        Vec2 textPos(xOffset, 20.0f);

        renderer->DrawText(resourceNames[i], textPos, Color::White(), 16.0f);

        // Draw value (formatted with current format preference)
        std::string formattedValue = GameUtils::FormatNumber(m_Resources[i], m_NumberFormat);
        Vec2 valuePos(xOffset, 50.0f);
        renderer->DrawText(formattedValue, valuePos, resourceColors[i], 24.0f);

        xOffset += 250.0f;
    }

    // Draw Quantum Essence (permanent meta-currency) - top-right corner
    Vec2 essencePos(static_cast<f32>(renderer->GetWidth()) - 500.0f, 10.0f);
    renderer->DrawText("Quantum Essence", essencePos, Color::Magenta(), 14.0f);
    Vec2 essenceValuePos(static_cast<f32>(renderer->GetWidth()) - 500.0f, 30.0f);
    std::string essenceStr = GameUtils::FormatNumber(m_QuantumEssence, m_NumberFormat);
    renderer->DrawText("💎 " + essenceStr, essenceValuePos, Color::Magenta() * 1.3f, 20.0f);

    // Draw Singularities (second prestige layer) - top-right corner
    Vec2 singularityPos(static_cast<f32>(renderer->GetWidth()) - 250.0f, 10.0f);
    renderer->DrawText("Singularities", singularityPos, Color(0.5f, 0.0f, 1.0f, 1.0f), 14.0f);
    Vec2 singularityValuePos(static_cast<f32>(renderer->GetWidth()) - 250.0f, 30.0f);
    std::string singularityStr = GameUtils::FormatNumber(m_Timeline.singularities, m_NumberFormat);
    renderer->DrawText("⭐ " + singularityStr, singularityValuePos, Color(0.8f, 0.0f, 1.0f, 1.0f), 20.0f);

    // Draw coherence bar
    f32 coherenceBarWidth = 200.0f;
    f32 coherenceBarHeight = 20.0f;
    Vec2 coherenceBarPos(static_cast<f32>(renderer->GetWidth()) - coherenceBarWidth - 20.0f, 65.0f);

    renderer->DrawText("Coherence", Vec2(coherenceBarPos.x, coherenceBarPos.y - 20.0f), Color::White(), 14.0f);

    Rect coherenceBarBg(coherenceBarPos.x, coherenceBarPos.y, coherenceBarWidth, coherenceBarHeight);
    renderer->DrawRect(coherenceBarBg, Color(0.2f, 0.2f, 0.2f, 1.0f), true);

    f32 coherenceFill = (m_Coherence / m_MaxCoherence) * coherenceBarWidth;
    Rect coherenceBarFill(coherenceBarPos.x, coherenceBarPos.y, coherenceFill, coherenceBarHeight);
    renderer->DrawRect(coherenceBarFill, Color::CoherenceGreen(), true);

    renderer->DrawRect(coherenceBarBg, Color::White() * 0.5f, false);
}

void GameState::RenderStations(Renderer* renderer) {
    // Stations start below the navigation bar (resources at 0-100, nav at 100-180)
    f32 startY = 190.0f;  // Start below bigger nav bar (100+80+10 gap)
    f32 stationHeight = 180.0f; // Increased from 150px to prevent overlap
    f32 margin = 25.0f; // Increased spacing between stations

    // Counter for accessing persistent buttons (4 buttons per station + 1 prestige button)
    size_t buttonIdx = 0;

    for (size_t i = 0; i < m_Stations.size(); i++) {
        auto& station = m_Stations[i];
        f32 y = startY + i * (stationHeight + margin) + m_ScrollOffset.y;

        // Skip rendering if station is clipped by fixed headers at top
        if (y + stationHeight < 150.0f) {
            // Station is completely above the navigation bar - skip it
            if (!station.unlocked) {
                buttonIdx += 1; // Skip unlock button
            } else {
                buttonIdx += 4; // Skip all four buttons (observe, upgrade, buy max, and skip unlock)
            }
            continue;
        }

        // Also skip if station would render over resource panel (top 100px)
        if (y < 100.0f) {
            // Part of station would be in resource area - skip it
            if (!station.unlocked) {
                buttonIdx += 1;
            } else {
                buttonIdx += 4;
            }
            continue;
        }

        // Skip rendering if station is below the screen
        if (y > static_cast<f32>(renderer->GetHeight())) {
            if (!station.unlocked) {
                buttonIdx += 1;
            } else {
                buttonIdx += 4;
            }
            continue;
        }

        // Define the main panel area
        Rect stationRect(30.0f, y, static_cast<f32>(renderer->GetWidth()) - 60.0f, stationHeight);

        // --- New Glass-Panel Background with Tier Colors ---
        Color bgColor, borderColor;
        if (station.unlocked) {
            bgColor = Color(0.1f, 0.12f, 0.18f, 0.8f);

            // Use tier-based color based on station level!
            Color tierColor = GetStationTierColor(station.level);
            borderColor = tierColor * 0.7f;
            borderColor.a = 0.8f;
        } else {
            bgColor = Color(0.15f, 0.1f, 0.1f, 0.6f);
            borderColor = Color(0.4f, 0.2f, 0.2f, 0.8f);
        }

        // Draw the background panel 
        renderer->DrawRect(stationRect, bgColor, true);
        renderer->DrawRect(stationRect, borderColor, false);
        
        // Draw a light internal separator line for the Information block
        Rect separator(stationRect.x + 5.0f, stationRect.y + 70.0f, stationRect.width - 10.0f, 2.0f);
        renderer->DrawRect(separator, borderColor * 0.5f, true);


        // ----------------------------------------------------------------------
        // --- LOCKED STATION UI ---
        // ----------------------------------------------------------------------
        if (!station.unlocked) {
            // Title and description
            Vec2 titlePos(stationRect.x + 20.0f, y + 25.0f);
            renderer->DrawText(station.name + " // CLASSIFIED", titlePos, Color(0.7f, 0.4f, 0.4f, 1.0f), 22.0f);
            
            Vec2 descPos(stationRect.x + 20.0f, y + 50.0f);
            renderer->DrawText(station.description, descPos, Color(0.5f, 0.5f, 0.55f, 1.0f), 13.0f);

            // Update unlock button (below the separator line)
            UIButton& unlockBtn = m_StationButtons[buttonIdx++];
            unlockBtn.bounds = Rect(stationRect.x + 20.0f, y + 85.0f, 300.0f, 45.0f);
            unlockBtn.text = "UNLOCK FIELD (" + std::to_string(static_cast<i64>(station.unlockCost)) + " Qubits)";
            unlockBtn.enabled = m_Resources[0] >= station.unlockCost;
            unlockBtn.affordability = std::min(1.0, m_Resources[0] / station.unlockCost);

            unlockBtn.Render(renderer);

            // Skip observe, upgrade, and buy max buttons
            buttonIdx += 3;
            continue;
        }

        // ----------------------------------------------------------------------
        // --- UNLOCKED STATION UI (Information Block - Top Half) ---
        // ----------------------------------------------------------------------
        
        // Station Title and Level (Main Header)
        Vec2 titlePos(stationRect.x + 20.0f, y + 15.0f);
        renderer->DrawText(station.name + " | Lv." + std::to_string(station.level), titlePos, Color::White(), 22.0f);

        // Station Description (Sub-Header)
        Vec2 descPos(stationRect.x + 20.0f, y + 40.0f);
        renderer->DrawText(station.description, descPos, Color(0.75f, 0.75f, 0.8f, 1.0f), 13.0f);

        // Production Rate (Left Block)
        std::ostringstream prodOss;
        prodOss.precision(2);
        // *** FIX APPLIED HERE: m_Timeline.photonBonus instead of GetTimeline()->photonBonus ***
        prodOss << std::fixed << (station.currentProduction * m_Timeline.photonBonus) << "/s"; 
        Vec2 prodPos(stationRect.x + 20.0f, y + 58.0f);
        renderer->DrawText("PROD: " + prodOss.str(), prodPos, Color::CoherenceGreen() * 1.1f, 15.0f);

        // Superposition Value (Center Block)
        std::ostringstream superOss;
        superOss.precision(1);
        superOss << std::fixed << station.superpositionValue;
        Vec2 superPos(stationRect.x + 250.0f, y + 58.0f);
        renderer->DrawText("SUPERPOSITION: " + superOss.str(), superPos, Color::QuantumPurple() * 1.2f, 15.0f);

        // Probability (Right Block)
        std::ostringstream probOss;
        probOss.precision(0);
        probOss << std::fixed << (station.superpositionProbability * 100.0f) << "%";
        Vec2 probPos(stationRect.x + 480.0f, y + 58.0f);
        Color probColor = station.superpositionProbability > 0.7f ? Color::CoherenceGreen() : Color::Yellow();
        renderer->DrawText("COLLAPSE CHANCE: " + probOss.str(), probPos, probColor, 15.0f);


        // ----------------------------------------------------------------------
        // --- UNLOCKED STATION UI (Action Block - Bottom Half) ---
        // ----------------------------------------------------------------------
        
        // Skip unlock button 
        buttonIdx++;

        // Calculate effective upgrade cost (with challenge modifiers)
        f64 effectiveUpgradeCost = station.upgradeCost;
        if (m_ChallengeManager.HasModifier(ChallengeModifier::ExpensiveUpgrades)) {
            effectiveUpgradeCost *= 3.0; // Upgrades cost 3x more
        }

        // Get observe button (Left Button)
        UIButton& observeBtn = m_StationButtons[buttonIdx++];
        observeBtn.bounds = Rect(stationRect.x + 20.0f, y + 85.0f, 200.0f, 45.0f);
        observeBtn.enabled = station.superpositionValue > 0.1 &&
                            !m_ChallengeManager.HasModifier(ChallengeModifier::NoObserve);
        observeBtn.Render(renderer);

        // Get upgrade button (Middle Button)
        UIButton& upgradeBtn = m_StationButtons[buttonIdx++];
        upgradeBtn.bounds = Rect(stationRect.x + 240.0f, y + 85.0f, 180.0f, 45.0f);
        upgradeBtn.text = "UPGRADE (" + GameUtils::FormatNumber(effectiveUpgradeCost, m_NumberFormat) + ")";
        upgradeBtn.enabled = m_Resources[0] >= effectiveUpgradeCost &&
                            !m_ChallengeManager.HasModifier(ChallengeModifier::NoUpgrades);
        upgradeBtn.affordability = std::min(1.0, m_Resources[0] / effectiveUpgradeCost);
        upgradeBtn.Render(renderer);

        // Get buy max button (Right Button)
        UIButton& buyMaxBtn = m_StationButtons[buttonIdx++];
        buyMaxBtn.bounds = Rect(stationRect.x + 440.0f, y + 85.0f, 140.0f, 45.0f);
        buyMaxBtn.text = "BUY MAX";
        buyMaxBtn.enabled = m_Resources[0] >= effectiveUpgradeCost &&
                           !m_ChallengeManager.HasModifier(ChallengeModifier::NoUpgrades);
        buyMaxBtn.affordability = std::min(1.0, m_Resources[0] / effectiveUpgradeCost);
        buyMaxBtn.Render(renderer);

        // Auto-upgrade toggle (small button in top-right corner of station panel)
        f32 autoToggleSize = 60.0f;
        f32 autoToggleX = stationRect.x + stationRect.width - autoToggleSize - 10.0f;
        f32 autoToggleY = y + 10.0f;
        Rect autoToggleRect(autoToggleX, autoToggleY, autoToggleSize, 25.0f);

        Color autoToggleColor = station.autoUpgrade ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);
        renderer->DrawRect(autoToggleRect, autoToggleColor * 0.4f, true);
        renderer->DrawRect(autoToggleRect, autoToggleColor, false);

        std::string autoText = station.autoUpgrade ? "AUTO" : "AUTO";
        Vec2 autoTextPos(autoToggleX + 12.0f, autoToggleY + 6.0f);
        renderer->DrawText(autoText, autoTextPos, Color::White(), 11.0f);

        // Progress bar showing how close to affording next upgrade
        f32 progressBarY = y + 135.0f;
        f32 progressBarWidth = stationRect.width - 40.0f;
        f32 progressBarHeight = 18.0f;
        Vec2 progressBarPos(stationRect.x + 20.0f, progressBarY);

        // Calculate progress (0-100% based on current resources vs upgrade cost)
        f64 currentQubits = m_Resources[0];
        f64 upgradeCost = effectiveUpgradeCost; // Use effective cost with challenge modifiers
        f64 progress = std::min(1.0, currentQubits / upgradeCost);

        // Color-code based on affordability
        Color progressColor;
        if (progress >= 1.0) {
            progressColor = Color::CoherenceGreen();  // Can afford now
        } else if (progress >= 0.75) {
            progressColor = Color::Yellow();  // Almost there
        } else if (progress >= 0.5) {
            progressColor = Color::EntanglementOrange();  // Halfway
        } else {
            progressColor = Color::QuantumPurple();  // Still far away
        }

        renderer->DrawProgressBar(progressBarPos, progressBarWidth, progressBarHeight,
                                 currentQubits, upgradeCost, progressColor,
                                 Color(0.15f, 0.15f, 0.2f, 1.0f), true, 11.0f);
    }

    // ----------------------------------------------------------------------
    // --- PRESTIGE BUTTON ---
    // ----------------------------------------------------------------------

    // Update prestige button from persistent list (last button)
    UIButton& prestigeBtn = m_StationButtons.back();

    f32 prestigeY = startY + m_Stations.size() * (stationHeight + margin) + 20.0f + m_ScrollOffset.y;
    f64 photonsOnPrestige = CalculatePhotonsOnPrestige();
    
    // Make the prestige button full width and more prominent
    prestigeBtn.bounds = Rect(30.0f, prestigeY, static_cast<f32>(renderer->GetWidth()) - 60.0f, 60.0f);
    prestigeBtn.text = "QUANTUM LEAP: INITIATE PRESTIGE (+" + std::to_string(static_cast<i64>(photonsOnPrestige)) + " PHOTONS)";
    prestigeBtn.enabled = photonsOnPrestige > 0;

    prestigeBtn.Render(renderer);

    // ----------------------------------------------------------------------
    // --- AUTO-PRESTIGE CONTROLS ---
    // ----------------------------------------------------------------------
    if (m_ResearchTree.IsResearched(ResearchID::AutoPrestige)) {
        f32 autoPrestigeY = prestigeY + 70.0f; // Below prestige button

        // Label showing current threshold
        std::string thresholdLabel = "Auto-Prestige Threshold: " + std::to_string(static_cast<i64>(m_AutoPrestigeThreshold)) + " photons";
        Vec2 labelPos(40.0f, autoPrestigeY + 10.0f);
        renderer->DrawText(thresholdLabel, labelPos, Color::QuantumPurple(), 14.0f);

        // Adjustment buttons (right side)
        f32 btnW = 50.0f;
        f32 btnH = 30.0f;
        f32 btnSpacing = 10.0f;
        f32 startX = 500.0f;

        // -10 button
        Rect minusTenRect(startX, autoPrestigeY + 5.0f, btnW, btnH);
        renderer->DrawRect(minusTenRect, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
        renderer->DrawRect(minusTenRect, Color::Red() * 0.6f, false);
        Vec2 minusTenTextPos(startX + 14.0f, autoPrestigeY + 12.0f);
        renderer->DrawText("-10", minusTenTextPos, Color::White(), 12.0f);

        // -1 button
        Rect minusOneRect(startX + btnW + btnSpacing, autoPrestigeY + 5.0f, btnW, btnH);
        renderer->DrawRect(minusOneRect, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
        renderer->DrawRect(minusOneRect, Color::Red() * 0.6f, false);
        Vec2 minusOneTextPos(startX + btnW + btnSpacing + 18.0f, autoPrestigeY + 12.0f);
        renderer->DrawText("-1", minusOneTextPos, Color::White(), 12.0f);

        // +1 button
        Rect plusOneRect(startX + (btnW + btnSpacing) * 2, autoPrestigeY + 5.0f, btnW, btnH);
        renderer->DrawRect(plusOneRect, Color(0.1f, 0.3f, 0.1f, 0.8f), true);
        renderer->DrawRect(plusOneRect, Color::CoherenceGreen() * 0.6f, false);
        Vec2 plusOneTextPos(startX + (btnW + btnSpacing) * 2 + 18.0f, autoPrestigeY + 12.0f);
        renderer->DrawText("+1", plusOneTextPos, Color::White(), 12.0f);

        // +10 button
        Rect plusTenRect(startX + (btnW + btnSpacing) * 3, autoPrestigeY + 5.0f, btnW, btnH);
        renderer->DrawRect(plusTenRect, Color(0.1f, 0.3f, 0.1f, 0.8f), true);
        renderer->DrawRect(plusTenRect, Color::CoherenceGreen() * 0.6f, false);
        Vec2 plusTenTextPos(startX + (btnW + btnSpacing) * 3 + 14.0f, autoPrestigeY + 12.0f);
        renderer->DrawText("+10", plusTenTextPos, Color::White(), 12.0f);
    }

    // ----------------------------------------------------------------------
    // --- SINGULARITY COLLAPSE BUTTON ---
    // ----------------------------------------------------------------------
    f32 collapseY = prestigeY + 115.0f; // Below auto-prestige controls (or below prestige if no auto-prestige)
    if (!m_ResearchTree.IsResearched(ResearchID::AutoPrestige)) {
        collapseY = prestigeY + 70.0f; // Directly below prestige button
    }

    f64 singularitiesOnCollapse = CalculateSingularitiesOnCollapse();
    bool canCollapse = singularitiesOnCollapse > 0;

    // Collapse button - full width, cosmic purple theme
    Rect collapseBtn(30.0f, collapseY, static_cast<f32>(renderer->GetWidth()) - 60.0f, 60.0f);
    Color collapseColor = canCollapse ? Color(0.5f, 0.0f, 1.0f, 1.0f) : Color(0.2f, 0.0f, 0.3f, 0.5f);
    renderer->DrawRect(collapseBtn, collapseColor * 0.3f, true);
    renderer->DrawRect(collapseBtn, collapseColor, false);

    std::string collapseText = canCollapse ?
        "⭐ SINGULARITY COLLAPSE: RESET EVERYTHING (+" + std::to_string(static_cast<i64>(singularitiesOnCollapse)) + " SINGULARITIES)" :
        "⭐ SINGULARITY COLLAPSE (Requires 10,000 photons)";

    f32 collapseTextWidth = static_cast<f32>(collapseText.length()) * 8.0f;
    Vec2 collapseTextPos(30.0f + (collapseBtn.width - collapseTextWidth) / 2.0f, collapseY + 22.0f);
    renderer->DrawText(collapseText, collapseTextPos, canCollapse ? Color::White() : Color(0.5f, 0.5f, 0.5f, 1.0f), 16.0f);
}

void GameState::RenderUI(Renderer* renderer) {
    // Top navigation bar with modern cyberpunk design (sits at 100-180px) - INCREASED FOR MOBILE
    f32 navY = 100.0f;
    f32 navHeight = 80.0f; // Increased from 50px for bigger touch targets
    Rect navBar(0, navY, static_cast<f32>(renderer->GetWidth()), navHeight);

    // Navigation bar background with dark panel
    renderer->DrawRect(navBar, Color::DarkPanel(), true);

    // Glowing cyan bottom border for cyberpunk feel
    Rect navBorder(0, navY + navHeight - 2.0f, static_cast<f32>(renderer->GetWidth()), 2.0f);
    renderer->DrawRect(navBorder, Color::NeonCyan() * 0.6f, true);

    // Navigation buttons - BIGGER for mobile/touch
    f32 btnWidth = 180.0f; // Increased from 130px
    f32 btnHeight = 60.0f; // Increased from 32px
    f32 btnY = navY + (navHeight - btnHeight) * 0.5f;
    f32 spacing = 10.0f; // Tighter spacing since buttons are bigger
    f32 startX = 15.0f; // Reduced to fit more buttons

    struct NavButton {
        const char* label;
        bool* showFlag;
        Color color;
    };

    // Simplified navigation - only most important pages (mobile-friendly)
    NavButton navButtons[] = {
        {"BUYABLES", &m_ShowBuyables, Color::ElectricBlue()},
        {"CHALLENGES", &m_ShowChallenges, Color::Red()},
        {"ESSENCE", &m_ShowEssenceShop, Color::Magenta()},
        {"RESEARCH", &m_ShowResearch, Color::QuantumPurple()},
        {"STATS", &m_ShowStats, Color::EntanglementOrange()},
        {"MILESTONES", &m_ShowMilestones, Color::NeonPink()},
    };

    int numButtons = 6; // Show 6 main buttons
    for (int i = 0; i < numButtons; i++) {
        auto& btn = navButtons[i];
        f32 x = startX + i * (btnWidth + spacing);

        // Wrap to second row if needed (for smaller screens)
        if (x + btnWidth > renderer->GetWidth() - 200.0f) {
            break; // Don't draw if it goes off screen
        }

        Rect btnRect(x, btnY, btnWidth, btnHeight);

        bool active = *btn.showFlag;
        Color btnColor = active ? btn.color : btn.color * 0.5f;

        // Button background
        renderer->DrawRect(btnRect, btnColor * 0.25f, true);

        // Glowing border (thicker for mobile)
        if (active) {
            // Active - bright glow
            Rect glowRect(x - 2.0f, btnY - 2.0f, btnWidth + 4.0f, btnHeight + 4.0f);
            renderer->DrawRect(glowRect, btn.color * 0.9f, false);
        } else {
            // Inactive - subtle border
            renderer->DrawRect(btnRect, Color::DarkBorder(), false);
        }

        // Button text (centered, LARGER font for readability)
        // Better text width calculation: ~7.5px per character for 16px font
        f32 textWidth = strlen(btn.label) * 7.5f;
        Vec2 textPos(x + (btnWidth - textWidth) * 0.5f, btnY + (btnHeight - 16.0f) * 0.5f + 2.0f);
        renderer->DrawText(btn.label, textPos, Color::White(), 16.0f);
    }

    // MORE menu button (hamburger menu for overflow items) - far right before boost
    f32 boostBtnWidth = 200.0f; // Declare early for positioning
    f32 moreBtnWidth = 80.0f;
    f32 moreBtnX = static_cast<f32>(renderer->GetWidth()) - boostBtnWidth - moreBtnWidth - 35.0f;
    Rect moreBtnRect(moreBtnX, btnY, moreBtnWidth, btnHeight);

    // More visible colors for hamburger menu
    Color moreColor = m_ShowMoreMenu ? Color::NeonCyan() : Color(0.3f, 0.4f, 0.5f, 1.0f); // Subtle blue-gray tint
    Color moreBgColor = m_ShowMoreMenu ? (moreColor * 0.4f) : (moreColor * 0.35f); // Brighter background
    renderer->DrawRect(moreBtnRect, moreBgColor, true);

    if (m_ShowMoreMenu) {
        Rect glowRect(moreBtnX - 2.0f, btnY - 2.0f, moreBtnWidth + 4.0f, btnHeight + 4.0f);
        renderer->DrawRect(glowRect, Color::NeonCyan() * 0.9f, false);
    } else {
        // More visible border when inactive
        renderer->DrawRect(moreBtnRect, Color(0.4f, 0.5f, 0.6f, 0.8f), false);
    }

    // Hamburger icon (three lines) - brighter and more visible
    f32 lineWidth = 30.0f;
    f32 lineHeight = 3.0f;
    f32 lineSpacing = 8.0f;
    f32 lineStartX = moreBtnX + (moreBtnWidth - lineWidth) * 0.5f;
    f32 lineStartY = btnY + (btnHeight - (lineHeight * 3 + lineSpacing * 2)) * 0.5f;

    Color lineColor = m_ShowMoreMenu ? Color::NeonCyan() : Color(0.8f, 0.9f, 1.0f, 1.0f); // Bright cyan-white
    for (int i = 0; i < 3; i++) {
        Rect line(lineStartX, lineStartY + i * (lineHeight + lineSpacing), lineWidth, lineHeight);
        renderer->DrawRect(line, lineColor, true);
    }

    // Boost button (right side of nav bar) - BIGGER for touch
    // boostBtnWidth already declared earlier for menu positioning
    f32 boostBtnHeight = 60.0f; // Increased from 38px to match nav buttons
    f32 boostBtnX = static_cast<f32>(renderer->GetWidth()) - boostBtnWidth - 25.0f;
    f32 boostBtnY = navY + (navHeight - boostBtnHeight) * 0.5f;
    Rect boostBtnRect(boostBtnX, boostBtnY, boostBtnWidth, boostBtnHeight);

    // Determine boost button state and color
    Color boostColor;
    std::string boostText;
    bool boostClickable = false;

    // Check if boost is disabled by challenge
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

    // Draw boost button
    renderer->DrawRect(boostBtnRect, boostColor * 0.3f, true);

    if (m_BoostActive) {
        // Pulsing glow when active
        Rect glowRect(boostBtnX - 2.0f, boostBtnY - 2.0f, boostBtnWidth + 4.0f, boostBtnHeight + 4.0f);
        renderer->DrawRect(glowRect, boostColor * 0.9f, false);
    } else if (boostClickable) {
        renderer->DrawRect(boostBtnRect, boostColor * 0.7f, false);
    } else {
        renderer->DrawRect(boostBtnRect, Color(0.3f, 0.3f, 0.3f, 1.0f), false);
    }

    // Button text
    f32 boostTextWidth = boostText.length() * 5.5f;
    Vec2 boostTextPos(boostBtnX + (boostBtnWidth - boostTextWidth) * 0.5f, boostBtnY + (boostBtnHeight - 12.0f) * 0.5f);
    renderer->DrawText(boostText, boostTextPos, Color::White(), 12.0f);

    // Scroll indicator (bottom right corner - animated)
    if (m_ScrollOffset.y > -50.0f && !m_ShowAchievements && !m_ShowStats && !m_ShowResearch && !m_ShowMilestones) {
        f32 indicatorY = static_cast<f32>(renderer->GetHeight()) - 40.0f;
        Vec2 arrowPos(static_cast<f32>(renderer->GetWidth()) - 90.0f, indicatorY);

        // Animated pulsing glow
        f32 pulse = 0.5f + 0.5f * static_cast<f32>(sin(m_TotalTimePlayed * 3.0));
        Color glowColor = Color::NeonCyan() * pulse;

        // Draw pulsing text
        renderer->DrawText("SCROLL DOWN", arrowPos, glowColor, 13.0f);
        Vec2 arrowPos2(static_cast<f32>(renderer->GetWidth()) - 60.0f, indicatorY + 15.0f);
        renderer->DrawText("v v v", arrowPos2, glowColor, 14.0f);
    }

    // MORE Menu Popup (shows Achievements, Singularity Shop, Spaceship, Combat, Summon, Skills, and Enhancement)
    if (m_ShowMoreMenu) {
        f32 menuWidth = 250.0f;
        f32 menuHeight = 490.0f; // Increased from 420 to fit 7 items
        f32 menuX = moreBtnX;
        f32 menuY = navY + navHeight + 5.0f;

        // Background
        Rect menuBg(menuX, menuY, menuWidth, menuHeight);
        renderer->DrawRect(menuBg, Color::DarkPanel(), true);
        renderer->DrawRect(menuBg, Color::NeonCyan() * 0.8f, false);

        // Menu items
        f32 itemHeight = 60.0f;
        f32 itemY = menuY + 10.0f;

        // Achievements button
        Rect achievementsRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
        Color achievementsColor = m_ShowAchievements ? Color::ElectricBlue() : Color(0.3f, 0.3f, 0.3f, 1.0f);
        renderer->DrawRect(achievementsRect, achievementsColor * 0.3f, true);
        renderer->DrawRect(achievementsRect, achievementsColor, false);

        f32 achievementsTextWidth = strlen("ACHIEVEMENTS") * 7.5f;
        Vec2 achievementsTextPos(menuX + (menuWidth - achievementsTextWidth) * 0.5f, itemY + (itemHeight - 16.0f) * 0.5f + 2.0f);
        renderer->DrawText("ACHIEVEMENTS", achievementsTextPos, Color::White(), 16.0f);

        // Singularity Shop button
        itemY += itemHeight + 10.0f;
        Rect singularityRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
        Color singularityColor = m_ShowSingularityShop ? Color(0.5f, 0.0f, 1.0f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f);
        renderer->DrawRect(singularityRect, singularityColor * 0.3f, true);
        renderer->DrawRect(singularityRect, singularityColor, false);

        f32 singularityTextWidth = strlen("SINGULARITY") * 7.5f;
        Vec2 singularityTextPos(menuX + (menuWidth - singularityTextWidth) * 0.5f, itemY + (itemHeight - 16.0f) * 0.5f + 2.0f);
        renderer->DrawText("SINGULARITY", singularityTextPos, Color::White(), 16.0f);

        // Spaceship button
        itemY += itemHeight + 10.0f;
        Rect shipRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
        Color shipColor = m_ShowSpaceship ? Color(1.0f, 0.7f, 0.0f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f); // Gold color
        renderer->DrawRect(shipRect, shipColor * 0.3f, true);
        renderer->DrawRect(shipRect, shipColor, false);

        f32 shipTextWidth = strlen("SPACESHIP") * 7.5f;
        Vec2 shipTextPos(menuX + (menuWidth - shipTextWidth) * 0.5f, itemY + (itemHeight - 16.0f) * 0.5f + 2.0f);
        renderer->DrawText("SPACESHIP", shipTextPos, Color::White(), 16.0f);

        // Combat/Battle button
        itemY += itemHeight + 10.0f;
        Rect battleRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
        Color battleColor = m_ShowCombat ? Color(1.0f, 0.3f, 0.3f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f); // Red color
        renderer->DrawRect(battleRect, battleColor * 0.3f, true);
        renderer->DrawRect(battleRect, battleColor, false);

        f32 battleTextWidth = strlen("BATTLE") * 7.5f;
        Vec2 battleTextPos(menuX + (menuWidth - battleTextWidth) * 0.5f, itemY + (itemHeight - 16.0f) * 0.5f + 2.0f);
        renderer->DrawText("BATTLE", battleTextPos, Color::White(), 16.0f);

        // Summon/Gatcha button
        itemY += itemHeight + 10.0f;
        Rect summonRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
        Color summonColor = m_ShowGatcha ? Color(1.0f, 0.3f, 1.0f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f); // Magenta color
        renderer->DrawRect(summonRect, summonColor * 0.3f, true);
        renderer->DrawRect(summonRect, summonColor, false);

        f32 summonTextWidth = strlen("SUMMON") * 7.5f;
        Vec2 summonTextPos(menuX + (menuWidth - summonTextWidth) * 0.5f, itemY + (itemHeight - 16.0f) * 0.5f + 2.0f);
        renderer->DrawText("SUMMON", summonTextPos, Color::White(), 16.0f);

        // Skills button
        itemY += itemHeight + 10.0f;
        Rect skillsRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
        Color skillsColor = m_ShowSkills ? Color(0.0f, 1.0f, 0.5f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f); // Green color
        renderer->DrawRect(skillsRect, skillsColor * 0.3f, true);
        renderer->DrawRect(skillsRect, skillsColor, false);

        f32 skillsTextWidth = strlen("SKILLS") * 7.5f;
        Vec2 skillsTextPos(menuX + (menuWidth - skillsTextWidth) * 0.5f, itemY + (itemHeight - 16.0f) * 0.5f + 2.0f);
        renderer->DrawText("SKILLS", skillsTextPos, Color::White(), 16.0f);

        // Enhancement button
        itemY += itemHeight + 10.0f;
        Rect enhanceRect(menuX + 10.0f, itemY, menuWidth - 20.0f, itemHeight);
        Color enhanceColor = m_ShowEnhancement ? Color(0.8f, 0.6f, 0.2f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f); // Gold color
        renderer->DrawRect(enhanceRect, enhanceColor * 0.3f, true);
        renderer->DrawRect(enhanceRect, enhanceColor, false);

        f32 enhanceTextWidth = strlen("ENHANCE") * 7.5f;
        Vec2 enhanceTextPos(menuX + (menuWidth - enhanceTextWidth) * 0.5f, itemY + (itemHeight - 16.0f) * 0.5f + 2.0f);
        renderer->DrawText("ENHANCE", enhanceTextPos, Color::White(), 16.0f);
    }
}

void GameState::AddResource(QuantumResource type, f64 amount) {
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

    // Improved JSON format with all game state
    file << "{\n";
    file << "  \"version\": 2,\n";
    file << "  \"saveTimestamp\": " << m_LastSaveTimestamp << ",\n";

    // Resources
    file << "  \"resources\": [" << m_Resources[0] << ", " << m_Resources[1] << ", " << m_Resources[2] << "],\n";
    file << "  \"coherence\": " << m_Coherence << ",\n";

    // Prestige
    file << "  \"photons\": " << m_Timeline.photons << ",\n";
    file << "  \"resets\": " << m_Timeline.completedResets << ",\n";

    // Time
    file << "  \"timePlayed\": " << m_TotalTimePlayed << ",\n";

    // Stations
    file << "  \"stations\": [\n";
    for (size_t i = 0; i < m_Stations.size(); i++) {
        const auto& s = m_Stations[i];
        file << "    {\"level\": " << s.level << ", \"unlocked\": " << (s.unlocked ? "true" : "false") << "}";
        if (i < m_Stations.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ],\n";

    // Statistics
    file << "  \"statistics\": {\n";
    file << "    \"totalQubitsEarned\": " << m_Statistics.totalQubitsEarned << ",\n";
    file << "    \"totalObservations\": " << m_Statistics.totalObservations << ",\n";
    file << "    \"totalUpgrades\": " << m_Statistics.totalUpgrades << ",\n";
    file << "    \"totalPrestiges\": " << m_Statistics.totalPrestigesPerformed << ",\n";
    file << "    \"highestQubits\": " << m_Statistics.highestQubits << ",\n";
    file << "    \"fastestPrestige\": " << m_Statistics.fastestPrestige << ",\n";
    file << "    \"currentStreak\": " << m_Statistics.currentStreak << ",\n";
    file << "    \"lastLogin\": " << m_Statistics.lastLoginTimestamp << "\n";
    file << "  },\n";

    // Achievements
    file << "  \"achievements\": [\n";
    for (size_t i = 0; i < m_Achievements.size(); i++) {
        const auto& ach = m_Achievements[i];
        file << "    {";
        file << "\"id\": " << static_cast<i32>(ach.id) << ", ";
        file << "\"unlocked\": " << (ach.unlocked ? "true" : "false") << ", ";
        file << "\"progress\": " << ach.progress;
        file << "}";
        if (i < m_Achievements.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ],\n";

    // Research Tree
    file << "  \"research\": [\n";
    auto researchedNodes = m_ResearchTree.GetResearchedNodes();
    for (size_t i = 0; i < researchedNodes.size(); i++) {
        file << "    " << static_cast<i32>(researchedNodes[i]->id);
        if (i < researchedNodes.size() - 1) file << ",";
        file << "\n";
    }
    file << "  ]\n";

    file << "}\n";

    file.close();
    Log::Debugf("Game saved to ", filepath);
    return true;
}

bool GameState::Load(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return false;
    }

    try {
        // Simple parsing using utility functions
        std::string line;
        bool inStatistics = false;
        bool inAchievements = false;
        bool inResearch = false;

        while (std::getline(file, line)) {
            // Track sections
            if (line.find("\"statistics\"") != std::string::npos) {
                inStatistics = true;
                inAchievements = false;
                inResearch = false;
                continue;
            } else if (line.find("\"achievements\"") != std::string::npos) {
                inStatistics = false;
                inAchievements = true;
                inResearch = false;
                continue;
            } else if (line.find("\"research\"") != std::string::npos) {
                inStatistics = false;
                inAchievements = false;
                inResearch = true;
                continue;
            } else if (line.find("}") != std::string::npos || line.find("]") != std::string::npos) {
                if (line.find("},") == std::string::npos) {
                    inStatistics = false;
                    inAchievements = false;
                    inResearch = false;
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
                            ResearchNode* node = m_ResearchTree.GetNode(id);
                            if (node) {
                                node->unlocked = true;
                                node->researched = true;
                            }
                        }
                    } catch (...) {
                        // Ignore parse errors
                    }
                }
            } else {
                // Parse main game state
                if (line.find("\"saveTimestamp\"") != std::string::npos) {
                    m_LastSaveTimestamp = static_cast<i64>(GameUtils::ParseJsonNumber(line, "saveTimestamp"));
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
                } else if (line.find("\"photons\"") != std::string::npos) {
                    m_Timeline.photons = GameUtils::ParseJsonNumber(line, "photons");
                    m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1);
                } else if (line.find("\"resets\"") != std::string::npos) {
                    m_Timeline.completedResets = static_cast<i32>(GameUtils::ParseJsonNumber(line, "resets"));
                } else if (line.find("\"timePlayed\"") != std::string::npos) {
                    m_TotalTimePlayed = GameUtils::ParseJsonNumber(line, "timePlayed");
                }
            }
        }

        file.close();

        // After loading, unlock available research based on current prestige level
        m_ResearchTree.UnlockAvailableResearch(m_Timeline.completedResets, m_ResearchTree.GetResearchedCount());

        // Apply research bonuses to all stations
        UpdateResearchBonuses();

        Log::Infof("Game loaded from ", filepath);
        return true;
    } catch (const std::exception& e) {
        Log::Errorf("Error loading save file: ", e.what());
        file.close();
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
    p.position = position;
    p.velocity = Vec2(
        static_cast<f32>(GameUtils::RandomRange(-50.0, 50.0)),
        static_cast<f32>(GameUtils::RandomRange(-100.0, -50.0))
    );
    p.color = color;
    p.lifetime = 0.0f;
    p.maxLifetime = static_cast<f32>(lifetime);
    m_Particles.push_back(p);
}

void GameState::SpawnParticleBurst(const Vec2& position, const Color& color, i32 count) {
    for (i32 i = 0; i < count; i++) {
        SpawnParticle(position, color, GameUtils::RandomRange(0.5, 1.5));
    }
}

void GameState::UpdateParticles(f64 deltaTime) {
    // Update and remove dead particles
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

        // Remove if dead
        if (it->lifetime >= it->maxLifetime) {
            it = m_Particles.erase(it);
        } else {
            ++it;
        }
    }

    // Limit particle count to prevent lag
    if (m_Particles.size() > 500) {
        m_Particles.erase(m_Particles.begin(), m_Particles.begin() + 100);
    }
}

void GameState::RenderParticleEffects(Renderer* renderer, f64 deltaTime) {
    UpdateParticles(deltaTime);

    for (const auto& particle : m_Particles) {
        // Draw small circle for each particle
        f32 size = 3.0f * (1.0f - (particle.lifetime / particle.maxLifetime));
        Rect particleRect(
            particle.position.x - size/2,
            particle.position.y - size/2,
            size,
            size
        );
        renderer->DrawRect(particleRect, particle.color, true);
    }
}

// Achievement UI Rendering
void GameState::RenderAchievements(Renderer* renderer) {
    if (!m_ShowAchievements) return;

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Achievement panel
    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    // Background
    Rect panelBg(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelBg, Color::DarkPanel(), true);
    renderer->DrawRect(panelBg, Color::CoherenceGreen() * 0.8f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("ACHIEVEMENTS", titlePos, Color::CoherenceGreen(), 24.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);
    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color::Red() * 0.8f, false);
    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 200.0f, panelY + 45.0f);
    renderer->DrawText("(Click X or press A/ESC)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // List achievements
    f32 yOffset = panelY + 50.0f;
    i32 unlocked = 0;
    for (const auto& ach : m_Achievements) {
        if (ach.unlocked) unlocked++;

        Color achColor = ach.unlocked ? Color::CoherenceGreen() : Color(0.4f, 0.4f, 0.4f, 1.0f);

        // Achievement name
        std::string achText = (ach.unlocked ? "✓ " : "☐ ") + ach.name;
        Vec2 achPos(panelX + 30.0f, yOffset);
        renderer->DrawText(achText, achPos, achColor, 14.0f);

        // Description
        Vec2 descPos(panelX + 50.0f, yOffset + 18.0f);
        renderer->DrawText(ach.description, descPos, Color(0.7f, 0.7f, 0.7f, 1.0f), 11.0f);

        // Progress bar for incomplete achievements
        if (!ach.unlocked && ach.target > 0) {
            f32 barWidth = 200.0f;
            f32 barHeight = 8.0f;
            Rect barBg(panelX + 50.0f, yOffset + 35.0f, barWidth, barHeight);
            renderer->DrawRect(barBg, Color(0.2f, 0.2f, 0.2f, 1.0f), true);

            f32 progress = static_cast<f32>(GameUtils::Clamp(ach.progress / ach.target, 0.0, 1.0));
            Rect barFill(panelX + 50.0f, yOffset + 35.0f, barWidth * progress, barHeight);
            renderer->DrawRect(barFill, Color::QuantumBlue(), true);

            // Progress text
            std::string progressText = GameUtils::FormatNumber(ach.progress, m_NumberFormat) + " / " + GameUtils::FormatNumber(ach.target, m_NumberFormat);
            Vec2 progressPos(panelX + 260.0f, yOffset + 32.0f);
            renderer->DrawText(progressText, progressPos, Color::White(), 10.0f);
        }

        yOffset += 55.0f;
        if (yOffset > panelY + panelHeight - 60.0f) break; // Don't overflow
    }

    // Summary at bottom
    std::string summary = std::to_string(unlocked) + " / " + std::to_string(m_Achievements.size()) + " unlocked";
    Vec2 summaryPos(panelX + 20.0f, panelY + panelHeight - 30.0f);
    renderer->DrawText(summary, summaryPos, Color::QuantumBlue(), 16.0f);
}

// Statistics UI Rendering
void GameState::RenderStatistics(Renderer* renderer) {
    if (!m_ShowStats) return;

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Stats panel
    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    // Background
    Rect panelBg(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelBg, Color::DarkPanel(), true);
    renderer->DrawRect(panelBg, Color::EntanglementOrange() * 0.8f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("STATISTICS", titlePos, Color::EntanglementOrange(), 24.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);
    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color::Red() * 0.8f, false);
    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 200.0f, panelY + 45.0f);
    renderer->DrawText("(Click X or press S/ESC)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // Display stats
    f32 yOffset = panelY + 50.0f;
    auto renderStat = [&](const std::string& label, const std::string& value) {
        Vec2 labelPos(panelX + 30.0f, yOffset);
        Vec2 valuePos(panelX + 300.0f, yOffset);
        renderer->DrawText(label, labelPos, Color::White(), 14.0f);
        renderer->DrawText(value, valuePos, Color::EntanglementOrange(), 14.0f);
        yOffset += 25.0f;
    };

    renderStat("Total Qubits Earned:", GameUtils::FormatNumber(m_Statistics.totalQubitsEarned, m_NumberFormat));
    renderStat("Total Observations:", std::to_string(m_Statistics.totalObservations));
    renderStat("Total Upgrades:", std::to_string(m_Statistics.totalUpgrades));
    renderStat("Total Prestiges:", std::to_string(m_Statistics.totalPrestigesPerformed));

    yOffset += 10.0f;
    renderStat("Session Time:", GameUtils::FormatTime(m_Statistics.sessionTime));
    renderStat("Session Qubits:", GameUtils::FormatNumber(m_Statistics.sessionQubits, m_NumberFormat));
    renderStat("Session Observations:", std::to_string(m_Statistics.sessionObservations));

    yOffset += 10.0f;
    renderStat("Highest Qubits:", GameUtils::FormatNumber(m_Statistics.highestQubits, m_NumberFormat));
    renderStat("Fastest Prestige:", GameUtils::FormatTime(m_Statistics.fastestPrestige));
    renderStat("Current Streak:", std::to_string(m_Statistics.currentStreak) + " days");
}

// Active Event Display
void GameState::RenderActiveEvent(Renderer* renderer) {
    if (!m_CurrentEvent || !m_CurrentEvent->active) return;

    // Event banner at top-center
    f32 bannerWidth = 400.0f;
    f32 bannerHeight = 80.0f;
    f32 bannerX = (renderer->GetWidth() - bannerWidth) / 2.0f;
    f32 bannerY = 120.0f;

    // Animated background
    f32 pulse = static_cast<f32>(0.8 + 0.2 * std::sin(m_TotalTimePlayed * 3.0));
    Color bgColor = Color::QuantumBlue() * pulse;
    bgColor.a = 0.9f;

    Rect bannerBg(bannerX, bannerY, bannerWidth, bannerHeight);
    renderer->DrawRect(bannerBg, bgColor, true);
    renderer->DrawRect(bannerBg, Color::White(), false);

    // Event name
    Vec2 namePos(bannerX + 10.0f, bannerY + 10.0f);
    renderer->DrawText("⚡ " + m_CurrentEvent->name, namePos, Color::White(), 18.0f);

    // Event description
    Vec2 descPos(bannerX + 10.0f, bannerY + 35.0f);
    renderer->DrawText(m_CurrentEvent->description, descPos, Color(0.9f, 0.9f, 1.0f, 1.0f), 13.0f);

    // Time remaining bar
    f32 barWidth = bannerWidth - 20.0f;
    f32 barHeight = 10.0f;
    Rect timeBg(bannerX + 10.0f, bannerY + 60.0f, barWidth, barHeight);
    renderer->DrawRect(timeBg, Color(0.2f, 0.2f, 0.2f, 1.0f), true);

    f32 progress = static_cast<f32>(m_CurrentEvent->timeRemaining / m_CurrentEvent->duration);
    Rect timeFill(bannerX + 10.0f, bannerY + 60.0f, barWidth * progress, barHeight);
    renderer->DrawRect(timeFill, Color::CoherenceGreen(), true);

    // Time remaining text
    std::string timeText = GameUtils::FormatTime(m_CurrentEvent->timeRemaining) + " remaining";
    Vec2 timePos(bannerX + barWidth - 80.0f, bannerY + 58.0f);
    renderer->DrawText(timeText, timePos, Color::White(), 11.0f);
}

// Achievement Notification Popups
void GameState::RenderAchievementNotifications(Renderer* renderer) {
    if (m_RecentUnlocks.empty()) return;

    // Display recent unlocks as popups
    f32 notifWidth = 350.0f;
    f32 notifHeight = 60.0f;
    f32 notifX = renderer->GetWidth() - notifWidth - 20.0f;
    f32 notifY = renderer->GetHeight() - notifHeight - 20.0f;

    // Only show the most recent achievement
    AchievementID recentID = m_RecentUnlocks.back();
    Achievement* ach = GetAchievement(recentID);
    if (!ach) return;

    // Animated slide-in effect (would need time tracking for full animation)
    Rect notifBg(notifX, notifY, notifWidth, notifHeight);
    renderer->DrawRect(notifBg, Color(0.1f, 0.1f, 0.15f, 0.95f), true);
    renderer->DrawRect(notifBg, Color::CoherenceGreen(), false);

    // Achievement icon and text
    Vec2 titlePos(notifX + 10.0f, notifY + 10.0f);
    renderer->DrawText("🏆 Achievement Unlocked!", titlePos, Color::CoherenceGreen(), 14.0f);

    Vec2 namePos(notifX + 10.0f, notifY + 28.0f);
    renderer->DrawText(ach->name, namePos, Color::White(), 16.0f);

    // Clear old notifications (keep last 3)
    while (m_RecentUnlocks.size() > 3) {
        m_RecentUnlocks.erase(m_RecentUnlocks.begin());
    }
}

// Research Tree UI Rendering
void GameState::RenderResearchTree(Renderer* renderer) {
    if (!m_ShowResearch) return;

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Research panel
    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color::DarkPanel(), true);
    renderer->DrawRect(panel, Color::QuantumPurple() * 0.8f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("RESEARCH TREE", titlePos, Color::QuantumPurple(), 24.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);
    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color::Red() * 0.8f, false);
    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 200.0f, panelY + 45.0f);
    renderer->DrawText("(Click X or press R/ESC)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // Research count
    i32 researched = m_ResearchTree.GetResearchedCount();
    i32 total = static_cast<i32>(ResearchID::COUNT);
    std::string countText = "Researched: " + std::to_string(researched) + "/" + std::to_string(total);
    Vec2 countPos(panelX + 20.0f, panelY + 45.0f);
    renderer->DrawText(countText, countPos, Color(0.9f, 0.9f, 1.0f, 1.0f), 14.0f);

    // Available research nodes
    f32 nodeStartY = panelY + 80.0f;
    f32 nodeX = panelX + 20.0f;
    f32 nodeWidth = panelWidth - 40.0f;
    f32 nodeHeight = 100.0f;
    f32 nodeSpacing = 10.0f;

    auto availableNodes = m_ResearchTree.GetAvailableResearch(m_Timeline.completedResets);
    auto researchedNodes = m_ResearchTree.GetResearchedNodes();

    // Show available research first
    i32 displayedCount = 0;
    i32 maxDisplay = 5;

    for (const ResearchNode* node : availableNodes) {
        if (displayedCount >= maxDisplay) break;

        f32 nodeY = nodeStartY + (nodeHeight + nodeSpacing) * displayedCount;

        // Node background
        bool canAfford = CanAffordResearch(node->id);
        Color nodeBg = canAfford ? Color(0.2f, 0.3f, 0.2f, 1.0f) : Color(0.2f, 0.2f, 0.25f, 1.0f);
        Color nodeBorder = canAfford ? Color::CoherenceGreen() : Color::QuantumBlue();

        Rect nodeRect(nodeX, nodeY, nodeWidth, nodeHeight);
        renderer->DrawRect(nodeRect, nodeBg, true);
        renderer->DrawRect(nodeRect, nodeBorder, false);

        // Node name
        Vec2 namePos(nodeX + 10.0f, nodeY + 10.0f);
        renderer->DrawText(node->name, namePos, Color::White(), 16.0f);

        // Node description
        Vec2 descPos(nodeX + 10.0f, nodeY + 32.0f);
        renderer->DrawText(node->description, descPos, Color(0.8f, 0.8f, 0.9f, 1.0f), 12.0f);

        // Costs
        f32 costY = nodeY + 55.0f;
        std::string costText = "Cost: ";
        if (node->qubitCost > 0) {
            costText += GameUtils::FormatNumber(node->qubitCost, m_NumberFormat) + " Qubits  ";
        }
        if (node->coherenceCost > 0) {
            costText += GameUtils::FormatNumber(node->coherenceCost, m_NumberFormat) + " Coherence  ";
        }
        if (node->entanglementCost > 0) {
            costText += GameUtils::FormatNumber(node->entanglementCost, m_NumberFormat) + " Entanglement  ";
        }
        if (node->photonCost > 0) {
            costText += std::to_string(node->photonCost) + " Photons";
        }

        Vec2 costPos(nodeX + 10.0f, costY);
        Color costColor = canAfford ? Color::CoherenceGreen() : Color::QuantumPurple();
        renderer->DrawText(costText, costPos, costColor, 11.0f);

        // Prerequisites
        if (!node->prerequisites.empty()) {
            std::string prereqText = "Requires: ";
            for (size_t i = 0; i < node->prerequisites.size(); i++) {
                const ResearchNode* prereq = m_ResearchTree.GetNode(node->prerequisites[i]);
                if (prereq) {
                    prereqText += prereq->name;
                    if (i < node->prerequisites.size() - 1) prereqText += ", ";
                }
            }
            Vec2 prereqPos(nodeX + 10.0f, costY + 18.0f);
            renderer->DrawText(prereqText, prereqPos, Color(0.7f, 0.7f, 0.7f, 1.0f), 10.0f);
        }

        // Auto-research toggle button (top-right corner of node)
        f32 autoToggleW = 60.0f;
        f32 autoToggleH = 25.0f;
        f32 autoToggleX = nodeX + nodeWidth - autoToggleW - 10.0f;
        f32 autoToggleY = nodeY + 10.0f;
        Rect autoToggleRect(autoToggleX, autoToggleY, autoToggleW, autoToggleH);

        // Get mutable node pointer for checking autoResearch flag
        ResearchNode* mutableNode = m_ResearchTree.GetNode(node->id);
        bool autoEnabled = (mutableNode && mutableNode->autoResearch);

        Color autoToggleColor = autoEnabled ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);
        renderer->DrawRect(autoToggleRect, autoToggleColor * 0.4f, true);
        renderer->DrawRect(autoToggleRect, autoToggleColor, false);

        Vec2 autoTextPos(autoToggleX + 12.0f, autoToggleY + 6.0f);
        renderer->DrawText("AUTO", autoTextPos, Color::White(), 11.0f);

        displayedCount++;
    }

    // Show researched nodes count if no available research
    if (availableNodes.empty()) {
        Vec2 noResearchPos(panelX + panelWidth / 2.0f - 150.0f, panelY + 200.0f);
        renderer->DrawText("No research available at current prestige level!", noResearchPos,
                          Color(0.7f, 0.7f, 0.7f, 1.0f), 14.0f);

        // Show some completed research
        Vec2 completedTitlePos(panelX + 20.0f, panelY + 250.0f);
        renderer->DrawText("Completed Research:", completedTitlePos, Color::CoherenceGreen(), 14.0f);

        i32 completedCount = 0;
        for (const ResearchNode* node : researchedNodes) {
            if (completedCount >= 8) break;

            Vec2 completedPos(panelX + 30.0f, panelY + 280.0f + completedCount * 20.0f);
            std::string completedText = "✓ " + node->name;
            renderer->DrawText(completedText, completedPos, Color(0.8f, 0.9f, 0.8f, 1.0f), 12.0f);

            completedCount++;
        }
    }

    // Bonuses summary
    f32 bonusY = panelY + panelHeight - 60.0f;
    Vec2 bonusTitle(panelX + 20.0f, bonusY);
    renderer->DrawText("Active Bonuses:", bonusTitle, Color::QuantumBlue(), 14.0f);

    f64 prodMult = m_ResearchTree.GetTotalProductionMultiplier();
    f64 obsMult = m_ResearchTree.GetTotalObservationBonus();
    f64 cohMult = m_ResearchTree.GetTotalCoherenceBonus();

    std::string bonusText = "Production: +" + std::to_string(static_cast<i32>((prodMult - 1.0) * 100.0)) + "%  ";
    bonusText += "Observation: +" + std::to_string(static_cast<i32>(obsMult * 100.0)) + "%  ";
    bonusText += "Coherence: +" + std::to_string(static_cast<i32>(cohMult * 100.0)) + "%";

    Vec2 bonusPos(panelX + 20.0f, bonusY + 22.0f);
    renderer->DrawText(bonusText, bonusPos, Color::CoherenceGreen(), 12.0f);
}

// Research Tree Methods
bool GameState::CanAffordResearch(ResearchID id) const {
    const ResearchNode* node = m_ResearchTree.GetNode(id);
    if (!node) return false;

    // Apply singularity shop research cost discount
    f64 costMultiplier = m_SingularityShopManager.GetResearchCostMultiplier();

    // Check resource costs (with discount applied)
    if (GetResource(QuantumResource::Qubits) < node->qubitCost * costMultiplier) return false;
    if (GetResource(QuantumResource::Coherence) < node->coherenceCost * costMultiplier) return false;
    if (GetResource(QuantumResource::Entanglement) < node->entanglementCost * costMultiplier) return false;

    // Check photon cost
    if (m_Timeline.photons < node->photonCost * costMultiplier) return false;

    // Check if can be researched
    if (!m_ResearchTree.CanResearch(id, m_Timeline.completedResets)) return false;

    return true;
}

bool GameState::PurchaseResearch(ResearchID id) {
    if (!CanAffordResearch(id)) return false;

    ResearchNode* node = m_ResearchTree.GetNode(id);
    if (!node) return false;

    // Apply singularity shop research cost discount
    f64 costMultiplier = m_SingularityShopManager.GetResearchCostMultiplier();

    // Spend resources (with discount applied)
    SpendResource(QuantumResource::Qubits, node->qubitCost * costMultiplier);
    SpendResource(QuantumResource::Coherence, node->coherenceCost * costMultiplier);
    SpendResource(QuantumResource::Entanglement, node->entanglementCost * costMultiplier);
    m_Timeline.photons -= node->photonCost * costMultiplier;

    // Research it
    m_ResearchTree.Research(id);

    // Update bonuses
    UpdateResearchBonuses();

    // Unlock new research
    m_ResearchTree.UnlockAvailableResearch(m_Timeline.completedResets, m_ResearchTree.GetResearchedCount());

    // Spawn celebration particles
    SpawnParticleBurst(Vec2(640.0f, 360.0f), Color::QuantumPurple(), 20);

    Log::Info("Researched: " + node->name);

    return true;
}

void GameState::UpdateResearchBonuses() {
    // Apply research bonuses to all stations
    f64 productionMult = m_ResearchTree.GetTotalProductionMultiplier();

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
    if (m_ResearchTree.IsResearched(ResearchID::PhotonMultiplier)) {
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
    if (m_ResearchTree.IsResearched(ResearchID::PhotonMultiplier)) {
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
                milestone->progress = static_cast<f64>(m_ResearchTree.GetResearchedCount());
                break;

            case MilestoneID::QuantumMaster:
                if (m_Timeline.completedResets >= 50 && m_Timeline.photons >= 500.0) {
                    milestone->progress = 1.0;
                }
                break;

            case MilestoneID::TrueEnding:
                if (m_ResearchTree.IsResearched(ResearchID::QuantumSingularity)) {
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

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Milestones panel
    f32 panelWidth = 950.0f;
    f32 panelHeight = 670.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color::DarkPanel(), true);
    renderer->DrawRect(panel, Color::NeonPink() * 0.8f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("MILESTONES", titlePos, Color::NeonPink(), 24.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);

    // Draw close button background
    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color::Red() * 0.8f, false);

    // Draw X symbol
    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 200.0f, panelY + 45.0f);
    renderer->DrawText("(Click X or press M/ESC)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // Completion stats
    auto completedMilestones = m_MilestoneSystem.GetCompletedMilestones();
    i32 totalMilestones = static_cast<i32>(MilestoneID::COUNT);
    std::string statsText = "Completed: " + std::to_string(completedMilestones.size()) + "/" + std::to_string(totalMilestones);
    Vec2 statsPos(panelX + 20.0f, panelY + 45.0f);
    renderer->DrawText(statsText, statsPos, Color::CoherenceGreen(), 14.0f);

    // Production bonus from milestones
    f64 prodBonus = m_MilestoneSystem.GetTotalProductionBonus() * 100.0;
    std::string bonusText = "Total Production Bonus: +" + std::to_string(static_cast<i32>(prodBonus)) + "%";
    Vec2 bonusPos(panelX + panelWidth - 350.0f, panelY + 45.0f);
    renderer->DrawText(bonusText, bonusPos, Color::QuantumPurple(), 14.0f);

    // Active milestones
    f32 milestoneStartY = panelY + 80.0f;
    f32 milestoneX = panelX + 20.0f;
    f32 milestoneWidth = panelWidth - 40.0f;
    f32 milestoneHeight = 85.0f;
    f32 milestoneSpacing = 8.0f;

    auto activeMilestones = m_MilestoneSystem.GetActiveMilestones();
    
    i32 displayCount = 0;
    i32 maxDisplay = 6;

    for (const Milestone* milestone : activeMilestones) {
        if (displayCount >= maxDisplay) break;

        f32 milestoneY = milestoneStartY + (milestoneHeight + milestoneSpacing) * displayCount;

        // Milestone background
        Color milestoneBg = Color(0.15f, 0.15f, 0.2f, 1.0f);
        Rect milestoneRect(milestoneX, milestoneY, milestoneWidth, milestoneHeight);
        renderer->DrawRect(milestoneRect, milestoneBg, true);
        renderer->DrawRect(milestoneRect, Color::QuantumBlue() * 0.6f, false);

        // Milestone name
        Vec2 namePos(milestoneX + 10.0f, milestoneY + 10.0f);
        renderer->DrawText(milestone->name, namePos, Color::White(), 16.0f);

        // Milestone description
        Vec2 descPos(milestoneX + 10.0f, milestoneY + 32.0f);
        renderer->DrawText(milestone->description, descPos, Color(0.8f, 0.8f, 0.9f, 1.0f), 12.0f);

        // Progress bar
        f32 barWidth = milestoneWidth - 20.0f;
        f32 barHeight = 14.0f;
        f32 barY = milestoneY + 55.0f;

        Rect progressBg(milestoneX + 10.0f, barY, barWidth, barHeight);
        renderer->DrawRect(progressBg, Color(0.2f, 0.2f, 0.25f, 1.0f), true);

        f64 progressPercent = GameUtils::Clamp(milestone->progress / milestone->target, 0.0, 1.0);
        f32 fillWidth = barWidth * static_cast<f32>(progressPercent);
        Rect progressFill(milestoneX + 10.0f, barY, fillWidth, barHeight);
        
        // Color based on progress
        Color barColor = progressPercent >= 1.0 ? Color::CoherenceGreen() : Color::QuantumBlue();
        renderer->DrawRect(progressFill, barColor, true);

        // Progress text
        std::string progressText = GameUtils::FormatNumber(milestone->progress, m_NumberFormat) + " / " + GameUtils::FormatNumber(milestone->target, m_NumberFormat);
        if (milestone->id == MilestoneID::HalfAchievements || milestone->id == MilestoneID::AllAchievements) {
            progressText = std::to_string(static_cast<i32>(progressPercent * 100.0)) + "%";
        }
        Vec2 progressPos(milestoneX + 15.0f, barY + 1.0f);
        renderer->DrawText(progressText, progressPos, Color::White(), 11.0f);

        // Reward text (right side)
        Vec2 rewardPos(milestoneX + milestoneWidth - 280.0f, barY + 1.0f);
        renderer->DrawText(milestone->rewardDescription, rewardPos, Color(0.9f, 0.9f, 0.5f, 1.0f), 10.0f);

        displayCount++;
    }

    // If no active milestones, show completed ones
    if (activeMilestones.empty()) {
        Vec2 nonePos(panelX + panelWidth / 2.0f - 150.0f, panelY + 200.0f);
        renderer->DrawText("🎉 All Milestones Completed! 🎉", nonePos, Color::CoherenceGreen(), 18.0f);

        // Show completed list
        Vec2 completedTitlePos(panelX + 20.0f, panelY + 260.0f);
        renderer->DrawText("Completed Milestones:", completedTitlePos, Color::QuantumBlue(), 14.0f);

        i32 completedCount = 0;
        for (const Milestone* m : completedMilestones) {
            if (completedCount >= 10) break;

            Vec2 completedPos(panelX + 30.0f, panelY + 290.0f + completedCount * 22.0f);
            std::string completedText = "✓ " + m->name;
            renderer->DrawText(completedText, completedPos, Color::CoherenceGreen(), 12.0f);

            completedCount++;
        }
    }
}

void GameState::RenderBuyables(Renderer* renderer) {
    if (!m_ShowBuyables) return;

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Buyables panel
    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color::DarkPanel(), true);
    renderer->DrawRect(panel, Color::ElectricBlue() * 0.8f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("BUYABLE UPGRADES", titlePos, Color::ElectricBlue(), 24.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);

    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color::Red() * 0.8f, false);

    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 200.0f, panelY + 45.0f);
    renderer->DrawText("(Click X or press B/ESC)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // Buyables list
    f32 buyableStartY = panelY + 70.0f;
    f32 buyableX = panelX + 20.0f;
    f32 buyableWidth = panelWidth - 40.0f;
    f32 buyableHeight = 100.0f;
    f32 buyableSpacing = 12.0f;

    auto& buyables = m_BuyableManager.GetBuyables();

    for (size_t i = 0; i < buyables.size(); i++) {
        const auto& buyable = buyables[i];
        f32 buyableY = buyableStartY + (buyableHeight + buyableSpacing) * i;

        // Buyable background
        bool maxed = buyable.IsMaxed();
        Color buyableBg = maxed ? Color(0.1f, 0.2f, 0.15f, 1.0f) : Color(0.15f, 0.15f, 0.2f, 1.0f);
        Color buyableBorder = maxed ? Color::CoherenceGreen() * 0.6f : Color::ElectricBlue() * 0.6f;

        Rect buyableRect(buyableX, buyableY, buyableWidth, buyableHeight);
        renderer->DrawRect(buyableRect, buyableBg, true);
        renderer->DrawRect(buyableRect, buyableBorder, false);

        // Buyable name
        Vec2 namePos(buyableX + 10.0f, buyableY + 10.0f);
        renderer->DrawText(buyable.name, namePos, Color::White(), 18.0f);

        // Purchase count
        std::string progressStr = buyable.GetProgressString();
        Vec2 progressPos(buyableX + 400.0f, buyableY + 10.0f);
        renderer->DrawText("Owned: " + progressStr, progressPos, Color::QuantumPurple(), 14.0f);

        // Description
        Vec2 descPos(buyableX + 10.0f, buyableY + 35.0f);
        renderer->DrawText(buyable.description, descPos, Color(0.8f, 0.8f, 0.9f, 1.0f), 13.0f);

        // Cost and Buy button
        f64 cost = buyable.GetCurrentCost();
        bool canAfford = buyable.CanAfford(m_Resources[0]);
        bool disabledByChallenge = m_ChallengeManager.HasModifier(ChallengeModifier::NoBuyables);

        if (!maxed) {
            std::string costStr = GameUtils::FormatNumber(cost, m_NumberFormat) + " Qubits";
            Vec2 costPos(buyableX + 10.0f, buyableY + 60.0f);
            Color costColor = (canAfford && !disabledByChallenge) ? Color::CoherenceGreen() : Color(0.7f, 0.5f, 0.5f, 1.0f);
            renderer->DrawText("Cost: " + costStr, costPos, costColor, 14.0f);

            // Buy button
            Rect buyBtn(buyableX + buyableWidth - 120.0f, buyableY + 55.0f, 110.0f, 35.0f);
            Color btnColor = (canAfford && !disabledByChallenge) ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);
            renderer->DrawRect(buyBtn, btnColor * 0.3f, true);
            renderer->DrawRect(buyBtn, (canAfford && !disabledByChallenge) ? Color::CoherenceGreen() : Color(0.4f, 0.4f, 0.4f, 1.0f), false);

            Vec2 btnTextPos(buyBtn.x + 28.0f, buyBtn.y + 10.0f);
            std::string btnText = disabledByChallenge ? "DISABLED" : "PURCHASE";
            renderer->DrawText(btnText, btnTextPos, Color::White(), 14.0f);
        } else {
            Vec2 maxedPos(buyableX + 10.0f, buyableY + 60.0f);
            renderer->DrawText("MAXED OUT", maxedPos, Color::CoherenceGreen(), 16.0f);
        }
    }
}

void GameState::RenderMilestoneNotifications(Renderer* renderer) {
    auto recentCompletions = m_MilestoneSystem.GetRecentCompletions();
    if (recentCompletions.empty()) return;

    // Display recent milestone completions as notifications
    f32 notifWidth = 400.0f;
    f32 notifHeight = 90.0f;
    f32 notifX = renderer->GetWidth() - notifWidth - 20.0f;
    f32 notifY = renderer->GetHeight() - notifHeight - 100.0f; // Above achievement notifications

    // Only show the most recent
    const Milestone* recent = recentCompletions.back();

    // Animated background
    f32 pulse = static_cast<f32>(0.9 + 0.1 * std::sin(m_TotalTimePlayed * 4.0));
    Color bgColor = Color::QuantumBlue() * pulse;
    bgColor.a = 0.95f;

    Rect notifBg(notifX, notifY, notifWidth, notifHeight);
    renderer->DrawRect(notifBg, bgColor, true);
    renderer->DrawRect(notifBg, Color::CoherenceGreen(), false);

    // Title
    Vec2 titlePos(notifX + 15.0f, notifY + 12.0f);
    renderer->DrawText("🎯 Milestone Completed!", titlePos, Color::CoherenceGreen(), 16.0f);

    // Milestone name
    Vec2 namePos(notifX + 15.0f, notifY + 35.0f);
    renderer->DrawText(recent->name, namePos, Color::White(), 18.0f);

    // Reward
    Vec2 rewardPos(notifX + 15.0f, notifY + 60.0f);
    renderer->DrawText(recent->rewardDescription, rewardPos, Color(1.0f, 1.0f, 0.6f, 1.0f), 12.0f);

    // Clear old notifications after a delay (would need time tracking for animation)
    // For now, keep last 2
    if (m_TotalTimePlayed - static_cast<i32>(m_TotalTimePlayed) > 0.98) {
        // Clear once per second
        static f64 lastClear = 0;
        if (m_TotalTimePlayed - lastClear > 5.0) {
            m_MilestoneSystem.ClearRecentCompletions();
            lastClear = m_TotalTimePlayed;
        }
    }
}

void GameState::RenderChallenges(Renderer* renderer) {
    if (!m_ShowChallenges) return;

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Challenges panel
    f32 panelWidth = 900.0f;
    f32 panelHeight = 650.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color::DarkPanel(), true);
    renderer->DrawRect(panel, Color::Red() * 0.8f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("QUANTUM CHALLENGES", titlePos, Color::Red(), 24.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);

    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color::Red() * 0.8f, false);

    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 200.0f, panelY + 45.0f);
    renderer->DrawText("(Click X or press C/ESC)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // Current challenge info (if in challenge)
    const Challenge* currentChallenge = m_ChallengeManager.GetCurrentChallenge();
    if (currentChallenge) {
        Vec2 currentPos(panelX + 20.0f, panelY + 50.0f);
        renderer->DrawText("⚠ ACTIVE CHALLENGE: " + currentChallenge->name, currentPos, Color::Red(), 16.0f);

        Vec2 goalPos(panelX + 20.0f, panelY + 70.0f);
        f64 currentQubits = GetResource(QuantumResource::Qubits);
        std::string goalText = "Goal: " + GameUtils::FormatNumber(currentQubits, m_NumberFormat) +
                               " / " + GameUtils::FormatNumber(currentChallenge->goalQubits, m_NumberFormat) + " Qubits";
        renderer->DrawText(goalText, goalPos, Color::Yellow(), 13.0f);
    }

    // Challenges list
    f32 challengeStartY = currentChallenge ? panelY + 100.0f : panelY + 70.0f;
    f32 challengeX = panelX + 20.0f;
    f32 challengeWidth = panelWidth - 40.0f;
    f32 challengeHeight = 120.0f;
    f32 challengeSpacing = 10.0f;

    auto& challenges = m_ChallengeManager.GetChallenges();
    i32 currentPrestige = m_Statistics.totalPrestigesPerformed;

    for (size_t i = 0; i < challenges.size(); i++) {
        const auto& challenge = challenges[i];
        f32 challengeY = challengeStartY + (challengeHeight + challengeSpacing) * i;

        // Challenge background
        bool completed = challenge.completed;
        bool active = challenge.active;
        bool canEnter = challenge.CanEnter(currentPrestige, currentChallenge != nullptr);

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

        Rect challengeRect(challengeX, challengeY, challengeWidth, challengeHeight);
        renderer->DrawRect(challengeRect, challengeBg, true);
        renderer->DrawRect(challengeRect, challengeBorder, false);

        // Challenge name
        Vec2 namePos(challengeX + 10.0f, challengeY + 10.0f);
        std::string nameStr = challenge.name;
        if (active) nameStr += " [ACTIVE]";
        if (completed) nameStr += " [COMPLETED]";
        Color nameColor = completed ? Color::CoherenceGreen() : (active ? Color::Red() : Color::White());
        renderer->DrawText(nameStr, namePos, nameColor, 18.0f);

        // Description
        Vec2 descPos(challengeX + 10.0f, challengeY + 35.0f);
        renderer->DrawText(challenge.description, descPos, Color(0.8f, 0.8f, 0.9f, 1.0f), 13.0f);

        // Requirements
        Vec2 reqPos(challengeX + 10.0f, challengeY + 55.0f);
        std::string reqText = "Requires: " + std::to_string(challenge.minPrestigeLevel) + " prestiges";
        Color reqColor = currentPrestige >= challenge.minPrestigeLevel ? Color::CoherenceGreen() : Color(0.7f, 0.5f, 0.5f, 1.0f);
        renderer->DrawText(reqText, reqPos, reqColor, 12.0f);

        // Goal
        Vec2 goalPos(challengeX + 10.0f, challengeY + 75.0f);
        std::string goalText = "Goal: " + GameUtils::FormatNumber(challenge.goalQubits, m_NumberFormat) + " Qubits";
        renderer->DrawText(goalText, goalPos, Color::Yellow(), 12.0f);

        // Reward
        Vec2 rewardPos(challengeX + 10.0f, challengeY + 95.0f);
        renderer->DrawText("Reward: " + challenge.rewardDescription, rewardPos, Color::QuantumPurple(), 12.0f);

        // Enter/Exit button
        if (!completed) {
            Rect actionBtn(challengeX + challengeWidth - 120.0f, challengeY + 80.0f, 110.0f, 30.0f);

            if (active) {
                // Exit button
                renderer->DrawRect(actionBtn, Color::Red() * 0.3f, true);
                renderer->DrawRect(actionBtn, Color::Red(), false);
                Vec2 btnTextPos(actionBtn.x + 35.0f, actionBtn.y + 8.0f);
                renderer->DrawText("EXIT", btnTextPos, Color::White(), 14.0f);
            } else {
                // Enter button
                Color btnColor = canEnter ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                renderer->DrawRect(actionBtn, btnColor * 0.3f, true);
                renderer->DrawRect(actionBtn, canEnter ? Color::CoherenceGreen() : Color(0.4f, 0.4f, 0.4f, 1.0f), false);
                Vec2 btnTextPos(actionBtn.x + 30.0f, actionBtn.y + 8.0f);
                renderer->DrawText("ENTER", btnTextPos, Color::White(), 14.0f);
            }
        }
    }
}
void GameState::RenderEssenceShop(Renderer* renderer) {
    if (!m_ShowEssenceShop) return;

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Essence shop panel
    f32 panelWidth = 900.0f;
    f32 panelHeight = 650.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color::DarkPanel(), true);
    renderer->DrawRect(panel, Color::Magenta() * 0.8f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("💎 ESSENCE SHOP - PERMANENT UPGRADES", titlePos, Color::Magenta(), 22.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);

    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color::Magenta() * 0.8f, false);

    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 200.0f, panelY + 45.0f);
    renderer->DrawText("(Click X or press E/ESC)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // Current essence display
    Vec2 essencePos(panelX + 20.0f, panelY + 50.0f);
    std::string essenceText = "Your Quantum Essence: " + GameUtils::FormatNumber(m_QuantumEssence, m_NumberFormat);
    renderer->DrawText(essenceText, essencePos, Color::Magenta() * 1.3f, 16.0f);

    // Upgrades list
    f32 upgradeStartY = panelY + 85.0f;
    f32 upgradeX = panelX + 20.0f;
    f32 upgradeWidth = panelWidth - 40.0f;
    f32 upgradeHeight = 100.0f;
    f32 upgradeSpacing = 10.0f;

    auto& upgrades = m_EssenceShopManager.GetUpgrades();

    for (size_t i = 0; i < upgrades.size(); i++) {
        const auto& upgrade = upgrades[i];
        f32 upgradeY = upgradeStartY + (upgradeHeight + upgradeSpacing) * i;

        // Upgrade background
        bool maxed = upgrade.IsMaxed();
        Color upgradeBg = maxed ? Color(0.1f, 0.2f, 0.15f, 1.0f) : Color(0.15f, 0.15f, 0.2f, 1.0f);
        Color upgradeBorder = maxed ? Color::CoherenceGreen() * 0.6f : Color::Magenta() * 0.6f;

        Rect upgradeRect(upgradeX, upgradeY, upgradeWidth, upgradeHeight);
        renderer->DrawRect(upgradeRect, upgradeBg, true);
        renderer->DrawRect(upgradeRect, upgradeBorder, false);

        // Upgrade name
        Vec2 namePos(upgradeX + 10.0f, upgradeY + 10.0f);
        renderer->DrawText(upgrade.name, namePos, Color::White(), 18.0f);

        // Purchase count
        std::string progressStr = upgrade.GetProgressString();
        Vec2 progressPos(upgradeX + 400.0f, upgradeY + 10.0f);
        renderer->DrawText("Owned: " + progressStr, progressPos, Color::Magenta(), 14.0f);

        // Description
        Vec2 descPos(upgradeX + 10.0f, upgradeY + 35.0f);
        renderer->DrawText(upgrade.description, descPos, Color(0.8f, 0.8f, 0.9f, 1.0f), 13.0f);

        // Cost and Buy button
        f64 cost = upgrade.GetCurrentCost();
        bool canAfford = upgrade.CanAfford(m_QuantumEssence);

        if (!maxed) {
            std::string costStr = GameUtils::FormatNumber(cost, m_NumberFormat) + " Essence";
            Vec2 costPos(upgradeX + 10.0f, upgradeY + 60.0f);
            Color costColor = canAfford ? Color::Magenta() * 1.3f : Color(0.7f, 0.5f, 0.5f, 1.0f);
            renderer->DrawText("Cost: " + costStr, costPos, costColor, 14.0f);

            // Buy button
            Rect buyBtn(upgradeX + upgradeWidth - 120.0f, upgradeY + 55.0f, 110.0f, 35.0f);
            Color btnColor = canAfford ? Color::Magenta() : Color(0.3f, 0.3f, 0.3f, 1.0f);
            renderer->DrawRect(buyBtn, btnColor * 0.3f, true);
            renderer->DrawRect(buyBtn, canAfford ? Color::Magenta() : Color(0.4f, 0.4f, 0.4f, 1.0f), false);

            Vec2 btnTextPos(buyBtn.x + 28.0f, buyBtn.y + 10.0f);
            renderer->DrawText("PURCHASE", btnTextPos, Color::White(), 14.0f);
        } else {
            Vec2 maxedPos(upgradeX + 10.0f, upgradeY + 60.0f);
            renderer->DrawText("MAXED OUT", maxedPos, Color::CoherenceGreen(), 16.0f);
        }
    }
}

void GameState::RenderSingularityShop(Renderer* renderer) {
    if (!m_ShowSingularityShop) return;

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Singularity shop panel
    f32 panelWidth = 900.0f;
    f32 panelHeight = 650.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color::DarkPanel(), true);
    renderer->DrawRect(panel, Color(0.5f, 0.0f, 1.0f, 1.0f) * 0.8f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("⭐ SINGULARITY SHOP - COSMIC UPGRADES", titlePos, Color(0.8f, 0.0f, 1.0f, 1.0f), 22.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);

    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color(0.5f, 0.0f, 1.0f, 1.0f) * 0.8f, false);

    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 200.0f, panelY + 45.0f);
    renderer->DrawText("(Click X or press ESC)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // Current singularities display
    Vec2 singularityPos(panelX + 20.0f, panelY + 50.0f);
    std::string singularityText = "Your Singularities: " + GameUtils::FormatNumber(m_Timeline.singularities, m_NumberFormat);
    renderer->DrawText(singularityText, singularityPos, Color(0.8f, 0.0f, 1.0f, 1.0f) * 1.3f, 16.0f);

    // Upgrades list
    f32 upgradeStartY = panelY + 85.0f;
    f32 upgradeX = panelX + 20.0f;
    f32 upgradeWidth = panelWidth - 40.0f;
    f32 upgradeHeight = 100.0f;
    f32 upgradeSpacing = 10.0f;

    auto& upgrades = m_SingularityShopManager.GetUpgrades();

    for (size_t i = 0; i < upgrades.size(); i++) {
        const auto& upgrade = upgrades[i];
        f32 upgradeY = upgradeStartY + (upgradeHeight + upgradeSpacing) * i;

        // Upgrade background
        bool maxed = upgrade.IsMaxed();
        Color upgradeBg = maxed ? Color(0.1f, 0.15f, 0.2f, 1.0f) : Color(0.1f, 0.1f, 0.15f, 1.0f);
        Color upgradeBorder = maxed ? Color::CoherenceGreen() * 0.6f : Color(0.5f, 0.0f, 1.0f, 1.0f) * 0.6f;

        Rect upgradeRect(upgradeX, upgradeY, upgradeWidth, upgradeHeight);
        renderer->DrawRect(upgradeRect, upgradeBg, true);
        renderer->DrawRect(upgradeRect, upgradeBorder, false);

        // Upgrade name
        Vec2 namePos(upgradeX + 10.0f, upgradeY + 10.0f);
        renderer->DrawText(upgrade.name, namePos, Color::White(), 18.0f);

        // Purchase count
        std::string progressStr = upgrade.GetProgressString();
        Vec2 progressPos(upgradeX + 400.0f, upgradeY + 10.0f);
        renderer->DrawText("Owned: " + progressStr, progressPos, Color(0.8f, 0.0f, 1.0f, 1.0f), 14.0f);

        // Description
        Vec2 descPos(upgradeX + 10.0f, upgradeY + 35.0f);
        renderer->DrawText(upgrade.description, descPos, Color(0.7f, 0.7f, 0.9f, 1.0f), 13.0f);

        // Cost and Buy button
        f64 cost = upgrade.GetCurrentCost();
        bool canAfford = upgrade.CanAfford(m_Timeline.singularities);

        if (!maxed) {
            std::string costStr = GameUtils::FormatNumber(cost, m_NumberFormat) + " Singularities";
            Vec2 costPos(upgradeX + 10.0f, upgradeY + 60.0f);
            Color costColor = canAfford ? Color(0.8f, 0.0f, 1.0f, 1.0f) * 1.3f : Color(0.7f, 0.5f, 0.5f, 1.0f);
            renderer->DrawText("Cost: " + costStr, costPos, costColor, 14.0f);

            // Buy button
            Rect buyBtn(upgradeX + upgradeWidth - 120.0f, upgradeY + 55.0f, 110.0f, 35.0f);
            Color btnColor = canAfford ? Color(0.5f, 0.0f, 1.0f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f);
            renderer->DrawRect(buyBtn, btnColor * 0.3f, true);
            renderer->DrawRect(buyBtn, canAfford ? Color(0.8f, 0.0f, 1.0f, 1.0f) : Color(0.4f, 0.4f, 0.4f, 1.0f), false);

            Vec2 btnTextPos(buyBtn.x + 28.0f, buyBtn.y + 10.0f);
            renderer->DrawText("PURCHASE", btnTextPos, Color::White(), 14.0f);
        } else {
            Vec2 maxedPos(upgradeX + 10.0f, upgradeY + 60.0f);
            renderer->DrawText("MAXED OUT", maxedPos, Color::CoherenceGreen(), 16.0f);
        }
    }
}

void GameState::RenderSpaceship(Renderer* renderer) {
    if (!m_ShowSpaceship) return;

    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.0f, 0.8f), true);

    // Main spaceship panel
    f32 panelWidth = 1100.0f;
    f32 panelHeight = 700.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color::DarkPanel(), true);
    renderer->DrawRect(panel, Color(1.0f, 0.7f, 0.0f, 1.0f) * 0.8f, false); // Gold border

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("SPACESHIP - REPAIR AND UPGRADE", titlePos, Color(1.0f, 0.7f, 0.0f, 1.0f), 22.0f);

    // Close button (X) in top right
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);

    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color(1.0f, 0.7f, 0.0f, 1.0f) * 0.8f, false);

    Vec2 xPos(closeBtnX + 10.0f, closeBtnY + 8.0f);
    renderer->DrawText("X", xPos, Color::White(), 16.0f);

    // Hint text
    Vec2 hintPos(panelX + panelWidth - 250.0f, panelY + 45.0f);
    renderer->DrawText("(Press H or ESC to close)", hintPos, Color(0.6f, 0.6f, 0.6f, 1.0f), 11.0f);

    // Split panel into left (ship status) and right (inventory)
    f32 leftPanelWidth = panelWidth * 0.5f - 15.0f;
    f32 rightPanelWidth = panelWidth * 0.5f - 15.0f;
    f32 contentY = panelY + 70.0f;
    f32 contentHeight = panelHeight - 90.0f;

    // Left panel: Ship status and installed parts
    f32 leftPanelX = panelX + 10.0f;
    m_Spaceship.RenderShipPanel(renderer, leftPanelX, contentY, leftPanelWidth, contentHeight);

    // Right panel: Part inventory
    f32 rightPanelX = panelX + leftPanelWidth + 20.0f;
    m_Spaceship.RenderInventoryPanel(renderer, rightPanelX, contentY, rightPanelWidth, contentHeight);

    // Instructions at bottom
    Vec2 instructionPos(panelX + 20.0f, panelY + panelHeight - 25.0f);
    renderer->DrawText("Ship parts drop from Research Station observations. Install parts to increase production & unlock travel!",
                     instructionPos, Color(0.7f, 0.7f, 0.7f, 1.0f), 12.0f);
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
    for (auto it = m_Anomalies.begin(); it != m_Anomalies.end();) {
        it->lifetime += deltaTime;

        // Remove if expired or clicked
        if (it->lifetime >= it->maxLifetime || it->clicked) {
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
    for (const auto& anomaly : m_Anomalies) {
        f32 lifeRatio = 1.0f - (anomaly.lifetime / anomaly.maxLifetime);

        // Pulsing effect (makes it more noticeable)
        f32 pulseScale = 1.0f + 0.2f * sinf(anomaly.lifetime * 5.0f);
        f32 currentRadius = anomaly.radius * pulseScale;

        // Outer glow (larger, transparent)
        Color glowColor = anomaly.color;
        glowColor.a = 0.3f * lifeRatio;
        renderer->DrawCircle(anomaly.position, currentRadius * 1.5f, glowColor, true);

        // Inner core (bright, opaque)
        Color coreColor = anomaly.color;
        coreColor.a = 0.9f * lifeRatio;
        renderer->DrawCircle(anomaly.position, currentRadius, coreColor, true);

        // White center dot
        renderer->DrawCircle(anomaly.position, currentRadius * 0.3f, Color::White(), true);

        // Show reward multiplier above it
        std::string rewardText = std::to_string(static_cast<i32>(anomaly.rewardMultiplier)) + "x";
        Vec2 textPos(anomaly.position.x - 15.0f, anomaly.position.y - currentRadius - 20.0f);
        renderer->DrawText(rewardText, textPos, Color::White(), 14.0f);

        // Lifetime bar below it
        f32 barWidth = anomaly.radius * 2.0f;
        f32 barHeight = 4.0f;
        Vec2 barPos(anomaly.position.x - barWidth / 2.0f, anomaly.position.y + currentRadius + 10.0f);

        // Background
        Rect barBg(barPos.x, barPos.y, barWidth, barHeight);
        renderer->DrawRect(barBg, Color(0.2f, 0.2f, 0.2f, 0.8f), true);

        // Fill
        Rect barFill(barPos.x, barPos.y, barWidth * lifeRatio, barHeight);
        renderer->DrawRect(barFill, anomaly.color, true);
    }
}

void GameState::ClickQuantumAnomaly(const Vec2& clickPos) {
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
    m_CombatSystem.StartCombat(&m_CurrentEnemy, m_PlayerLevel, &m_Spaceship, this);
    
    Log::Infof("Starting combat with ", m_CurrentEnemy.GetName());
}

void GameState::EndCombat() {
    if (!m_CombatSystem.IsInCombat()) return;
    
    // Check if we won and award rewards
    if (m_CombatSystem.GetState() == CombatState::Victory) {
        // Award credits (as Qubits)
        AddResource(QuantumResource::Qubits, static_cast<f64>(m_CombatSystem.GetCreditsEarned()));
        
        // Award XP
        AddXP(static_cast<f64>(m_CombatSystem.GetXPEarned()));

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

void GameState::RenderCombat(Renderer* renderer) {
    if (!m_ShowCombat) return;
    
    // Render the combat UI
    m_CombatSystem.RenderCombatUI(renderer);
    
    // Show level/XP bar at the top
    f32 xpBarWidth = 300.0f;
    f32 xpBarHeight = 25.0f;
    f32 xpBarX = 10.0f;
    f32 xpBarY = 10.0f;
    
    f64 xpPercent = m_PlayerXP / GetXPForNextLevel();
    
    Rect xpBarBg(xpBarX, xpBarY, xpBarWidth, xpBarHeight);
    Rect xpBarFill(xpBarX, xpBarY, xpBarWidth * xpPercent, xpBarHeight);
    
    renderer->DrawRect(xpBarBg, Color(0.2f, 0.2f, 0.2f, 0.8f), true);
    renderer->DrawRect(xpBarFill, Color(1.0f, 0.9f, 0.0f, 1.0f), true);
    
    char levelText[64];
    snprintf(levelText, sizeof(levelText), "Level %d - %.0f / %.0f XP", m_PlayerLevel, m_PlayerXP, GetXPForNextLevel());
    renderer->DrawText(levelText, Vec2(xpBarX + 5.0f, xpBarY + 5.0f), Color::White(), 14.0f);
}

void GameState::RenderGatcha(Renderer* renderer) {
    if (!m_ShowGatcha) return;

    // Render the gatcha UI
    m_GatchaSystem.RenderSummonUI(renderer, this);
}

void GameState::RenderSkillTree(Renderer* renderer) {
    if (!m_ShowSkills) return;

    // Render the skill tree UI
    m_SkillTree.RenderSkillTree(renderer, this);
}
