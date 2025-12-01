#include "UIManager.h"
#include "ImGuiUtils.h"
#include "imgui.h"
#include "GameUtils.h"
#include <string>

// NOTE: This implementation assumes the UIManager.h DrawNavButton signature
// has been updated to include width and height parameters.

UIManager::UIManager(GameState* state) : m_GameState(state) {}

void UIManager::Initialize() {
    // Check if the UITheme header is included in the project's include path
    UITheme::SetupStyle();
}

void UIManager::Render() {
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
        RenderOverlays();

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
        float btnWidth = (width - (UITheme::ItemSpacing * (btnCount - 1))) / btnCount;

        ImGui::SetCursorPosY(10.0f); // Padding top

        // 1. Stations (Home)
        // Check if we are in "Base" state (no modal overlays open)
        bool isBaseState = !m_GameState->m_ShowResearch && !m_GameState->m_ShowAchievements && !m_GameState->m_ShowCombat; // etc
        if (DrawNavButton("STATIONS", isBaseState, UITheme::ColorAccent, btnWidth, btnHeight)) {
            // Close all overlays
            m_GameState->m_ShowResearch = false;
            m_GameState->m_ShowAchievements = false;
            m_GameState->m_ShowStats = false;
            m_GameState->m_ShowCombat = false;
            // ... close others
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing); // Use ItemSpacing for layout consistency

        // 2. Research
        if (DrawNavButton("RESEARCH", m_GameState->m_ShowResearch, UITheme::ColorPrimary, btnWidth, btnHeight)) {
            m_GameState->m_ShowResearch = !m_GameState->m_ShowResearch;
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        // 3. Upgrades (Buyables)
        if (DrawNavButton("UPGRADES", m_GameState->m_ShowBuyables, UITheme::ColorSuccess, btnWidth, btnHeight)) {
            m_GameState->m_ShowBuyables = !m_GameState->m_ShowBuyables;
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        // 4. Combat/Map
        if (DrawNavButton("COMBAT", m_GameState->m_ShowCombat, UITheme::ColorDanger, btnWidth, btnHeight)) {
            if(!m_GameState->m_ShowCombat) m_GameState->StartRandomCombat();
            m_GameState->m_ShowCombat = !m_GameState->m_ShowCombat;
        }
        ImGui::SameLine(0.0f, UITheme::ItemSpacing);

        // 5. Menu (More)
        if (DrawNavButton("MENU", m_GameState->m_ShowMoreMenu, UITheme::ColorText, btnWidth, btnHeight)) {
            m_GameState->m_ShowMoreMenu = !m_GameState->m_ShowMoreMenu;
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
        // Render the actual station content inside this scrollable panel
        if (ImGui::BeginChild("ScrollContent", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            m_GameState->RenderStationsContent();
            ImGui::EndChild();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void UIManager::RenderOverlays() {
    // Render popups on top of everything
    // NOTE: For simplicity, overlays are still rendered by GameState's original functions.

    // Handle "More" menu as a proper modal or popup
    if (m_GameState->m_ShowMoreMenu) {
        ImGui::OpenPopup("MoreMenuPopup");
    }

    if (ImGui::BeginPopup("MoreMenuPopup")) {
        // Render button items from GameState logic
        if (ImGui::MenuItem("Achievements")) { m_GameState->m_ShowAchievements = true; m_GameState->m_ShowMoreMenu = false; }
        if (ImGui::MenuItem("Statistics")) { m_GameState->m_ShowStats = true; m_GameState->m_ShowMoreMenu = false; }
        ImGui::Separator();
        if (ImGui::MenuItem("Close")) m_GameState->m_ShowMoreMenu = false;
        ImGui::EndPopup();
    }
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