#pragma once
#include "GameState.h"
#include "../src/game/ui/UITheme.h"

// Forward declarations
class Renderer;

class UIManager {
public:
    UIManager(GameState* gameState);
    ~UIManager() = default;

    static void Initialize();
    void Render(Renderer* renderer);

private:
    GameState* m_GameState;

    // --- Modular Render Functions ---
    void RenderTopBar();
    void RenderBottomNavigation();
    void RenderMainContent();

    // --- Sub-Panels ---
    void RenderStationsPanel();
    void RenderOverlays(Renderer* renderer); // Handles popups and notifications

    // --- Notification Overlays ---
    void RenderActiveEvent() const;
    void RenderAchievementNotifications() const;
    void RenderMilestoneNotifications() const;

    // --- Helpers ---
    void DrawResourceCounter(const char* label, double value, const ImVec4& color) const;
    static bool DrawNavButton(const char* label, bool isActive, const ImVec4& activeColor, float width, float height);
};