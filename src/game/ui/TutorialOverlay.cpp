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
{
}

void TutorialOverlay::Update(GameState* state) {
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
            break;

        default:
            break;
    }
}

void TutorialOverlay::Render(GameState* state, Renderer* renderer) {
    if (!IsActive()) {
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
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;
    ImVec2 windowSize(500, 300);
    ImVec2 windowPos((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);

    // Full screen dimming
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screenSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.7f));
    ImGui::Begin("##TutorialDim", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoInputs);
    ImGui::End();
    ImGui::PopStyleColor();

    // Welcome dialog
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorAccent);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

    ImGui::Begin("Welcome to Quantum Idle", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Use default font

    ImGui::TextWrapped("Welcome to the Quantum Realm!");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped(
        "In this incremental game, you'll harness the power of quantum mechanics "
        "to generate resources and expand your research empire."
    );
    ImGui::Spacing();
    ImGui::TextWrapped(
        "The core mechanic revolves around SUPERPOSITION and OBSERVATION:"
    );
    ImGui::Spacing();
    ImGui::BulletText("Research stations build up potential resources in superposition");
    ImGui::BulletText("Observing collapses the wave function to collect resources");
    ImGui::BulletText("Manage coherence to maintain quantum stability");

    ImGui::Spacing();
    ImGui::Spacing();

    // Center the buttons
    float buttonWidth = 120.0f;
    float spacing = 10.0f;
    float totalWidth = buttonWidth * 2 + spacing;
    float startX = (windowSize.x - totalWidth) * 0.5f;

    ImGui::SetCursorPosX(startX);
    if (ImGui::Button("Start Tutorial", ImVec2(buttonWidth, 30))) {
        AdvanceStep();
    }

    ImGui::SameLine(0, spacing);
    if (ImGui::Button("Skip", ImVec2(buttonWidth, 30))) {
        Skip();
    }

    ImGui::PopFont();
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void TutorialOverlay::RenderObserveStep(GameState* state) {
    // Find the position of the first "Observe" button in the station view
    // For now, we'll show a centered tooltip with instructions

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    // Semi-transparent dimming except for station area
    DrawDimmingMask(ImVec2(0, 100), ImVec2(screenSize.x, screenSize.y - 100));

    // Tooltip window pointing to station area
    ImVec2 tooltipSize(400, 200);
    ImVec2 tooltipPos(screenSize.x * 0.5f - tooltipSize.x * 0.5f, screenSize.y * 0.3f);

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorAccent);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    ImGui::Begin("##ObserveTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::PushTextWrapPos(tooltipSize.x - 30);

    ImGui::TextColored(UITheme::ColorAccent, "Step 1: Observe");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped(
        "Look at your research station below. It's building up resources in SUPERPOSITION "
        "(the yellow/gold bar)."
    );
    ImGui::Spacing();
    ImGui::TextWrapped("Click the OBSERVE button to collapse the wave function and collect qubits!");
    ImGui::Spacing();
    ImGui::TextColored(UITheme::ColorWarning, "Try observing now!");

    // Animated arrow pointing down
    float time = ImGui::GetTime();
    float bounce = sinf(time * 3.0f) * 10.0f;
    ImVec2 arrowStart(tooltipSize.x * 0.5f, tooltipSize.y - 20 + bounce);
    ImVec2 arrowEnd(tooltipSize.x * 0.5f, tooltipSize.y - 10 + bounce);

    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 windowPos = ImGui::GetWindowPos();
    drawList->AddLine(
        ImVec2(windowPos.x + arrowStart.x, windowPos.y + arrowStart.y),
        ImVec2(windowPos.x + arrowEnd.x, windowPos.y + arrowEnd.y),
        ImGui::ColorConvertFloat4ToU32(UITheme::ColorAccent), 3.0f
    );

    // Arrow head
    drawList->AddTriangleFilled(
        ImVec2(windowPos.x + arrowEnd.x, windowPos.y + arrowEnd.y),
        ImVec2(windowPos.x + arrowEnd.x - 5, windowPos.y + arrowEnd.y - 10),
        ImVec2(windowPos.x + arrowEnd.x + 5, windowPos.y + arrowEnd.y - 10),
        ImGui::ColorConvertFloat4ToU32(UITheme::ColorAccent)
    );

    ImGui::PopTextWrapPos();
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void TutorialOverlay::RenderUpgradeStep(GameState* state) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    // Semi-transparent dimming
    DrawDimmingMask(ImVec2(0, 100), ImVec2(screenSize.x, screenSize.y - 100));

    // Tooltip window
    ImVec2 tooltipSize(400, 220);
    ImVec2 tooltipPos(screenSize.x * 0.5f - tooltipSize.x * 0.5f, screenSize.y * 0.3f);

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorSuccess);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    ImGui::Begin("##UpgradeTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::PushTextWrapPos(tooltipSize.x - 30);

    ImGui::TextColored(UITheme::ColorSuccess, "Step 2: Upgrade");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Great! You've collected qubits.");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Now you can UPGRADE your station to increase production. "
        "Each upgrade makes your station generate resources faster!"
    );
    ImGui::Spacing();
    ImGui::TextColored(UITheme::ColorSuccess, "Click the UPGRADE button to improve your station.");
    ImGui::Spacing();

    ImGui::Text("Current Qubits: %.0f", state->GetResource(QuantumResource::Qubits));

    ImGui::PopTextWrapPos();
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void TutorialOverlay::RenderCoherenceStep(GameState* state) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    // Dim entire screen except top bar (where resources are)
    DrawDimmingMask(ImVec2(0, 0), ImVec2(screenSize.x, 80));

    // Tooltip window
    ImVec2 tooltipSize(450, 250);
    ImVec2 tooltipPos(screenSize.x * 0.5f - tooltipSize.x * 0.5f, 100);

    ImGui::SetNextWindowPos(tooltipPos);
    ImGui::SetNextWindowSize(tooltipSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorDanger);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 3.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    ImGui::Begin("##CoherenceTutorial", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::PushTextWrapPos(tooltipSize.x - 30);

    ImGui::TextColored(UITheme::ColorDanger, "Step 3: Manage Coherence");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("Warning! Your COHERENCE is dropping!");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Coherence represents quantum stability. When it drops too low, your observation "
        "success rate decreases."
    );
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Look at the top bar - Coherence is the second resource (shown in cyan/blue). "
        "You can stabilize it by purchasing Coherence upgrades from the Research menu."
    );
    ImGui::Spacing();
    ImGui::TextColored(UITheme::ColorWarning,
        "For now, just observe how coherence affects your gameplay. "
        "It will regenerate slowly over time."
    );
    ImGui::Spacing();

    ImGui::Text("Current Coherence: %.1f / 100.0", state->GetResource(QuantumResource::Coherence));

    ImGui::Spacing();
    if (ImGui::Button("I Understand", ImVec2(150, 30))) {
        AdvanceStep();
    }

    ImGui::PopTextWrapPos();
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void TutorialOverlay::RenderCompletedStep(GameState* state) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;
    ImVec2 windowSize(450, 250);
    ImVec2 windowPos((screenSize.x - windowSize.x) * 0.5f, (screenSize.y - windowSize.y) * 0.5f);

    // Full screen dimming
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screenSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.7f));
    ImGui::Begin("##TutorialCompleteDim", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoInputs);
    ImGui::End();
    ImGui::PopStyleColor();

    // Completion dialog
    ImGui::SetNextWindowPos(windowPos);
    ImGui::SetNextWindowSize(windowSize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 12.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorSuccess);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);

    ImGui::Begin("Tutorial Complete!", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::PushTextWrapPos(windowSize.x - 30);

    ImGui::TextColored(UITheme::ColorSuccess, "Congratulations!");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::TextWrapped("You've learned the basics of Quantum Idle:");
    ImGui::Spacing();
    ImGui::BulletText("Observing stations to collect resources");
    ImGui::BulletText("Upgrading to increase production");
    ImGui::BulletText("Managing coherence for quantum stability");

    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::TextWrapped(
        "There's much more to discover: Research Trees, Prestige, Combat, "
        "Spaceships, and more! Experiment and have fun!"
    );

    ImGui::Spacing();
    ImGui::Spacing();

    // Center the button
    float buttonWidth = 150.0f;
    float startX = (windowSize.x - buttonWidth) * 0.5f;

    ImGui::SetCursorPosX(startX);
    if (ImGui::Button("Start Playing!", ImVec2(buttonWidth, 35))) {
        AdvanceStep(); // Will set to None
    }

    ImGui::PopTextWrapPos();
    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

// ============================================================================
// Helper Functions
// ============================================================================

void TutorialOverlay::DrawDimmingMask(const ImVec2& cutoutMin, const ImVec2& cutoutMax) {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 screenSize = io.DisplaySize;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(screenSize);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0.5f));
    ImGui::Begin("##TutorialMask", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoInputs);

    ImDrawList* drawList = ImGui::GetWindowDrawList();

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
