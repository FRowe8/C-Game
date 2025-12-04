#include "TutorialOverlay.h"
#include "GameState.h"
#include "Renderer.h"
#include "imgui.h"
#include "UITheme.h"
#include <cmath>

namespace UI {

TutorialOverlay::TutorialOverlay()
    : m_CurrentStep(TutorialStep::Welcome)
    , m_StepStartTime(0.0f)
    , m_TutorialCompleted(false)
{
}

void TutorialOverlay::Update(GameState* state) {
    if (m_TutorialCompleted) {
        return;
    }

    if (!IsActive()) {
        return;
    }

    // Auto-advance based on game state conditions
    switch (m_CurrentStep) {
        case TutorialStep::Welcome:
            // Welcome stays until player clicks "Continue"
            break;

        case TutorialStep::ObserveButton:
            // Advance to upgrade step when player has earned some resources
            if (state->GetResource(QuantumResource::Qubits) > 10.0) {
                AdvanceStep();
            }
            break;

        case TutorialStep::UpgradeButton:
            // Advance to coherence warning when coherence drops below 50%
            if (state->GetResource(QuantumResource::Coherence) < 50.0) {
                AdvanceStep();
            }
            break;

        case TutorialStep::CoherenceWarning:
            // Advance to completed when player has stabilized coherence
            if (state->GetResource(QuantumResource::Coherence) > 60.0) {
                AdvanceStep();
            }
            break;

        case TutorialStep::Completed:
            // Tutorial finished
            m_CurrentStep = TutorialStep::None;
            m_TutorialCompleted = true;
            break;

        default:
            break;
    }
}

void TutorialOverlay::Render(GameState* state, Renderer* renderer) {
    if (!IsActive()) {
        return;
    }

    // Allow keyboard-only users to close the tutorial quickly
    if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
        Skip();
        return;
    }

    // Render step-specific content
    switch (m_CurrentStep) {
        case TutorialStep::Welcome:
            RenderWelcomeStep(state);
            break;

        case TutorialStep::ObserveButton:
            RenderObserveStep(state);
            break;

        case TutorialStep::UpgradeButton:
            RenderUpgradeStep(state);
            break;

        case TutorialStep::CoherenceWarning:
            RenderCoherenceStep(state);
            break;

        case TutorialStep::Completed:
            RenderCompletedStep(state);
            break;

        default:
            break;
    }
}

void TutorialOverlay::Skip() {
    m_CurrentStep = TutorialStep::None;
    m_TutorialCompleted = true;
}

void TutorialOverlay::AdvanceStep() {
    switch (m_CurrentStep) {
        case TutorialStep::Welcome:
            m_CurrentStep = TutorialStep::ObserveButton;
            break;
        case TutorialStep::ObserveButton:
            m_CurrentStep = TutorialStep::UpgradeButton;
            break;
        case TutorialStep::UpgradeButton:
            m_CurrentStep = TutorialStep::CoherenceWarning;
            break;
        case TutorialStep::CoherenceWarning:
            m_CurrentStep = TutorialStep::Completed;
            break;
        case TutorialStep::Completed:
            m_CurrentStep = TutorialStep::None;
            m_TutorialCompleted = true;
            break;
        default:
            break;
    }

    m_StepStartTime = ImGui::GetTime();
}

bool TutorialOverlay::ShouldAllowInteraction(const char* elementId) const {
    if (!IsActive()) {
        return true; // Tutorial not active, allow all interactions
    }

    // Only allow interaction with the current tutorial target
    switch (m_CurrentStep) {
        case TutorialStep::ObserveButton:
            return std::string(elementId).find("Observe") != std::string::npos;

        case TutorialStep::UpgradeButton:
            return std::string(elementId).find("Upgrade") != std::string::npos;

        case TutorialStep::CoherenceWarning:
            return std::string(elementId).find("Coherence") != std::string::npos;

        default:
            return true;
    }
}

// ============================================================================
// Step-Specific Rendering
// ============================================================================

void TutorialOverlay::RenderWelcomeStep(GameState* state) {
    (void)state;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    // Responsive window sizing - scales with viewport while respecting a max size
    float windowWidth = std::min(screenSize.x * 0.92f, 520.0f);
    float windowHeight = std::min(screenSize.y * 0.78f, 420.0f);
    ImVec2 windowSize(windowWidth, windowHeight);
    ImVec2 windowPos((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);

    // Full screen dimming (clicking the backdrop closes the tutorial)
    bool backdropClicked = RenderBackdrop(ImVec4(0, 0, 0, 0.7f), "##TutorialDim", true);
    if (backdropClicked) {
        Skip();
        return;
    }

    // Welcome dialog
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorAccent);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

    ImGui::SetNextWindowFocus();
    ImGui::Begin("Welcome to Quantum Idle", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    // Skip/close control placed at the top for visibility and keyboard focus
    ImVec2 closeButtonSize(std::min(windowSize.x * 0.4f, 130.0f), 34.0f);
    ImGui::SetCursorPos(ImVec2(windowSize.x - closeButtonSize.x - 10.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    if (ImGui::Button("Skip tutorial", closeButtonSize)) {
        Skip();
        ImGui::PopStyleColor(3);
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        return;
    }
    ImGui::PopStyleColor(3);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);

    ImGui::TextWrapped("Welcome to the Quantum Realm!");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped(
        "In this incremental game, you'll harness quantum mechanics "
        "to generate resources and expand your empire."
    );
    ImGui::Spacing();
    ImGui::TextWrapped("Core mechanics:");
    ImGui::Spacing();
    ImGui::BulletText("Stations build resources in superposition");
    ImGui::BulletText("Tap OBSERVE to collect resources");
    ImGui::BulletText("Manage coherence for stability");

    ImGui::Spacing();
    ImGui::Spacing();

    // Mobile-friendly buttons - minimum 48px height for touch targets
    float buttonHeight = 48.0f;
    float buttonWidth = (windowSize.x - 60.0f) * 0.48f; // 48% of available width each
    float spacing = (windowSize.x - 60.0f) * 0.04f;
    float startX = 10.0f;

    ImGui::SetCursorPosX(startX);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.8f, 0.5f, 1.0f));
    if (ImGui::Button("START TUTORIAL", ImVec2(buttonWidth, buttonHeight))) {
        AdvanceStep();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0, spacing);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    if (ImGui::Button("SKIP", ImVec2(buttonWidth, buttonHeight))) {
        Skip();
    }
    ImGui::PopStyleColor(3);

    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void TutorialOverlay::RenderObserveStep(GameState* state) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    // Semi-transparent dimming except for station area
    bool backdropClicked = DrawDimmingMask(ImVec2(0, 100), ImVec2(screenSize.x, screenSize.y - 100), true);
    if (backdropClicked) {
        Skip();
        return;
    }

    // Responsive tooltip window
    float tooltipWidth = std::min(screenSize.x * 0.9f, 420.0f);
    float tooltipHeight = std::min(screenSize.y * 0.4f, 280.0f);
    ImVec2 tooltipSize(tooltipWidth, tooltipHeight);
    ImVec2 tooltipPos(screenSize.x * 0.5f - tooltipSize.x * 0.5f, 20.0f);

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorAccent);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    ImGui::SetNextWindowFocus();
    ImGui::Begin("##ObserveTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImVec2 closeButtonSize(std::min(tooltipSize.x * 0.45f, 140.0f), 32.0f);
    ImGui::SetCursorPos(ImVec2(tooltipSize.x - closeButtonSize.x - 6.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
    if (ImGui::Button("Skip tutorial", closeButtonSize)) {
        Skip();
        ImGui::PopStyleColor(3);
        ImGui::PopTextWrapPos();
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        return;
    }
    ImGui::PopStyleColor(3);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

    ImGui::PushTextWrapPos(tooltipSize.x - 30);

    ImGui::TextColored(UITheme::ColorAccent, "Step 1: Observe");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped(
        "Your research station is building resources in SUPERPOSITION (the gold bar)."
    );
    ImGui::Spacing();
    ImGui::TextWrapped("Tap the OBSERVE button below to collect qubits!");
    ImGui::Spacing();

    // Show current progress
    ImGui::TextColored(UITheme::ColorWarning, "Qubits: %.0f / 10 (auto-advances at 10)",
                      state->GetResource(QuantumResource::Qubits));

    ImGui::Spacing();
    ImGui::Spacing();

    // Mobile-friendly buttons
    float buttonHeight = 48.0f;
    float buttonWidth = (tooltipSize.x - 50.0f) * 0.48f;
    float spacing = (tooltipSize.x - 50.0f) * 0.04f;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.7f, 0.9f, 1.0f));
    ImGui::SetItemDefaultFocus();
    if (ImGui::Button("NEXT", ImVec2(buttonWidth, buttonHeight))) {
        AdvanceStep();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0, spacing);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    if (ImGui::Button("SKIP ALL", ImVec2(buttonWidth, buttonHeight))) {
        Skip();
    }
    ImGui::PopStyleColor(3);

    ImGui::PopTextWrapPos();
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void TutorialOverlay::RenderUpgradeStep(GameState* state) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    // Semi-transparent dimming
    bool backdropClicked = DrawDimmingMask(ImVec2(0, 100), ImVec2(screenSize.x, screenSize.y - 100), true);
    if (backdropClicked) {
        Skip();
        return;
    }

    // Responsive tooltip window
    float tooltipWidth = std::min(screenSize.x * 0.9f, 420.0f);
    float tooltipHeight = std::min(screenSize.y * 0.45f, 300.0f);
    ImVec2 tooltipSize(tooltipWidth, tooltipHeight);
    ImVec2 tooltipPos(screenSize.x * 0.5f - tooltipSize.x * 0.5f, 20.0f);

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorSuccess);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    ImGui::SetNextWindowFocus();
    ImGui::Begin("##UpgradeTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImVec2 closeButtonSize(std::min(tooltipSize.x * 0.45f, 140.0f), 32.0f);
    ImGui::SetCursorPos(ImVec2(tooltipSize.x - closeButtonSize.x - 6.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
    if (ImGui::Button("Skip tutorial", closeButtonSize)) {
        Skip();
        ImGui::PopStyleColor(3);
        ImGui::PopTextWrapPos();
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        return;
    }
    ImGui::PopStyleColor(3);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

    ImGui::PushTextWrapPos(tooltipSize.x - 30);

    ImGui::TextColored(UITheme::ColorSuccess, "Step 2: Upgrade");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Great! You've collected qubits.");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Now tap UPGRADE to increase production speed!"
    );
    ImGui::Spacing();

    ImGui::Text("Current Qubits: %.0f", state->GetResource(QuantumResource::Qubits));
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(Auto-advances when coherence < 50%%)");

    ImGui::Spacing();
    ImGui::Spacing();

    // Mobile-friendly buttons
    float buttonHeight = 48.0f;
    float buttonWidth = (tooltipSize.x - 50.0f) * 0.48f;
    float spacing = (tooltipSize.x - 50.0f) * 0.04f;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 0.8f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.7f, 0.9f, 1.0f));
    ImGui::SetItemDefaultFocus();
    if (ImGui::Button("NEXT", ImVec2(buttonWidth, buttonHeight))) {
        AdvanceStep();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0, spacing);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    if (ImGui::Button("SKIP ALL", ImVec2(buttonWidth, buttonHeight))) {
        Skip();
    }
    ImGui::PopStyleColor(3);

    ImGui::PopTextWrapPos();
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void TutorialOverlay::RenderCoherenceStep(GameState* state) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    // Dim entire screen except top bar (where resources are)
    bool backdropClicked = DrawDimmingMask(ImVec2(0, 0), ImVec2(screenSize.x, 80), true);
    if (backdropClicked) {
        Skip();
        return;
    }

    // Responsive tooltip window
    float tooltipWidth = std::min(screenSize.x * 0.9f, 450.0f);
    float tooltipHeight = std::min(screenSize.y * 0.55f, 340.0f);
    ImVec2 tooltipSize(tooltipWidth, tooltipHeight);
    ImVec2 tooltipPos(screenSize.x * 0.5f - tooltipSize.x * 0.5f, 90);

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorDanger);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    ImGui::SetNextWindowFocus();
    ImGui::Begin("##CoherenceTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImVec2 closeButtonSize(std::min(tooltipSize.x * 0.45f, 140.0f), 32.0f);
    ImGui::SetCursorPos(ImVec2(tooltipSize.x - closeButtonSize.x - 6.0f, 2.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
    if (ImGui::Button("Skip tutorial", closeButtonSize)) {
        Skip();
        ImGui::PopStyleColor(3);
        ImGui::PopTextWrapPos();
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        return;
    }
    ImGui::PopStyleColor(3);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4.0f);

    ImGui::PushTextWrapPos(tooltipSize.x - 30);

    ImGui::TextColored(UITheme::ColorDanger, "Step 3: Manage Coherence");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Coherence = quantum stability.");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Low coherence reduces observation success. Watch the cyan bar at the top!"
    );
    ImGui::Spacing();

    ImGui::Text("Current Coherence: %.1f / 100.0", state->GetResource(QuantumResource::Coherence));

    ImGui::Spacing();
    ImGui::Spacing();

    // Mobile-friendly buttons
    float buttonHeight = 48.0f;
    float buttonWidth = (tooltipSize.x - 50.0f) * 0.48f;
    float spacing = (tooltipSize.x - 50.0f) * 0.04f;

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.8f, 0.5f, 1.0f));
    ImGui::SetItemDefaultFocus();
    if (ImGui::Button("GOT IT!", ImVec2(buttonWidth, buttonHeight))) {
        AdvanceStep();
    }
    ImGui::PopStyleColor(3);

    ImGui::SameLine(0, spacing);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    if (ImGui::Button("SKIP ALL", ImVec2(buttonWidth, buttonHeight))) {
        Skip();
    }
    ImGui::PopStyleColor(3);

    ImGui::PopTextWrapPos();
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

void TutorialOverlay::RenderCompletedStep(GameState* state) {
    (void)state;
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    // Responsive window sizing
    float windowWidth = std::min(screenSize.x * 0.92f, 470.0f);
    float windowHeight = std::min(screenSize.y * 0.65f, 340.0f);
    ImVec2 windowSize(windowWidth, windowHeight);
    ImVec2 windowPos((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);

    // Full screen dimming
    bool backdropClicked = RenderBackdrop(ImVec4(0, 0, 0, 0.7f), "##TutorialCompleteDim", true);
    if (backdropClicked) {
        AdvanceStep();
        return;
    }

    // Completion dialog
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorSuccess);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20.0f, 20.0f));

    ImGui::SetNextWindowFocus();
    ImGui::Begin("Tutorial Complete!", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImVec2 closeButtonSize(std::min(windowSize.x * 0.4f, 140.0f), 32.0f);
    ImGui::SetCursorPos(ImVec2(windowSize.x - closeButtonSize.x - 10.0f, 4.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    if (ImGui::Button("Close tutorial", closeButtonSize)) {
        AdvanceStep();
        ImGui::PopStyleColor(3);
        ImGui::PopTextWrapPos();
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
        return;
    }
    ImGui::PopStyleColor(3);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 6.0f);

    ImGui::PushTextWrapPos(windowSize.x - 40);

    ImGui::TextColored(UITheme::ColorSuccess, "Congratulations!");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("You've learned the basics:");
    ImGui::Spacing();
    ImGui::BulletText("Observe to collect resources");
    ImGui::BulletText("Upgrade for faster production");
    ImGui::BulletText("Manage coherence stability");

    ImGui::Spacing();
    ImGui::TextWrapped("Explore Research, Prestige, Combat & more!");

    ImGui::Spacing();
    ImGui::Spacing();

    // Mobile-friendly centered button
    float buttonHeight = 52.0f;
    float buttonWidth = windowSize.x - 60.0f;
    float startX = 10.0f;

    ImGui::SetCursorPosX(startX);

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.8f, 0.5f, 1.0f));
    ImGui::SetItemDefaultFocus();
    if (ImGui::Button("START PLAYING!", ImVec2(buttonWidth, buttonHeight))) {
        AdvanceStep(); // Will set to None
    }
    ImGui::PopStyleColor(3);

    ImGui::PopTextWrapPos();
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}

// ============================================================================
// Helper Functions
// ============================================================================

bool TutorialOverlay::DrawDimmingMask(const ImVec2& cutoutMin, const ImVec2& cutoutMax, bool allowDismiss) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screenSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));
    ImGui::Begin("##TutorialMask", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::InvisibleButton("##TutorialMaskClickArea", screenSize);
    bool clicked = allowDismiss && ImGui::IsItemClicked();
    ImGui::PopStyleVar(2);

    // Draw four rectangles around the cutout area to create dimming effect
    ImU32 dimColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 0.6f));

    // Top
    drawList->AddRectFilled(ImVec2(0, 0), ImVec2(screenSize.x, cutoutMin.y), dimColor);

    // Bottom
    drawList->AddRectFilled(ImVec2(0, cutoutMax.y), screenSize, dimColor);

    // Left
    drawList->AddRectFilled(ImVec2(0, cutoutMin.y), ImVec2(cutoutMin.x, cutoutMax.y), dimColor);

    // Right
    drawList->AddRectFilled(ImVec2(cutoutMax.x, cutoutMin.y), ImVec2(screenSize.x, cutoutMax.y), dimColor);

    // Highlight border around cutout
    drawList->AddRect(cutoutMin, cutoutMax,
                     ImGui::ColorConvertFloat4ToU32(UITheme::ColorAccent),
                     0.0f, 0, 3.0f);

    ImGui::End();
    ImGui::PopStyleColor();

    return clicked;
}

bool TutorialOverlay::RenderBackdrop(const ImVec4& color, const char* id, bool allowDismiss) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screenSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, color);
    ImGui::Begin(id, nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoSavedSettings);

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));
    ImGui::InvisibleButton("##TutorialBackdropArea", screenSize);
    bool clicked = allowDismiss && ImGui::IsItemClicked();
    ImGui::PopStyleVar(2);

    ImGui::End();
    ImGui::PopStyleColor();

    return clicked;
}

void TutorialOverlay::DrawTooltipArrow(const ImVec2& targetPos, const char* text) {
    // Draw an animated arrow and tooltip pointing to targetPos
    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    float time = ImGui::GetTime();
    float pulse = sinf(time * 2.0f) * 0.2f + 0.8f; // Pulse between 0.6 and 1.0

    ImU32 arrowColor = ImGui::ColorConvertFloat4ToU32(
        ImVec4(UITheme::ColorAccent.x * pulse,
               UITheme::ColorAccent.y * pulse,
               UITheme::ColorAccent.z * pulse,
               1.0f)
    );

    // Draw arrow pointing down to target
    ImVec2 arrowStart(targetPos.x, targetPos.y - 50);
    drawList->AddLine(arrowStart, targetPos, arrowColor, 4.0f);

    // Arrow head
    drawList->AddTriangleFilled(
        targetPos,
        ImVec2(targetPos.x - 8, targetPos.y - 15),
        ImVec2(targetPos.x + 8, targetPos.y - 15),
        arrowColor
    );

    // Tooltip text above arrow
    ImVec2 textSize = ImGui::CalcTextSize(text);
    ImVec2 textPos(arrowStart.x - textSize.x * 0.5f, arrowStart.y - textSize.y - 10);

    drawList->AddRectFilled(
        ImVec2(textPos.x - 10, textPos.y - 5),
        ImVec2(textPos.x + textSize.x + 10, textPos.y + textSize.y + 5),
        ImGui::ColorConvertFloat4ToU32(UITheme::ColorPanelBg),
        5.0f
    );

    drawList->AddText(textPos, ImGui::ColorConvertFloat4ToU32(UITheme::ColorText), text);
}

const char* TutorialOverlay::GetTooltipText() const {
    switch (m_CurrentStep) {
        case TutorialStep::Welcome:
            return "Welcome to Quantum Idle!";
        case TutorialStep::ObserveButton:
            return "Click to collapse the wave function";
        case TutorialStep::UpgradeButton:
            return "Upgrade your station";
        case TutorialStep::CoherenceWarning:
            return "Watch your coherence";
        case TutorialStep::Completed:
            return "Tutorial complete!";
        default:
            return "";
    }
}

} // namespace UI
