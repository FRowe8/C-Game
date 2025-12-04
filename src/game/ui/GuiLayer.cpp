#include "GuiLayer.h"
#include "TutorialOverlay.h"
#include "FloatingTextManager.h"
#include "GameState.h"
#include "ParticleCollection.h"
#include "Renderer.h"
#include "ImGuiUtils.h"
#include "GameUtils.h"
#include "Logger.h"
#include "Research.h"
#include "UITheme.h"
#include "FeatureUnlockManager.h"
#include "imgui.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>

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
    m_CollectionView = std::make_unique<CollectionView>();  // Phase 4.1

    // Tutorial System
    m_TutorialOverlay = std::make_unique<TutorialOverlay>();

    // Floating Text System
    m_FloatingTextManager = std::make_unique<FloatingTextManager>();

    Log::Info("GuiLayer initialized successfully");
}

void GuiLayer::Update(GameState* state) {
    // Update tutorial system
    if (m_TutorialOverlay) {
        m_TutorialOverlay->Update(state);
    }

    // Update floating text
    if (m_FloatingTextManager) {
        m_FloatingTextManager->Update(ImGui::GetIO().DeltaTime);
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

        case ActiveModal::Collection:  // Phase 4.1
            m_CollectionView->Render(state, renderer);
            break;

        case ActiveModal::MoreMenu:
            // MoreMenu is handled in NavigationView, no separate view needed
            m_StationView->Render(state, renderer);
            break;
    }

    // Render always-visible overlays
    RenderActiveEvent(state, renderer);
    RenderNotifications(state, renderer);

    // Render floating text (before tutorial so tutorial can overlay)
    if (m_FloatingTextManager) {
        m_FloatingTextManager->Render(renderer);
    }

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

        // Qubits (Phase 2.3: with tooltip)
        ImGui::TextColored(ToImVec4(Color::QuantumBlue()), "QUBITS");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Primary currency - Used to upgrade stations and unlock new features");
        }
        ImGui::SameLine();
        ImGui::Text("%s", GameUtils::FormatNumber(qubits, fmt).c_str());

        ImGui::SameLine(0.0f, 30.0f);

        // Coherence (Phase 2.3: with tooltip)
        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "COHERENCE");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Quantum stability - Affects observation success rate\nLow coherence reduces rewards from observations");
        }
        ImGui::SameLine();
        ImGui::Text("%s", GameUtils::FormatNumber(coherence, fmt).c_str());

        ImGui::SameLine(0.0f, 30.0f);

        // Entanglement (Phase 2.3: with tooltip)
        ImGui::TextColored(ToImVec4(Color::EntanglementOrange()), "ENTANGLEMENT");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Quantum connections - Used for research and advanced upgrades");
        }
        ImGui::SameLine();
        ImGui::Text("%s", GameUtils::FormatNumber(entanglement, fmt).c_str());

        ImGui::SameLine(0.0f, 30.0f);

        // Photons (prestige currency) (Phase 2.3: with tooltip)
        ImGui::TextColored(ToImVec4(Color::Magenta()), "PHOTONS");
        if (ImGui::IsItemHovered()) {
            ImGui::SetTooltip("Prestige currency - Earned by performing prestige\nProvides permanent production bonuses (+10%% per photon)");
        }
        ImGui::SameLine();
        ImGui::Text("%s", GameUtils::FormatNumber(photons, fmt).c_str());

        // Phase 3.3: Exotic Materials (required for Tier 3+ research)
        i32 exoticMaterials = state->GetExoticMaterials();
        if (exoticMaterials > 0 || photons > 5) { // Show if player has any, or if they're mid-game (5+ photons)
            ImGui::SameLine(0.0f, 30.0f);
            ImGui::TextColored(ToImVec4(Color(1.0f, 0.5f, 1.0f, 1.0f)), "EXOTIC MATERIALS");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Rare materials required for Tier 3+ research\nPrimarily recovered via Spaceship Expeditions");
            }
            ImGui::SameLine();
            ImGui::Text("%d", exoticMaterials);
        }

        // Research Data
        i32 researchData = state->GetResearchData();
        if (researchData > 0) {
            ImGui::SameLine(0.0f, 30.0f);
            ImGui::TextColored(ToImVec4(Color(0.3f, 0.9f, 1.0f, 1.0f)), "RESEARCH DATA");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Intel gathered from combat victories. Useful for higher-tier research projects.");
            }
            ImGui::SameLine();
            ImGui::Text("%d", researchData);
        }

        // Singularities (if any) (Phase 2.3: with tooltip)
        if (singularities > 0) {
            ImGui::SameLine(0.0f, 30.0f);
            ImGui::TextColored(ToImVec4(Color(0.5f, 0.0f, 1.0f, 1.0f)), "SINGULARITIES");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Ultimate currency - Earned from singularity collapse\nUnlocks powerful permanent upgrades");
            }
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
            {"COLLECTION", ActiveModal::Collection, Color::CoherenceGreen()},  // Phase 4.1
        };

        ImGui::SetCursorPos(ImVec2(currentX, btnY));

        ActiveModal currentModal = state->GetActiveModal();

        for (int i = 0; i < 7; i++) {  // Phase 4.1: Updated from 6 to 7 buttons
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

                // Visual/audio feedback hook
                ImVec2 itemMin = ImGui::GetItemRectMin();
                ImVec2 itemMax = ImGui::GetItemRectMax();
                ImVec2 itemCenter((itemMin.x + itemMax.x) * 0.5f, (itemMin.y + itemMax.y) * 0.5f);
                state->RegisterUIButtonFeedback(btn.label, itemCenter, ToImVec4(btn.color));
            } else if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Open the %s panel", btn.label);
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

            // Phase 1.2: Use ActiveModal enum for state machine with feature gating
            struct MoreButton {
                const char* label;
                ActiveModal modal;
                Color color;
                GameFeature feature;  // For unlock checking
            };

            MoreButton moreButtons[] = {
                {"ACHIEVEMENTS", ActiveModal::Achievements, Color::ElectricBlue(), GameFeature::Achievements},
                {"SINGULARITY", ActiveModal::SingularityShop, Color(0.5f, 0.0f, 1.0f, 1.0f), GameFeature::SingularityShop},
                {"SPACESHIP", ActiveModal::Spaceship, Color(1.0f, 0.7f, 0.0f, 1.0f), GameFeature::Spaceship},
                {"BATTLE", ActiveModal::Combat, Color(1.0f, 0.3f, 0.3f, 1.0f), GameFeature::Combat},
                {"SUMMON", ActiveModal::Gatcha, Color(1.0f, 0.3f, 1.0f, 1.0f), GameFeature::Summon},
                {"SKILLS", ActiveModal::Skills, Color(0.0f, 1.0f, 0.5f, 1.0f), GameFeature::SkillTree},
                {"ENHANCE", ActiveModal::Enhancement, Color(0.8f, 0.6f, 0.2f, 1.0f), GameFeature::Enhancement},
            };

            ActiveModal currentModal = state->GetActiveModal();
            const FeatureUnlockManager& unlockManager = state->GetUnlockManager();

            for (size_t i = 0; i < 7; i++) {
                auto& btn = moreButtons[i];

                // Feature gating: check if feature is unlocked
                bool isUnlocked = unlockManager.IsUnlocked(btn.feature);
                if (!isUnlocked) {
                    // Show locked button with tooltip
                    Color lockedColor = Color(0.2f, 0.2f, 0.2f, 0.5f);
                    ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(lockedColor));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(lockedColor));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(lockedColor));
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));

                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
                    ImGui::Button(("🔒 " + std::string(btn.label)).c_str(), ImVec2(itemWidth, itemHeight));
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(4);

                    if (ImGui::IsItemHovered()) {
                        i32 unlockLevel = unlockManager.GetUnlockLevel(btn.feature);
                        ImGui::SetTooltip("Unlocks at Level %d\n%s", unlockLevel,
                                         unlockManager.GetFeatureDescription(btn.feature));
                    }

                    if (i < 6) ImGui::Spacing();
                    continue;
                }

                bool active = (currentModal == btn.modal);
                Color btnColor = active ? btn.color : Color(0.3f, 0.3f, 0.3f, 1.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnColor * 0.5f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ToImVec4(btnColor * 0.7f));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnColor));

                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

                // Show "NEW!" badge for newly unlocked features
                std::string buttonLabel = btn.label;
                if (unlockManager.IsNewlyUnlocked(btn.feature)) {
                    buttonLabel = std::string(btn.label) + " ✨";
                }

                if (ImGui::Button(buttonLabel.c_str(), ImVec2(itemWidth, itemHeight))) {
                    // Phase 4.2: Play click sound
                    state->GetSoundManager().PlaySound(SoundEffect::ButtonPress, 0.8f);

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

namespace {
    ImVec4 WithMultipliedAlpha(const ImVec4& color, float alpha) {
        return ImVec4(color.x, color.y, color.z, color.w * alpha);
    }

    struct ScopedFontScale {
        explicit ScopedFontScale(float scale) {
            ImGui::SetWindowFontScale(scale);
        }
        ~ScopedFontScale() { ImGui::SetWindowFontScale(1.0f); }
    };

    void RenderTypographyText(const std::string& text, const UITheme::TypographyScale& scale, const ImVec4& color, bool wrap = false) {
        float fontScale = scale.size / ImGui::GetFontSize();
        ScopedFontScale scoped(fontScale);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        if (wrap) {
            ImGui::TextWrapped("%s", text.c_str());
        } else {
            ImGui::Text("%s", text.c_str());
        }
        ImGui::PopStyleColor();
    }

    void RenderLevelBadge(const std::string& label) {
        ImVec2 badgeSize(90.0f * UITheme::GetLayout().fontScale, 30.0f * UITheme::GetLayout().fontScale);
        float xOffset = ImGui::GetContentRegionAvail().x - badgeSize.x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + xOffset);
        ImGui::PushStyleColor(ImGuiCol_Button, UITheme::ColorBadgeBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, WithMultipliedAlpha(UITheme::ColorBadgeBg, 1.05f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, UITheme::ColorBadgeBg);
        ImGui::PushStyleColor(ImGuiCol_Text, UITheme::ColorBadgeText);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, UITheme::CardRounding());
        ImGui::Button(label.c_str(), badgeSize);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
    }

    void RenderCard(const std::string& id, const std::function<void()>& drawContent) {
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, UITheme::CardRounding());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UITheme::CardPadding(), UITheme::CardPadding()));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, UITheme::ColorCardBg);
        ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorCardBorder);
        if (ImGui::BeginChild(id.c_str(), ImVec2(0, 0), true, ImGuiWindowFlags_None)) {
            drawContent();
        }
        ImGui::EndChild();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }

    bool RenderTouchButton(const std::string& label, const ImVec2& size, const ImVec4& color, const ImVec4& textColor, bool disabled = false) {
        ImVec4 hoverColor = WithMultipliedAlpha(color, 1.05f);
        ImVec4 activeColor = WithMultipliedAlpha(color, 1.1f);

        if (disabled) {
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.55f);
        }

        ImGui::PushStyleColor(ImGuiCol_Button, color);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, activeColor);
        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, UITheme::NavigationButtonRounding());
        bool clicked = ImGui::Button(label.c_str(), size);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);

        if (disabled) {
            ImGui::PopStyleVar();
        }

        return clicked;
    }

    void RenderProgressBarDetailed(const std::string& id, f32 fraction, const std::string& headline, const std::string& detail) {
        f32 clamped = std::clamp(fraction, 0.0f, 1.0f);
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImVec2 size(ImGui::GetContentRegionAvail().x, UITheme::ProgressThickness() * 3.8f);
        ImVec2 end(pos.x + size.x, pos.y + size.y);
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddRectFilled(pos, end, ImGui::GetColorU32(UITheme::ColorCardBgAlt), UITheme::CardRounding());
        drawList->AddRectFilled(pos, ImVec2(pos.x + size.x * clamped, end.y), ImGui::GetColorU32(UITheme::ColorAccent), UITheme::CardRounding());
        drawList->AddRect(pos, end, ImGui::GetColorU32(UITheme::ColorCardBorder), UITheme::CardRounding());

        ImGui::InvisibleButton(id.c_str(), size);
        drawList->AddText(ImVec2(pos.x + UITheme::CardPadding(), pos.y + 4.0f), ImGui::GetColorU32(UITheme::ColorText), headline.c_str());
        drawList->AddText(ImVec2(pos.x + UITheme::CardPadding(), pos.y + size.y * 0.55f), ImGui::GetColorU32(UITheme::ColorTextDim), detail.c_str());
    }

    bool RenderInfoIcon(const std::string& id) {
        ImVec2 size = ImVec2(UITheme::TouchMinSizeVec2().x * 0.85f, UITheme::TouchMinSizeVec2().y * 0.85f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UITheme::TouchPadding()));
        bool clicked = RenderTouchButton("ℹ##" + id, size, WithMultipliedAlpha(UITheme::ColorCardHeader, 0.9f), UITheme::ColorText, false);
        ImGui::PopStyleVar();
        return clicked;
    }
}

// =============================================================================
// StationView Implementation
// =============================================================================

void StationView::Render(GameState* state, Renderer* renderer) {
    (void)renderer;

    // NOTE: This is extracted from GameState::RenderStationsContent
    // This content is rendered inside a window created by UIManager

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

    static std::vector<bool> s_InfoExpanded;
    if (s_InfoExpanded.size() <= index) {
        s_InfoExpanded.resize(index + 1, false);
    }

    Color tierColor = state->GetStationTierColor(station.level);
    RenderCard("StationCard##" + std::to_string(index), [&]() {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(UITheme::ItemSpacing(), UITheme::ItemSpacing() * 0.75f));

        if (ImGui::BeginTable(("StationHeader##" + std::to_string(index)).c_str(), 2, ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_NoBordersInBody)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 0.7f);
            ImGui::TableSetupColumn("Badge", ImGuiTableColumnFlags_WidthStretch, 0.3f);
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            RenderTypographyText(station.name, UITheme::HeaderText(), ImVec4(tierColor.r, tierColor.g, tierColor.b, 1.0f));
            ImGui::TableSetColumnIndex(1);
            RenderLevelBadge("Level " + std::to_string(station.level));
            ImGui::EndTable();
        }

        RenderTypographyText(station.description, UITheme::SubtitleText(), UITheme::ColorTextDim, true);

        f64 productionRate = station.currentProduction * effectiveBonus;
        f32 progress = 0.0f;
        if (station.upgradeCost > 0.0) {
            progress = static_cast<f32>(station.superpositionValue / (station.upgradeCost * 0.1));
        }

        std::string headline = "Superposition: " + GameUtils::FormatNumber(station.superpositionValue, state->m_NumberFormat);
        std::string passiveInfo;
        if (station.passiveCollapseRate > 0.0) {
            f64 passiveRate = station.superpositionValue * station.passiveCollapseRate;
            passiveInfo = " | +" + GameUtils::FormatNumber(passiveRate, state->m_NumberFormat) + "/s passive";
        }
        std::string detail = "Production: " + GameUtils::FormatNumber(productionRate, state->m_NumberFormat) + "/s" + passiveInfo;
        RenderProgressBarDetailed("SuperpositionBar##" + std::to_string(index), progress, headline, detail);

        ImGui::Dummy(ImVec2(0.0f, UITheme::ItemSpacing() * 0.2f));

        Color observeColor = Color::QuantumPurple();
        ImVec2 buttonPos = ImGui::GetCursorScreenPos();
        f32 primaryWidth = ImGui::GetContentRegionAvail().x;
        if (primaryWidth > 420.0f) {
            primaryWidth *= 0.7f;
        }
        ImVec2 buttonSize(primaryWidth, std::max(UITheme::NavigationButtonHeight(), UITheme::TouchMinSize()));
        bool observeClicked = RenderTouchButton("OBSERVE##ObserveBtn" + std::to_string(index), buttonSize,
                                               ImVec4(observeColor.r, observeColor.g, observeColor.b, 0.85f), UITheme::ColorText);

        if (ImGui::GetContentRegionAvail().x > UITheme::TouchMinSize()) {
            ImGui::SameLine();
        }
        if (RenderInfoIcon("ObserveInfo##" + std::to_string(index))) {
            s_InfoExpanded[index] = !s_InfoExpanded[index];
        }

        if (observeClicked) {
            station.Observe(state);
            Log::Infof("Observed ", station.name);

            Vec2 particlePos(buttonPos.x + buttonSize.x * 0.5f, buttonPos.y + buttonSize.y * 0.5f);
            Color particleColor = (station.resourceType == QuantumResource::Qubits) ? Color::QuantumBlue() :
                                 (station.resourceType == QuantumResource::Coherence) ? Color::CoherenceGreen() :
                                 Color::EntanglementOrange();
            state->SpawnParticleBurst(particlePos, particleColor, 10);

            ImVec2 itemMin = ImGui::GetItemRectMin();
            ImVec2 itemMax = ImGui::GetItemRectMax();
            ImVec2 itemCenter((itemMin.x + itemMax.x) * 0.5f, (itemMin.y + itemMax.y) * 0.5f);
            state->RegisterUIButtonFeedback("Observed", itemCenter, ToImVec4(observeColor));
        }

        if (s_InfoExpanded[index]) {
            RenderTypographyText(
                "Collapse the wave function to collect resources. Observation success depends on coherence and station upgrades.",
                UITheme::CaptionText(), UITheme::ColorText, true);
        }

        f64 effectiveUpgradeCost = station.upgradeCost * (state->GetChallengeManager().HasModifier(ChallengeModifier::ExpensiveUpgrades) ? 3.0 : 1.0);
        bool canAffordUpgrade = currentQubits >= effectiveUpgradeCost && canUpgrade;
        bool expensiveUpgrades = state->GetChallengeManager().HasModifier(ChallengeModifier::ExpensiveUpgrades);
        Color upgradeColor = Color::EntanglementOrange();
        Color buyMaxColor = Color::CoherenceGreen();

        ImGui::Dummy(ImVec2(0.0f, UITheme::ItemSpacing() * 0.3f));

        if (ImGui::BeginTable(("StationActions##" + std::to_string(index)).c_str(), 2, ImGuiTableFlags_SizingStretchProp)) {
            ImGui::TableSetupColumn("Upgrade", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("BuyMax", ImGuiTableColumnFlags_WidthStretch);

            ImGui::TableNextColumn();
            std::string upgradeLabel = "Upgrade • " + GameUtils::FormatNumber(effectiveUpgradeCost, state->m_NumberFormat);
            ImVec2 upgradeSize(ImGui::GetContentRegionAvail().x, std::max(UITheme::NavigationButtonHeight() * 0.9f, UITheme::TouchMinSize()));
            if (RenderTouchButton(upgradeLabel + "##UpgradeBtn" + std::to_string(index), upgradeSize,
                                  ImVec4(upgradeColor.r, upgradeColor.g, upgradeColor.b, 0.8f),
                                  canAffordUpgrade ? UITheme::ColorText : UITheme::ColorDanger, !canAffordUpgrade)) {
                f64 effectiveCost = station.upgradeCost;
                if (expensiveUpgrades) {
                    effectiveCost *= 3.0;
                }
                if (state->SpendResource(QuantumResource::Qubits, effectiveCost)) {
                    station.Upgrade();
                    state->UpdateResearchBonuses();
                    state->GetSpecializedSkills().AddExperience(SkillCategory::Engineering, SkillXP::UPGRADE_STATION);
                    state->GetSoundManager().PlaySound(SoundEffect::UpgradeComplete);
                    Log::Infof("Upgraded ", station.name, " to level ", station.level);

                    Vec2 particlePos(buttonPos.x + buttonSize.x * 0.5f, buttonPos.y + buttonSize.y * 0.5f);
                    state->SpawnParticleBurst(particlePos, Color::EntanglementOrange(), 15);

                    ImVec2 itemMin = ImGui::GetItemRectMin();
                    ImVec2 itemMax = ImGui::GetItemRectMax();
                    ImVec2 itemCenter((itemMin.x + itemMax.x) * 0.5f, (itemMin.y + itemMax.y) * 0.5f);
                    state->RegisterUIButtonFeedback("Upgraded", itemCenter, ToImVec4(upgradeColor));
                }
            }

            ImGui::TableNextColumn();
            std::string buyLabel = "Buy Max";
            ImVec2 buySize(ImGui::GetContentRegionAvail().x, std::max(UITheme::NavigationButtonHeight() * 0.9f, UITheme::TouchMinSize()));
            if (RenderTouchButton(buyLabel + "##BuyMaxBtn" + std::to_string(index), buySize,
                                  ImVec4(buyMaxColor.r, buyMaxColor.g, buyMaxColor.b, 0.8f), UITheme::ColorText, false)) {
                f64 qubits = state->GetResource(QuantumResource::Qubits);
                f64 effectiveCostMultiplier = expensiveUpgrades ? 3.0 : 1.0;
                f64 effectiveCurrentCost = station.upgradeCost * effectiveCostMultiplier;

                // O(1) calculation using geometric series formula
                i32 maxAffordable = GameUtils::CalculateMaxAffordableUpgrades(
                    qubits, effectiveCurrentCost, station.upgradeCostMultiplier, 10000);

                if (maxAffordable > 0) {
                    // Calculate total cost for all upgrades
                    f64 totalCost = GameUtils::CalculateTotalUpgradeCost(
                        effectiveCurrentCost, maxAffordable, station.upgradeCostMultiplier);

                    if (state->SpendResource(QuantumResource::Qubits, totalCost)) {
                        // Apply all upgrades at once
                        i32 startLevel = station.level;
                        for (i32 i = 0; i < maxAffordable; ++i) {
                            station.Upgrade();
                        }
                        // Award XP for all upgrades
                        state->GetSpecializedSkills().AddExperience(
                            SkillCategory::Engineering,
                            SkillXP::UPGRADE_STATION * maxAffordable);

                        state->UpdateResearchBonuses();
                        Log::Infof("Bought ", maxAffordable, " upgrades for ", station.name,
                                   " (level ", startLevel, " -> ", station.level, ")");
                        ImVec2 itemMin = ImGui::GetItemRectMin();
                        ImVec2 itemMax = ImGui::GetItemRectMax();
                        ImVec2 itemCenter((itemMin.x + itemMax.x) * 0.5f, (itemMin.y + itemMax.y) * 0.5f);
                        state->RegisterUIButtonFeedback("Upgraded", itemCenter, ToImVec4(upgradeColor));
                    }
                }
            }

            ImGui::EndTable();
        }

        ImGui::Dummy(ImVec2(0.0f, UITheme::ItemSpacing() * 0.2f));
        ImGui::Checkbox(("Auto-Upgrade##" + std::to_string(index)).c_str(), &station.autoUpgrade);
        ImGui::PopStyleVar();
    });
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
            ImVec2 itemMin = ImGui::GetItemRectMin();
            ImVec2 itemMax = ImGui::GetItemRectMax();
            ImVec2 itemCenter((itemMin.x + itemMax.x) * 0.5f, (itemMin.y + itemMax.y) * 0.5f);
            state->RegisterUIButtonFeedback("Unlocked", itemCenter, ToImVec4(unlockColor));
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
    (void)renderer; // Not used for ImGui rendering

    // Use ActiveModal instead of legacy m_ShowAchievements boolean
    if (state->GetActiveModal() != ActiveModal::Achievements) return;

    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
    );

    bool showWindow = true;
    if (ImGui::Begin("Achievements", &showWindow, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("--- Achievement Progress ---");
        ImGui::Separator();

        for (auto& achievement : state->m_Achievements) {
            // Determine text color based on status
            ImVec4 statusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Default: in-progress
            const char* statusText = "IN PROGRESS";

            if (achievement.unlocked) {
                statusColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green: Unlocked
                statusText = "UNLOCKED";
            } else if (achievement.progress >= achievement.target) {
                statusColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Gold: Ready to Claim
                statusText = "CLAIMABLE";
            }

            // Achievement Title and Status
            ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
            ImGui::Text("[%s] %s", statusText, achievement.name.c_str());
            ImGui::PopStyleColor();

            ImGui::Indent();
            ImGui::TextWrapped("%s", achievement.description.c_str());

            f32 progress = static_cast<f32>(achievement.progress / achievement.target);

            if (achievement.progress < achievement.target) {
                // Show progress bar if not complete
                char overlay[64];
                snprintf(overlay, sizeof(overlay), "%.0f / %.0f", achievement.progress, achievement.target);
                ImGui::ProgressBar(progress, ImVec2(-1, 0), overlay);
            } else if (achievement.unlocked) {
                // Show a full bar for completed, claimed achievements
                ImGui::ProgressBar(1.0f, ImVec2(-1, 0), "Completed");
            }

            // Claim Button Logic
            if (achievement.progress >= achievement.target && !achievement.unlocked) {
                ImGui::Spacing();
                std::string rewardText = "Claim: ";
                if (achievement.rewardQubits > 0) {
                    rewardText += GameUtils::FormatNumber(achievement.rewardQubits, state->m_NumberFormat) + " Qubits ";
                }
                if (achievement.rewardPhotons > 0) {
                    rewardText += std::to_string(achievement.rewardPhotons) + " Photons";
                }

                if (ImGui::Button((rewardText + "##ClaimAch" + std::to_string(static_cast<int>(achievement.id))).c_str(), ImVec2(150, 30))) {
                    achievement.unlocked = true;
                    state->AddResource(QuantumResource::Qubits, achievement.rewardQubits);
                    state->m_Timeline.photons += achievement.rewardPhotons;
                    state->GetSoundManager().PlaySound(SoundEffect::AchievementUnlock, 1.0f);
                }
                ImGui::Spacing();
            }

            ImGui::Unindent();
            ImGui::Separator();
        }
    }
    ImGui::End();

    // Close modal if window was closed
    if (!showWindow) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void StatisticsView::Render(GameState* state, Renderer* renderer) {
    (void)renderer; // Not used for ImGui rendering

    if (state->GetActiveModal() != ActiveModal::Statistics) return;

    ImGui::SetNextWindowSize(ImVec2(500, 600), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
    );

    bool showWindow = true;
    if (ImGui::Begin("Statistics", &showWindow, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("--- Total Lifetime Statistics ---");
        ImGui::Separator();

        ImGui::Columns(2, "StatColumns", true);
        ImGui::SetColumnWidth(0, 300.0f);

        auto format = [state](f64 value) { return GameUtils::FormatNumber(value, state->m_NumberFormat); };

        // --- Total Stats ---
        ImGui::Text("Total Qubits Earned:"); ImGui::NextColumn();
        ImGui::Text("%s", format(state->m_Statistics.totalQubitsEarned).c_str()); ImGui::NextColumn();

        ImGui::Text("Total Coherence Earned:"); ImGui::NextColumn();
        ImGui::Text("%s", format(state->m_Statistics.totalCoherenceEarned).c_str()); ImGui::NextColumn();

        ImGui::Text("Total Entanglement Earned:"); ImGui::NextColumn();
        ImGui::Text("%s", format(state->m_Statistics.totalEntanglementEarned).c_str()); ImGui::NextColumn();

        ImGui::Text("Total Observations:"); ImGui::NextColumn();
        ImGui::Text("%d", state->m_Statistics.totalObservations); ImGui::NextColumn();

        ImGui::Text("Total Upgrades Purchased:"); ImGui::NextColumn();
        ImGui::Text("%d", state->m_Statistics.totalUpgrades); ImGui::NextColumn();

        ImGui::Text("Prestiges Performed:"); ImGui::NextColumn();
        ImGui::Text("%d", state->m_Statistics.totalPrestigesPerformed); ImGui::NextColumn();

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::Text("--- Session Statistics ---");
        ImGui::Separator();

        ImGui::Columns(2, "SessionStatColumns", true);
        ImGui::SetColumnWidth(0, 300.0f);

        ImGui::Text("Session Qubits Earned:"); ImGui::NextColumn();
        ImGui::Text("%s", format(state->m_Statistics.sessionQubits).c_str()); ImGui::NextColumn();

        ImGui::Text("Session Time (seconds):"); ImGui::NextColumn();
        ImGui::Text("%.1f", state->m_Statistics.sessionTime); ImGui::NextColumn();

        ImGui::Text("Session Observations:"); ImGui::NextColumn();
        ImGui::Text("%d", state->m_Statistics.sessionObservations); ImGui::NextColumn();

        ImGui::Columns(1);
        ImGui::Separator();

        // Live telemetry
        ImGui::Text("--- Live Telemetry ---");
        ImGui::Separator();

        const TelemetryManager& telemetry = state->m_Telemetry;

        ImGui::Columns(2, "TelemetryStatColumns", true);
        ImGui::SetColumnWidth(0, 300.0f);

        ImGui::Text("Current Session Length:"); ImGui::NextColumn();
        ImGui::Text("%s", GameUtils::FormatTime(telemetry.GetSessionLengthSeconds()).c_str()); ImGui::NextColumn();

        ImGui::Text("Prestiges This Session:"); ImGui::NextColumn();
        ImGui::Text("%d", telemetry.GetSessionPrestiges()); ImGui::NextColumn();

        ImGui::Text("Prestiges Per Hour:"); ImGui::NextColumn();
        ImGui::Text("%.2f", telemetry.GetPrestigesPerHour()); ImGui::NextColumn();

        ImGui::Text("Avg. Minutes Between Prestiges:"); ImGui::NextColumn();
        ImGui::Text("%.2f", telemetry.GetAveragePrestigeIntervalMinutes()); ImGui::NextColumn();

        ImGui::Text("Last Prestige Interval (seconds):"); ImGui::NextColumn();
        ImGui::Text("%.1f", telemetry.GetLastPrestigeIntervalSeconds()); ImGui::NextColumn();

        ImGui::Columns(1);
        ImGui::Separator();

        ImGui::Text("--- Records ---");
        ImGui::Separator();

        ImGui::Columns(2, "RecordStatColumns", true);
        ImGui::SetColumnWidth(0, 300.0f);

        ImGui::Text("Highest Qubits Achieved:"); ImGui::NextColumn();
        ImGui::Text("%s", format(state->m_Statistics.highestQubits).c_str()); ImGui::NextColumn();

        ImGui::Text("Fastest Prestige (seconds):"); ImGui::NextColumn();
        if (state->m_Statistics.fastestPrestige < 99999.0) {
            ImGui::Text("%.1f", state->m_Statistics.fastestPrestige); ImGui::NextColumn();
        } else {
            ImGui::Text("N/A"); ImGui::NextColumn();
        }

        ImGui::Text("Longest Login Streak:"); ImGui::NextColumn();
        ImGui::Text("%d days", state->m_Statistics.longestStreak); ImGui::NextColumn();

        ImGui::Columns(1);
    }
    ImGui::End();

    if (!showWindow) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void ResearchView::Render(GameState* state, Renderer* renderer) {
    if (state->GetActiveModal() != ActiveModal::Research) return;
    (void)renderer;

    // 1. Responsive position and size
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    f32 panelWidth = std::min(displaySize.x * 0.95f, 900.0f);
    f32 panelHeight = std::min(displaySize.y * 0.9f, 600.0f);

    // Center the window
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles (Quantum Purple theme)
    Color mainBorderColor = Color::QuantumPurple() * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    // 3. Begin the main research window
    bool windowOpen = true;
    if (ImGui::Begin("Research Tree", &windowOpen,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Header ---
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press R or ESC to close)");

        // Research count
        i32 researched = state->m_ResearchTree->GetResearchedCount();
        i32 total = static_cast<i32>(ResearchID::COUNT);
        std::string countText = "Researched: " + std::to_string(researched) + "/" + std::to_string(total);

        ImGui::SetCursorPosY(45.0f);
        ImGui::TextColored(ToImVec4(Color(0.9f, 0.9f, 1.0f, 1.0f)), "%s", countText.c_str());

        ImGui::Separator();
        ImGui::Spacing();

        // --- Research Nodes List (Scrollable Child Window) ---
        f32 contentStartY = ImGui::GetCursorPosY();
        f32 nodesContentHeight = panelHeight - contentStartY - 60.0f - 30.0f;

        if (ImGui::BeginChild("##ResearchNodesList", ImVec2(0, nodesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {

            f32 nodeWidth = ImGui::GetContentRegionAvail().x;
            f32 nodeHeight = 100.0f;
            auto availableNodes = state->m_ResearchTree->GetAvailableResearch(state->m_Timeline.completedResets);
            auto researchedNodes = state->m_ResearchTree->GetResearchedNodes();

            // --- Render Available Nodes ---
            if (!availableNodes.empty()) {
                ImGui::TextColored(ToImVec4(Color::White()), "Available Research:");
                ImGui::Spacing();

                for (const ResearchNode* node : availableNodes) {
                    bool canAfford = state->CanAffordResearch(node->id);

                    // Node background
                    Color nodeBg = canAfford ? Color(0.2f, 0.3f, 0.2f, 1.0f) : Color(0.2f, 0.2f, 0.25f, 1.0f);
                    Color nodeBorder = canAfford ? Color::CoherenceGreen() : Color::QuantumBlue();

                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(nodeBg));
                    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(nodeBorder));

                    std::string nodeChildName = "##ResearchNode_" + std::to_string(static_cast<i32>(node->id));

                    if (ImGui::BeginChild(nodeChildName.c_str(), ImVec2(nodeWidth, nodeHeight), true, ImGuiWindowFlags_NoScrollbar)) {

                        // Node name
                        ImGui::TextColored(ToImVec4(Color::White()), "%s", node->name.c_str());

                        // Auto-research toggle button
                        f32 autoToggleW = 60.0f;
                        f32 autoToggleH = 25.0f;
                        ImGui::SameLine(nodeWidth - autoToggleW - 10.0f);
                        ImGui::SetCursorPosY(10.0f);

                        ResearchNode* mutableNode = state->m_ResearchTree->GetNode(node->id);
                        bool autoEnabled = (mutableNode && mutableNode->autoResearch);

                        Color autoToggleColor = autoEnabled ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(autoToggleColor * 0.4f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(autoToggleColor * 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(autoToggleColor));

                        if (ImGui::Button("AUTO", ImVec2(autoToggleW, autoToggleH))) {
                            if (mutableNode) mutableNode->autoResearch = !mutableNode->autoResearch;
                        }
                        ImGui::PopStyleColor(3);

                        // Node description
                        ImGui::TextColored(ToImVec4(Color(0.8f, 0.8f, 0.9f, 1.0f)), "%s", node->description.c_str());

                        // Costs
                        std::string costText = "Cost: ";
                        if (node->qubitCost > 0) costText += GameUtils::FormatNumber(node->qubitCost, state->m_NumberFormat) + " Qubits | ";
                        if (node->coherenceCost > 0) costText += GameUtils::FormatNumber(node->coherenceCost, state->m_NumberFormat) + " Coherence | ";
                        if (node->entanglementCost > 0) costText += GameUtils::FormatNumber(node->entanglementCost, state->m_NumberFormat) + " Entanglement | ";
                        if (node->photonCost > 0) costText += std::to_string(node->photonCost) + " Photons";
                        if (node->exoticMaterialsCost > 0) costText += " | " + std::to_string(node->exoticMaterialsCost) + " Exotic Materials";

                        Color costColor = canAfford ? Color::CoherenceGreen() : Color::QuantumPurple();
                        ImGui::TextColored(ToImVec4(costColor), "%s", costText.c_str());

                        // Prerequisites
                        if (!node->prerequisites.empty()) {
                            std::string prereqText = "Requires: ";
                            for (size_t i = 0; i < node->prerequisites.size(); i++) {
                                const ResearchNode* prereq = state->m_ResearchTree->GetNode(node->prerequisites[i]);
                                if (prereq) {
                                    prereqText += prereq->name;
                                    if (i < node->prerequisites.size() - 1) prereqText += ", ";
                                }
                            }
                            ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.7f, 1.0f)), "%s", prereqText.c_str());
                        }

                        // Research button
                        ImGui::SetCursorPosY(nodeHeight - 35.0f);
                        ImGui::SameLine(nodeWidth - 120.0f);

                        Color btnColor = canAfford ? Color::QuantumPurple() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                        Color btnHoveredColor = canAfford ? Color::QuantumPurple() * 1.5f : Color(0.4f, 0.4f, 0.4f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                        if (ImGui::Button("RESEARCH", ImVec2(110.0f, 30.0f)) && canAfford) {
                            state->PurchaseResearch(node->id);
                        }
                        ImGui::PopStyleColor(3);
                    }
                    ImGui::EndChild();

                    ImGui::PopStyleColor(2);
                    ImGui::PopStyleVar(2);
                    ImGui::Spacing();
                }
            }

            // --- Render Completed Research if no Available Nodes ---
            if (availableNodes.empty()) {
                ImGui::SetCursorPosX(nodeWidth / 2.0f - 180.0f);
                ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.7f, 1.0f)), "No research available at current prestige level!");
                ImGui::Spacing();

                ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "Completed Research:");

                i32 completedCount = 0;
                for (const ResearchNode* node : researchedNodes) {
                    if (completedCount == 0) ImGui::Separator();

                    std::string completedText = "* " + node->name;
                    ImGui::TextColored(ToImVec4(Color(0.8f, 0.9f, 0.8f, 1.0f)), "%s", completedText.c_str());

                    completedCount++;

                    if (completedCount % 2 == 1 && static_cast<size_t>(completedCount) < researchedNodes.size()) {
                        ImGui::SameLine(nodeWidth / 2.0f);
                    }

                    if (completedCount >= 8) {
                        ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.7f, 1.0f)), "(...and %d more)", (int)researchedNodes.size() - completedCount);
                        break;
                    }
                }
            }
        }
        ImGui::EndChild();

        // --- Bonuses Summary ---
        ImGui::SetCursorPosY(panelHeight - 60.0f);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::TextColored(ToImVec4(Color::QuantumBlue()), "Active Bonuses:");

        f64 prodMult = state->m_ResearchTree->GetTotalProductionMultiplier();
        f64 obsMult = state->m_ResearchTree->GetTotalObservationBonus();
        f64 cohMult = state->m_ResearchTree->GetTotalCoherenceBonus();

        std::string bonusText = "Production: +" + std::to_string(static_cast<i32>((prodMult - 1.0) * 100.0)) + "%%  |  ";
        std::string obsBonusStr = (obsMult >= 1.0) ? "+" + std::to_string(static_cast<i32>((obsMult - 1.0) * 100.0)) : std::to_string(static_cast<i32>((obsMult - 1.0) * 100.0));
        std::string cohBonusStr = (cohMult >= 1.0) ? "+" + std::to_string(static_cast<i32>((cohMult - 1.0) * 100.0)) : std::to_string(static_cast<i32>((cohMult - 1.0) * 100.0));

        bonusText += "Observation: " + obsBonusStr + "%%  |  ";
        bonusText += "Coherence: " + cohBonusStr + "%%";

        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "%s", bonusText.c_str());
    }
    ImGui::End();

    // 4. Pop styles
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    // Handle close
    if (!windowOpen) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void MilestoneView::Render(GameState* state, Renderer* renderer) {
    (void)renderer; // Not used for ImGui rendering

    if (state->GetActiveModal() != ActiveModal::Milestones) return;

    ImGui::SetNextWindowSize(ImVec2(600, 700), ImGuiCond_Once);
    ImGui::SetNextWindowPos(
        ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f),
        ImGuiCond_Once,
        ImVec2(0.5f, 0.5f)
    );

    bool showWindow = true;
    if (ImGui::Begin("Timeline Milestones", &showWindow, ImGuiWindowFlags_NoCollapse)) {
        ImGui::Text("--- Timeline Milestones ---");
        ImGui::Separator();

        for (auto& milestone : state->m_MilestoneSystem.GetMilestones()) {
            // Determine text color based on status
            ImVec4 statusColor = ImVec4(0.5f, 0.5f, 0.5f, 1.0f); // Default: in-progress
            const char* statusText = "IN PROGRESS";

            if (milestone.claimed) {
                statusColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green: Completed
                statusText = "COMPLETED";
            } else if (milestone.progress >= milestone.target) {
                statusColor = ImVec4(1.0f, 0.8f, 0.0f, 1.0f); // Gold: Ready to Claim
                statusText = "CLAIMABLE";
            }

            // Milestone Title and Status
            ImGui::PushStyleColor(ImGuiCol_Text, statusColor);
            ImGui::Text("[%s] %s", statusText, milestone.name.c_str());
            ImGui::PopStyleColor();

            ImGui::Indent();
            ImGui::TextWrapped("Target: %s %s",
                GameUtils::FormatNumber(milestone.target, state->m_NumberFormat).c_str(),
                milestone.featureName.c_str());

            f32 progress = static_cast<f32>(milestone.progress / milestone.target);

            if (!milestone.claimed) {
                char overlay[64];
                snprintf(overlay, sizeof(overlay), "%.0f / %.0f", milestone.progress, milestone.target);
                ImGui::ProgressBar(progress, ImVec2(-1, 0), overlay);
            } else {
                ImGui::ProgressBar(1.0f, ImVec2(-1, 0), "Completed");
            }

            // Claim Button Logic
            if (milestone.progress >= milestone.target && !milestone.claimed) {
                ImGui::Spacing();
                std::string rewardText = "Claim: " + std::to_string(milestone.rewardSingularities) + " Singularities";

                if (ImGui::Button((rewardText + "##ClaimMS" + std::to_string(static_cast<int>(milestone.id))).c_str(), ImVec2(200, 30))) {
                    milestone.claimed = true;
                    state->m_Timeline.singularities += milestone.rewardSingularities;
                    state->GetSoundManager().PlaySound(SoundEffect::Achievement, 1.0f);
                }
                ImGui::Spacing();
            }

            ImGui::Unindent();
            ImGui::Separator();
        }
    }
    ImGui::End();

    if (!showWindow) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void BuyablesView::Render(GameState* state, Renderer* renderer) {
    (void)renderer;

    if (state->GetActiveModal() != ActiveModal::Buyables) return;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    f32 panelWidth = std::min(displaySize.x * 0.95f, 900.0f);
    f32 panelHeight = std::min(displaySize.y * 0.9f, 600.0f);

    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    Color mainBorderColor = Color::ElectricBlue() * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    bool showWindow = true;
    if (ImGui::Begin("Buyable Upgrades", &showWindow,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press B or ESC to close)");
        ImGui::Separator();

        f32 contentStartY = ImGui::GetCursorPosY();
        f32 buyablesContentHeight = panelHeight - contentStartY - 30.0f;

        if (ImGui::BeginChild("##BuyablesList", ImVec2(0, buyablesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            f32 buyableWidth = ImGui::GetContentRegionAvail().x;
            f32 buyableHeight = 100.0f;

            auto& buyables = state->m_BuyableManager.GetBuyables();

            for (size_t i = 0; i < buyables.size(); i++) {
                const auto& buyable = buyables[i];
                bool maxed = buyable.IsMaxed();
                bool canAfford = buyable.CanAfford(state->m_Resources[0]);
                bool disabledByChallenge = state->m_ChallengeManager.HasModifier(ChallengeModifier::NoBuyables);
                bool interactive = canAfford && !disabledByChallenge;

                Color buyableBg = maxed ? Color(0.1f, 0.2f, 0.15f, 1.0f) : Color(0.15f, 0.15f, 0.2f, 1.0f);
                Color buyableBorder = maxed ? Color::CoherenceGreen() * 0.6f : Color::ElectricBlue() * 0.6f;

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(buyableBg));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(buyableBorder));

                std::string buyableChildName = "##Buyable_" + std::to_string(i);

                if (ImGui::BeginChild(buyableChildName.c_str(), ImVec2(buyableWidth, buyableHeight), true, ImGuiWindowFlags_NoScrollbar)) {
                    ImGui::TextColored(ToImVec4(Color::White()), "%s", buyable.name.c_str());

                    ImGui::SameLine(400.0f);
                    ImGui::TextColored(ToImVec4(Color::QuantumPurple()), "Owned: %s", buyable.GetProgressString().c_str());

                    ImGui::TextWrapped("%s", buyable.description.c_str());

                    ImGui::SetCursorPosY(buyableHeight - 35.0f);

                    if (!maxed) {
                        std::string costStr = GameUtils::FormatNumber(buyable.GetCurrentCost(), state->m_NumberFormat) + " Qubits";
                        Color costColor = interactive ? Color::CoherenceGreen() : Color(0.7f, 0.5f, 0.5f, 1.0f);
                        ImGui::TextColored(ToImVec4(costColor), "Cost: %s", costStr.c_str());

                        ImGui::SameLine(buyableWidth - 120.0f);
                        std::string btnText = disabledByChallenge ? "DISABLED" : "PURCHASE";

                        Color btnColor = interactive ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                        Color btnHoveredColor = interactive ? Color::CoherenceGreen() * 1.5f : Color(0.4f, 0.4f, 0.4f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                        if (ImGui::Button(btnText.c_str(), ImVec2(110.0f, 35.0f)) && interactive) {
                            state->m_BuyableManager.Purchase(buyable.id, state);
                            state->GetSoundManager().PlaySound(SoundEffect::ButtonPress, 0.8f);
                        }
                        ImGui::PopStyleColor(3);
                    } else {
                        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "MAXED OUT");
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                ImGui::Spacing();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    if (!showWindow) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void ChallengeView::Render(GameState* state, Renderer* renderer) {
    (void)renderer;

    if (state->GetActiveModal() != ActiveModal::Challenges) return;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    f32 panelWidth = std::min(displaySize.x * 0.95f, 900.0f);
    f32 panelHeight = std::min(displaySize.y * 0.9f, 650.0f);

    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    Color mainBorderColor = Color::Red() * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    bool showWindow = true;
    if (ImGui::Begin("Quantum Challenges", &showWindow,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press C or ESC to close)");
        ImGui::Separator();

        const Challenge* currentChallenge = state->m_ChallengeManager.GetCurrentChallenge();
        f32 contentStartY = ImGui::GetCursorPosY();

        if (currentChallenge) {
            ImGui::TextColored(ToImVec4(Color::Red()), "⚠ ACTIVE CHALLENGE: %s", currentChallenge->name.c_str());

            f64 currentQubits = state->GetResource(QuantumResource::Qubits);
            std::string goalText = "Goal: " + GameUtils::FormatNumber(currentQubits, state->m_NumberFormat) +
                                   " / " + GameUtils::FormatNumber(currentChallenge->goalQubits, state->m_NumberFormat) + " Qubits";
            ImGui::TextColored(ToImVec4(Color::Yellow()), "%s", goalText.c_str());

            ImGui::Spacing();
            contentStartY = ImGui::GetCursorPosY();
        }

        ImGui::Separator();

        f32 challengesContentHeight = panelHeight - contentStartY - 30.0f;

        if (ImGui::BeginChild("##ChallengesList", ImVec2(0, challengesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            f32 challengeWidth = ImGui::GetContentRegionAvail().x;
            f32 challengeHeight = 120.0f;

            auto& challenges = state->m_ChallengeManager.GetChallenges();
            i32 currentPrestige = state->m_Statistics.totalPrestigesPerformed;

            for (size_t i = 0; i < challenges.size(); i++) {
                const auto& challenge = challenges[i];
                bool completed = challenge.completed;
                bool active = challenge.active;
                bool canEnter = challenge.CanEnter(currentPrestige, currentChallenge != nullptr && currentChallenge != &challenge);

                Color challengeBg;
                Color challengeBorder;
                if (active) {
                    challengeBg = Color(0.2f, 0.1f, 0.1f, 1.0f);
                    challengeBorder = Color::Red();
                } else if (completed) {
                    challengeBg = Color(0.1f, 0.2f, 0.15f, 1.0f);
                    challengeBorder = Color::CoherenceGreen() * 0.6f;
                } else {
                    challengeBg = Color(0.15f, 0.15f, 0.2f, 1.0f);
                    challengeBorder = Color::Red() * 0.6f;
                }

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(challengeBg));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(challengeBorder));

                std::string challengeChildName = "##Challenge_" + std::to_string(i);

                if (ImGui::BeginChild(challengeChildName.c_str(), ImVec2(challengeWidth, challengeHeight), true, ImGuiWindowFlags_NoScrollbar)) {
                    std::string nameStr = challenge.name;
                    if (active) nameStr += " [ACTIVE]";
                    if (completed) nameStr += " [COMPLETED]";
                    Color nameColor = completed ? Color::CoherenceGreen() : (active ? Color::Red() : Color::White());
                    ImGui::TextColored(ToImVec4(nameColor), "%s", nameStr.c_str());

                    ImGui::TextWrapped("%s", challenge.description.c_str());

                    std::string reqText = "Requires: " + std::to_string(challenge.minPrestigeLevel) + " prestiges";
                    Color reqColor = currentPrestige >= challenge.minPrestigeLevel ? Color::CoherenceGreen() : Color(0.7f, 0.5f, 0.5f, 1.0f);
                    ImGui::TextColored(ToImVec4(reqColor), "%s", reqText.c_str());

                    std::string goalText = "Goal: " + GameUtils::FormatNumber(challenge.goalQubits, state->m_NumberFormat) + " Qubits";
                    ImGui::TextColored(ToImVec4(Color::Yellow()), "%s", goalText.c_str());

                    ImGui::TextColored(ToImVec4(Color::QuantumPurple()), "Reward: %s", challenge.rewardDescription.c_str());

                    if (!completed) {
                        ImGui::SetCursorPosY(challengeHeight - 40.0f);
                        ImGui::SameLine(challengeWidth - 120.0f);

                        if (active) {
                            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(Color::Red() * 0.3f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(Color::Red() * 0.5f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::Red()));
                            if (ImGui::Button("EXIT##ChallengeBtn", ImVec2(110.0f, 30.0f))) {
                                state->m_ChallengeManager.ExitChallenge(state);
                            }
                            ImGui::PopStyleColor(3);
                        } else {
                            Color btnColor = canEnter ? Color::CoherenceGreen() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                            Color btnHoveredColor = canEnter ? Color::CoherenceGreen() * 1.5f : Color(0.4f, 0.4f, 0.4f, 1.0f);

                            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                            ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                            if (ImGui::Button("ENTER##ChallengeBtn", ImVec2(110.0f, 30.0f)) && canEnter) {
                                state->m_ChallengeManager.EnterChallenge(challenge.id, state);
                            }
                            ImGui::PopStyleColor(3);
                        }
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                ImGui::Spacing();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    if (!showWindow) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void EssenceShopView::Render(GameState* state, Renderer* renderer) {
    (void)renderer;

    if (state->GetActiveModal() != ActiveModal::EssenceShop) return;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    f32 panelWidth = std::min(displaySize.x * 0.95f, 900.0f);
    f32 panelHeight = std::min(displaySize.y * 0.9f, 650.0f);

    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    Color mainBorderColor = Color::Magenta() * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    bool showWindow = true;
    if (ImGui::Begin("Essence Shop - Permanent Upgrades", &showWindow,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press E or ESC to close)");
        ImGui::Separator();

        std::string essenceText = "Your Quantum Essence: " + GameUtils::FormatNumber(state->m_QuantumEssence, state->m_NumberFormat);
        ImGui::TextColored(ToImVec4(Color::Magenta() * 1.3f), "%s", essenceText.c_str());

        ImGui::Spacing();
        ImGui::Separator();

        f32 upgradesContentHeight = panelHeight - ImGui::GetCursorPosY() - 30.0f;

        if (ImGui::BeginChild("##EssenceUpgradesList", ImVec2(0, upgradesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            f32 upgradeWidth = ImGui::GetContentRegionAvail().x;
            f32 upgradeHeight = 100.0f;

            auto& upgrades = state->m_EssenceShopManager.GetUpgrades();

            for (size_t i = 0; i < upgrades.size(); i++) {
                const auto& upgrade = upgrades[i];
                bool maxed = upgrade.IsMaxed();
                bool canAfford = upgrade.CanAfford(state->m_QuantumEssence);

                Color upgradeBg = maxed ? Color(0.1f, 0.2f, 0.15f, 1.0f) : Color(0.15f, 0.15f, 0.2f, 1.0f);
                Color upgradeBorder = maxed ? Color::CoherenceGreen() * 0.6f : Color::Magenta() * 0.6f;

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(upgradeBg));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(upgradeBorder));

                if (ImGui::BeginChild(upgrade.name.c_str(), ImVec2(upgradeWidth, upgradeHeight), true, ImGuiWindowFlags_NoScrollbar)) {
                    ImGui::TextColored(ToImVec4(Color::White()), "%s", upgrade.name.c_str());
                    ImGui::SameLine(400.0f);
                    ImGui::TextColored(ToImVec4(Color::Magenta()), "Owned: %s", upgrade.GetProgressString().c_str());

                    ImGui::TextWrapped("%s", upgrade.description.c_str());

                    ImGui::SetCursorPosY(upgradeHeight - 35.0f);

                    if (!maxed) {
                        f64 cost = upgrade.GetCurrentCost();
                        std::string costStr = "Cost: " + GameUtils::FormatNumber(cost, state->m_NumberFormat) + " Essence";
                        Color costColor = canAfford ? Color::Magenta() * 1.3f : Color(0.7f, 0.5f, 0.5f, 1.0f);
                        ImGui::TextColored(ToImVec4(costColor), "%s", costStr.c_str());

                        ImGui::SameLine(upgradeWidth - 120.0f);
                        Color btnColor = canAfford ? Color::Magenta() : Color(0.3f, 0.3f, 0.3f, 1.0f);
                        Color btnHoveredColor = canAfford ? Color::Magenta() * 1.5f : Color(0.4f, 0.4f, 0.4f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                        if (ImGui::Button("PURCHASE", ImVec2(110.0f, 35.0f)) && canAfford) {
                            state->m_EssenceShopManager.Purchase(upgrade.id, state);
                            state->GetSoundManager().PlaySound(SoundEffect::ButtonPress, 0.8f);
                        }
                        ImGui::PopStyleColor(3);
                    } else {
                        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "MAXED OUT");
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                ImGui::Spacing();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    if (!showWindow) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void SingularityShopView::Render(GameState* state, Renderer* renderer) {
    (void)renderer;

    if (state->GetActiveModal() != ActiveModal::SingularityShop) return;

    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    f32 panelWidth = std::min(displaySize.x * 0.95f, 900.0f);
    f32 panelHeight = std::min(displaySize.y * 0.9f, 650.0f);

    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    Color mainBorderColor = Color(0.5f, 0.0f, 1.0f, 1.0f) * 0.8f;
    Color mainBgColor = Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(mainBgColor));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(mainBorderColor));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15.0f, 15.0f));

    bool showWindow = true;
    if (ImGui::Begin("Singularity Shop - Cosmic Upgrades", &showWindow,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press ESC to close)");
        ImGui::Separator();

        std::string singularityText = "Your Singularities: " + GameUtils::FormatNumber(state->m_Timeline.singularities, state->m_NumberFormat);
        ImGui::TextColored(ToImVec4(Color(0.8f, 0.0f, 1.0f, 1.0f) * 1.3f), "%s", singularityText.c_str());

        ImGui::Spacing();
        ImGui::Separator();

        f32 upgradesContentHeight = panelHeight - ImGui::GetCursorPosY() - 30.0f;

        if (ImGui::BeginChild("##UpgradesList", ImVec2(0, upgradesContentHeight), false, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
            f32 upgradeWidth = ImGui::GetContentRegionAvail().x;
            f32 upgradeHeight = 100.0f;

            auto& upgrades = state->m_SingularityShopManager.GetUpgrades();

            for (size_t i = 0; i < upgrades.size(); i++) {
                const auto& upgrade = upgrades[i];
                bool maxed = upgrade.IsMaxed();
                bool canAfford = upgrade.CanAfford(state->m_Timeline.singularities);

                Color upgradeBg = maxed ? Color(0.1f, 0.15f, 0.2f, 1.0f) : Color(0.1f, 0.1f, 0.15f, 1.0f);
                Color upgradeBorder = maxed ? Color::CoherenceGreen() * 0.6f : mainBorderColor * 0.6f;

                ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
                ImGui::PushStyleColor(ImGuiCol_ChildBg, ToImVec4(upgradeBg));
                ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(upgradeBorder));

                if (ImGui::BeginChild(upgrade.name.c_str(), ImVec2(upgradeWidth, upgradeHeight), true, ImGuiWindowFlags_NoScrollbar)) {
                    ImGui::TextColored(ToImVec4(Color::White()), "%s", upgrade.name.c_str());
                    ImGui::SameLine(400.0f);
                    ImGui::TextColored(ToImVec4(Color(0.8f, 0.0f, 1.0f, 1.0f)), "Owned: %s", upgrade.GetProgressString().c_str());

                    ImGui::TextWrapped("%s", upgrade.description.c_str());

                    ImGui::SetCursorPosY(upgradeHeight - 35.0f);

                    if (!maxed) {
                        f64 cost = upgrade.GetCurrentCost();
                        std::string costStr = "Cost: " + GameUtils::FormatNumber(cost, state->m_NumberFormat) + " Singularities";
                        Color costColor = canAfford ? Color(0.8f, 0.0f, 1.0f, 1.0f) * 1.3f : Color(0.7f, 0.5f, 0.5f, 1.0f);
                        ImGui::TextColored(ToImVec4(costColor), "%s", costStr.c_str());

                        ImGui::SameLine(upgradeWidth - 120.0f);
                        Color btnColor = canAfford ? Color(0.5f, 0.0f, 1.0f, 1.0f) : Color(0.3f, 0.3f, 0.3f, 1.0f);
                        Color btnHoveredColor = canAfford ? Color(0.8f, 0.0f, 1.0f, 1.0f) : Color(0.4f, 0.4f, 0.4f, 1.0f);

                        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(btnColor * 0.3f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(btnHoveredColor * 0.5f));
                        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(btnHoveredColor));

                        if (ImGui::Button("PURCHASE", ImVec2(110.0f, 35.0f)) && canAfford) {
                            state->m_SingularityShopManager.Purchase(upgrade.id, state);
                            state->GetSoundManager().PlaySound(SoundEffect::ButtonPress, 0.8f);
                        }
                        ImGui::PopStyleColor(3);
                    } else {
                        ImGui::TextColored(ToImVec4(Color::CoherenceGreen()), "MAXED OUT");
                    }
                }
                ImGui::EndChild();

                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);

                ImGui::Spacing();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    if (!showWindow) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void SpaceshipView::Render(GameState* state, Renderer* renderer) {
    if (state->GetActiveModal() != ActiveModal::Spaceship) return;

    // We still pass the renderer to the m_Spaceship functions, but the main UI relies on ImGui
    (void)renderer;

    // 1. Responsive position and size
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    f32 panelWidth = std::min(displaySize.x * 0.95f, 1100.0f);
    f32 panelHeight = std::min(displaySize.y * 0.9f, 700.0f);

    // Center the window
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles (matching old aesthetics)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(Color(0.05f, 0.05f, 0.1f, 0.95f) * 0.8f));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color(1.0f, 0.7f, 0.0f, 1.0f) * 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 2.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    // 3. Begin the main spaceship window
    bool windowOpen = true;
    if (ImGui::Begin("Spaceship - Repair and Upgrade", &windowOpen,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // --- Header ---
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "(Press H or ESC to close)");
        ImGui::Separator();

        // --- Split Panel Layout ---
        f32 availableHeight = panelHeight - ImGui::GetCursorPosY() - 40.0f;
        f32 spacing = 20.0f;
        f32 totalContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x - spacing;
        f32 panelContentWidth = totalContentWidth * 0.5f;

        // Left panel: Ship status and installed parts
        ImGui::BeginChild("##ShipStatusPanel", ImVec2(panelContentWidth, availableHeight), true);
        {
            state->m_Spaceship.RenderShipPanel();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // Right panel: Part inventory
        ImGui::BeginChild("##InventoryPanel", ImVec2(panelContentWidth, availableHeight), true);
        {
            state->m_Spaceship.RenderInventoryPanel();
        }
        ImGui::EndChild();

        // --- Instructions at bottom ---
        ImGui::SetCursorPosY(panelHeight - 35.0f);
        ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.7f, 1.0f)),
                           "Ship parts drop from Research Station observations. Install parts to increase production & unlock travel!");
    }
    ImGui::End();

    // 4. Pop styles
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);

    // Handle close
    if (!windowOpen) {
        state->SetActiveModal(ActiveModal::None);
    }
}

void CombatView::Render(GameState* state, Renderer* renderer) {
    if (state->GetActiveModal() != ActiveModal::Combat) return;

    // Pass the renderer to CombatSystem (even if unused in ImGui logic)
    state->m_CombatSystem.RenderCombatUI(renderer);

    // Draw XP bar on the background layer for visibility
    f32 xpBarWidth = 300.0f;
    f32 xpBarHeight = 25.0f;

    ImDrawList* bg_draw_list = ImGui::GetBackgroundDrawList();

    ImVec2 xpBarStart(10.0f, 10.0f);
    ImVec2 xpBarEnd(xpBarStart.x + xpBarWidth, xpBarStart.y + xpBarHeight);

    f64 xpRequired = state->GetXPForNextLevel();
    f64 xpPercent = xpRequired > 0 ? (state->m_PlayerXP / xpRequired) : 0.0;

    // 1. Draw Background Rect
    bg_draw_list->AddRectFilled(xpBarStart, xpBarEnd, ImGui::GetColorU32(ImVec4(0.2f, 0.2f, 0.2f, 0.8f)));

    // 2. Draw Fill Rect
    ImVec2 xpBarFillEnd(xpBarStart.x + xpBarWidth * static_cast<f32>(xpPercent), xpBarEnd.y);
    bg_draw_list->AddRectFilled(xpBarStart, xpBarFillEnd, ImGui::GetColorU32(ImVec4(1.0f, 0.9f, 0.0f, 1.0f))); // Gold

    char levelText[64];
    snprintf(levelText, sizeof(levelText), "Level %d - %.0f / %.0f XP",
             state->m_PlayerLevel, state->m_PlayerXP, xpRequired);

    // 3. Draw Text (Centered over the bar)
    ImVec2 textPos(xpBarStart.x + 5.0f, xpBarStart.y + 5.0f);
    bg_draw_list->AddText(textPos, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)), levelText);
}

void GatchaView::Render(GameState* state, Renderer* renderer) {
    if (state->GetActiveModal() != ActiveModal::Gatcha) return;

    // GatchaSystem handles its own ImGui window creation
    state->m_GatchaSystem.RenderSummonUI(renderer, state);
}

void SkillTreeView::Render(GameState* state, Renderer* renderer) {
    if (state->GetActiveModal() != ActiveModal::Skills) return;

    // SkillTree handles its own ImGui window creation
    state->m_SkillTree.RenderSkillTree(renderer, state);
}

void EnhancementView::Render(GameState* state, Renderer* renderer) {
    if (state->GetActiveModal() != ActiveModal::Enhancement) return;

    // EnhancementSystem handles its own ImGui window creation
    state->m_EnhancementSystem.RenderEnhancementUI(renderer, state);
}

void SpecializedSkillsView::Render(GameState* state, Renderer* renderer) {
    if (state->GetActiveModal() != ActiveModal::SpecializedSkills) return;
    (void)renderer; // Unused in ImGui rendering

    // Responsive window setup
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    f32 panelWidth = std::min(displaySize.x * 0.95f, 800.0f);
    f32 panelHeight = std::min(displaySize.y * 0.9f, 600.0f);
    ImVec2 centerPos(displaySize.x * 0.5f, displaySize.y * 0.5f);
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 20));

    bool windowOpen = true;
    if (ImGui::Begin("Specialized Skills", &windowOpen,
                     ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoCollapse)) {

        // Header
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "SPECIALIZED SKILLS");
        ImGui::TextWrapped("Gain experience through gameplay actions. Each skill provides unique bonuses.");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Summary stats
        ImGui::Text("Total Skill Level: %d", state->m_SpecializedSkills.GetTotalSkillLevel());
        ImGui::Text("Average Skill Level: %d", state->m_SpecializedSkills.GetAverageSkillLevel());
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // Skill cards in a grid
        float cardWidth = 350.0f;
        float availableWidth = ImGui::GetContentRegionAvail().x;
        i32 cardsPerRow = static_cast<i32>(availableWidth / (cardWidth + 10.0f));
        if (cardsPerRow < 1) cardsPerRow = 1;

        // Define skill data
        struct SkillDisplay {
            SkillCategory category;
            const char* name;
            const char* icon;
            const char* description;
            ImVec4 color;
            const char* bonusDesc;
        };

        SkillDisplay skills[] = {
            {
                SkillCategory::Observation,
                "OBSERVATION",
                "",
                "Production & Discovery\nGain XP by observing and unlocking stations.",
                ImVec4(0.3f, 0.7f, 1.0f, 1.0f),
                "+%% Global Production"
            },
            {
                SkillCategory::Engineering,
                "ENGINEERING",
                "",
                "Efficiency & Building\nGain XP by upgrading stations and researching.",
                ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
                "+%% Cost Reduction"
            },
            {
                SkillCategory::Command,
                "COMMAND",
                "",
                "Combat & Management\nGain XP by winning battles and managing crew.",
                ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
                "+%% Combat Power"
            }
        };

        // Render each skill card
        for (i32 i = 0; i < 3; i++) {
            const SkillDisplay& display = skills[i];
            const SpecializedSkill& skill = state->m_SpecializedSkills.GetSkill(display.category);

            // Card background
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.2f, 0.9f));
            ImGui::PushStyleColor(ImGuiCol_Border, display.color);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

            char cardId[64];
            snprintf(cardId, sizeof(cardId), "SkillCard%d", i);

            if (ImGui::BeginChild(cardId, ImVec2(cardWidth, 220.0f), true)) {
                // Icon and title
                ImGui::TextColored(display.color, "%s %s", display.icon, display.name);

                ImGui::Spacing();

                // Level display
                ImGui::Text("Level: %d", skill.level);

                // XP Progress bar
                f32 progress = static_cast<f32>(state->m_SpecializedSkills.GetSkillProgress(display.category));
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, display.color);
                ImGui::ProgressBar(progress, ImVec2(-1, 25));
                ImGui::PopStyleColor();

                // XP text
                ImGui::Text("XP: %.0f / %.0f", skill.experience, skill.experienceToNextLevel);

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // Description
                ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + cardWidth - 40);
                ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "%s", display.description);
                ImGui::PopTextWrapPos();

                ImGui::Spacing();

                // Bonus display
                f64 bonusPercent = (skill.GetBonusMultiplier() - 1.0) * 100.0;
                ImGui::TextColored(ImVec4(1.0f, 0.9f, 0.3f, 1.0f), "Bonus: +%.0f%% (%s)",
                                 bonusPercent, display.bonusDesc);
            }
            ImGui::EndChild();

            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor(2);

            // Same line for next card if not the last in row
            if ((i + 1) % cardsPerRow != 0 && i < 2) {
                ImGui::SameLine(0, 10.0f);
            }
        }

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        // XP Rewards Reference Table
        if (ImGui::CollapsingHeader("XP Rewards Reference", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Columns(3, "XPTable", true);

            // Observation column
            ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), "OBSERVATION");
            ImGui::Separator();
            ImGui::Text("Observe Station: +5 XP");
            ImGui::Text("Unlock Station: +25 XP");
            ImGui::NextColumn();

            // Engineering column
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "ENGINEERING");
            ImGui::Separator();
            ImGui::Text("Upgrade Station: +10 XP");
            ImGui::Text("Purchase Research: +20 XP");
            ImGui::Text("Buy Upgrade: +15 XP");
            ImGui::NextColumn();

            // Command column
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "COMMAND");
            ImGui::Separator();
            ImGui::Text("Win Combat: +30 XP");
            ImGui::Text("Install Ship Part: +15 XP");
            ImGui::NextColumn();

            ImGui::Columns(1);
        }
    }
    ImGui::End();

    ImGui::PopStyleVar();

    // Handle close
    if (!windowOpen) {
        state->SetActiveModal(ActiveModal::None);
    }
}

// ============================================================================
// Phase 4.1: Particle Collection View
// ============================================================================

void CollectionView::Render(GameState* state, Renderer* renderer) {
    (void)renderer; // ImGui-only view

    ParticleCollection& collection = state->GetParticleCollection();

    // Render collection stats at the top
    RenderCollectionStats(state);

    ImGui::Separator();
    ImGui::Spacing();

    // Render equipped particles section
    RenderEquippedParticles(state);

    ImGui::Separator();
    ImGui::Spacing();

    // Render particle grid (28 particles in a 7x4 grid)
    ImGui::Text("Discovered Particles");
    ImGui::Spacing();

    const i32 cols = 7;
    const i32 rows = 4;

    for (i32 row = 0; row < rows; row++) {
        for (i32 col = 0; col < cols; col++) {
            i32 index = row * cols + col;
            if (index >= static_cast<i32>(ParticleType::COUNT)) break;

            ParticleType type = static_cast<ParticleType>(index);
            Particle* particle = collection.GetParticle(type);
            bool discovered = collection.IsDiscovered(type);

            RenderParticleSlot(state, particle, discovered);

            if (col < cols - 1) {
                ImGui::SameLine();
            }
        }
    }
}

void CollectionView::RenderCollectionStats(GameState* state) {
    ParticleCollection& collection = state->GetParticleCollection();

    i32 discovered = collection.GetDiscoveredCount();
    i32 total = collection.GetTotalParticles();
    f64 percentage = collection.GetCompletionPercentage();

    ImGui::Text("Particle Collection");
    ImGui::SameLine();
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "(%d / %d) - %.1f%%", discovered, total, percentage);

    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Discover particles by observing research stations\n0.5%% chance per observation");
    }

    // Show active bonuses from equipped particles
    f64 prodBonus = collection.GetTotalProductionBonus() * 100.0;
    f64 obsBonus = collection.GetTotalObservationBonus() * 100.0;
    f64 discoveryProd = collection.GetDiscoveryProductionBonus() * 100.0;
    f64 discoveryObs = collection.GetDiscoveryObservationBonus() * 100.0;

    if (prodBonus > 0.0 || obsBonus > 0.0 || discoveryProd > 0.0 || discoveryObs > 0.0) {
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "Active Bonuses:");
        if (prodBonus > 0.0) {
            ImGui::Text("  +%.1f%% Production", prodBonus);
        }
        if (obsBonus > 0.0) {
            ImGui::Text("  +%.1f%% Observation Rewards", obsBonus);
        }
        if (discoveryProd > 0.0) {
            ImGui::Text("  +%.1f%% Production (Discovery Tree)", discoveryProd);
        }
        if (discoveryObs > 0.0) {
            ImGui::Text("  +%.1f%% Observation (Discovery Tree)", discoveryObs);
        }
    }

    ImGui::Spacing();
    ImGui::Text("Equipment Slots: %d", collection.GetMaxEquipmentSlots());
    ImGui::Spacing();

    // Discovery tree progress
    ImGui::Text("Discovery Tree");
    ImGui::Separator();
    for (const auto& tier : collection.GetDiscoveryTiers()) {
        f32 progress = static_cast<f32>(discovered) / static_cast<f32>(tier.requiredDiscoveries);
        progress = progress > 1.0f ? 1.0f : progress;
        std::string overlay = std::to_string(discovered) + " / " + std::to_string(tier.requiredDiscoveries);
        ImGui::ProgressBar(progress, ImVec2(-1, 0), overlay.c_str());
        ImGui::SameLine();
        ImGui::Text("%s (+%.0f%% Prod, +%.0f%% Obs)", tier.name.c_str(), tier.productionBonus * 100.0, tier.observationBonus * 100.0);
    }
}

void CollectionView::RenderEquippedParticles(GameState* state) {
    ParticleCollection& collection = state->GetParticleCollection();
    auto equipped = collection.GetEquippedParticles();

    ImGui::Text("Equipped Particles (Max %d)", collection.GetMaxEquipmentSlots());
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Equipped particles provide passive bonuses to production and observation");
    }
    ImGui::Spacing();

    if (equipped.empty()) {
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No particles equipped");
    } else {
        for (Particle* p : equipped) {
            ImVec4 color = ToImVec4(p->particleColor);
            ImGui::PushStyleColor(ImGuiCol_Button, color);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 1.2f, color.y * 1.2f, color.z * 1.2f, color.w));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.8f, color.y * 0.8f, color.z * 0.8f, color.w));

            std::string buttonLabel = p->name + "##equipped_" + std::to_string(static_cast<i32>(p->type));
            if (ImGui::Button(buttonLabel.c_str(), ImVec2(150, 30))) {
                collection.UnequipParticle(p->type);
            }

            ImGui::PopStyleColor(3);

            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("%s", p->name.c_str());
                ImGui::Separator();
                ImGui::TextWrapped("%s", p->description.c_str());
                ImGui::Spacing();
                if (p->productionBonus > 0.0) {
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "+%.1f%% Production", p->productionBonus * 100.0);
                }
                if (p->observationBonus > 0.0) {
                    ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "+%.1f%% Observation", p->observationBonus * 100.0);
                }
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Click to unequip");
                ImGui::EndTooltip();
            }

            ImGui::SameLine();
        }
    }

    ImGui::Spacing();
}

void CollectionView::RenderParticleSlot(GameState* state, Particle* particle, bool discovered) {
    if (!particle) return;

    ParticleCollection& collection = state->GetParticleCollection();
    const f32 slotSize = 90.0f;

    ImVec4 slotColor;
    if (!discovered) {
        // Undiscovered - dark gray
        slotColor = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    } else {
        // Discovered - use particle color
        slotColor = ToImVec4(particle->particleColor);
    }

    // Dim the color if not equipped
    bool isEquipped = collection.IsEquipped(particle->type);
    if (!isEquipped && discovered) {
        slotColor.w = 0.7f; // Slightly transparent
    }

    ImGui::PushStyleColor(ImGuiCol_Button, slotColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(slotColor.x * 1.3f, slotColor.y * 1.3f, slotColor.z * 1.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(slotColor.x * 0.8f, slotColor.y * 0.8f, slotColor.z * 0.8f, 1.0f));

    std::string label = discovered ? particle->name : "???";
    std::string buttonId = "##particle_" + std::to_string(static_cast<i32>(particle->type));

    if (ImGui::Button((label + buttonId).c_str(), ImVec2(slotSize, slotSize))) {
        if (discovered) {
            if (isEquipped) {
                collection.UnequipParticle(particle->type);
            } else {
                collection.EquipParticle(particle->type);
            }
        }
    }

    ImGui::PopStyleColor(3);

    // Tooltip on hover
    if (ImGui::IsItemHovered()) {
        if (discovered) {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(300.0f);

            // Name and rarity
            const char* rarityNames[] = {"Common", "Uncommon", "Rare", "Exotic", "Legendary", "Mythical"};
            ImVec4 rarityColors[] = {
                ImVec4(0.7f, 0.7f, 0.7f, 1.0f), // Common
                ImVec4(0.3f, 1.0f, 0.3f, 1.0f), // Uncommon
                ImVec4(0.3f, 0.5f, 1.0f, 1.0f), // Rare
                ImVec4(0.8f, 0.3f, 1.0f, 1.0f), // Exotic
                ImVec4(1.0f, 0.6f, 0.0f, 1.0f), // Legendary
                ImVec4(1.0f, 0.0f, 0.5f, 1.0f)  // Mythical
            };

            ImGui::Text("%s", particle->name.c_str());
            ImGui::SameLine();
            i32 rarityIndex = static_cast<i32>(particle->rarity);
            ImGui::TextColored(rarityColors[rarityIndex], "[%s]", rarityNames[rarityIndex]);

            ImGui::Separator();
            ImGui::TextWrapped("%s", particle->description.c_str());
            ImGui::Spacing();

            // Bonuses
            if (particle->productionBonus > 0.0) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.5f, 1.0f), "+%.1f%% Production", particle->productionBonus * 100.0);
            }
            if (particle->observationBonus > 0.0) {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "+%.1f%% Observation", particle->observationBonus * 100.0);
            }

            ImGui::Spacing();
            if (isEquipped) {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "[EQUIPPED] Click to unequip");
            } else {
                ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "Click to equip");
            }

            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        } else {
            ImGui::SetTooltip("Not yet discovered\nKeep observing to find new particles!");
        }
    }
}

} // namespace UI
