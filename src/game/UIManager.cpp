#include "UIManager.h"
#include "ImGuiUtils.h"
#include "imgui.h"
#include "GameUtils.h"
#include <string>

UIManager::UIManager(GameState* state) : m_GameState(state) {}

void UIManager::Initialize() {
    UITheme::SetupStyle();
}

void UIManager::Render(Renderer* renderer) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar |
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
                                    ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    if (ImGui::Begin("MainLayout", nullptr, window_flags)) {
        ImGui::PopStyleVar();

        RenderTopBar();
        RenderMainContent();
        RenderBottomNavigation();
        RenderOverlays(renderer);

        ImGui::End();
    } else {
        ImGui::PopStyleVar();
    }
}

void UIManager::RenderTopBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, UITheme::TopBarHeight));

    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    if (ImGui::Begin("TopBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {
        auto* drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetWindowPos();
        p0.y += UITheme::TopBarHeight - 2.0f;
        ImVec2 p1 = ImVec2(p0.x + viewport->Size.x, p0.y + 2.0f);
        drawList->AddRectFilled(p0, p1, ImGui::GetColorU32(UITheme::ColorAccent));

        ImGui::Columns(5, "ResCols", false);

        DrawResourceCounter("Qubits", m_GameState->GetResource(QuantumResource::Qubits), ToImVec4(Color::QuantumBlue()));
        ImGui::NextColumn();
        DrawResourceCounter("Coherence", m_GameState->m_Coherence, ToImVec4(Color::CoherenceGreen()));
        ImGui::NextColumn();
        DrawResourceCounter("Entanglement", m_GameState->GetResource(QuantumResource::Entanglement), ToImVec4(Color::EntanglementOrange()));
        ImGui::NextColumn();
        DrawResourceCounter("Photons", m_GameState->GetTimeline().photons, UITheme::ColorPrimary);
        ImGui::NextColumn();
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

    if (ImGui::Begin("BottomNav", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {

        // Draw Top Border Glow
        auto* drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetWindowPos();
        ImVec2 p1 = ImVec2(p0.x + viewport->Size.x, p0.y + 2.0f);
        drawList->AddRectFilled(p0, p1, ImGui::GetColorU32(UITheme::ColorAccent));

        // Navigation Buttons
        float width = ImGui::GetContentRegionAvail().x;
        int btnCount = 5;
        float btnHeight = UITheme::BottomBarHeight - 20.0f;

        float btnWidth = (width - (UITheme::ItemSpacing * (btnCount - 1))) / btnCount;

        // === CRITICAL FIX IS HERE ===
        ImGui::SetCursorPosY(10.0f);        // Move cursor down
        ImGui::Dummy(ImVec2(0.0f, 0.0f));   // <--- THIS LINE PREVENTS THE CRASH
        // ============================

        bool isBaseState = (m_GameState->GetActiveModal() == ActiveModal::None);
        if (DrawNavButton("STATIONS", isBaseState, UITheme::ColorAccent, btnWidth, btnHeight)) {
            m_GameState->SetActiveModal(ActiveModal::None);
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        bool showingResearch = (m_GameState->GetActiveModal() == ActiveModal::Research);
        if (DrawNavButton("RESEARCH", showingResearch, UITheme::ColorPrimary, btnWidth, btnHeight)) {
            m_GameState->SetActiveModal(showingResearch ? ActiveModal::None : ActiveModal::Research);
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        bool showingBuyables = (m_GameState->GetActiveModal() == ActiveModal::Buyables);
        if (DrawNavButton("UPGRADES", showingBuyables, UITheme::ColorSuccess, btnWidth, btnHeight)) {
            m_GameState->SetActiveModal(showingBuyables ? ActiveModal::None : ActiveModal::Buyables);
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        bool showingCombat = (m_GameState->GetActiveModal() == ActiveModal::Combat);
        if (DrawNavButton("COMBAT", showingCombat, UITheme::ColorDanger, btnWidth, btnHeight)) {
            if(!showingCombat) m_GameState->StartRandomCombat();
            m_GameState->SetActiveModal(showingCombat ? ActiveModal::None : ActiveModal::Combat);
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

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

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

    if (ImGui::Begin("MainContent", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove)) {
        if (ImGui::BeginChild("ScrollContent", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            m_GameState->RenderStationsContent();
            ImGui::EndChild();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void UIManager::RenderOverlays(Renderer* renderer) {
    if (m_GameState->m_ShowMoreMenu) {
        ImGui::OpenPopup("MoreMenuPopup");
    }

    if (ImGui::BeginPopup("MoreMenuPopup")) {
        if (ImGui::MenuItem("Achievements")) { m_GameState->SetActiveModal(ActiveModal::Achievements); }
        if (ImGui::MenuItem("Statistics")) { m_GameState->SetActiveModal(ActiveModal::Statistics); }
        if (ImGui::MenuItem("Milestones")) { m_GameState->SetActiveModal(ActiveModal::Milestones); }
        if (ImGui::MenuItem("Challenges")) { m_GameState->SetActiveModal(ActiveModal::Challenges); }
        if (ImGui::MenuItem("Essence Shop")) { m_GameState->SetActiveModal(ActiveModal::EssenceShop); }
        if (ImGui::MenuItem("Singularity Shop")) { m_GameState->SetActiveModal(ActiveModal::SingularityShop); }
        if (ImGui::MenuItem("Spaceship")) { m_GameState->SetActiveModal(ActiveModal::Spaceship); }
        if (ImGui::MenuItem("Gatcha")) { m_GameState->SetActiveModal(ActiveModal::Gatcha); }
        if (ImGui::MenuItem("Skills")) { m_GameState->SetActiveModal(ActiveModal::Skills); }
        if (ImGui::MenuItem("Enhancement")) { m_GameState->SetActiveModal(ActiveModal::Enhancement); }
        ImGui::Separator();
        if (ImGui::MenuItem("Close")) m_GameState->SetActiveModal(ActiveModal::None);
        ImGui::EndPopup();
    }

    RenderActiveEvent(renderer);
    RenderAchievements(renderer);
    RenderStatistics(renderer);
    RenderResearchTree(renderer);
    RenderMilestones(renderer);
    RenderBuyables(renderer);
    RenderChallenges(renderer);
    RenderEssenceShop(renderer);
    RenderSingularityShop(renderer);
    RenderSpaceship(renderer);
    RenderCombat(renderer);
    RenderGatcha(renderer);
    RenderSkillTree(renderer);

    if (m_GameState->m_ShowEnhancement) {
        m_GameState->m_EnhancementSystem.RenderEnhancementUI(renderer, m_GameState);
    }

    RenderAchievementNotifications(renderer);
    RenderMilestoneNotifications(renderer);
    m_GameState->m_UnlockManager.RenderNotifications(renderer);
}

// --- Helpers ---

void UIManager::DrawResourceCounter(const char* label, double value, const ImVec4& color) {
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

    if (isActive) {
        ImVec2 p_min = ImGui::GetItemRectMin();
        ImVec2 p_max = ImGui::GetItemRectMax();
        ImVec2 line_p0 = ImVec2(p_min.x, p_max.y - 2.0f);
        ImVec2 line_p1 = ImVec2(p_max.x, p_max.y);
        ImGui::GetWindowDrawList()->AddRectFilled(line_p0, line_p1, ImGui::GetColorU32(activeColor));
    }

    return clicked;
}

void UIManager::RenderActiveEvent(Renderer* renderer) { m_GameState->RenderActiveEvent(renderer); }
void UIManager::RenderAchievements(Renderer* renderer) { m_GameState->RenderAchievements(renderer); }
void UIManager::RenderStatistics(Renderer* renderer) { m_GameState->RenderStatistics(renderer); }
void UIManager::RenderResearchTree(Renderer* renderer) { m_GameState->RenderResearchTree(renderer); }
void UIManager::RenderMilestones(Renderer* renderer) { m_GameState->RenderMilestones(renderer); }
void UIManager::RenderBuyables(Renderer* renderer) { m_GameState->RenderBuyables(renderer); }
void UIManager::RenderChallenges(Renderer* renderer) { m_GameState->RenderChallenges(renderer); }
void UIManager::RenderEssenceShop(Renderer* renderer) { m_GameState->RenderEssenceShop(renderer); }
void UIManager::RenderSingularityShop(Renderer* renderer) { m_GameState->RenderSingularityShop(renderer); }
void UIManager::RenderSpaceship(Renderer* renderer) { m_GameState->RenderSpaceship(renderer); }
void UIManager::RenderCombat(Renderer* renderer) { m_GameState->RenderCombat(renderer); }
void UIManager::RenderGatcha(Renderer* renderer) { m_GameState->RenderGatcha(renderer); }
void UIManager::RenderSkillTree(Renderer* renderer) { m_GameState->RenderSkillTree(renderer); }
void UIManager::RenderAchievementNotifications(Renderer* renderer) { m_GameState->RenderAchievementNotifications(renderer); }
void UIManager::RenderMilestoneNotifications(Renderer* renderer) { m_GameState->RenderMilestoneNotifications(renderer); }