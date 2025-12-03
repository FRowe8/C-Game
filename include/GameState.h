#pragma once

#include "Types.h"
#include "../src/game/ui/UITheme.h"
#include "Milestones.h"
#include "Buyables.h"
#include "Challenges.h"
#include "EssenceShop.h"
#include "SingularityShop.h"
#include "GameUtils.h"
#include "Spaceship.h"
#include "CombatSystem.h"
#include "Enemy.h"
#include "GatchaSystem.h"
#include "SkillTree.h"
#include "EnhancementSystem.h"
#include "FeatureUnlockManager.h"
#include "SpecializedSkills.h"
#include "SoundManager.h"
#include "ParticleCollection.h"
#include "Telemetry.h"
#include "imgui.h"
#include <vector>
#include <string>
#include <functional>

// Forward declarations
class Renderer;
class Input;
class ResearchTree;
class UIManager; // Forward declare

// Forward declarations for GuiLayer
namespace UI {
    class GuiLayer;
    class NavigationView;
    class StationView;
    class ResourceView;
    class AchievementView;
    class StatisticsView;
    class ResearchView;
    class MilestoneView;
    class BuyablesView;
    class ChallengeView;
    class EssenceShopView;
    class SingularityShopView;
    class SpaceshipView;
    class CombatView;
    class GatchaView;
    class SkillTreeView;
    class EnhancementView;
    class SpecializedSkillsView;
    class TutorialOverlay;
    class CollectionView;
}

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

    // Phase 3.1: Passive collapse for early game smoothing
    f64 passiveCollapseRate;      // Percentage of superposition auto-collapsed per second (0.0 to 1.0)

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
    // Spaceship achievements
    FirstShipPart,        // Acquire your first ship part
    ShipOperational,      // Repair ship to 25% (travel unlocked)
    ShipFullyRepaired,    // Repair ship to 100%
    FirstLegendaryPart,   // Find a legendary ship part
    PartCollector,        // Collect 50 ship parts
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

// GameMode replaces scattered boolean flags for active screens
enum class GameMode {
    None,              // Stations view (base state)
    Achievements,
    Statistics,
    Research,
    Milestones,
    Buyables,
    Challenges,
    EssenceShop,
    SingularityShop,
    Spaceship,
    Combat,
    Gatcha,
    Skills,
    Enhancement,
    SpecializedSkills,  // Activity-based progression system
    Collection,         // Phase 4.1: Particle collection system
    MoreMenu
};

// Backwards compatibility alias
using ActiveModal = GameMode;

class GameState {
public:
    GameState();
    ~GameState();

    void Initialize();
    void Update(f64 deltaTime, Input* input, Renderer* renderer);
    void Render(Renderer* renderer);

    // Resource management
    void AddResource(QuantumResource type, f64 amount, bool showFloatingText = true);
    bool SpendResource(QuantumResource type, f64 amount);
    f64 GetResource(QuantumResource type) const;

    // Quantum Essence management (permanent meta-currency)
    void AddEssence(f64 amount);
    bool SpendEssence(f64 amount);
    f64 GetEssence() const { return m_QuantumEssence; }
    f64 GetProductionMultiplier(QuantumResource type) const;

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
    TelemetryManager& GetTelemetry() { return m_Telemetry; }
    const TelemetryManager& GetTelemetry() const { return m_Telemetry; }

    // Research Tree
    ResearchTree& GetResearchTree() { return *m_ResearchTree; }
    bool CanAffordResearch(ResearchID id) const;
    bool PurchaseResearch(ResearchID id);
    void UpdateResearchBonuses();

    // Milestone System
    MilestoneSystem& GetMilestoneSystem() { return m_MilestoneSystem; }
    void CheckMilestones();
    void AddPhotons(f64 amount);
    f64 GetPhotons() const;

    // Buyables System
    BuyableManager& GetBuyableManager() { return m_BuyableManager; }

    // Challenge System
    ChallengeManager& GetChallengeManager() { return m_ChallengeManager; }

    // Essence Shop System
    EssenceShopManager& GetEssenceShopManager() { return m_EssenceShopManager; }

    // Singularity Shop System
    SingularityShopManager& GetSingularityShopManager() { return m_SingularityShopManager; }
    QuantumTimeline& GetTimeline() { return m_Timeline; }

    // Spaceship System
    Spaceship& GetSpaceship() { return m_Spaceship; }

    // Combat System
    CombatSystem& GetCombatSystem() { return m_CombatSystem; }
    void StartRandomCombat();
    void EndCombat();
    i32 GetPlayerLevel() const { return m_PlayerLevel; }
    f64 GetPlayerXP() const { return m_PlayerXP; }
    f64 GetXPForNextLevel() const;
    void AddXP(f64 amount);

    // Gatcha/Summon System
    GatchaSystem& GetGatchaSystem() { return m_GatchaSystem; }

    // Skill Tree System
    SkillTreeSystem& GetSkillTree() { return m_SkillTree; }

    // Enhancement System
    EnhancementSystem& GetEnhancementSystem() { return m_EnhancementSystem; }

    // Phase 4.1: Particle Collection System
    ParticleCollection& GetParticleCollection() { return m_ParticleCollection; }

    // Specialized Skills System
    SpecializedSkillsSystem& GetSpecializedSkills() { return m_SpecializedSkills; }

    // Sound Manager
    SoundManager& GetSoundManager() { return m_SoundManager; }

    // Feature Unlock Manager
    FeatureUnlockManager& GetUnlockManager() { return m_UnlockManager; }
    const FeatureUnlockManager& GetUnlockManager() const { return m_UnlockManager; }

    // Combo System (public so ResearchStation::Observe can use it)
    void AddComboPoint();
    f64 GetComboMultiplier() const;

    i32 GetPlayerCredits();
    void AddPlayerCredits(i32 amount);

    void DeductPlayerCredits(i32 amount);

    // Phase 3.2: Credit Conversion System
    void ConvertCreditsToProduction(i32 credits);
    f64 GetCreditProductionMultiplier() const { return m_CreditProductionMultiplier; }
    f64 CalculateProductionBonusFromCredits(i32 credits) const;

    // UI feedback hooks
    void RegisterUIButtonFeedback(const std::string& label, const ImVec2& screenPos, const ImVec4& color);

    // Phase 3.3: Exotic Materials Management
    i32 GetExoticMaterials() const { return m_ExoticMaterials; }
    void AddExoticMaterials(i32 amount);
    bool SpendExoticMaterials(i32 amount);

    // Combat Research Data
    i32 GetResearchData() const { return m_ResearchData; }
    void AddResearchData(i32 amount);
    bool SpendResearchData(i32 amount);

    bool IsGatchaUIVisible() const {
        return m_ShowGatcha;
    }

    void SetGatchaUIVisible(bool visible);

    // NEW: Expose render logic content only (no window creation)
    void RenderStationsContent();

    // Modal Window Management
    void SetGameMode(GameMode mode);
    GameMode GetGameMode() const { return m_GameMode; }

    // Legacy compatibility for older callers
    void SetActiveModal(GameMode modal) { SetGameMode(modal); }
    GameMode GetActiveModal() const { return GetGameMode(); }

    // NEW: Friend the manager and GuiLayer so they can access private members
    friend class UIManager;
    friend class UI::GuiLayer;
    friend class UI::NavigationView;
    friend class UI::StationView;
    friend class UI::ResourceView;
    friend class UI::AchievementView;
    friend class UI::StatisticsView;
    friend class UI::ResearchView;
    friend class UI::MilestoneView;
    friend class UI::BuyablesView;
    friend class UI::ChallengeView;
    friend class UI::EssenceShopView;
    friend class UI::SingularityShopView;
    friend class UI::SpaceshipView;
    friend class UI::CombatView;
    friend class UI::GatchaView;
    friend class UI::SkillTreeView;
    friend class UI::EnhancementView;
    friend class UI::SpecializedSkillsView;
    friend class UI::TutorialOverlay;

private:
    void InitializeStations();
    void InitializeUI();
    void UpdateStations(f64 deltaTime);
    void UpdateCoherence(f64 deltaTime);
    void UpdateDynamicMusic();  // Phase 4.2: Switch music based on active modal
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
    void RenderSpaceship(Renderer* renderer);
    void RenderCombat(Renderer* renderer);
    void RenderGatcha(Renderer* renderer);
    void RenderSkillTree(Renderer* renderer);
    void RenderSpecializedSkills(Renderer* renderer);

    // Particle system helpers
    void SpawnParticle(const Vec2& position, const Color& color, f64 lifetime = 1.0);
    void SpawnParticleBurst(const Vec2& position, const Color& color, i32 count = 10);
    void UpdateParticles(f64 deltaTime);

    // Resources
    f64 m_Resources[3]; // Qubits, Coherence, Entanglement

    i32 m_PlayerCredits; // Currency for Gatcha/Summon system (or similar)

    // Phase 3.2: Combat Integration - Credit Conversion
    f64 m_CreditProductionMultiplier; // Production bonus from converted credits (1.0 = no bonus)
    f64 m_CreditConversionRate;       // Credits per 1% production bonus (default: 100 credits = 1%)
    f64 m_MatterConverterBuffer;      // Fractional credit buffer for continuous conversion

    // Phase 3.3: Mid-Game Gatekeeping - Exotic Materials
    i32 m_ExoticMaterials;            // Special materials required for Tier 3+ research (from Spaceship/Combat)
    i32 m_ResearchData;               // Combat-sourced data used for advanced research

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
    TelemetryManager m_Telemetry;

    // Research Tree
    std::unique_ptr<ResearchTree> m_ResearchTree;

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

    // Spaceship System
    Spaceship m_Spaceship;

    // Combat System
    CombatSystem m_CombatSystem;
    Enemy m_CurrentEnemy;
    i32 m_PlayerLevel;
    f64 m_PlayerXP;

    // Gatcha System
    GatchaSystem m_GatchaSystem;

    // Skill Tree System
    SkillTreeSystem m_SkillTree;

    // Enhancement System
    EnhancementSystem m_EnhancementSystem;

    // Feature Unlock Manager
    FeatureUnlockManager m_UnlockManager;

    // Phase 4.1: Particle Collection System
    ParticleCollection m_ParticleCollection;

    // Specialized Skills System
    SpecializedSkillsSystem m_SpecializedSkills;

    // Sound Manager
    SoundManager m_SoundManager;

    // Offline progress
    i64 m_LastSaveTimestamp;

    // UI
    Vec2 m_ScrollOffset;

    // Modal Window Management System
    GameMode m_GameMode;
    bool m_ShowMoreMenu; // Overflow menu for less frequent pages
    bool m_ShowAchievements;
    bool m_ShowStats;
    bool m_ShowResearch;
    bool m_ShowMilestones;
    bool m_ShowBuyables;
    bool m_ShowChallenges;
    bool m_ShowEssenceShop;
    bool m_ShowSingularityShop;
    bool m_ShowSpaceship;
    bool m_ShowCombat;
    bool m_ShowGatcha;
    bool m_ShowSkills;
    bool m_ShowEnhancement;
    GameUtils::NumberFormat m_NumberFormat; // Toggle between suffix (1.23M) and scientific (1.23e6)

    // Game time
    f64 m_TotalTimePlayed;
    f64 m_TimeSinceLastSave;
    f64 m_TimeSinceLastPrestige; // Track time for fastest prestige achievement

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
    std::vector<Particle> m_ParticlePool;
    size_t m_MaxActiveParticles = 500;

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

    // Theme System (Phase D)
    i32 m_LastThemeUnlocked; // Track which theme was last unlocked (0=none, 1=facility, 2=observatory, 3=void)

    // Helper methods for new systems (private)
    void SpawnQuantumAnomaly();
    void UpdateQuantumAnomalies(f64 deltaTime);
    void RenderQuantumAnomalies(Renderer* renderer);
    void ClickQuantumAnomaly(const Vec2& clickPos);

    void ResetCombo();

    Color GetStationTierColor(i32 level) const;
    void SpawnResourceParticles(const Vec2& start, const Vec2& end, const Color& color, i32 count);

    std::unique_ptr<UIManager> m_UIManager;
    std::unique_ptr<UI::GuiLayer> m_GuiLayer; // Phase 1.1: Decoupled UI layer
};




