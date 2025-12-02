#include "TutorialManager.h"
#include "Renderer.h"
#include "Logger.h"
#include "ImGuiUtils.h"
#include "imgui.h"
#include <cmath>
#include <fstream>

TutorialManager::TutorialManager()
    : m_Enabled(true)
    , m_Active(false)
    , m_CurrentStep(TutorialStep::Welcome)
    , m_PulseTimer(0.0f)
    , m_FadeAlpha(0.0f)
{
    for (i32 i = 0; i < static_cast<i32>(TutorialStep::COUNT); i++) {
        m_CompletedSteps[i] = false;
    }
}

void TutorialManager::Initialize() {
    m_Active = false;
    m_CurrentStep = TutorialStep::Welcome;
    m_PulseTimer = 0.0f;
    m_FadeAlpha = 0.0f;

    // Check if tutorial should auto-start (if not completed)
    if (m_Enabled && !IsTutorialCompleted()) {
        // Tutorial will start on first update
    }
}

void TutorialManager::Update(f64 deltaTime) {
    if (!m_Active || !m_Enabled) return;

    // Update pulse animation
    m_PulseTimer += static_cast<f32>(deltaTime);

    // Update fade-in
    if (m_FadeAlpha < 1.0f) {
        m_FadeAlpha += static_cast<f32>(deltaTime) * 2.0f; // Fade in over 0.5s
        if (m_FadeAlpha > 1.0f) m_FadeAlpha = 1.0f;
    }
}

void TutorialManager::Render(Renderer* renderer) {
    if (!m_Active || !m_Enabled) return;

    // Draw semi-transparent overlay (blocks input to non-highlighted elements)
    ImDrawList* bgDrawList = ImGui::GetBackgroundDrawList();
    ImGuiIO& io = ImGui::GetIO();

    // Dark overlay
    ImU32 overlayColor = IM_COL32(0, 0, 0, static_cast<int>(150 * m_FadeAlpha));
    bgDrawList->AddRectFilled(ImVec2(0, 0), io.DisplaySize, overlayColor);

    // Render highlight
    RenderHighlight(renderer);

    // Render tooltip
    RenderTooltip(renderer);

    // Render controls (Next/Skip buttons)
    RenderControls(renderer);
}

void TutorialManager::StartTutorial() {
    if (!m_Enabled) return;

    m_Active = true;
    m_CurrentStep = TutorialStep::Welcome;
    m_PulseTimer = 0.0f;
    m_FadeAlpha = 0.0f;
    SetupStep(m_CurrentStep);

    Log::Info("Tutorial started");
}

void TutorialManager::NextStep() {
    // Mark current step as completed
    MarkStepCompleted(m_CurrentStep);

    // Move to next step
    i32 nextStepIndex = static_cast<i32>(m_CurrentStep) + 1;
    if (nextStepIndex >= static_cast<i32>(TutorialStep::Completed)) {
        // Tutorial completed
        MarkStepCompleted(TutorialStep::Completed);
        m_Active = false;
        Log::Info("Tutorial completed!");
        return;
    }

    m_CurrentStep = static_cast<TutorialStep>(nextStepIndex);
    SetupStep(m_CurrentStep);
    m_PulseTimer = 0.0f;
}

void TutorialManager::PreviousStep() {
    i32 prevStepIndex = static_cast<i32>(m_CurrentStep) - 1;
    if (prevStepIndex < 0) return;

    m_CurrentStep = static_cast<TutorialStep>(prevStepIndex);
    SetupStep(m_CurrentStep);
    m_PulseTimer = 0.0f;
}

void TutorialManager::SkipTutorial() {
    m_Active = false;
    MarkStepCompleted(TutorialStep::Completed);
    Log::Info("Tutorial skipped");
}

void TutorialManager::ResetTutorial() {
    for (i32 i = 0; i < static_cast<i32>(TutorialStep::COUNT); i++) {
        m_CompletedSteps[i] = false;
    }
    m_CurrentStep = TutorialStep::Welcome;
    m_Active = false;
    Log::Info("Tutorial reset");
}

void TutorialManager::MarkStepCompleted(TutorialStep step) {
    m_CompletedSteps[static_cast<i32>(step)] = true;
}

bool TutorialManager::IsStepCompleted(TutorialStep step) const {
    return m_CompletedSteps[static_cast<i32>(step)];
}

void TutorialManager::TriggerEvent(const std::string& eventName) {
    if (!m_Active) return;

    // Auto-advance when player performs expected action
    if (eventName == "ObserveStation" && m_CurrentStep == TutorialStep::ObserveStation) {
        NextStep();
    } else if (eventName == "UnlockStation" && m_CurrentStep == TutorialStep::UnlockStation) {
        NextStep();
    } else if (eventName == "UpgradeStation" && m_CurrentStep == TutorialStep::UpgradeStation) {
        NextStep();
    } else if (eventName == "OpenResearch" && m_CurrentStep == TutorialStep::OpenResearch) {
        NextStep();
    } else if (eventName == "PurchaseResearch" && m_CurrentStep == TutorialStep::PurchaseResearch) {
        NextStep();
    } else if (eventName == "BuyUpgrade" && m_CurrentStep == TutorialStep::BuyUpgrade) {
        NextStep();
    } else if (eventName == "StartCombat" && m_CurrentStep == TutorialStep::FirstCombat) {
        NextStep();
    } else if (eventName == "InstallPart" && m_CurrentStep == TutorialStep::InstallPart) {
        NextStep();
    } else if (eventName == "LearnSkill" && m_CurrentStep == TutorialStep::LearnSkill) {
        NextStep();
    }
}

void TutorialManager::SetupStep(TutorialStep step) {
    m_CurrentHighlight = GetHighlightForStep(step);
}

TutorialHighlight TutorialManager::GetHighlightForStep(TutorialStep step) const {
    TutorialHighlight highlight;

    switch (step) {
        case TutorialStep::Welcome:
            highlight.title = "Welcome to Quantum Idle!";
            highlight.description = "This tutorial will guide you through the basics.\n\n"
                                  "Observe research stations to generate Qubits,\n"
                                  "unlock powerful upgrades, and explore the quantum realm!";
            highlight.actionHint = "Click Next to begin";
            highlight.position = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f - 200, ImGui::GetIO().DisplaySize.y * 0.3f);
            highlight.size = ImVec2(400, 200);
            highlight.highlightPulse = false;
            break;

        case TutorialStep::ObserveStation:
            highlight.title = "Observe Research Stations";
            highlight.description = "Click the OBSERVE button on any research station\n"
                                  "to generate Qubits!\n\n"
                                  "Qubits are your primary resource.";
            highlight.actionHint = "Click OBSERVE on a station";
            highlight.position = ImVec2(300, 400);
            highlight.size = ImVec2(200, 60);
            highlight.arrowAngle = 3.14159f / 2.0f; // Point down
            break;

        case TutorialStep::UnlockStation:
            highlight.title = "Unlock More Stations";
            highlight.description = "Use Qubits to unlock new research stations.\n\n"
                                  "More stations = more production!";
            highlight.actionHint = "Click UNLOCK on a locked station";
            highlight.position = ImVec2(300, 500);
            highlight.size = ImVec2(200, 60);
            highlight.arrowAngle = 3.14159f / 2.0f;
            break;

        case TutorialStep::UpgradeStation:
            highlight.title = "Upgrade Stations";
            highlight.description = "Upgrade stations to increase their production.\n\n"
                                  "Higher levels = exponentially more Qubits!";
            highlight.actionHint = "Click UPGRADE on any station";
            highlight.position = ImVec2(300, 400);
            highlight.size = ImVec2(200, 60);
            break;

        case TutorialStep::OpenResearch:
            highlight.title = "Research Tree";
            highlight.description = "The Research tab contains powerful upgrades.\n\n"
                                  "Research new technologies to boost production!";
            highlight.actionHint = "Click the RESEARCH tab";
            highlight.position = ImVec2(ImGui::GetIO().DisplaySize.x * 0.3f, ImGui::GetIO().DisplaySize.y - 80);
            highlight.size = ImVec2(150, 60);
            highlight.arrowAngle = 3.14159f; // Point left
            break;

        case TutorialStep::PurchaseResearch:
            highlight.title = "Purchase Research";
            highlight.description = "Click on research nodes to unlock them.\n\n"
                                  "Each node provides unique bonuses!";
            highlight.actionHint = "Click on an available research node";
            highlight.position = ImVec2(400, 300);
            highlight.size = ImVec2(100, 100);
            break;

        case TutorialStep::NavigateToUpgrades:
            highlight.title = "Permanent Upgrades";
            highlight.description = "The UPGRADES tab contains buyable improvements.\n\n"
                                  "These persist through resets!";
            highlight.actionHint = "Click the UPGRADES tab";
            highlight.position = ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y - 80);
            highlight.size = ImVec2(150, 60);
            break;

        case TutorialStep::BuyUpgrade:
            highlight.title = "Buy Upgrades";
            highlight.description = "Purchase upgrades to permanently boost your progress.\n\n"
                                  "Cost increases with each purchase.";
            highlight.actionHint = "Buy any upgrade";
            highlight.position = ImVec2(400, 300);
            highlight.size = ImVec2(200, 80);
            break;

        case TutorialStep::NavigateToCombat:
            highlight.title = "Combat System";
            highlight.description = "Test your strength against enemies!\n\n"
                                  "Earn rewards and level up your character.";
            highlight.actionHint = "Click the COMBAT tab";
            highlight.position = ImVec2(ImGui::GetIO().DisplaySize.x * 0.7f, ImGui::GetIO().DisplaySize.y - 80);
            highlight.size = ImVec2(150, 60);
            break;

        case TutorialStep::FirstCombat:
            highlight.title = "Start Combat";
            highlight.description = "Click to start a combat encounter.\n\n"
                                  "Victory grants XP, loot, and credits!";
            highlight.actionHint = "Start your first combat";
            highlight.position = ImVec2(400, 300);
            highlight.size = ImVec2(200, 80);
            break;

        case TutorialStep::OpenShip:
            highlight.title = "Spaceship System";
            highlight.description = "Manage your ship and install parts.\n\n"
                                  "Ship parts provide powerful bonuses!";
            highlight.actionHint = "Open the More menu and select Spaceship";
            highlight.position = ImVec2(ImGui::GetIO().DisplaySize.x - 200, ImGui::GetIO().DisplaySize.y - 80);
            highlight.size = ImVec2(150, 60);
            break;

        case TutorialStep::InstallPart:
            highlight.title = "Install Ship Parts";
            highlight.description = "Drag parts from inventory to ship slots.\n\n"
                                  "Or use the Install button!";
            highlight.actionHint = "Install a ship part";
            highlight.position = ImVec2(400, 300);
            highlight.size = ImVec2(200, 100);
            break;

        case TutorialStep::OpenSkills:
            highlight.title = "Skill Tree";
            highlight.description = "Learn skills to enhance your abilities.\n\n"
                                  "Skills provide combat and production bonuses!";
            highlight.actionHint = "Open Skills from the More menu";
            highlight.position = ImVec2(ImGui::GetIO().DisplaySize.x - 200, ImGui::GetIO().DisplaySize.y - 80);
            highlight.size = ImVec2(150, 60);
            break;

        case TutorialStep::LearnSkill:
            highlight.title = "Learn Skills";
            highlight.description = "Spend skill points on passive bonuses.\n\n"
                                  "You can respec later for Photons!";
            highlight.actionHint = "Learn a skill";
            highlight.position = ImVec2(400, 300);
            highlight.size = ImVec2(150, 150);
            break;

        case TutorialStep::UseMoreMenu:
            highlight.title = "More Options";
            highlight.description = "The MENU button opens additional features:\n"
                                  "- Achievements\n"
                                  "- Statistics\n"
                                  "- Shops\n"
                                  "- And more!";
            highlight.actionHint = "Explore the menu when ready";
            highlight.position = ImVec2(ImGui::GetIO().DisplaySize.x - 200, ImGui::GetIO().DisplaySize.y - 80);
            highlight.size = ImVec2(150, 60);
            break;

        default:
            break;
    }

    return highlight;
}

void TutorialManager::RenderHighlight(Renderer* renderer) {
    ImDrawList* fgDrawList = ImGui::GetForegroundDrawList();

    // Get pulse scale
    f32 pulseScale = m_CurrentHighlight.highlightPulse ? GetPulseScale() : 1.0f;

    // Calculate highlight rect with pulse
    ImVec2 center = ImVec2(
        m_CurrentHighlight.position.x + m_CurrentHighlight.size.x * 0.5f,
        m_CurrentHighlight.position.y + m_CurrentHighlight.size.y * 0.5f
    );
    ImVec2 halfSize = ImVec2(
        m_CurrentHighlight.size.x * 0.5f * pulseScale,
        m_CurrentHighlight.size.y * 0.5f * pulseScale
    );

    ImVec2 p0 = ImVec2(center.x - halfSize.x, center.y - halfSize.y);
    ImVec2 p1 = ImVec2(center.x + halfSize.x, center.y + halfSize.y);

    // Draw highlight border (glowing effect)
    ImU32 highlightColor = GetHighlightColor();
    fgDrawList->AddRect(p0, p1, highlightColor, 8.0f, 0, 4.0f);

    // Draw inner glow
    ImU32 glowColor = IM_COL32(0, 200, 255, static_cast<int>(80 * m_FadeAlpha * pulseScale));
    fgDrawList->AddRectFilled(p0, p1, glowColor, 8.0f);

    // Draw arrow pointing to highlight
    f32 arrowLength = 60.0f;
    f32 arrowAngle = m_CurrentHighlight.arrowAngle;
    ImVec2 arrowStart = ImVec2(
        center.x - std::cos(arrowAngle) * (halfSize.x + 30.0f),
        center.y - std::sin(arrowAngle) * (halfSize.y + 30.0f)
    );
    ImVec2 arrowEnd = ImVec2(
        center.x - std::cos(arrowAngle) * (halfSize.x + 10.0f),
        center.y - std::sin(arrowAngle) * (halfSize.y + 10.0f)
    );

    // Arrow line
    fgDrawList->AddLine(arrowStart, arrowEnd, highlightColor, 3.0f);

    // Arrow head
    f32 headLength = 15.0f;
    f32 headAngle = 0.4f;
    ImVec2 head1 = ImVec2(
        arrowEnd.x - std::cos(arrowAngle - headAngle) * headLength,
        arrowEnd.y - std::sin(arrowAngle - headAngle) * headLength
    );
    ImVec2 head2 = ImVec2(
        arrowEnd.x - std::cos(arrowAngle + headAngle) * headLength,
        arrowEnd.y - std::sin(arrowAngle + headAngle) * headLength
    );
    fgDrawList->AddTriangleFilled(arrowEnd, head1, head2, highlightColor);
}

void TutorialManager::RenderTooltip(Renderer* renderer) {
    ImDrawList* fgDrawList = ImGui::GetForegroundDrawList();
    ImGuiIO& io = ImGui::GetIO();

    // Tooltip position (center-right of screen)
    f32 tooltipWidth = 400.0f;
    f32 tooltipHeight = 250.0f;
    ImVec2 tooltipPos = ImVec2(
        io.DisplaySize.x * 0.5f - tooltipWidth * 0.5f,
        100.0f
    );

    // Background
    ImU32 bgColor = IM_COL32(20, 20, 40, static_cast<int>(240 * m_FadeAlpha));
    ImU32 borderColor = IM_COL32(0, 200, 255, static_cast<int>(255 * m_FadeAlpha));
    fgDrawList->AddRectFilled(tooltipPos, ImVec2(tooltipPos.x + tooltipWidth, tooltipPos.y + tooltipHeight), bgColor, 10.0f);
    fgDrawList->AddRect(tooltipPos, ImVec2(tooltipPos.x + tooltipWidth, tooltipPos.y + tooltipHeight), borderColor, 10.0f, 0, 2.0f);

    // Title
    ImVec2 titlePos = ImVec2(tooltipPos.x + 20, tooltipPos.y + 20);
    ImU32 titleColor = IM_COL32(0, 200, 255, static_cast<int>(255 * m_FadeAlpha));
    ImFont* font = ImGui::GetFont();
    fgDrawList->AddText(font, 24.0f, titlePos, titleColor, m_CurrentHighlight.title.c_str());

    // Description
    ImVec2 descPos = ImVec2(tooltipPos.x + 20, tooltipPos.y + 60);
    ImU32 descColor = IM_COL32(220, 220, 220, static_cast<int>(255 * m_FadeAlpha));
    fgDrawList->AddText(font, 16.0f, descPos, descColor, m_CurrentHighlight.description.c_str());

    // Action hint
    ImVec2 hintPos = ImVec2(tooltipPos.x + 20, tooltipPos.y + tooltipHeight - 40);
    ImU32 hintColor = IM_COL32(255, 255, 100, static_cast<int>(255 * m_FadeAlpha));
    fgDrawList->AddText(font, 18.0f, hintPos, hintColor, m_CurrentHighlight.actionHint.c_str());
}

void TutorialManager::RenderControls(Renderer* renderer) {
    ImGuiIO& io = ImGui::GetIO();

    // Control buttons at bottom center
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f - 200, io.DisplaySize.y - 100), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 80), ImGuiCond_Always);

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.2f, 0.9f * m_FadeAlpha));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);

    if (ImGui::Begin("TutorialControls", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {
        ImGui::SetCursorPosY(20);

        // Previous button (if not first step)
        if (m_CurrentStep != TutorialStep::Welcome) {
            if (ImGui::Button("< Previous", ImVec2(100, 40))) {
                PreviousStep();
            }
            ImGui::SameLine();
        }

        // Next button
        if (ImGui::Button("Next >", ImVec2(100, 40))) {
            NextStep();
        }
        ImGui::SameLine();

        // Skip button
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Skip Tutorial", ImVec2(150, 40))) {
            SkipTutorial();
        }
        ImGui::PopStyleColor();
    }
    ImGui::End();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

f32 TutorialManager::GetPulseScale() const {
    // Sine wave pulse between 0.95 and 1.05
    return 1.0f + std::sin(m_PulseTimer * 3.0f) * 0.05f;
}

ImU32 TutorialManager::GetHighlightColor() const {
    // Pulsing color
    f32 intensity = 0.8f + std::sin(m_PulseTimer * 3.0f) * 0.2f;
    return IM_COL32(0, static_cast<int>(200 * intensity), 255, static_cast<int>(255 * m_FadeAlpha));
}

void TutorialManager::SaveToJson(std::ofstream& file) const {
    file << "\"tutorialEnabled\":" << (m_Enabled ? "true" : "false") << ",\n";
    file << "\"tutorialCompleted\":" << (IsTutorialCompleted() ? "true" : "false") << ",\n";
    file << "\"completedSteps\":[";
    for (i32 i = 0; i < static_cast<i32>(TutorialStep::COUNT); i++) {
        file << (m_CompletedSteps[i] ? "true" : "false");
        if (i < static_cast<i32>(TutorialStep::COUNT) - 1) file << ",";
    }
    file << "],\n";
}

void TutorialManager::LoadFromJson(const std::string& line) {
    // Simple JSON parsing (to be replaced with nlohmann/json)
    if (line.find("\"tutorialEnabled\":true") != std::string::npos) {
        m_Enabled = true;
    } else if (line.find("\"tutorialEnabled\":false") != std::string::npos) {
        m_Enabled = false;
    }

    if (line.find("\"tutorialCompleted\":true") != std::string::npos) {
        MarkStepCompleted(TutorialStep::Completed);
    }

    // TODO: Parse completedSteps array
}
