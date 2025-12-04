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

ImVec2 TutorialOverlay::CalculateModalSize(const ImVec2& screenSize, float maxWidth, float maxHeight) const {
    const float horizontalMargin = UITheme::SpacingLG() * 2.5f;
    const float verticalMargin = UITheme::SpacingLG() * 2.5f;

    const float width = std::min(screenSize.x - horizontalMargin * 2.0f, maxWidth);
    const float height = std::min(screenSize.y - verticalMargin * 2.0f, maxHeight);

    return ImVec2(std::max(width, UITheme::SpacingLG() * 20.0f),
                  std::max(height, UITheme::SpacingLG() * 15.0f));
}

ImVec2 TutorialOverlay::CenterModal(const ImVec2& windowSize, const ImVec2& screenSize) const {
    return ImVec2((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);
}

void TutorialOverlay::RenderSectionHeader(const char* title, const char* subtitle) const {
    ImGui::TextColored(UITheme::ColorAccent, "%s", title);
    ImGui::PushStyleColor(ImGuiCol_Separator, UITheme::ColorCardBorder);
    ImGui::Separator();
    ImGui::PopStyleColor();

    if (subtitle && subtitle[0] != '\0') {
        AddVerticalSpace(UITheme::SpacingSM());
        ImGui::TextColored(UITheme::ColorTextDim, "%s", subtitle);
    }

    AddVerticalSpace(UITheme::SpacingMD());
}

void TutorialOverlay::AddVerticalSpace(float amount) const {
    ImGui::Dummy(ImVec2(0.0f, amount));
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
    ImVec2 windowSize = CalculateModalSize(screenSize, 540.0f, 440.0f);
    ImVec2 windowPos = CenterModal(windowSize, screenSize);

    // Full screen dimming (clicking the backdrop closes the tutorial)
    bool backdropClicked = RenderBackdrop(ImVec4(0, 0, 0, 0.7f), "##TutorialDim", true);
    if (backdropClicked) {
        Skip();
        return;
    }

    // Welcome dialog
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, UITheme::CardRounding());
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorAccent);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, UITheme::BorderThickness() * 1.6f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, UITheme::DialogPadding());

    ImGui::SetNextWindowFocus();
    ImGui::Begin("Welcome to Quantum Idle", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    // Skip/close control placed at the top for visibility and keyboard focus
    ImVec2 closeButtonSize(std::min(windowSize.x * 0.4f, 130.0f), 34.0f);
    ImGui::SetCursorPos(ImVec2(windowSize.x - closeButtonSize.x - UITheme::SpacingMD(), UITheme::SpacingSM()));
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
    AddVerticalSpace(UITheme::SpacingMD());
    RenderSectionHeader("Welcome to the Quantum Realm!", "Learn the basics before you dive in.");

    ImGui::TextWrapped(
        "In this incremental game, you'll harness quantum mechanics "
        "to generate resources and expand your empire."
    );
    AddVerticalSpace(UITheme::SpacingSM());
    ImGui::TextWrapped("Core mechanics:");
    AddVerticalSpace(UITheme::SpacingSM());
    ImGui::BulletText("Stations build resources in superposition");
    ImGui::BulletText("Tap OBSERVE to collect resources");
    ImGui::BulletText("Manage coherence for stability");

    AddVerticalSpace(UITheme::SpacingLG());

    // Mobile-friendly buttons - minimum 48px height for touch targets
    float buttonHeight = 48.0f;
    float horizontalPadding = UITheme::SpacingLG() * 1.5f;
    float availableWidth = windowSize.x - horizontalPadding * 2.0f;
    float spacing = UITheme::SpacingMD();
    float buttonWidth = (availableWidth - spacing) * 0.5f;

    ImGui::SetCursorPosX(horizontalPadding);

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
    ImVec2 tooltipSize = CalculateModalSize(screenSize, 460.0f, 320.0f);
    ImVec2 tooltipPos = CenterModal(tooltipSize, screenSize);
    tooltipPos.y = UITheme::SpacingLG() * 2.0f;

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorAccent);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, UITheme::BorderThickness() * 1.8f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, UITheme::CardRounding());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, UITheme::DialogPadding());

    ImGui::SetNextWindowFocus();
    ImGui::Begin("##ObserveTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImVec2 closeButtonSize(std::min(tooltipSize.x * 0.45f, 140.0f), 32.0f);
    ImGui::SetCursorPos(ImVec2(tooltipSize.x - closeButtonSize.x - UITheme::SpacingSM(), UITheme::SpacingXS()));
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
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + UITheme::SpacingSM());

    ImGui::PushTextWrapPos(tooltipSize.x - UITheme::SpacingLG());

    RenderSectionHeader("Step 1: Observe", "Collect qubits to advance.");

    ImGui::TextWrapped(
        "Your research station is building resources in SUPERPOSITION (the gold bar)."
    );
    AddVerticalSpace(UITheme::SpacingSM());
    ImGui::TextWrapped("Tap the OBSERVE button below to collect qubits!");
    AddVerticalSpace(UITheme::SpacingSM());

    // Show current progress
    ImGui::TextColored(UITheme::ColorWarning, "Qubits: %.0f / 10 (auto-advances at 10)",
                      state->GetResource(QuantumResource::Qubits));

    AddVerticalSpace(UITheme::SpacingLG());

    // Mobile-friendly buttons
    float buttonHeight = 48.0f;
    float horizontalPadding = UITheme::SpacingLG() * 1.25f;
    float availableWidth = tooltipSize.x - horizontalPadding * 2.0f;
    float spacing = UITheme::SpacingMD();
    float buttonWidth = (availableWidth - spacing) * 0.5f;

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
    ImVec2 tooltipSize = CalculateModalSize(screenSize, 460.0f, 340.0f);
    ImVec2 tooltipPos = CenterModal(tooltipSize, screenSize);
    tooltipPos.y = UITheme::SpacingLG() * 2.0f;

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorSuccess);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, UITheme::BorderThickness() * 1.8f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, UITheme::CardRounding());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, UITheme::DialogPadding());

    ImGui::SetNextWindowFocus();
    ImGui::Begin("##UpgradeTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImVec2 closeButtonSize(std::min(tooltipSize.x * 0.45f, 140.0f), 32.0f);
    ImGui::SetCursorPos(ImVec2(tooltipSize.x - closeButtonSize.x - UITheme::SpacingSM(), UITheme::SpacingXS()));
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
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + UITheme::SpacingSM());

    ImGui::PushTextWrapPos(tooltipSize.x - UITheme::SpacingLG());

    RenderSectionHeader("Step 2: Upgrade", "Spend qubits to boost production.");

    ImGui::TextWrapped("Great! You've collected qubits.");
    AddVerticalSpace(UITheme::SpacingSM());
    ImGui::TextWrapped(
        "Now tap UPGRADE to increase production speed!"
    );
    AddVerticalSpace(UITheme::SpacingSM());

    ImGui::Text("Current Qubits: %.0f", state->GetResource(QuantumResource::Qubits));
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "(Auto-advances when coherence < 50%%)");

    AddVerticalSpace(UITheme::SpacingLG());

    // Mobile-friendly buttons
    float buttonHeight = 48.0f;
    float horizontalPadding = UITheme::SpacingLG() * 1.25f;
    float availableWidth = tooltipSize.x - horizontalPadding * 2.0f;
    float spacing = UITheme::SpacingMD();
    float buttonWidth = (availableWidth - spacing) * 0.5f;

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
    ImVec2 tooltipSize = CalculateModalSize(screenSize, 480.0f, 360.0f);
    ImVec2 tooltipPos = CenterModal(tooltipSize, screenSize);
    tooltipPos.y = UITheme::SpacingLG() * 2.5f;

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorDanger);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, UITheme::BorderThickness() * 1.8f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, UITheme::CardRounding());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, UITheme::DialogPadding());

    ImGui::SetNextWindowFocus();
    ImGui::Begin("##CoherenceTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImVec2 closeButtonSize(std::min(tooltipSize.x * 0.45f, 140.0f), 32.0f);
    ImGui::SetCursorPos(ImVec2(tooltipSize.x - closeButtonSize.x - UITheme::SpacingSM(), UITheme::SpacingXS()));
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
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + UITheme::SpacingSM());

    ImGui::PushTextWrapPos(tooltipSize.x - UITheme::SpacingLG());

    RenderSectionHeader("Step 3: Manage Coherence", "Protect observation success.");

    ImGui::TextWrapped("Coherence = quantum stability.");
    AddVerticalSpace(UITheme::SpacingSM());
    ImGui::TextWrapped(
        "Low coherence reduces observation success. Watch the cyan bar at the top!"
    );
    AddVerticalSpace(UITheme::SpacingSM());

    ImGui::Text("Current Coherence: %.1f / 100.0", state->GetResource(QuantumResource::Coherence));

    AddVerticalSpace(UITheme::SpacingLG());

    // Mobile-friendly buttons
    float buttonHeight = 48.0f;
    float horizontalPadding = UITheme::SpacingLG() * 1.25f;
    float availableWidth = tooltipSize.x - horizontalPadding * 2.0f;
    float spacing = UITheme::SpacingMD();
    float buttonWidth = (availableWidth - spacing) * 0.5f;

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
    ImVec2 windowSize = CalculateModalSize(screenSize, 500.0f, 360.0f);
    ImVec2 windowPos = CenterModal(windowSize, screenSize);

    // Full screen dimming
    bool backdropClicked = RenderBackdrop(ImVec4(0, 0, 0, 0.7f), "##TutorialCompleteDim", true);
    if (backdropClicked) {
        AdvanceStep();
        return;
    }

    // Completion dialog
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, UITheme::CardRounding());
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorSuccess);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, UITheme::BorderThickness() * 1.6f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, UITheme::DialogPadding());

    ImGui::SetNextWindowFocus();
    ImGui::Begin("Tutorial Complete!", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImVec2 closeButtonSize(std::min(windowSize.x * 0.4f, 140.0f), 32.0f);
    ImGui::SetCursorPos(ImVec2(windowSize.x - closeButtonSize.x - UITheme::SpacingMD(), UITheme::SpacingSM()));
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
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + UITheme::SpacingSM());

    ImGui::PushTextWrapPos(windowSize.x - UITheme::SpacingLG() * 2.0f);

    RenderSectionHeader("Congratulations!", "You're ready to explore.");

    ImGui::TextWrapped("You've learned the basics:");
    AddVerticalSpace(UITheme::SpacingSM());
    ImGui::BulletText("Observe to collect resources");
    ImGui::BulletText("Upgrade for faster production");
    ImGui::BulletText("Manage coherence stability");

    AddVerticalSpace(UITheme::SpacingSM());
    ImGui::TextWrapped("Explore Research, Prestige, Combat & more!");

    AddVerticalSpace(UITheme::SpacingLG());

    // Mobile-friendly centered button
    float buttonHeight = 52.0f;
    float horizontalPadding = UITheme::SpacingLG() * 1.5f;
    float buttonWidth = windowSize.x - horizontalPadding * 2.0f;

    ImGui::SetCursorPosX(horizontalPadding);

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
