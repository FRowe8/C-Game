#pragma once

#include "Types.h"
#include "imgui.h"
#include <string>

// Forward declarations
class GameState;
class Renderer;

namespace UI {

/**
 * Tutorial Step Enumeration
 * Defines the sequence of tutorial steps that guide new players
 */
enum class TutorialStep {
    None,               // Tutorial completed or skipped
    Welcome,            // Initial welcome message
    ObserveButton,      // Step 1: Point to Observe button - "Click to collapse wave function"
    UpgradeButton,      // Step 2: Point to Upgrade button (triggers when Resources > 10)
    CoherenceWarning,   // Step 3: Point to Coherence Stabilizer (triggers when Coherence < 50%)
    Completed           // All steps done
};

/**
 * TutorialOverlay
 *
 * Implements a step-based tutorial system that guides new players through
 * the core mechanics of Quantum Idle:
 * - Observation (collapsing wave function)
 * - Upgrading stations
 * - Managing Coherence
 *
 * Features:
 * - Dimming mask that highlights specific UI elements
 * - Context-sensitive tooltips
 * - Automatic progression based on game state
 * - Persistent progress (saved via SaveManager)
 */
class TutorialOverlay {
public:
    TutorialOverlay();
    ~TutorialOverlay() = default;

    /**
     * Update tutorial logic and check for step progression
     * @param state GameState reference to read resources and check conditions
     */
    void Update(GameState* state);

    /**
     * Render the tutorial overlay (dimming mask + tooltips + arrows)
     * @param state GameState reference
     * @param renderer Renderer reference (may be unused for ImGui-only)
     */
    void Render(GameState* state, Renderer* renderer);

    /**
     * Check if tutorial is active
     */
    bool IsActive() const {
        return m_CurrentStep != TutorialStep::None &&
               m_CurrentStep != TutorialStep::Completed;
    }

    /**
     * Get current tutorial step
     */
    TutorialStep GetCurrentStep() const { return m_CurrentStep; }

    /**
     * Set current tutorial step (used when loading from save)
     */
    void SetCurrentStep(TutorialStep step) { m_CurrentStep = step; }

    /**
     * Skip tutorial entirely
     */
    void Skip();

    /**
     * Advance to next step (called when player completes current objective)
     */
    void AdvanceStep();

    /**
     * Check if a specific UI element should be interactable
     * Returns true if the element is the current tutorial target
     */
    bool ShouldAllowInteraction(const char* elementId) const;

private:
    TutorialStep m_CurrentStep;
    float m_StepStartTime; // Time when current step started (for animations)

    // Step-specific rendering
    void RenderWelcomeStep(GameState* state);
    void RenderObserveStep(GameState* state);
    void RenderUpgradeStep(GameState* state);
    void RenderCoherenceStep(GameState* state);
    void RenderCompletedStep(GameState* state);

    /**
     * Draw dimming mask over entire screen except for cutout area
     * @param cutoutMin Top-left corner of the highlighted area
     * @param cutoutMax Bottom-right corner of the highlighted area
     */
    void DrawDimmingMask(const ImVec2& cutoutMin, const ImVec2& cutoutMax);

    /**
     * Draw an animated arrow pointing to a UI element
     * @param targetPos Position to point the arrow at
     * @param text Tooltip text to show near the arrow
     */
    void DrawTooltipArrow(const ImVec2& targetPos, const char* text);

    /**
     * Get tooltip text for current step
     */
    const char* GetTooltipText() const;
};

} // namespace UI
