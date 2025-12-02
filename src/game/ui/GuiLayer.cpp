#include "GuiLayer.h"
#include "TutorialOverlay.h"
#include "GameState.h"
#include "Renderer.h"
#include "ImGuiUtils.h"
#include "GameUtils.h"
#include "Logger.h"
#include "Research.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>

namespace UI {

// =============================================================================
// GuiLayer Implementation
// =============================================================================

GuiLayer::GuiLayer() = default;
GuiLayer::~GuiLayer() = default;

void GuiLayer::Initialize() {
    Log::Info("Initializing GuiLayer...");

    // Create all views
    m_ResourceView = std::make_unique<ResourceView>();
    m_NavigationView = std::make_unique<NavigationView>();
    m_StationView = std::make_unique<StationView>();

    m_AchievementView = std::make_unique<AchievementView>();
    m_StatisticsView = std::make_unique<StatisticsView>();
    m_ResearchView = std::make_unique<ResearchView>();
    m_MilestoneView = std::make_unique<MilestoneView>();
    m_BuyablesView = std::make_unique<BuyablesView>();
    m_ChallengeView = std::make_unique<ChallengeView>();
    m_EssenceShopView = std::make_unique<EssenceShopView>();
    m_SingularityShopView = std::make_unique<SingularityShopView>();
    m_SpaceshipView = std::make_unique<SpaceshipView>();
    m_CombatView = std::make_unique<CombatView>();
    m_GatchaView = std::make_unique<GatchaView>();
    m_SkillTreeView = std::make_unique<SkillTreeView>();
    m_EnhancementView = std::make_unique<EnhancementView>();
    m_SpecializedSkillsView = std::make_unique<SpecializedSkillsView>();

    // Tutorial System
    m_TutorialOverlay = std::make_unique<TutorialOverlay>();

    Log::Info("GuiLayer initialized successfully");
}

void GuiLayer::Update(GameState* state) {
    // Update tutorial system
    if (m_TutorialOverlay) {
        m_TutorialOverlay->Update(state);
    }
}

void GuiLayer::Render(GameState* state, Renderer* renderer) {
    // Render core UI (always visible)
    m_ResourceView->Render(state, renderer);
    m_NavigationView->Render(state, renderer);

    // Phase 1.2: State machine - render based on active modal
    ActiveModal activeModal = state->GetActiveModal();

    switch (activeModal) {
        case ActiveModal::None:
            // Show stations view (default/base state)
            m_StationView->Render(state, renderer);
            break;

        case ActiveModal::Achievements:
            m_AchievementView->Render(state, renderer);
            break;

        case ActiveModal::Statistics:
            m_StatisticsView->Render(state, renderer);
            break;

        case ActiveModal::Research:
            m_ResearchView->Render(state, renderer);
            break;

        case ActiveModal::Milestones:
            m_MilestoneView->Render(state, renderer);
            break;

        case ActiveModal::Buyables:
            m_BuyablesView->Render(state, renderer);
            break;

        case ActiveModal::Challenges:
            m_ChallengeView->Render(state, renderer);
            break;

        case ActiveModal::EssenceShop:
            m_EssenceShopView->Render(state, renderer);
            break;

        case ActiveModal::SingularityShop:
            m_SingularityShopView->Render(state, renderer);
            break;

        case ActiveModal::Spaceship:
            m_SpaceshipView->Render(state, renderer);
            break;

        case ActiveModal::Combat:
            m_CombatView->Render(state, renderer);
            break;

        case ActiveModal::Gatcha:
            m_GatchaView->Render(state, renderer);
            break;

        case ActiveModal::Skills:
            m_SkillTreeView->Render(state, renderer);
            break;

        case ActiveModal::Enhancement:
            m_EnhancementView->Render(state, renderer);
            break;

        case ActiveModal::SpecializedSkills:
            m_SpecializedSkillsView->Render(state, renderer);
            break;

        case ActiveModal::MoreMenu:
            // MoreMenu is handled in NavigationView, no separate view needed
            m_StationView->Render(state, renderer);
            break;
    }

    // Render always-visible overlays
    RenderActiveEvent(state, renderer);
    RenderNotifications(state, renderer);

    // Render tutorial overlay (should be last so it's on top of everything)
    if (m_TutorialOverlay) {
        m_TutorialOverlay->Render(state, renderer);
    }
}

void GuiLayer::RenderNotifications(GameState* state, Renderer* renderer) {
    (void)renderer;

    // Achievement notifications
    if (!state->m_RecentUnlocks.empty()) {
        f32 screenWidth = ImGui::GetIO().DisplaySize.x;
        f32 notifWidth = 300.0f;
        f32 notifHeight = 80.0f;
        f32 notifX = screenWidth - notifWidth - 20.0f;
        f32 notifY = 220.0f;

        ImGui::SetNextWindowPos(ImVec2(notifX, notifY), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(notifWidth, notifHeight), ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.8f, 0.3f, 0.95f));

        if (ImGui::Begin("##AchievementNotif", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                         ImGuiWindowFlags_NoCollapse)) {

            Achievement* ach = state->GetAchievement(state->m_RecentUnlocks[0]);
            if (ach) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "ACHIEVEMENT UNLOCKED!");
                ImGui::Separator();
                ImGui::Text("%s", ach->name.c_str());
                ImGui::TextWrapped("%s", ach->description.c_str());
            }
        }
        ImGui::End();
        ImGui::PopStyleColor();
    }

    // Milestone notifications
    // Use GetRecentCompletions() instead of iterating all milestones
    auto recentCompletions = state->GetMilestoneSystem().GetRecentCompletions();
    if (!recentCompletions.empty()) {
        const Milestone* recent = recentCompletions.back();

        f32 screenWidth = ImGui::GetIO().DisplaySize.x;
        f32 screenHeight = ImGui::GetIO().DisplaySize.y;
        f32 notifWidth = 400.0f;
        f32 notifHeight = 90.0f;

        ImVec2 notifPos(screenWidth - notifWidth - 20.0f, screenHeight - notifHeight - 100.0f);

        ImGui::SetNextWindowPos(notifPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(notifWidth, notifHeight), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.95f);

        f32 pulse = static_cast<f32>(0.9 + 0.1 * std::sin(state->m_TotalTimePlayed * 4.0));
        Color bgColor = Color::QuantumBlue() * pulse;

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(bgColor));
        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::CoherenceGreen()));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 12.0f));

        if (ImGui::Begin("##MilestoneNotif", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {

            ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "🎯 Milestone Completed!");
            ImGui::TextColored(ToImVec4(Color::White()), "%s", recent->name.c_str());
            ImGui::TextColored(ToImVec4(Color(1.0f, 1.0f, 0.6f, 1.0f)), "%s", recent->rewardDescription.c_str());
        }
        ImGui::End();
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }
}

void GuiLayer::RenderActiveEvent(GameState* state, Renderer* renderer) {
    (void)renderer;

    QuantumEvent* activeEvent = state->GetActiveEvent();
    if (!activeEvent || !activeEvent->active) return;

    f32 screenWidth = ImGui::GetIO().DisplaySize.x;
    f32 eventBannerWidth = 400.0f;
    f32 eventBannerHeight = 80.0f;
    f32 eventBannerX = (screenWidth - eventBannerWidth) * 0.5f;
    f32 eventBannerY = 30.0f;

    ImGui::SetNextWindowPos(ImVec2(eventBannerX, eventBannerY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(eventBannerWidth, eventBannerHeight), ImGuiCond_Always);

    Color eventColor(0.8f, 0.2f, 0.8f, 0.95f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(eventColor));

    if (ImGui::Begin("##ActiveEventBanner", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse)) {

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "QUANTUM EVENT ACTIVE!");
        ImGui::Separator();
        ImGui::Text("%s", activeEvent->name.c_str());
        ImGui::Text("Time Remaining: %.1f s", activeEvent->timeRemaining);

        // Progress bar
        f32 progress = static_cast<f32>(activeEvent->timeRemaining / activeEvent->duration);
        ImGui::ProgressBar(progress, ImVec2(-1, 0));
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

// =============================================================================
// ResourceView Implementation
// =============================================================================

void ResourceView::Render(GameState* state, Renderer* renderer) {
    (void)renderer;

    // NOTE: This is extracted from GameState::RenderResources
    // The original implementation will be removed from GameState.cpp

    f32 topBarY = 10.0f;
    f32 topBarHeight = 80.0f;
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;

    ImGui::SetNextWindowPos(ImVec2(0, topBarY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenWidth, topBarHeight), ImGuiCond_Always);

    Color darkPanel = Color::DarkPanel();
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(darkPanel));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 10.0f));

    if (ImGui::Begin("##ResourceBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse)) {

        // Display resources in a horizontal layout
        f64 qubits = state->GetResource(QuantumResource::Qubits);
        f64 coherence = state->GetResource(QuantumResource::Coherence);
        f64 entanglement = state->GetResource(QuantumResource::Entanglement);
        f64 photons = state->GetPhotons();
        f64 singularities = state->GetTimeline().singularities;

        auto fmt = state->m_NumberFormat;

        // Qubits
        ImGui::TextColored(ToImVec4(Color::QuantumBlue()), "QUBITS");
        ImGui::SameLine();
        ImGui::Text("%s", GameUtils::FormatNumber(qubits, fmt).c_str());

        ImGui::SameLine(0.0f, 30.0f);

        // Coherence
        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "COHERENCE");
        ImGui::SameLine();
        ImGui::Text("%s", GameUtils::FormatNumber(coherence, fmt).c_str());

        ImGui::SameLine(0.0f, 30.0f);

        // Entanglement
        ImGui::TextColored(ToImVec4(Color::EntanglementOrange()), "ENTANGLEMENT");
        ImGui::SameLine();
        ImGui::Text("%s", GameUtils::FormatNumber(entanglement, fmt).c_str());

        ImGui::SameLine(0.0f, 30.0f);

        // Photons (prestige currency)
        ImGui::TextColored(ToImVec4(Color::Magenta()), "PHOTONS");
        ImGui::SameLine();
        ImGui::Text("%s", GameUtils::FormatNumber(photons, fmt).c_str());

        // Singularities (if any)
        if (singularities > 0) {
            ImGui::SameLine(0.0f, 30.0f);
            ImGui::TextColored(ToImVec4(Color(0.5f, 0.0f, 1.0f, 1.0f)), "SINGULARITIES");
            ImGui::SameLine();
            ImGui::Text("%s", GameUtils::FormatNumber(singularities, fmt).c_str());
        }
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();
}

// =============================================================================
// NavigationView Implementation
// =============================================================================

void NavigationView::Render(GameState* state, Renderer* renderer) {
    (void)renderer;

    // NOTE: This is extracted from GameState::RenderUI
    // The original implementation will be removed from GameState.cpp

    f32 navY = 100.0f;
    f32 navHeight = 80.0f;
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;

    ImGui::SetNextWindowPos(ImVec2(0, navY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(screenWidth, navHeight), ImGuiCond_Always);

    Color darkPanel = Color::DarkPanel();
    Color neonCyan = Color::NeonCyan();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(darkPanel));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

    ImVec2 moreBtnPos = ImVec2(0.0f, 0.0f);

    if (ImGui::Begin("##NavigationBar", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                     ImGuiWindowFlags_NoCollapse)) {

        // Draw the glowing cyan bottom border
        ImVec2 barStart = ImGui::GetWindowPos();
        ImVec2 barEnd(
            ImGui::GetWindowPos().x + ImGui::GetWindowSize().x,
            ImGui::GetWindowPos().y + ImGui::GetWindowSize().y
        );
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        draw_list->AddRectFilled(
            ImVec2(barStart.x, barEnd.y - 2.0f),
            ImVec2(barEnd.x, barEnd.y),
            ImGui::GetColorU32(ToImVec4(neonCyan * 0.6f))
        );

        // Navigation Buttons
        f32 btnWidth = 180.0f;
        f32 btnHeight = 60.0f;
        f32 spacing = 10.0f;
        f32 currentX = 15.0f;
        f32 btnY = (navHeight - btnHeight) * 0.5f;

        // Phase 1.2: Button structure uses ActiveModal enum for state machine
        struct NavButton {
            const char* label;
            ActiveModal modal;
            Color color;
        };

        NavButton navButtons[] = {
            {"BUYABLES", ActiveModal::Buyables, Color::ElectricBlue()},
            {"CHALLENGES", ActiveModal::Challenges, Color::Red()},
            {"ESSENCE", ActiveModal::EssenceShop, Color::Magenta()},
            {"RESEARCH", ActiveModal::Research, Color::QuantumPurple()},
            {"STATS", ActiveModal::Statistics, Color::EntanglementOrange()},
            {"MILESTONES", ActiveModal::Milestones, Color::NeonPink()},
        };

        ImGui::SetCursorPos(ImVec2(currentX, btnY));

        ActiveModal currentModal = state->GetActiveModal();

        for (int i = 0; i < 6; i++) {
            auto& btn = navButtons[i];

            if (currentX + btnWidth + spacing + 200.0f + 80.0f + 35.0f > screenWidth) {
                break;
            }

            bool active = (currentModal == btn.modal);
            Color btnColor = active ? btn.color : btn.color * 0.5f;

            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.25f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnColor * 0.5f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(btnColor * 0.7f));
            ImGui::PushStyleColor(ImGuiCol_Text, ToImVec4(Color::White()));

            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

            if (ImGui::Button(btn.label, ImVec2(btnWidth, btnHeight))) {
                // Phase 1.2: Use state machine - toggle between modal and None
                if (active) {
                    state->SetActiveModal(ActiveModal::None);
                } else {
                    state->SetActiveModal(btn.modal);
                }
            }

            // Draw custom border/glow
            if (active) {
                ImVec2 rectMin(
                    ImGui::GetItemRectMin().x - 2.0f,
                    ImGui::GetItemRectMin().y - 2.0f
                );
                ImVec2 rectMax(
                    ImGui::GetItemRectMax().x + 2.0f,
                    ImGui::GetItemRectMax().y + 2.0f
                );
                draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(btn.color * 0.9f)), 0.0f, 0, 2.0f);
            } else {
                ImVec2 rectMin = ImGui::GetItemRectMin();
                ImVec2 rectMax = ImGui::GetItemRectMax();
                draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(Color::DarkBorder())), 0.0f, 0, 1.0f);
            }

            ImGui::PopStyleVar();
            ImGui::PopStyleColor(4);

            if (i < 5) {
                ImGui::SameLine(0.0f, spacing);
            }
        }

        // MORE Menu Button
        RenderMoreMenu(state);
        moreBtnPos = ImGui::GetItemRectMin();

        // Boost Button
        RenderBoostButton(state);

        // Force window boundary extension
        ImGui::SetCursorPosY(navHeight);
        ImGui::Dummy(ImVec2(1.0f, 1.0f));
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor();

    // Render MORE Menu Popup
    if (state->m_ShowMoreMenu) {
        f32 menuWidth = 250.0f;
        f32 menuHeight = 490.0f;
        ImVec2 menuPos(moreBtnPos.x, navY + navHeight + 5.0f);

        ImGui::SetNextWindowPos(menuPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(menuWidth, menuHeight), ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(Color::DarkPanel()));
        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::NeonCyan() * 0.8f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

        if (ImGui::Begin("##MoreMenuPopup", &state->m_ShowMoreMenu,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar)) {

            f32 itemWidth = menuWidth - 20.0f;
            f32 itemHeight = 60.0f;

            // Phase 1.2: Use ActiveModal enum for state machine
            struct MoreButton {
                const char* label;
                ActiveModal modal;
                Color color;
            };

            MoreButton moreButtons[] = {
                {"ACHIEVEMENTS", ActiveModal::Achievements, Color::ElectricBlue()},
                {"SINGULARITY", ActiveModal::SingularityShop, Color(0.5f, 0.0f, 1.0f, 1.0f)},
                {"SPACESHIP", ActiveModal::Spaceship, Color(1.0f, 0.7f, 0.0f, 1.0f)},
                {"BATTLE", ActiveModal::Combat, Color(1.0f, 0.3f, 0.3f, 1.0f)},
                {"SUMMON", ActiveModal::Gatcha, Color(1.0f, 0.3f, 1.0f, 1.0f)},
                {"SKILLS", ActiveModal::Skills, Color(0.0f, 1.0f, 0.5f, 1.0f)},
                {"ENHANCE", ActiveModal::Enhancement, Color(0.8f, 0.6f, 0.2f, 1.0f)},
            };

            ActiveModal currentModal = state->GetActiveModal();

            for (size_t i = 0; i < 7; i++) {
                auto& btn = moreButtons[i];
                bool active = (currentModal == btn.modal);
                Color btnColor = active ? btn.color : Color(0.3f, 0.3f, 0.3f, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnColor * 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(btnColor * 0.7f));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnColor));

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

                if (ImGui::Button(btn.label, ImVec2(itemWidth, itemHeight))) {
                    // Phase 1.2: Use state machine - toggle between modal and None
                    if (active) {
                        state->SetActiveModal(ActiveModal::None);
                    } else {
                        state->SetActiveModal(btn.modal);
                    }
                    state->m_ShowMoreMenu = false;
                }

                ImGui::PopStyleVar();
                ImGui::PopStyleColor(4);

                if (i < 6) ImGui::Spacing();
            }
        }
        ImGui::End();

        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }
}

void NavigationView::RenderMoreMenu(GameState* state) {
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;
    f32 navHeight = 80.0f;
    f32 btnHeight = 60.0f;
    f32 btnY = (navHeight - btnHeight) * 0.5f;

    f32 boostBtnWidth = 200.0f;
    f32 moreBtnWidth = 80.0f;
    f32 moreBtnX = screenWidth - boostBtnWidth - moreBtnWidth - 35.0f;

    ImGui::SetCursorPosX(moreBtnX);
    ImGui::SetCursorPosY(btnY);

    Color moreColor = state->m_ShowMoreMenu ? Color::NeonCyan() : Color(0.3f, 0.4f, 0.5f, 1.0f);
    Color moreBgColor = state->m_ShowMoreMenu ? (moreColor * 0.4f) : (moreColor * 0.35f);

    ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(moreBgColor));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(moreColor * 0.6f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(moreColor * 0.8f));
    ImGui::PushStyleColor(ImGuiCol_Text, ToImVec4(Color::Transparent()));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    if (ImGui::Button("##MoreMenuBtn", ImVec2(moreBtnWidth, btnHeight))) {
        state->m_ShowMoreMenu = !state->m_ShowMoreMenu;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Draw custom border/glow
    if (state->m_ShowMoreMenu) {
        ImVec2 rectMin(
            ImGui::GetItemRectMin().x - 2.0f,
            ImGui::GetItemRectMin().y - 2.0f
        );
        ImVec2 rectMax(
            ImGui::GetItemRectMax().x + 2.0f,
            ImGui::GetItemRectMax().y + 2.0f
        );
        draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(Color::NeonCyan() * 0.9f)), 0.0f, 0, 2.0f);
    } else {
        ImVec2 rectMin = ImGui::GetItemRectMin();
        ImVec2 rectMax = ImGui::GetItemRectMax();
        draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(Color(0.4f, 0.5f, 0.6f, 0.8f))), 0.0f, 0, 1.0f);
    }

    // Draw Hamburger Icon
    ImVec2 btnMin = ImGui::GetItemRectMin();
    f32 lineWidth = 30.0f;
    f32 lineHeight = 3.0f;
    f32 lineSpacing = 8.0f;
    f32 lineStartX = btnMin.x + (moreBtnWidth - lineWidth) * 0.5f;
    f32 lineStartY = btnMin.y + (btnHeight - (lineHeight * 3 + lineSpacing * 2)) * 0.5f;
    Color lineColor = state->m_ShowMoreMenu ? Color::NeonCyan() : Color(0.8f, 0.9f, 1.0f, 1.0f);

    for (int i = 0; i < 3; i++) {
        draw_list->AddRectFilled(
            ImVec2(lineStartX, lineStartY + i * (lineHeight + lineSpacing)),
            ImVec2(lineStartX + lineWidth, lineStartY + i * (lineHeight + lineSpacing) + lineHeight),
            ImGui::GetColorU32(ToImVec4(lineColor))
        );
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
}

void NavigationView::RenderBoostButton(GameState* state) {
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;
    f32 navHeight = 80.0f;
    f32 btnHeight = 60.0f;
    f32 btnY = (navHeight - btnHeight) * 0.5f;

    f32 boostBtnWidth = 200.0f;
    f32 boostBtnX = screenWidth - boostBtnWidth - 25.0f;

    ImGui::SetCursorPosX(boostBtnX);
    ImGui::SetCursorPosY(btnY);

    // Determine boost button state
    Color boostColor;
    std::string boostText;
    bool boostClickable = false;
    bool boostDisabled = state->GetChallengeManager().HasModifier(ChallengeModifier::NoBoost);

    if (boostDisabled) {
        boostColor = Color(0.3f, 0.3f, 0.3f, 1.0f);
        boostText = "BOOST DISABLED";
    } else if (state->m_BoostActive) {
        boostColor = Color::CoherenceGreen();
        boostText = "BOOST ACTIVE! " + std::to_string(static_cast<i32>(state->m_BoostTimeRemaining)) + "s";
    } else if (state->m_BoostCooldownRemaining > 0) {
        boostColor = Color(0.4f, 0.4f, 0.4f, 1.0f);
        boostText = "COOLDOWN " + std::to_string(static_cast<i32>(state->m_BoostCooldownRemaining)) + "s";
    } else {
        boostColor = Color::NeonCyan();
        boostText = "BOOST (2x)";
        boostClickable = true;
    }

    Color boostBgColor = boostColor * 0.3f;
    Color boostBorderColor = state->m_BoostActive ? boostColor * 0.9f : (boostClickable ? boostColor * 0.7f : Color(0.3f, 0.3f, 0.3f, 1.0f));

    ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(boostBgColor));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(boostColor * 0.5f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(boostColor * 0.7f));
    ImGui::PushStyleColor(ImGuiCol_Text, ToImVec4(Color::White()));

    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);

    if (ImGui::Button(boostText.c_str(), ImVec2(boostBtnWidth, btnHeight))) {
        if (boostClickable) {
            state->m_BoostActive = true;
            state->m_BoostTimeRemaining = state->m_BoostDuration;
        }
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 rectMin = ImGui::GetItemRectMin();
    ImVec2 rectMax = ImGui::GetItemRectMax();

    if (state->m_BoostActive) {
        ImVec2 glowMin(rectMin.x - 2.0f, rectMin.y - 2.0f);
        ImVec2 glowMax(rectMax.x + 2.0f, rectMax.y + 2.0f);
        draw_list->AddRect(glowMin, glowMax, ImGui::GetColorU32(ToImVec4(boostColor * 0.9f)), 0.0f, 0, 2.0f);
    } else {
        draw_list->AddRect(rectMin, rectMax, ImGui::GetColorU32(ToImVec4(boostBorderColor)), 0.0f, 0, 1.0f);
    }

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(4);
}

// =============================================================================
// StationView Implementation
// =============================================================================

void StationView::Render(GameState* state, Renderer* renderer) {
    (void)renderer;

    // NOTE: This is extracted from GameState::RenderStationsContent
    // This content is rendered inside a window created by UIManager

    f64 currentQubits = state->GetResource(QuantumResource::Qubits);
    f64 effectiveBonus = state->GetProductionMultiplier(QuantumResource::Qubits);
    bool canUpgrade = !state->GetChallengeManager().HasModifier(ChallengeModifier::NoUpgrades);

    // Iterate through all stations
    for (size_t i = 0; i < state->m_Stations.size(); i++) {
        auto& station = state->m_Stations[i];

        if (station.unlocked) {
            RenderUnlockedStation(state, station, i);
        } else {
            RenderLockedStation(state, station, i);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
    }

    // Prestige button at the end
    RenderPrestigeButton(state);
}

void StationView::RenderUnlockedStation(GameState* state, ResearchStation& station, size_t index) {
    f64 currentQubits = state->GetResource(QuantumResource::Qubits);
    f64 effectiveBonus = state->GetProductionMultiplier(QuantumResource::Qubits);
    bool canUpgrade = !state->GetChallengeManager().HasModifier(ChallengeModifier::NoUpgrades);

    // Station Display Header
    Color tierColor = state->GetStationTierColor(station.level);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(tierColor.r, tierColor.g, tierColor.b, 1.0f));
    ImGui::Text(">> %s (Level %d)", station.name.c_str(), station.level);
    ImGui::PopStyleColor();

    ImGui::TextWrapped("%s", station.description.c_str());
    ImGui::Separator();

    f64 productionRate = station.currentProduction * effectiveBonus;

    // Superposition progress bar
    char barOverlay[64];
    f32 progress = 0.0f;
    if (station.upgradeCost > 0.0) {
        progress = static_cast<f32>(station.superpositionValue / (station.upgradeCost * 0.1));
        if (progress > 1.0f) progress = 1.0f;
    }

    snprintf(barOverlay, sizeof(barOverlay), "Superposition: %s (%s / sec)",
             GameUtils::FormatNumber(station.superpositionValue, state->m_NumberFormat).c_str(),
             GameUtils::FormatNumber(productionRate, state->m_NumberFormat).c_str());

    ImGui::ProgressBar(progress, ImVec2(-1, 0), barOverlay);

    // OBSERVE Button - Purple
    Color observeColor = Color::QuantumPurple();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(observeColor.r * 0.7f, observeColor.g * 0.7f, observeColor.b * 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(observeColor.r, observeColor.g, observeColor.b, 1.0f));
    if (ImGui::Button(("OBSERVE##ObserveBtn" + std::to_string(index)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.30f, 40.0f))) {
        station.Observe(state);
        Log::Infof("Observed ", station.name);
    }
    ImGui::PopStyleColor(2);

    // UPGRADE Button - Orange
    ImGui::SameLine();
    f64 effectiveUpgradeCost = station.upgradeCost * (state->GetChallengeManager().HasModifier(ChallengeModifier::ExpensiveUpgrades) ? 3.0 : 1.0);
    bool canAffordUpgrade = currentQubits >= effectiveUpgradeCost && canUpgrade;
    if (!canAffordUpgrade) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

    Color upgradeColor = Color::EntanglementOrange();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(upgradeColor.r * 0.7f, upgradeColor.g * 0.7f, upgradeColor.b * 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(upgradeColor.r, upgradeColor.g, upgradeColor.b, 1.0f));

    std::string upgradeText = "Upgrade (" + GameUtils::FormatNumber(effectiveUpgradeCost, state->m_NumberFormat) + ")";

    if (ImGui::Button((upgradeText + "##UpgradeBtn" + std::to_string(index)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.45f, 40.0f)) && canAffordUpgrade) {
        f64 effectiveCost = station.upgradeCost;
        if (state->GetChallengeManager().HasModifier(ChallengeModifier::ExpensiveUpgrades)) {
            effectiveCost *= 3.0;
        }
        if (state->SpendResource(QuantumResource::Qubits, effectiveCost)) {
            station.Upgrade();
            state->UpdateResearchBonuses();
            state->GetSpecializedSkills().AddExperience(SkillCategory::Engineering, SkillXP::UPGRADE_STATION);
            state->GetSoundManager().PlaySound(SoundEffect::UpgradeComplete);
            Log::Infof("Upgraded ", station.name, " to level ", station.level);
        }
    }
    ImGui::PopStyleColor(2);
    if (!canAffordUpgrade) ImGui::PopStyleVar();

    // BUY MAX Button - Green
    ImGui::SameLine();
    Color buyMaxColor = Color::CoherenceGreen();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(buyMaxColor.r * 0.7f, buyMaxColor.g * 0.7f, buyMaxColor.b * 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(buyMaxColor.r, buyMaxColor.g, buyMaxColor.b, 1.0f));

    if (ImGui::Button(("BUY MAX##BuyMaxBtn" + std::to_string(index)).c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 40.0f))) {
        f64 qubits = state->GetResource(QuantumResource::Qubits);
        i32 upgradesBought = 0;
        bool expensiveUpgrades = state->GetChallengeManager().HasModifier(ChallengeModifier::ExpensiveUpgrades);

        while (upgradesBought < 1000) {
            f64 effectiveCost = station.upgradeCost;
            if (expensiveUpgrades) effectiveCost *= 3.0;

            if (qubits >= effectiveCost) {
                if (state->SpendResource(QuantumResource::Qubits, effectiveCost)) {
                    station.Upgrade();
                    state->GetSpecializedSkills().AddExperience(SkillCategory::Engineering, SkillXP::UPGRADE_STATION);
                    qubits = state->GetResource(QuantumResource::Qubits);
                    upgradesBought++;
                } else {
                    break;
                }
            } else {
                break;
            }
        }

        if (upgradesBought > 0) {
            state->UpdateResearchBonuses();
            Log::Infof("Bought ", upgradesBought, " upgrades for ", station.name, " (now level ", station.level, ")");
        }
    }
    ImGui::PopStyleColor(2);

    // Auto-Upgrade Toggle
    ImGui::Checkbox(("Auto-Upgrade##" + std::to_string(index)).c_str(), &station.autoUpgrade);
}

void StationView::RenderLockedStation(GameState* state, ResearchStation& station, size_t index) {
    f64 currentQubits = state->GetResource(QuantumResource::Qubits);

    // Station Display Header
    Color tierColor = state->GetStationTierColor(station.level);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(tierColor.r, tierColor.g, tierColor.b, 1.0f));
    ImGui::Text(">> %s (Level %d)", station.name.c_str(), station.level);
    ImGui::PopStyleColor();

    ImGui::TextWrapped("%s", station.description.c_str());
    ImGui::Separator();

    // UNLOCK Button - Blue
    bool canAffordUnlock = currentQubits >= station.unlockCost;
    if (!canAffordUnlock) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

    Color unlockColor = Color::QuantumBlue();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(unlockColor.r * 0.7f, unlockColor.g * 0.7f, unlockColor.b * 0.7f, 0.7f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(unlockColor.r, unlockColor.g, unlockColor.b, 1.0f));

    std::string unlockText = "Unlock for " + GameUtils::FormatNumber(station.unlockCost, state->m_NumberFormat) + " Qubits";
    if (ImGui::Button((unlockText + "##UnlockBtn" + std::to_string(index)).c_str(), ImVec2(-1, 50.0f)) && canAffordUnlock) {
        if (state->SpendResource(QuantumResource::Qubits, station.unlockCost)) {
            station.unlocked = true;
            station.level = 0;
            Log::Infof("Unlocked: ", station.name);
        }
    }
    ImGui::PopStyleColor(2);
    if (!canAffordUnlock) ImGui::PopStyleVar();
}

void StationView::RenderPrestigeButton(GameState* state) {
    f32 photonsToGain = state->CalculatePhotonsOnPrestige();
    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.0f, 0.5f, 0.8f)); // Magenta
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.0f, 1.0f, 1.0f));

    std::string prestigeText = "PERFORM PRESTIGE (" + std::to_string(static_cast<i32>(photonsToGain)) + " Photons)";
    if (photonsToGain <= 0.0) {
        prestigeText = "PRESTIGE (Need more progress)";
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.3f);
    }

    if (ImGui::Button(prestigeText.c_str(), ImVec2(-1, 80.0f)) && photonsToGain > 0.0) {
        state->PerformPrestige();
    }

    if (photonsToGain <= 0.0) {
        ImGui::PopStyleVar();
    }

    ImGui::PopStyleColor(2);
}

// =============================================================================
// Stub Implementations for Other Views
// NOTE: These currently delegate to the original GameState methods
// They will be gradually refactored in future phases
// =============================================================================

void AchievementView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderAchievements
    state->RenderAchievements(renderer);
}

void StatisticsView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderStatistics
    state->RenderStatistics(renderer);
}

void ResearchView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderResearchTree
    state->RenderResearchTree(renderer);
}

void MilestoneView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderMilestones
    state->RenderMilestones(renderer);
}

void BuyablesView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderBuyables
    state->RenderBuyables(renderer);
}

void ChallengeView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderChallenges
    state->RenderChallenges(renderer);
}

void EssenceShopView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderEssenceShop
    state->RenderEssenceShop(renderer);
}

void SingularityShopView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderSingularityShop
    state->RenderSingularityShop(renderer);
}

void SpaceshipView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderSpaceship
    state->RenderSpaceship(renderer);
}

void CombatView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderCombat
    state->RenderCombat(renderer);
}

void GatchaView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderGatcha
    state->RenderGatcha(renderer);
}

void SkillTreeView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderSkillTree
    state->RenderSkillTree(renderer);
}

void EnhancementView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderEnhancement
    // Note: This method doesn't exist yet in GameState
    (void)state;
    (void)renderer;
    ImGui::Text("Enhancement view - Coming soon!");
}

void SpecializedSkillsView::Render(GameState* state, Renderer* renderer) {
    // TODO: Extract from GameState::RenderSpecializedSkills
    state->RenderSpecializedSkills(renderer);
}

} // namespace UI
