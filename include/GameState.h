#pragma once

#include "Types.h"
#include "Research.h"
#include "Milestones.h"
#include "Buyables.h"
#include "Challenges.h"
#include "EssenceShop.h"
#include "SingularityShop.h"
#include "GameUtils.h"
#include <vector>
#include <string>
#include <functional>

// Forward declarations
class Renderer;
class Input;

// Quantum resource types
enum class QuantumResource {
    Qubits,        // Primary currency
    Coherence,     // Stability resource
    Entanglement   // Strategic resource
};

// Research station that generates resources
struct ResearchStation {
    std::string name;
    std::string description;
    QuantumResource resourceType;

    f64 baseProduction;        // Base production per second
    f64 currentProduction;     // Current production (with bonuses)

    i32 level;                 // Upgrade level
    f64 upgradeCost;           // Cost to upgrade
    f64 upgradeCostMultiplier; // How much cost increases per level

    // Superposition state (resources waiting to be observed)
    f64 superpositionValue;
    f64 superpositionProbability; // Chance of getting full value on observation

    bool unlocked;
    f64 unlockCost;

    // Automation
    bool autoUpgrade; // Automatically upgrade when can afford 10x the cost

    ResearchStation();
    void Upgrade();
    void Observe(class GameState* state); // Collapse superposition
    void Update(f64 deltaTime);
};

// Entanglement pairs - link stations for bonuses
struct EntanglementPair {
    i32 stationA;
    i32 stationB;
    f64 bonusMultiplier;
    f64 coherenceCost;
    bool active;
};

// Achievement system
enum class AchievementID {
    FirstQubit,           // Earn your first qubit
    Observe100Times,      // Observe 100 times
    Reach1Million,        // Reach 1 million qubits
    FirstPrestige,        // Perform first prestige
    TenStations,          // Own 10 research stations
    MaxCoherence,         // Reach maximum coherence
    Entangle5Pairs,       // Create 5 entanglement pairs
    OfflineMillionaire,   // Earn 1M qubits offline
    SpeedRunner,          // Prestige within 10 minutes
    Hoarder,              // Save 100M qubits without spending
    QuantumMaster,        // Reach prestige level 10
    Collector,            // Unlock all station types
    EventHunter,          // Experience 50 quantum events
    WeekStreak,           // Play 7 days in a row
    TotalCount            // Keep this last
};

struct Achievement {
    AchievementID id;
    std::string name;
    std::string description;
    bool unlocked;
    f64 progress;         // For incremental achievements
    f64 target;           // Target value for completion
    f64 rewardQubits;     // Qubit reward
    f64 rewardPhotons;    // Photon reward

    Achievement();
};

// Statistics tracking
struct GameStatistics {
    // Lifetime stats
    f64 totalQubitsEarned;
    f64 totalCoherenceEarned;
    f64 totalEntanglementEarned;
    i32 totalObservations;
    i32 totalUpgrades;
    i32 totalPrestigesPerformed;

    // Session stats
    f64 sessionQubits;
    f64 sessionTime;
    i32 sessionObservations;

    // Records
    f64 highestQubits;
    f64 fastestPrestige;
    i32 longestStreak;

    // Daily login tracking
    i32 currentStreak;
    i64 lastLoginTimestamp;

    GameStatistics();
    void Reset();
    void UpdateSession(f64 deltaTime);
};

// Random quantum events
enum class QuantumEventType {
    WaveCollapse,         // Bonus observation probability
    CoherenceBoost,       // Temporary coherence boost
    QuantumFluctuation,   // Random resource bonus
    EntanglementSurge,    // Temporarily free entanglements
    TimeDialation,        // 2x production for 30 seconds
    LuckyObservation,     // Next observation guaranteed success
    ResourceRain,         // All resources rain from sky
    COUNT
};

struct QuantumEvent {
    QuantumEventType type;
    std::string name;
    std::string description;
    f64 duration;         // How long the event lasts
    f64 timeRemaining;    // Time left on current event
    f64 multiplier;       // Effect strength
    bool active;

    QuantumEvent();
};

// Prestige currency and bonuses
struct QuantumTimeline {
    i32 completedResets;
    f64 photons; // Prestige currency
    f64 photonBonus; // Global multiplier from photons

    // Second prestige layer
    i32 completedCollapses; // Number of singularity collapses performed
    f64 singularities; // Second-layer currency (gained by collapsing photons)

    std::vector<bool> permanentUpgrades;

    QuantumTimeline();
};

// UI Button
struct UIButton {
    Rect bounds;
    std::string text;
    Color color;
    Color hoverColor;
    std::function<void()> onClick;
    bool enabled = true;
    bool hovered = false;
    f64 affordability = 1.0; // 0.0-1.0: How close to affording (for visual feedback)

    void Update(const Vec2& mousePos);
    void Render(Renderer* renderer);
    bool WasClicked(const Vec2& mousePos, bool mousePressed);
};

class GameState {
public:
    GameState();
    ~GameState();

    void Initialize();
    void Update(f64 deltaTime, Input* input, Renderer* renderer);
    void Render(Renderer* renderer);

    // Resource management
    void AddResource(QuantumResource type, f64 amount);
    bool SpendResource(QuantumResource type, f64 amount);
    f64 GetResource(QuantumResource type) const;

    // Quantum Essence management (permanent meta-currency)
    void AddEssence(f64 amount);
    bool SpendEssence(f64 amount);
    f64 GetEssence() const { return m_QuantumEssence; }

    // Save/Load
    bool Save(const std::string& filepath);
    bool Load(const std::string& filepath);

    // Prestige
    void PerformPrestige();
    f64 CalculatePhotonsOnPrestige() const;

    // Singularity Collapse (Second prestige layer)
    void PerformCollapse();
    f64 CalculateSingularitiesOnCollapse() const;

    // Achievements
    void CheckAchievements();
    void UnlockAchievement(AchievementID id);
    Achievement* GetAchievement(AchievementID id);

    // Quantum Events
    void TriggerRandomEvent();
    void UpdateEvents(f64 deltaTime);
    QuantumEvent* GetActiveEvent();

    // Offline Progress
    void CalculateOfflineProgress();

    // Statistics
    GameStatistics& GetStatistics() { return m_Statistics; }

    // Research Tree
    ResearchTree& GetResearchTree() { return m_ResearchTree; }
    bool CanAffordResearch(ResearchID id) const;
    bool PurchaseResearch(ResearchID id);
    void UpdateResearchBonuses();

    // Milestone System
    MilestoneSystem& GetMilestoneSystem() { return m_MilestoneSystem; }
    void CheckMilestones();
    void AddPhotons(f64 amount);

    // Buyables System
    BuyableManager& GetBuyableManager() { return m_BuyableManager; }

    // Challenge System
    ChallengeManager& GetChallengeManager() { return m_ChallengeManager; }

    // Essence Shop System
    EssenceShopManager& GetEssenceShopManager() { return m_EssenceShopManager; }

    // Singularity Shop System
    SingularityShopManager& GetSingularityShopManager() { return m_SingularityShopManager; }
    QuantumTimeline& GetTimeline() { return m_Timeline; }

private:
    void InitializeStations();
    void InitializeUI();
    void UpdateStations(f64 deltaTime);
    void UpdateCoherence(f64 deltaTime);
    void UpdateUI(Input* input);
    void RenderUI(Renderer* renderer);
    void RenderResources(Renderer* renderer);
    void RenderStations(Renderer* renderer);
    void RenderParticleEffects(Renderer* renderer, f64 deltaTime);
    void RenderAchievements(Renderer* renderer);
    void RenderStatistics(Renderer* renderer);
    void RenderActiveEvent(Renderer* renderer);
    void RenderAchievementNotifications(Renderer* renderer);
    void RenderResearchTree(Renderer* renderer);
    void RenderMilestones(Renderer* renderer);
    void RenderMilestoneNotifications(Renderer* renderer);
    void RenderBuyables(Renderer* renderer);
    void RenderChallenges(Renderer* renderer);
    void RenderEssenceShop(Renderer* renderer);
    void RenderSingularityShop(Renderer* renderer);

    // Particle system helpers
    void SpawnParticle(const Vec2& position, const Color& color, f64 lifetime = 1.0);
    void SpawnParticleBurst(const Vec2& position, const Color& color, i32 count = 10);
    void UpdateParticles(f64 deltaTime);

    // Resources
    f64 m_Resources[3]; // Qubits, Coherence, Entanglement

    // Game objects
    std::vector<ResearchStation> m_Stations;
    std::vector<EntanglementPair> m_Entanglements;
    QuantumTimeline m_Timeline;

    // Achievements
    std::vector<Achievement> m_Achievements;
    std::vector<AchievementID> m_RecentUnlocks; // For UI notifications

    // Random Events
    std::vector<QuantumEvent> m_Events;
    QuantumEvent* m_CurrentEvent;
    f64 m_TimeSinceLastEvent;
    f64 m_EventCooldown;

    // Statistics
    GameStatistics m_Statistics;

    // Research Tree
    ResearchTree m_ResearchTree;

    // Milestone System
    MilestoneSystem m_MilestoneSystem;

    // Buyables System
    BuyableManager m_BuyableManager;

    // Challenge System
    ChallengeManager m_ChallengeManager;

    // Quantum Essence (permanent meta-currency)
    f64 m_QuantumEssence; // Never lost on prestige

    // Essence Shop System
    EssenceShopManager m_EssenceShopManager;

    // Singularity Shop System
    SingularityShopManager m_SingularityShopManager;

    // Offline progress
    i64 m_LastSaveTimestamp;

    // UI
    std::vector<UIButton> m_StationButtons; // Persistent buttons (unlock, observe, upgrade per station + prestige)
    Vec2 m_ScrollOffset;
    bool m_ShowAchievements;
    bool m_ShowStats;
    bool m_ShowResearch;
    bool m_ShowMilestones;
    bool m_ShowBuyables;
    bool m_ShowChallenges;
    bool m_ShowEssenceShop;
    bool m_ShowSingularityShop;
    GameUtils::NumberFormat m_NumberFormat; // Toggle between suffix (1.23M) and scientific (1.23e6)

    // Game time
    f64 m_TotalTimePlayed;
    f64 m_TimeSinceLastSave;

    // Coherence decay
    f64 m_Coherence;
    f64 m_MaxCoherence;
    f64 m_CoherenceDecayRate;

    // Boost system (temporary production multiplier)
    bool m_BoostActive;
    f64 m_BoostTimeRemaining;
    f64 m_BoostCooldownRemaining;
    f64 m_BoostDuration;          // How long boost lasts (30 seconds)
    f64 m_BoostCooldown;          // How long until can boost again (120 seconds)
    f64 m_BoostMultiplier;        // Production multiplier during boost (2.0x)

    // Auto-Prestige system (automatically prestige at photon threshold)
    f64 m_AutoPrestigeThreshold;  // Minimum photons before auto-prestige (default: 10)

    // Particle effects for visual feedback
    struct Particle {
        Vec2 position;
        Vec2 velocity;
        Color color;
        f32 lifetime;
        f32 maxLifetime;
    };
    std::vector<Particle> m_Particles;

    // Quantum Anomaly System (clickable orbs for active gameplay)
    struct QuantumAnomaly {
        Vec2 position;
        f32 radius;
        f32 lifetime;
        f32 maxLifetime;
        f64 rewardMultiplier;  // How much bonus (10-100x production)
        Color color;
        bool clicked;
    };
    std::vector<QuantumAnomaly> m_Anomalies;
    f64 m_TimeSinceLastAnomaly;
    f64 m_AnomalySpawnInterval;  // How often anomalies spawn (30-60 seconds)

    // Combo System (reward multiple clicks in sequence)
    i32 m_ComboCount;
    f64 m_ComboTimeRemaining;
    f64 m_ComboWindow;  // 5 seconds to maintain combo

    // Visual Effects State
    f64 m_PrestigeFlashTimer;  // For screen flash effect
    bool m_PrestigeFlashActive;

    // Helper methods for new systems
    void SpawnQuantumAnomaly();
    void UpdateQuantumAnomalies(f64 deltaTime);
    void RenderQuantumAnomalies(Renderer* renderer);
    void ClickQuantumAnomaly(const Vec2& clickPos);

    void AddComboPoint();
    void ResetCombo();
    f64 GetComboMultiplier() const;

    Color GetStationTierColor(i32 level) const;
    void SpawnResourceParticles(const Vec2& start, const Vec2& end, const Color& color, i32 count);
};
