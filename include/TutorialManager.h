#pragma once

#include "Types.h"
#include "imgui.h" // Added to define ImVec2 and ImU32
#include <string>
#include <vector>
#include <functional>

// Forward declarations
class Renderer;

/**
 * TutorialManager - Interactive tutorial overlay system
 *
 * Highlights UI elements and guides new players through game mechanics.
 * Tutorials are shown once and can be dismissed or replayed.
 */

enum class TutorialStep {
    Welcome,
    ObserveStation,
    UnlockStation,
    UpgradeStation,
    OpenResearch,
    PurchaseResearch,
    NavigateToUpgrades,
    BuyUpgrade,
    NavigateToCombat,
    FirstCombat,
    OpenShip,
    InstallPart,
    OpenSkills,
    LearnSkill,
    UseMoreMenu,
    Completed,
    COUNT
};

struct TutorialHighlight {
    std::string targetElement;      // UI element ID/name to highlight
    ImVec2 position;                // Screen position (auto-calculated if element found)
    ImVec2 size;                    // Highlight size
    std::string title;              // Tooltip title
    std::string description;        // Tooltip text
    std::string actionHint;         // What the player should do (e.g., "Click here")
    bool highlightPulse;            // Whether to pulse the highlight
    bool blockOtherInput;           // Block input to other UI elements
    float arrowAngle;               // Arrow direction in radians (0 = right, PI/2 = down)

    TutorialHighlight()
        : position(ImVec2(0, 0))
        , size(ImVec2(100, 100))
        , highlightPulse(true)
        , blockOtherInput(false)
        , arrowAngle(0.0f)
    {}
};

class TutorialManager {
public:
    TutorialManager();
    ~TutorialManager() = default;

    // Initialization
    void Initialize();

    // Update (handles animations)
    void Update(f64 deltaTime);

    // Render tutorial overlays
    void Render(Renderer* renderer);

    // Control
    void StartTutorial();
    void NextStep();
    void PreviousStep();
    void SkipTutorial();
    void ResetTutorial();

    // State queries
    bool IsTutorialActive() const { return m_Active; }
    bool IsTutorialCompleted() const { return m_CompletedSteps[static_cast<i32>(TutorialStep::Completed)]; }
    TutorialStep GetCurrentStep() const { return m_CurrentStep; }

    // Step completion tracking
    void MarkStepCompleted(TutorialStep step);
    bool IsStepCompleted(TutorialStep step) const;

    // Enable/disable
    void Enable() { m_Enabled = true; }
    void Disable() { m_Enabled = false; }
    bool IsEnabled() const { return m_Enabled; }

    // Auto-progress (when player performs the action)
    void TriggerEvent(const std::string& eventName);

    // Save/Load
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    bool m_Enabled;
    bool m_Active;
    TutorialStep m_CurrentStep;
    bool m_CompletedSteps[static_cast<i32>(TutorialStep::COUNT)];

    // Animation state
    f32 m_PulseTimer;
    f32 m_FadeAlpha;

    // Current highlight
    TutorialHighlight m_CurrentHighlight;

    // Helper methods
    void SetupStep(TutorialStep step);
    void RenderHighlight(Renderer* renderer);
    void RenderTooltip(Renderer* renderer);
    void RenderControls(Renderer* renderer);

    // Step definitions
    TutorialHighlight GetHighlightForStep(TutorialStep step) const;

    // Animation helpers
    f32 GetPulseScale() const;
    ImU32 GetHighlightColor() const;
};