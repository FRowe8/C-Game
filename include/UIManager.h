#pragma once
#include "GameState.h"
#include "UITheme.h"

class UIManager {
public:
    UIManager(GameState* gameState);
    ~UIManager() = default;

    void Initialize();
    void Render();

private:
    GameState* m_GameState;

    // --- Modular Render Functions ---
    void RenderTopBar();
    void RenderBottomNavigation();
    void RenderMainContent();
    
    // --- Sub-Panels ---
    void RenderStationsPanel();
    void RenderOverlays(); // Handles modal popups (Research, Achievements, etc)
    
    // --- Helpers ---
    void DrawResourceCounter(const char* label, double value, const ImVec4& color);
    bool DrawNavButton(const char* label, bool isActive, const ImVec4& activeColor);
};