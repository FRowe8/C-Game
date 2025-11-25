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
    : completedResets(0), photons(0), photonBonus(1.0) {
}

// ResearchStation implementation
ResearchStation::ResearchStation()
    : baseProduction(0), currentProduction(0), level(0),
      upgradeCost(0), upgradeCostMultiplier(1.15f),
      superpositionValue(0), superpositionProbability(0.5f),
      unlocked(false), unlockCost(0) {
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

    state->AddResource(resourceType, collapsedValue);
    superpositionValue = 0;

    // Update statistics
    auto& stats = state->GetStatistics();
    stats.totalObservations++;
    stats.sessionObservations++;

    // Spawn particles for visual feedback
    // (Will be called from GameState with renderer access)
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
        // Disabled: Muted gray/dark blue
        renderColor = Color(0.15f, 0.15f, 0.2f, baseAlpha * 0.5f);
        borderColor = Color(0.2f, 0.2f, 0.25f, baseAlpha * 0.5f);
        textColor = Color(0.5f, 0.5f, 0.55f, 1.0f);
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
    : m_TotalTimePlayed(0), m_TimeSinceLastSave(0),
      m_Coherence(100), m_MaxCoherence(100), m_CoherenceDecayRate(1.0),
      m_CurrentEvent(nullptr), m_TimeSinceLastEvent(0), m_EventCooldown(120.0),
      m_LastSaveTimestamp(0), m_ShowAchievements(false), m_ShowStats(false), m_ShowResearch(false), m_ShowMilestones(false) {

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

    // Try to load save file
    std::string savePath = Platform::GetSaveDirectory() + "quantum_save.json";
    if (Platform::FileExists(savePath)) {
        if (Load(savePath)) {
            Log::Info("Save file loaded successfully");
            CalculateOfflineProgress(); // Calculate what happened while away
        }
    }

    Log::Info("Game state initialized");
}

void GameState::InitializeStations() {
    // Station 1: Basic Qubit Generator
    ResearchStation station1;
    station1.name = "Qubit Generator";
    station1.description = "Generates qubits in superposition";
    station1.resourceType = QuantumResource::Qubits;
    station1.baseProduction = 1.0;
    station1.currentProduction = 1.0; // Start producing immediately
    station1.level = 1; // Start at level 1 for immediate passive income
    station1.upgradeCost = 10.0;
    station1.upgradeCostMultiplier = 1.15;
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
    station2.upgradeCost = 25.0;
    station2.upgradeCostMultiplier = 1.18;
    station2.superpositionProbability = 0.8;
    station2.unlocked = false;
    station2.unlockCost = 50.0;
    m_Stations.push_back(station2);

    // Station 3: Entanglement Chamber
    ResearchStation station3;
    station3.name = "Entanglement Chamber";
    station3.description = "Creates quantum entanglement";
    station3.resourceType = QuantumResource::Entanglement;
    station3.baseProduction = 0.2;
    station3.level = 0;
    station3.upgradeCost = 100.0;
    station3.upgradeCostMultiplier = 1.2;
    station3.superpositionProbability = 0.6;
    station3.unlocked = false;
    station3.unlockCost = 150.0;
    m_Stations.push_back(station3);

    // Station 4: Advanced Qubit Synthesizer
    ResearchStation station4;
    station4.name = "Qubit Synthesizer";
    station4.description = "Advanced qubit generation";
    station4.resourceType = QuantumResource::Qubits;
    station4.baseProduction = 5.0;
    station4.level = 0;
    station4.upgradeCost = 500.0;
    station4.upgradeCostMultiplier = 1.25;
    station4.superpositionProbability = 0.5;
    station4.unlocked = false;
    station4.unlockCost = 300.0;
    m_Stations.push_back(station4);

    // Station 5: Quantum Supercomputer
    ResearchStation station5;
    station5.name = "Quantum Supercomputer";
    station5.description = "Massive parallel processing";
    station5.resourceType = QuantumResource::Qubits;
    station5.baseProduction = 20.0;
    station5.level = 0;
    station5.upgradeCost = 2000.0;
    station5.upgradeCostMultiplier = 1.3;
    station5.superpositionProbability = 0.4;
    station5.unlocked = false;
    station5.unlockCost = 1000.0;
    m_Stations.push_back(station5);
}

void GameState::InitializeUI() {
    m_ScrollOffset = Vec2(0, 0);
    m_StationButtons.clear();

    // Create persistent buttons for each research station
    // For each station, we create: unlock button (index 3*i), observe button (index 3*i+1), upgrade button (index 3*i+2)
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
            if (SpendResource(QuantumResource::Qubits, m_Stations[i].upgradeCost)) {
                m_Stations[i].Upgrade();
                Log::Infof("Upgraded ", m_Stations[i].name, " to level ", m_Stations[i].level);
            }
        };
        m_StationButtons.push_back(upgradeBtn);
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
    m_TotalTimePlayed += deltaTime;
    m_TimeSinceLastSave += deltaTime;
    m_TimeSinceLastEvent += deltaTime;

    // Update statistics
    m_Statistics.UpdateSession(deltaTime);

    // Update stations
    UpdateStations(deltaTime);

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

    // Update UI
    UpdateUI(input);

    // Auto-save every 30 seconds
    if (m_TimeSinceLastSave >= 30.0) {
        std::string savePath = Platform::GetSaveDirectory() + "quantum_save.json";
        Save(savePath);
        m_TimeSinceLastSave = 0;
    }
}

void GameState::UpdateStations(f64 deltaTime) {
    // Apply prestige bonus
    f64 globalMultiplier = m_Timeline.photonBonus;

    for (auto& station : m_Stations) {
        station.Update(deltaTime * globalMultiplier);
    }
}

void GameState::UpdateCoherence(f64 deltaTime) {
    // Coherence slowly decays
    m_Coherence -= m_CoherenceDecayRate * deltaTime;
    if (m_Coherence < 0) m_Coherence = 0;

    // Coherence affects production
    f64 coherenceMultiplier = m_Coherence / m_MaxCoherence;
    // Apply to stations (already done in UpdateStations)
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

        // Clamp scroll bounds
        f32 maxScroll = 0.0f; // Can't scroll up past the top
        f32 contentHeight = (m_Stations.size() * 145.0f) + 200.0f; // Total content height
        f32 viewportHeight = 600.0f; // Visible area height
        f32 minScroll = -(contentHeight - viewportHeight);
        if (minScroll > 0) minScroll = 0; // If content fits on screen, don't allow scrolling

        m_ScrollOffset.y = std::max(minScroll, std::min(maxScroll, m_ScrollOffset.y));
    }

    // Keyboard shortcuts
    // SDL_SCANCODE_A = 4, M = 13, R = 15, S = 16, ESCAPE = 41
    const int KEY_A = 4;
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
    if (input->IsKeyPressed(KEY_ESCAPE)) {
        m_ShowAchievements = false;
        m_ShowStats = false;
        m_ShowResearch = false;
        m_ShowMilestones = false;
    }

    // Update buttons
    for (auto& button : m_StationButtons) {
        button.Update(mousePos);

        if (button.WasClicked(mousePos, mousePressed) && button.onClick) {
            button.onClick();
        }
    }
}

void GameState::Render(Renderer* renderer) {
    RenderResources(renderer);
    RenderStations(renderer);
    RenderActiveEvent(renderer);
    RenderUI(renderer);
    RenderAchievements(renderer);
    RenderStatistics(renderer);
    RenderResearchTree(renderer);
    RenderMilestones(renderer);
    RenderParticleEffects(renderer, 1.0/60.0); // Assume 60 FPS for particles
    RenderAchievementNotifications(renderer);
    RenderMilestoneNotifications(renderer);
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

        // Draw value (formatted)
        std::string formattedValue = GameUtils::FormatNumber(m_Resources[i]);
        Vec2 valuePos(xOffset, 50.0f);
        renderer->DrawText(formattedValue, valuePos, resourceColors[i], 24.0f);

        xOffset += 250.0f;
    }

    // Draw coherence bar
    f32 coherenceBarWidth = 200.0f;
    f32 coherenceBarHeight = 20.0f;
    Vec2 coherenceBarPos(static_cast<f32>(renderer->GetWidth()) - coherenceBarWidth - 20.0f, 40.0f);

    renderer->DrawText("Coherence", Vec2(coherenceBarPos.x, coherenceBarPos.y - 20.0f), Color::White(), 14.0f);

    Rect coherenceBarBg(coherenceBarPos.x, coherenceBarPos.y, coherenceBarWidth, coherenceBarHeight);
    renderer->DrawRect(coherenceBarBg, Color(0.2f, 0.2f, 0.2f, 1.0f), true);

    f32 coherenceFill = (m_Coherence / m_MaxCoherence) * coherenceBarWidth;
    Rect coherenceBarFill(coherenceBarPos.x, coherenceBarPos.y, coherenceFill, coherenceBarHeight);
    renderer->DrawRect(coherenceBarFill, Color::CoherenceGreen(), true);

    renderer->DrawRect(coherenceBarBg, Color::White() * 0.5f, false);
}

void GameState::RenderStations(Renderer* renderer) {
    // Buttons are persistent and created in InitializeUI
    // Here we just update their bounds, text, and enabled state, then render them

    f32 startY = 120.0f;
    f32 stationHeight = 150.0f; // Increased height for the new layout
    f32 margin = 20.0f; // Generous margin for breathing room

    // Counter for accessing persistent buttons (3 buttons per station + 1 prestige button)
    size_t buttonIdx = 0;

    for (size_t i = 0; i < m_Stations.size(); i++) {
        auto& station = m_Stations[i];
        f32 y = startY + i * (stationHeight + margin) + m_ScrollOffset.y;
        
        // Define the main panel area
        Rect stationRect(30.0f, y, static_cast<f32>(renderer->GetWidth()) - 60.0f, stationHeight);

        // --- New Glass-Panel Background ---
        Color bgColor, borderColor;
        if (station.unlocked) {
            bgColor = Color(0.1f, 0.12f, 0.18f, 0.8f); // Darker, subtle background
            borderColor = Color::QuantumBlue() * 0.7f;
            borderColor.a = 0.8f;
        } else {
            bgColor = Color(0.15f, 0.1f, 0.1f, 0.6f);
            borderColor = Color(0.4f, 0.2f, 0.2f, 0.8f);
        }

        // Draw the background panel (No heavy shadow for a flatter look)
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

            unlockBtn.Render(renderer);

            // Skip observe and upgrade buttons
            buttonIdx += 2;
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
        prodOss << std::fixed << (station.currentProduction * GetTimeline()->photonBonus) << "/s";
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
        
        // Skip unlock button (not needed for unlocked stations)
        buttonIdx++;

        // Get observe button (Left Button)
        UIButton& observeBtn = m_StationButtons[buttonIdx++];
        observeBtn.bounds = Rect(stationRect.x + 20.0f, y + 85.0f, 200.0f, 45.0f);
        observeBtn.enabled = station.superpositionValue > 0.1;
        observeBtn.Render(renderer);

        // Get upgrade button (Right Button)
        UIButton& upgradeBtn = m_StationButtons[buttonIdx++];
        upgradeBtn.bounds = Rect(stationRect.x + 240.0f, y + 85.0f, 250.0f, 45.0f);
        upgradeBtn.text = "UPGRADE CORE (" + std::to_string(static_cast<i64>(station.upgradeCost)) + ")";
        upgradeBtn.enabled = m_Resources[0] >= station.upgradeCost;
        upgradeBtn.Render(renderer);
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
}

void GameState::RenderUI(Renderer* renderer) {
    // Additional UI elements can go here
    // Prestige info, achievements, etc.
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

f64 GameState::CalculatePhotonsOnPrestige() const {
    // Photons based on total qubits earned
    f64 totalQubits = m_Resources[0];
    if (totalQubits < 1000.0) return 0;

    return std::floor(std::sqrt(totalQubits / 100.0));
}

void GameState::PerformPrestige() {
    f64 photons = CalculatePhotonsOnPrestige();
    if (photons <= 0) {
        Log::Warning("Not enough progress for prestige");
        return;
    }

    Log::Infof("Performing prestige! Gained ", photons, " photons");

    m_Timeline.photons += photons;
    m_Timeline.completedResets++;
    m_Timeline.photonBonus = 1.0 + (m_Timeline.photons * 0.1); // 10% per photon

    // Reset resources
    for (int i = 0; i < 3; i++) {
        m_Resources[i] = 0;
    }
    m_Resources[0] = 10.0; // Start with 10 qubits

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

    // Achievement panel
    f32 panelWidth = 600.0f;
    f32 panelHeight = 500.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    // Background
    Rect panelBg(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelBg, Color(0.05f, 0.05f, 0.1f, 0.95f), true);
    renderer->DrawRect(panelBg, Color::QuantumBlue() * 0.5f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 10.0f);
    renderer->DrawText("Achievements", titlePos, Color::QuantumBlue(), 24.0f);

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
            std::string progressText = GameUtils::FormatNumber(ach.progress) + " / " + GameUtils::FormatNumber(ach.target);
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

    // Stats panel
    f32 panelWidth = 500.0f;
    f32 panelHeight = 400.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    // Background
    Rect panelBg(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelBg, Color(0.05f, 0.05f, 0.1f, 0.95f), true);
    renderer->DrawRect(panelBg, Color::EntanglementOrange() * 0.5f, false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 10.0f);
    renderer->DrawText("Statistics", titlePos, Color::EntanglementOrange(), 24.0f);

    // Display stats
    f32 yOffset = panelY + 50.0f;
    auto renderStat = [&](const std::string& label, const std::string& value) {
        Vec2 labelPos(panelX + 30.0f, yOffset);
        Vec2 valuePos(panelX + 300.0f, yOffset);
        renderer->DrawText(label, labelPos, Color::White(), 14.0f);
        renderer->DrawText(value, valuePos, Color::EntanglementOrange(), 14.0f);
        yOffset += 25.0f;
    };

    renderStat("Total Qubits Earned:", GameUtils::FormatNumber(m_Statistics.totalQubitsEarned));
    renderStat("Total Observations:", std::to_string(m_Statistics.totalObservations));
    renderStat("Total Upgrades:", std::to_string(m_Statistics.totalUpgrades));
    renderStat("Total Prestiges:", std::to_string(m_Statistics.totalPrestigesPerformed));

    yOffset += 10.0f;
    renderStat("Session Time:", GameUtils::FormatTime(m_Statistics.sessionTime));
    renderStat("Session Qubits:", GameUtils::FormatNumber(m_Statistics.sessionQubits));
    renderStat("Session Observations:", std::to_string(m_Statistics.sessionObservations));

    yOffset += 10.0f;
    renderStat("Highest Qubits:", GameUtils::FormatNumber(m_Statistics.highestQubits));
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
    f32 panelHeight = 650.0f;
    f32 panelX = (renderer->GetWidth() - panelWidth) / 2.0f;
    f32 panelY = (renderer->GetHeight() - panelHeight) / 2.0f;

    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color(0.1f, 0.1f, 0.15f, 1.0f), true);
    renderer->DrawRect(panel, Color::QuantumBlue(), false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("🔬 Research Tree", titlePos, Color::QuantumBlue(), 24.0f);

    // Close button hint
    Vec2 closeHintPos(panelX + panelWidth - 120.0f, panelY + 18.0f);
    renderer->DrawText("[R to Close]", closeHintPos, Color(0.7f, 0.7f, 0.7f, 1.0f), 12.0f);

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
            costText += GameUtils::FormatNumber(node->qubitCost) + " Qubits  ";
        }
        if (node->coherenceCost > 0) {
            costText += GameUtils::FormatNumber(node->coherenceCost) + " Coherence  ";
        }
        if (node->entanglementCost > 0) {
            costText += GameUtils::FormatNumber(node->entanglementCost) + " Entanglement  ";
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

    // Check resource costs
    if (GetResource(QuantumResource::Qubits) < node->qubitCost) return false;
    if (GetResource(QuantumResource::Coherence) < node->coherenceCost) return false;
    if (GetResource(QuantumResource::Entanglement) < node->entanglementCost) return false;

    // Check photon cost
    if (m_Timeline.photons < node->photonCost) return false;

    // Check if can be researched
    if (!m_ResearchTree.CanResearch(id, m_Timeline.completedResets)) return false;

    return true;
}

bool GameState::PurchaseResearch(ResearchID id) {
    if (!CanAffordResearch(id)) return false;

    ResearchNode* node = m_ResearchTree.GetNode(id);
    if (!node) return false;

    // Spend resources
    SpendResource(QuantumResource::Qubits, node->qubitCost);
    SpendResource(QuantumResource::Coherence, node->coherenceCost);
    SpendResource(QuantumResource::Entanglement, node->entanglementCost);
    m_Timeline.photons -= node->photonCost;

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

    for (auto& station : m_Stations) {
        if (station.unlocked && station.level > 0) {
            station.currentProduction = station.baseProduction * station.level * productionMult;
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
    renderer->DrawRect(panel, Color(0.1f, 0.1f, 0.15f, 1.0f), true);
    renderer->DrawRect(panel, Color::QuantumBlue(), false);

    // Title
    Vec2 titlePos(panelX + 20.0f, panelY + 15.0f);
    renderer->DrawText("🎯 Milestones", titlePos, Color::QuantumBlue(), 24.0f);

    // Close button hint
    Vec2 closeHintPos(panelX + panelWidth - 120.0f, panelY + 18.0f);
    renderer->DrawText("[M to Close]", closeHintPos, Color(0.7f, 0.7f, 0.7f, 1.0f), 12.0f);

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
        std::string progressText = GameUtils::FormatNumber(milestone->progress) + " / " + GameUtils::FormatNumber(milestone->target);
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
