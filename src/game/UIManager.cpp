#include "UIManager.h"
#include "ImGuiUtils.h"
#include "imgui.h"
#include "GameUtils.h"
#include "GameState.h"
#include "Research.h"
#include "CombatSystem.h"
#include "Spaceship.h"
#include "GatchaSystem.h"
#include "Challenges.h"
#include "SkillTree.h"
#include "EnhancementSystem.h"
#include <string>

// NOTE: This implementation assumes the UIManager.h DrawNavButton signature
// has been updated to include width and height parameters.

UIManager::UIManager(GameState* state) : m_GameState(state) {}

void UIManager::Initialize() {
    // Check if the UITheme header is included in the project's include path
    UITheme::SetupStyle();
}

void UIManager::Render(Renderer* renderer) {
    // 1. Setup Main Layout Window (Invisible container covering the entire viewport)
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    // ImGui::SetNextWindowViewport(viewport->ID); <-- REMOVED (Docking feature)

    // Removed ImGuiWindowFlags_NoDocking
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("MainLayout", nullptr, window_flags)) {
        ImGui::PopStyleVar(); // Pop for MainLayout

        // 2. Render Components
        RenderTopBar();
        RenderMainContent();
        RenderBottomNavigation();
        RenderOverlays(renderer);

        ImGui::End();
    } else {
        ImGui::PopStyleVar(); // Pop if Begin fails
    }
}

void UIManager::RenderTopBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, UITheme::TopBarHeight));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // Removed ImGuiWindowFlags_NoDocking
    if (ImGui::Begin("TopBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {

        // Draw Bottom Border Glow
        auto* drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetWindowPos();
        p0.y += UITheme::TopBarHeight - 2.0f;
        ImVec2 p1 = ImVec2(p0.x + viewport->Size.x, p0.y + 2.0f);
        drawList->AddRectFilled(p0, p1, ImGui::GetColorU32(UITheme::ColorAccent));

        // Resources Columns
        ImGui::Columns(5, "ResCols", false);

        // 1. Qubits
        DrawResourceCounter("Qubits", m_GameState->GetResource(QuantumResource::Qubits), ToImVec4(Color::QuantumBlue()));
        ImGui::NextColumn();

        // 2. Coherence (Use actual values)
        DrawResourceCounter("Coherence", m_GameState->m_Coherence, ToImVec4(Color::CoherenceGreen()));
        ImGui::NextColumn();

        // 3. Entanglement
        DrawResourceCounter("Entanglement", m_GameState->GetResource(QuantumResource::Entanglement), ToImVec4(Color::EntanglementOrange()));
        ImGui::NextColumn();

        // 4. Photons
        DrawResourceCounter("Photons", m_GameState->GetTimeline().photons, UITheme::ColorPrimary);
        ImGui::NextColumn();

        // 5. Singularities
        DrawResourceCounter("Singularities", m_GameState->GetTimeline().singularities, UITheme::ColorAccent);
        ImGui::NextColumn();

        ImGui::Columns(1);
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void UIManager::RenderBottomNavigation() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - UITheme::BottomBarHeight));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, UITheme::BottomBarHeight));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    // Removed ImGuiWindowFlags_NoDocking
    if (ImGui::Begin("BottomNav", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {

        // Draw Top Border Glow
        auto* drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetWindowPos();
        ImVec2 p1 = ImVec2(p0.x + viewport->Size.x, p0.y + 2.0f);
        drawList->AddRectFilled(p0, p1, ImGui::GetColorU32(UITheme::ColorAccent));

        // Navigation Buttons
        float width = ImGui::GetContentRegionAvail().x;
        int btnCount = 5; // Stations, Research, Upgrades, Combat, Menu
        float btnHeight = UITheme::BottomBarHeight - 20.0f; // Padding

        // Calculate dynamic button width, accounting for spacing
        float btnWidth = (width - (UITheme::ItemSpacing * static_cast<float>(btnCount - 1))) / static_cast<float>(btnCount);

        ImGui::SetCursorPosY(10.0f); // Padding top

        // 1. Stations (Home)
        bool isBaseState = (m_GameState->GetActiveModal() == ActiveModal::None);
        if (DrawNavButton("STATIONS", isBaseState, UITheme::ColorAccent, btnWidth, btnHeight)) {
            // Close all overlays - return to base state
            m_GameState->SetActiveModal(ActiveModal::None);
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        // 2. Research
        bool showingResearch = (m_GameState->GetActiveModal() == ActiveModal::Research);
        if (DrawNavButton("RESEARCH", showingResearch, UITheme::ColorPrimary, btnWidth, btnHeight)) {
            m_GameState->SetActiveModal(showingResearch ? ActiveModal::None : ActiveModal::Research);
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        // 3. Upgrades (Buyables)
        bool showingBuyables = (m_GameState->GetActiveModal() == ActiveModal::Buyables);
        if (DrawNavButton("UPGRADES", showingBuyables, UITheme::ColorSuccess, btnWidth, btnHeight)) {
            m_GameState->SetActiveModal(showingBuyables ? ActiveModal::None : ActiveModal::Buyables);
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        // 4. Combat/Map
        bool showingCombat = (m_GameState->GetActiveModal() == ActiveModal::Combat);
        if (DrawNavButton("COMBAT", showingCombat, UITheme::ColorDanger, btnWidth, btnHeight)) {
            if(!showingCombat) m_GameState->StartRandomCombat();
            m_GameState->SetActiveModal(showingCombat ? ActiveModal::None : ActiveModal::Combat);
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        // 5. Menu (More)
        bool showingMenu = (m_GameState->GetActiveModal() == ActiveModal::MoreMenu);
        if (DrawNavButton("MENU", showingMenu, UITheme::ColorText, btnWidth, btnHeight)) {
            m_GameState->SetActiveModal(showingMenu ? ActiveModal::None : ActiveModal::MoreMenu);
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void UIManager::RenderMainContent() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    float topY = viewport->Pos.y + UITheme::TopBarHeight;
    float bottomY = viewport->Pos.y + viewport->Size.y - UITheme::BottomBarHeight;
    float height = bottomY - topY;

    ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, topY));
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, height));

    // Transparent background for content area to see game particles
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

    if (ImGui::Begin("MainContent", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {
        // Render content based on active modal (tab-based navigation)
        ActiveModal activeModal = m_GameState->GetActiveModal();

        if (ImGui::BeginChild("ScrollContent", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            switch(activeModal) {
                case ActiveModal::None:
                    // Default: Stations view
                    m_GameState->RenderStationsContent();
                    break;

                case ActiveModal::Research:
                    // Research Tree (full screen in content area)
                    // FIX: RenderResearchTree is in GameState, not ResearchTree
                    m_GameState->RenderResearchTree(nullptr);
                    break;

                case ActiveModal::Buyables:
                    // Upgrades/Buyables (full screen in content area)
                    m_GameState->RenderBuyables(nullptr);
                    break;

                case ActiveModal::Combat:
                    // Combat view (full screen in content area)
                    if (m_GameState->GetCombatSystem().IsInCombat()) {
                        m_GameState->GetCombatSystem().RenderCombatUI(nullptr);
                    } else {
                        ImGui::TextColored(ImVec4(1, 1, 0, 1), "No active combat");
                        ImGui::Text("Click COMBAT button to start a random encounter");
                    }
                    break;

                case ActiveModal::Spaceship:
                    // Ship panel (full screen in content area)
                    m_GameState->GetSpaceship().RenderShipPanel();
                    ImGui::Separator();
                    m_GameState->GetSpaceship().RenderInventoryPanel();
                    break;

                case ActiveModal::Skills:
                    // Skill Tree (full screen in content area)
                    m_GameState->GetSkillTree().RenderSkillTree(nullptr, m_GameState);
                    break;

                case ActiveModal::Gatcha:
                    // Gatcha system (full screen in content area)
                    m_GameState->GetGatchaSystem().RenderSummonUI(nullptr, m_GameState);
                    break;

                case ActiveModal::Achievements:
                    // Achievements (full screen in content area)
                    m_GameState->RenderAchievements(nullptr);
                    break;

                case ActiveModal::Statistics:
                    // Statistics (full screen in content area)
                    m_GameState->RenderStatistics(nullptr);
                    break;

                case ActiveModal::Milestones:
                    // Milestones (full screen in content area)
                    m_GameState->RenderMilestones(nullptr);
                    break;

                case ActiveModal::Challenges:
                    // Challenges (full screen in content area)
                    // FIX: RenderChallenges is in GameState, not ChallengeManager
                    m_GameState->RenderChallenges(nullptr);
                    break;

                case ActiveModal::EssenceShop:
                    // Essence Shop (full screen in content area)
                    m_GameState->RenderEssenceShop(nullptr);
                    break;

                case ActiveModal::SingularityShop:
                    // Singularity Shop (full screen in content area)
                    m_GameState->RenderSingularityShop(nullptr);
                    break;

                case ActiveModal::Enhancement:
                    // Enhancement system (full screen in content area)
                    m_GameState->GetEnhancementSystem().RenderEnhancementUI(nullptr, m_GameState);
                    break;

                case ActiveModal::SpecializedSkills:
                    // Specialized skills progression (full screen in content area)
                    m_GameState->RenderSpecializedSkills(nullptr);
                    break;

                case ActiveModal::MoreMenu:
                default:
                    // Fallback: show stations
                    m_GameState->RenderStationsContent();
                    break;
            }

            ImGui::EndChild();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void UIManager::RenderOverlays(Renderer* renderer) {
    // Render popups and notifications on top of everything
    // Main modal windows are now rendered in the content area (tab-based navigation)

    // Handle "More" menu as a popup
    if (m_GameState->m_ShowMoreMenu) {
        ImGui::OpenPopup("MoreMenuPopup");
    }

    if (ImGui::BeginPopup("MoreMenuPopup")) {
        // Menu items switch to different tabs
        if (ImGui::MenuItem("Achievements")) {
            m_GameState->SetActiveModal(ActiveModal::Achievements);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Statistics")) {
            m_GameState->SetActiveModal(ActiveModal::Statistics);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Milestones")) {
            m_GameState->SetActiveModal(ActiveModal::Milestones);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Challenges")) {
            m_GameState->SetActiveModal(ActiveModal::Challenges);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Essence Shop")) {
            m_GameState->SetActiveModal(ActiveModal::EssenceShop);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Singularity Shop")) {
            m_GameState->SetActiveModal(ActiveModal::SingularityShop);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Spaceship")) {
            m_GameState->SetActiveModal(ActiveModal::Spaceship);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Gatcha")) {
            m_GameState->SetActiveModal(ActiveModal::Gatcha);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Skills")) {
            m_GameState->SetActiveModal(ActiveModal::Skills);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Specialized Skills")) {
            m_GameState->SetActiveModal(ActiveModal::SpecializedSkills);
            m_GameState->m_ShowMoreMenu = false;
        }
        if (ImGui::MenuItem("Enhancement")) {
            m_GameState->SetActiveModal(ActiveModal::Enhancement);
            m_GameState->m_ShowMoreMenu = false;
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Close")) {
            m_GameState->m_ShowMoreMenu = false;
        }
        ImGui::EndPopup();
    }

    // Render active event (if any) - this can stay as an overlay
    RenderActiveEvent();

    // Render notifications (always on top)
    RenderAchievementNotifications();
    RenderMilestoneNotifications();
    m_GameState->GetUnlockManager().RenderNotifications(renderer);
}

// --- Helpers ---

void UIManager::DrawResourceCounter(const char* label, double value, const ImVec4& color) const {
    ImGui::PushStyleColor(ImGuiCol_Text, UITheme::ColorTextDim);
    ImGui::Text("%s", label);
    ImGui::PopStyleColor();

    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::SetWindowFontScale(1.2f);
    ImGui::Text("%s", GameUtils::FormatNumber(value, m_GameState->m_NumberFormat).c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
}

bool UIManager::DrawNavButton(const char* label, bool isActive, const ImVec4& activeColor, float width, float height) {
    ImVec4 btnColor = isActive ? activeColor : ImVec4(0.2f, 0.2f, 0.2f, 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, btnColor);

    bool clicked = ImGui::Button(label, ImVec2(width, height));

    ImGui::PopStyleColor();

    // Active Indicator Line (Draw glow/line at the bottom of the button)
    if (isActive) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();

        ImVec2 line_p0 = ImVec2(p_min.x, p_max.y - 2.0f);
        ImVec2 line_p1 = ImVec2(p_max.x, p_max.y);

        ImGui::GetWindowDrawList()->AddRectFilled(line_p0, line_p1, ImGui::GetColorU32(activeColor));
    }

    return clicked;
}
// --- Notification Overlays ---

void UIManager::RenderActiveEvent() const {
    m_GameState->RenderActiveEvent(nullptr);
}

void UIManager::RenderAchievementNotifications() const {
    m_GameState->RenderAchievementNotifications(nullptr);
}

void UIManager::RenderMilestoneNotifications() const {
    m_GameState->RenderMilestoneNotifications(nullptr);
}