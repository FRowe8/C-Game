#include "imgui.h"
#include "UIManager.h"
#include "ImGuiUtils.h"
#include <string>

UIManager::UIManager(GameState* state) : m_GameState(state) {}

void UIManager::Initialize() {
    UITheme::SetupStyle();
}

void UIManager::Render() {
    // 1. Setup Main Docking Window (Invisible container)
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | 
                                    ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | 
                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus | 
                                    ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("MainLayout", nullptr, window_flags);
    ImGui::PopStyleVar();

    // 2. Render Components
    RenderTopBar();
    RenderMainContent();
    RenderBottomNavigation();
    RenderOverlays();

    ImGui::End();
}

void UIManager::RenderTopBar() {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, UITheme::TopBarHeight));
    
    ImGui::PushStyleColor(ImGuiCol_WindowBg, UITheme::ColorPanelBg);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    
    if (ImGui::Begin("TopBar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking)) {
        
        // Draw Bottom Border Glow
        auto* drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        p0.y += UITheme::TopBarHeight - 2.0f;
        ImVec2 p1 = ImVec2(p0.x + viewport->Size.x, p0.y + 2.0f);
        drawList->AddRectFilled(p0, p1, ImGui::GetColorU32(UITheme::ColorAccent));

        // Resources Columns
        ImGui::Columns(3, "ResCols", false);
        
        DrawResourceCounter("Qubits", m_GameState->GetResource(QuantumResource::Qubits), UITheme::ColorAccent);
        ImGui::NextColumn();
        
        DrawResourceCounter("Coherence", 100.0, UITheme::ColorSuccess); // Use real coherence value getter
        ImGui::NextColumn();
        
        DrawResourceCounter("Photons", m_GameState->GetTimeline().photons, UITheme::ColorPrimary);
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
    
    if (ImGui::Begin("BottomNav", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDocking)) {
        
        // Draw Top Border Glow
        auto* drawList = ImGui::GetWindowDrawList();
        ImVec2 p0 = ImGui::GetWindowPos();
        ImVec2 p1 = ImVec2(p0.x + viewport->Size.x, p0.y + 2.0f);
        drawList->AddRectFilled(p0, p1, ImGui::GetColorU32(UITheme::ColorAccent));

        // Navigation Buttons
        // We use a simple layout logic here. Calculate button width based on screen count.
        float width = ImGui::GetContentRegionAvail().x;
        int btnCount = 5;
        float btnWidth = (width - (UITheme::ItemSpacing * (btnCount - 1))) / btnCount;
        float btnHeight = UITheme::BottomBarHeight - 20.0f; // Padding

        ImGui::SetCursorPosY(10.0f); // Padding top

        // 1. Stations (Home)
        // Check if we are in "Base" state (no overlays open)
        bool isBaseState = !m_GameState->m_ShowResearch && !m_GameState->m_ShowAchievements; // etc
        if (DrawNavButton("STATIONS", isBaseState, UITheme::ColorAccent)) {
            // Close all overlays
            m_GameState->m_ShowResearch = false;
            m_GameState->m_ShowAchievements = false;
            m_GameState->m_ShowStats = false;
            // ... close others
        }
        ImGui::SameLine();

        // 2. Research
        if (DrawNavButton("RESEARCH", m_GameState->m_ShowResearch, UITheme::ColorPrimary)) {
            m_GameState->m_ShowResearch = !m_GameState->m_ShowResearch;
        }
        ImGui::SameLine();

        // 3. Upgrades (Buyables)
        if (DrawNavButton("UPGRADES", m_GameState->m_ShowBuyables, UITheme::ColorSuccess)) {
            m_GameState->m_ShowBuyables = !m_GameState->m_ShowBuyables;
        }
        ImGui::SameLine();

        // 4. Combat/Map
        if (DrawNavButton("COMBAT", m_GameState->m_ShowCombat, UITheme::ColorDanger)) {
            if(!m_GameState->m_ShowCombat) m_GameState->StartRandomCombat();
            m_GameState->m_ShowCombat = !m_GameState->m_ShowCombat;
        }
        ImGui::SameLine();

        // 5. Menu (More)
        if (DrawNavButton("MENU", m_GameState->m_ShowMoreMenu, UITheme::ColorText)) {
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
        RenderStationsPanel();
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
}

void UIManager::RenderStationsPanel() {
    // We can call the existing logic, but we wrap it to ensure it fits our new layout
    // Assuming RenderStations logic is refactored or we call specific parts.
    // Ideally, we move the station loop here.
    
    // For now, we can adapt the GameState logic:
    // Access m_GameState->m_Stations manually (requires friendship or public access)
    
    // Placeholder for actual station list loop (Copy loop from GameState::RenderStations but remove Window creation)
    // ...
    // Since GameState::RenderStations creates its own window, we should REFACTOR GameState to 
    // expose a method that just draws the *widgets*, not the window.
    
    // TEMPORARY: Just telling user to utilize existing logic, but strictly speaking,
    // GameState::RenderStations needs to be stripped of ImGui::Begin/End.
    // See step 4.
    m_GameState->RenderStationsContent(); 
}

void UIManager::RenderOverlays() {
    // Render popups on top of everything
    if (m_GameState->m_ShowResearch) m_GameState->RenderResearchTree(nullptr);
    if (m_GameState->m_ShowAchievements) m_GameState->RenderAchievements(nullptr);
    // ... add others
    
    // Handle "More" menu as a proper modal or popup
    if (m_GameState->m_ShowMoreMenu) {
        ImGui::OpenPopup("MoreMenuPopup");
    }
    
    if (ImGui::BeginPopup("MoreMenuPopup")) {
        if (ImGui::MenuItem("Achievements")) m_GameState->m_ShowAchievements = true;
        if (ImGui::MenuItem("Statistics")) m_GameState->m_ShowStats = true;
        if (ImGui::MenuItem("Settings")) { /* TODO */ }
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
    ImGui::Text("%s", GameUtils::FormatNumber(value).c_str());
    ImGui::SetWindowFontScale(1.0f);
    ImGui::PopStyleColor();
}

bool UIManager::DrawNavButton(const char* label, bool isActive, const ImVec4& activeColor) {
    ImVec4 btnColor = isActive ? activeColor : ImVec4(0.2f, 0.2f, 0.2f, 0.5f);
    ImGui::PushStyleColor(ImGuiCol_Button, btnColor);
    
    // Calculate size (Fit width, full height)
    float width = ImGui::GetContentRegionAvail().x; // This works if in columns, otherwise manual calc needed
    // Actually simpler to use the layout logic from RenderBottomNavigation
    
    bool clicked = ImGui::Button(label, ImVec2(-1, 50.0f)); // -1 width fills column/space
    
    ImGui::PopStyleColor();
    
    // Active Indicator Line
    if (isActive) {
        ImVec2 p0 = ImGui::GetItemRectMin();
        ImVec2 p1 = ImGui::GetItemRectMax();
        p0.y = p1.y + 2.0f;
        p1.y += 4.0f;
        ImGui::GetWindowDrawList()->AddRectFilled(p0, p1, ImGui::GetColorU32(activeColor));
    }
    
    return clicked;
}