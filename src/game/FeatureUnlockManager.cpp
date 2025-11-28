#include "FeatureUnlockManager.h"
#include "Renderer.h"
#include "Logger.h"
#include <algorithm>

FeatureUnlockManager::FeatureUnlockManager()
    : m_LastCheckedLevel(0) {
}

void FeatureUnlockManager::Initialize() {
    Log::Info("Initializing feature unlock manager...");
    InitializeFeatures();
    m_LastCheckedLevel = 0;
    Log::Info("Feature unlock manager initialized");
}

void FeatureUnlockManager::InitializeFeatures() {
    m_Features.clear();

    // Core features (always unlocked)
    m_Features.push_back(FeatureUnlock(GameFeature::Resources, "Resources", "Generate Qubits, Coherence, and Entanglement", 0, true));
    m_Features.push_back(FeatureUnlock(GameFeature::Research, "Research", "Unlock upgrades and new technologies", 0, true));
    m_Features.push_back(FeatureUnlock(GameFeature::Stats, "Stats", "View your progress and statistics", 0, false));

    // Tier 1: Level 5
    m_Features.push_back(FeatureUnlock(GameFeature::Spaceship, "Spaceship", "Customize your vessel with parts", 5, false));
    m_Features.push_back(FeatureUnlock(GameFeature::Achievements, "Achievements", "Track your accomplishments", 5, false));

    // Tier 2: Level 10
    m_Features.push_back(FeatureUnlock(GameFeature::Combat, "Combat", "Battle hostile forces in turn-based combat", 10, true));
    m_Features.push_back(FeatureUnlock(GameFeature::Zones, "Zones", "Explore different galactic regions", 10, false));
    m_Features.push_back(FeatureUnlock(GameFeature::Milestones, "Milestones", "Complete major objectives", 10, false));

    // Tier 3: Level 15
    m_Features.push_back(FeatureUnlock(GameFeature::Summon, "Summon", "Acquire rare ship parts from the Parts Exchange", 15, true));
    m_Features.push_back(FeatureUnlock(GameFeature::Buyables, "Buyables", "Purchase permanent upgrades", 15, false));

    // Tier 4: Level 20
    m_Features.push_back(FeatureUnlock(GameFeature::SkillTree, "Skills", "Train your crew with permanent skills", 20, true));

    // Tier 5: Level 30
    m_Features.push_back(FeatureUnlock(GameFeature::Enhancement, "Enhancement", "Upgrade parts beyond their limits", 30, true));
    m_Features.push_back(FeatureUnlock(GameFeature::Challenges, "Challenges", "Test your skills in special modes", 30, false));

    // Tier 6: Level 50+
    m_Features.push_back(FeatureUnlock(GameFeature::EssenceShop, "Essence Shop", "Spend Essence for powerful bonuses", 50, false));
    m_Features.push_back(FeatureUnlock(GameFeature::SingularityShop, "Singularity Shop", "Ultimate prestige upgrades", 60, false));
}

bool FeatureUnlockManager::IsUnlocked(GameFeature feature) const {
    const FeatureUnlock* data = GetFeatureData(feature);
    if (!data) return false;
    return m_LastCheckedLevel >= data->unlockLevel;
}

bool FeatureUnlockManager::IsUnlocked(GameFeature feature, i32 playerLevel) const {
    const FeatureUnlock* data = GetFeatureData(feature);
    if (!data) return false;
    return playerLevel >= data->unlockLevel;
}

i32 FeatureUnlockManager::GetUnlockLevel(GameFeature feature) const {
    const FeatureUnlock* data = GetFeatureData(feature);
    return data ? data->unlockLevel : 999;
}

const char* FeatureUnlockManager::GetFeatureName(GameFeature feature) const {
    const FeatureUnlock* data = GetFeatureData(feature);
    return data ? data->name : "Unknown";
}

const char* FeatureUnlockManager::GetFeatureDescription(GameFeature feature) const {
    const FeatureUnlock* data = GetFeatureData(feature);
    return data ? data->description : "";
}

void FeatureUnlockManager::OnLevelUp(i32 newLevel) {
    i32 oldLevel = m_LastCheckedLevel;
    m_LastCheckedLevel = newLevel;

    // Check for newly unlocked features
    for (auto& feature : m_Features) {
        if (feature.unlockLevel > oldLevel && feature.unlockLevel <= newLevel) {
            // Feature just unlocked!
            feature.isNew = true;
            feature.hasSeenTutorial = false;
            AddUnlockNotification(feature.feature);
            Log::Infof("Feature unlocked: ", feature.name, " at level ", newLevel);
        }
    }
}

void FeatureUnlockManager::CheckUnlocks(i32 currentLevel) {
    if (currentLevel != m_LastCheckedLevel) {
        OnLevelUp(currentLevel);
    }
}

bool FeatureUnlockManager::HasSeenTutorial(GameFeature feature) const {
    const FeatureUnlock* data = GetFeatureData(feature);
    return data ? data->hasSeenTutorial : true;
}

void FeatureUnlockManager::MarkTutorialSeen(GameFeature feature) {
    FeatureUnlock* data = GetFeatureData(feature);
    if (data) {
        data->hasSeenTutorial = true;
    }
}

void FeatureUnlockManager::AddUnlockNotification(GameFeature feature) {
    const char* title = "NEW FEATURE UNLOCKED!";
    const char* message = "";

    switch (feature) {
        case GameFeature::Spaceship:
            message = "Access your ship customization interface";
            break;
        case GameFeature::Achievements:
            message = "Track your accomplishments and progress";
            break;
        case GameFeature::Combat:
            title = "COMBAT SYSTEMS ONLINE!";
            message = "Engage hostile forces in the Asteroid Belt";
            break;
        case GameFeature::Summon:
            title = "PARTS EXCHANGE AVAILABLE!";
            message = "Summon rare ship components - First 10-pull FREE!";
            break;
        case GameFeature::SkillTree:
            title = "SKILL TRAINING UNLOCKED!";
            message = "Train your crew with permanent skills - 20 points awarded!";
            break;
        case GameFeature::Enhancement:
            title = "ENGINEERING BAY OPERATIONAL!";
            message = "Enhance ship parts beyond their limits";
            break;
        default:
            message = GetFeatureDescription(feature);
            break;
    }

    m_ActiveNotifications.push_back(UnlockNotification(feature, title, message, 7.0));
}

void FeatureUnlockManager::Update(f64 deltaTime) {
    // Update notification timers
    for (auto& notif : m_ActiveNotifications) {
        notif.timeRemaining -= deltaTime;
    }

    // Remove expired notifications
    m_ActiveNotifications.erase(
        std::remove_if(m_ActiveNotifications.begin(), m_ActiveNotifications.end(),
            [](const UnlockNotification& n) { return n.timeRemaining <= 0.0; }),
        m_ActiveNotifications.end()
    );
}

void FeatureUnlockManager::RenderNotifications(Renderer* renderer) {
    if (m_ActiveNotifications.empty()) return;

    f32 screenWidth = static_cast<f32>(renderer->GetWidth());
    f32 notifWidth = 400.0f;
    f32 notifHeight = 100.0f;
    f32 notifX = (screenWidth - notifWidth) * 0.5f;
    f32 notifY = 100.0f;
    f32 notifSpacing = 10.0f;

    for (size_t i = 0; i < m_ActiveNotifications.size(); i++) {
        const UnlockNotification& notif = m_ActiveNotifications[i];

        f32 currentY = notifY + i * (notifHeight + notifSpacing);

        // Background with pulsing effect
        f32 pulseAlpha = 0.9f + 0.1f * sinf(static_cast<f32>(notif.displayTime - notif.timeRemaining) * 3.0f);
        Rect bgRect(notifX, currentY, notifWidth, notifHeight);
        renderer->DrawRect(bgRect, Color(0.1f, 0.05f, 0.2f, pulseAlpha), true);

        // Glowing border
        Color borderColor(1.0f, 0.8f, 0.3f, pulseAlpha);
        renderer->DrawRect(bgRect, borderColor, false);
        renderer->DrawRect(Rect(notifX + 2, currentY + 2, notifWidth - 4, notifHeight - 4), borderColor * 0.7f, false);

        // Title
        renderer->DrawText(notif.title, Vec2(notifX + 20.0f, currentY + 15.0f),
                         Color(1.0f, 0.9f, 0.3f, 1.0f), 18.0f);

        // Message
        renderer->DrawText(notif.message, Vec2(notifX + 20.0f, currentY + 45.0f),
                         Color(0.9f, 0.9f, 0.9f, 1.0f), 14.0f);

        // Progress bar (time remaining)
        f32 progress = notif.timeRemaining / notif.displayTime;
        f32 barWidth = notifWidth - 40.0f;
        Rect progressBg(notifX + 20.0f, currentY + notifHeight - 20.0f, barWidth, 8.0f);
        Rect progressFill(notifX + 20.0f, currentY + notifHeight - 20.0f, barWidth * progress, 8.0f);
        renderer->DrawRect(progressBg, Color(0.2f, 0.2f, 0.2f, 0.8f), true);
        renderer->DrawRect(progressFill, Color(1.0f, 0.8f, 0.3f, 0.8f), true);

        // Click to dismiss hint
        renderer->DrawText("Click to dismiss", Vec2(notifX + notifWidth - 120.0f, currentY + 12.0f),
                         Color(0.6f, 0.6f, 0.6f, 1.0f), 10.0f);
    }
}

void FeatureUnlockManager::DismissNotification(i32 index) {
    if (index >= 0 && index < static_cast<i32>(m_ActiveNotifications.size())) {
        m_ActiveNotifications.erase(m_ActiveNotifications.begin() + index);
    }
}

bool FeatureUnlockManager::ShouldShowInMainMenu(GameFeature feature) const {
    const FeatureUnlock* data = GetFeatureData(feature);
    return data ? data->showInMainMenu : false;
}

bool FeatureUnlockManager::IsNewlyUnlocked(GameFeature feature) const {
    const FeatureUnlock* data = GetFeatureData(feature);
    return data ? data->isNew : false;
}

void FeatureUnlockManager::MarkFeatureAsViewed(GameFeature feature) {
    FeatureUnlock* data = GetFeatureData(feature);
    if (data) {
        data->isNew = false;
    }
}

std::vector<GameFeature> FeatureUnlockManager::GetMainMenuFeatures(i32 playerLevel) const {
    std::vector<GameFeature> features;

    for (const auto& feature : m_Features) {
        if (feature.showInMainMenu && playerLevel >= feature.unlockLevel) {
            features.push_back(feature.feature);
        }
    }

    return features;
}

std::vector<GameFeature> FeatureUnlockManager::GetMoreMenuFeatures(i32 playerLevel) const {
    std::vector<GameFeature> features;

    for (const auto& feature : m_Features) {
        if (!feature.showInMainMenu && playerLevel >= feature.unlockLevel) {
            features.push_back(feature.feature);
        }
    }

    return features;
}

FeatureUnlock* FeatureUnlockManager::GetFeatureData(GameFeature feature) {
    for (auto& f : m_Features) {
        if (f.feature == feature) {
            return &f;
        }
    }
    return nullptr;
}

const FeatureUnlock* FeatureUnlockManager::GetFeatureData(GameFeature feature) const {
    for (const auto& f : m_Features) {
        if (f.feature == feature) {
            return &f;
        }
    }
    return nullptr;
}

std::string FeatureUnlockManager::SaveToJson() const {
    // TODO: Implement save
    return "{}";
}

void FeatureUnlockManager::LoadFromJson(const std::string& line) {
    (void)line;
    // TODO: Implement load
}
