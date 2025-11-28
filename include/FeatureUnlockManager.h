#pragma once

#include "Types.h"
#include <vector>
#include <string>

// Forward declarations
class Renderer;

// Feature identifiers
enum class GameFeature {
    // Core (Always unlocked)
    Resources,
    Research,
    Stats,

    // Tier 1 (Level 5)
    Spaceship,
    Achievements,

    // Tier 2 (Level 10)
    Combat,
    Zones,
    Milestones,

    // Tier 3 (Level 15)
    Summon,
    Buyables,

    // Tier 4 (Level 20)
    SkillTree,

    // Tier 5 (Level 30)
    Enhancement,
    Challenges,

    // Tier 6 (Level 50+)
    EssenceShop,
    SingularityShop,

    COUNT
};

// Feature unlock data
struct FeatureUnlock {
    GameFeature feature;
    const char* name;
    const char* description;
    i32 unlockLevel;
    bool showInMainMenu;        // Show as main button vs in MORE menu
    bool isNew;                 // Just unlocked this session?
    bool hasSeenTutorial;       // Has player seen the tutorial for this?

    FeatureUnlock()
        : feature(GameFeature::Resources), name(""), description(""),
          unlockLevel(0), showInMainMenu(false), isNew(false), hasSeenTutorial(false) {}

    FeatureUnlock(GameFeature f, const char* n, const char* desc, i32 level, bool mainMenu)
        : feature(f), name(n), description(desc), unlockLevel(level),
          showInMainMenu(mainMenu), isNew(false), hasSeenTutorial(false) {}
};

// Notification for newly unlocked features
struct UnlockNotification {
    GameFeature feature;
    const char* title;
    const char* message;
    f64 displayTime;        // How long to show (seconds)
    f64 timeRemaining;

    UnlockNotification()
        : feature(GameFeature::Resources), title(""), message(""),
          displayTime(5.0), timeRemaining(5.0) {}

    UnlockNotification(GameFeature f, const char* t, const char* msg, f64 time = 5.0)
        : feature(f), title(t), message(msg), displayTime(time), timeRemaining(time) {}
};

// Manages feature unlocks based on player progression
class FeatureUnlockManager {
public:
    FeatureUnlockManager();

    void Initialize();
    void Update(f64 deltaTime);

    // Unlock checking
    bool IsUnlocked(GameFeature feature) const;
    bool IsUnlocked(GameFeature feature, i32 playerLevel) const;
    i32 GetUnlockLevel(GameFeature feature) const;
    const char* GetFeatureName(GameFeature feature) const;
    const char* GetFeatureDescription(GameFeature feature) const;

    // Player level updates
    void OnLevelUp(i32 newLevel);
    void CheckUnlocks(i32 currentLevel);

    // Tutorial tracking
    bool HasSeenTutorial(GameFeature feature) const;
    void MarkTutorialSeen(GameFeature feature);

    // Notifications
    bool HasActiveNotification() const { return !m_ActiveNotifications.empty(); }
    void RenderNotifications(Renderer* renderer);
    void DismissNotification(i32 index = 0); // Dismiss oldest (0) or specific index

    // UI helpers
    bool ShouldShowInMainMenu(GameFeature feature) const;
    bool IsNewlyUnlocked(GameFeature feature) const;
    void MarkFeatureAsViewed(GameFeature feature); // Clear "NEW!" badge

    // Main menu button ordering
    std::vector<GameFeature> GetMainMenuFeatures(i32 playerLevel) const;
    std::vector<GameFeature> GetMoreMenuFeatures(i32 playerLevel) const;

    // Serialization
    std::string SaveToJson() const;
    void LoadFromJson(const std::string& line);

private:
    std::vector<FeatureUnlock> m_Features;
    std::vector<UnlockNotification> m_ActiveNotifications;
    i32 m_LastCheckedLevel;

    void InitializeFeatures();
    void AddUnlockNotification(GameFeature feature);
    FeatureUnlock* GetFeatureData(GameFeature feature);
    const FeatureUnlock* GetFeatureData(GameFeature feature) const;
};
