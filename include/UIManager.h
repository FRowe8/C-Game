#pragma once
#include "GameState.h"
#include "../src/game/ui/UITheme.h"

// Forward declarations
class Renderer;

class UIManager {
public:
    UIManager(GameState* gameState);
    ~UIManager() = default;

    void Initialize();
    void Render(Renderer* renderer);

private:
    GameState* m_GameState;

    // --- Modular Render Functions ---
    void RenderTopBar();
    void RenderBottomNavigation();
    void RenderMainContent();

    // --- Sub-Panels ---
    void RenderStationsPanel();
    void RenderOverlays(Renderer* renderer); // Handles modal popups (Research, Achievements, etc)

    // --- Overlay/Modal Windows (moved from GameState) ---
    void RenderActiveEvent(Renderer* renderer);
    void RenderAchievements(Renderer* renderer);
    void RenderStatistics(Renderer* renderer);
    void RenderResearchTree(Renderer* renderer);
    void RenderMilestones(Renderer* renderer);
    void RenderBuyables(Renderer* renderer);
    void RenderChallenges(Renderer* renderer);
    void RenderEssenceShop(Renderer* renderer);
    void RenderSingularityShop(Renderer* renderer);
    void RenderSpaceship(Renderer* renderer);
    void RenderCombat(Renderer* renderer);
    void RenderGatcha(Renderer* renderer);
    void RenderSkillTree(Renderer* renderer);
    void RenderAchievementNotifications(Renderer* renderer);
    void RenderMilestoneNotifications(Renderer* renderer);

    // --- Helpers ---
    void DrawResourceCounter(const char* label, double value, const ImVec4& color);
    bool DrawNavButton(const char* label, bool isActive, const ImVec4& activeColor, float width, float height);
};