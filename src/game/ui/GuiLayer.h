#pragma once

#include "Types.h"
#include "UITheme.h"
#include <memory>

// Forward declarations
class GameState;
class Renderer;
struct ResearchStation;

namespace UI {

/**
 * Base View Interface
 * All UI views should inherit from this and implement Render()
 * Views are stateless and only render based on GameState data
 */
class IView {
public:
    virtual ~IView() = default;

    /**
     * Render the view content
     * @param state GameState reference for reading data
     * @param renderer Renderer reference (may be unused for ImGui-only views)
     */
    virtual void Render(GameState* state, Renderer* renderer) = 0;
};

/**
 * Navigation Bar View
 * Renders the top navigation bar with mode switching buttons
 */
class NavigationView : public IView {
public:
    NavigationView() = default;
    ~NavigationView() override = default;

    void Render(GameState* state, Renderer* renderer) override;

private:
    void RenderBoostButton(GameState* state);
    void RenderMoreMenu(GameState* state);
};

/**
 * Station View
 * Renders the research stations with observe/upgrade buttons
 */
class StationView : public IView {
public:
    StationView() = default;
    ~StationView() override = default;

    void Render(GameState* state, Renderer* renderer) override;

private:
    void RenderUnlockedStation(GameState* state, ResearchStation& station, size_t index);
    void RenderLockedStation(GameState* state, ResearchStation& station, size_t index);
    void RenderPrestigeButton(GameState* state);
};

/**
 * Resource Display View
 * Renders the top resource bar showing Qubits, Coherence, Entanglement
 */
class ResourceView : public IView {
public:
    ResourceView() = default;
    ~ResourceView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Achievement View
 * Renders the achievements modal window
 */
class AchievementView : public IView {
public:
    AchievementView() = default;
    ~AchievementView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Statistics View
 * Renders the statistics modal window
 */
class StatisticsView : public IView {
public:
    StatisticsView() = default;
    ~StatisticsView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Research Tree View
 * Renders the research tree modal window
 */
class ResearchView : public IView {
public:
    ResearchView() = default;
    ~ResearchView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Milestone View
 * Renders the milestones modal window
 */
class MilestoneView : public IView {
public:
    MilestoneView() = default;
    ~MilestoneView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Buyables View
 * Renders the buyables modal window
 */
class BuyablesView : public IView {
public:
    BuyablesView() = default;
    ~BuyablesView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Challenge View
 * Renders the challenges modal window
 */
class ChallengeView : public IView {
public:
    ChallengeView() = default;
    ~ChallengeView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Essence Shop View
 * Renders the essence shop modal window
 */
class EssenceShopView : public IView {
public:
    EssenceShopView() = default;
    ~EssenceShopView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Singularity Shop View
 * Renders the singularity shop modal window
 */
class SingularityShopView : public IView {
public:
    SingularityShopView() = default;
    ~SingularityShopView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Spaceship View
 * Renders the spaceship modal window
 */
class SpaceshipView : public IView {
public:
    SpaceshipView() = default;
    ~SpaceshipView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Combat View
 * Renders the combat modal window
 */
class CombatView : public IView {
public:
    CombatView() = default;
    ~CombatView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Gatcha View
 * Renders the gatcha/summon modal window
 */
class GatchaView : public IView {
public:
    GatchaView() = default;
    ~GatchaView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Skill Tree View
 * Renders the skill tree modal window
 */
class SkillTreeView : public IView {
public:
    SkillTreeView() = default;
    ~SkillTreeView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Enhancement View
 * Renders the enhancement modal window
 */
class EnhancementView : public IView {
public:
    EnhancementView() = default;
    ~EnhancementView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Specialized Skills View
 * Renders the specialized skills modal window
 */
class SpecializedSkillsView : public IView {
public:
    SpecializedSkillsView() = default;
    ~SpecializedSkillsView() override = default;

    void Render(GameState* state, Renderer* renderer) override;
};

/**
 * Tutorial Overlay
 * Renders step-based tutorial system for new players
 */
class TutorialOverlay;

/**
 * GuiLayer
 *
 * Central UI orchestration layer that owns all views and delegates rendering.
 * This is the single entry point for all UI rendering in the game.
 *
 * Design Principles:
 * - Views are stateless and only read from GameState
 * - GuiLayer does not contain game logic, only rendering coordination
 * - All ImGui calls should be in views, not in GameState
 */
class GuiLayer {
public:
    GuiLayer();
    ~GuiLayer();

    /**
     * Initialize the GuiLayer and create all views
     */
    void Initialize();

    /**
     * Update tutorial logic and check for step progression
     * @param state GameState reference to read resources and check conditions
     */
    void Update(GameState* state);

    /**
     * Render all UI elements
     * @param state GameState reference for views to read data
     * @param renderer Renderer reference (may be unused for ImGui-only views)
     */
    void Render(GameState* state, Renderer* renderer);

    /**
     * Get tutorial overlay (for save/load)
     */
    TutorialOverlay* GetTutorialOverlay() { return m_TutorialOverlay.get(); }

private:
    // Core Views (always visible)
    std::unique_ptr<ResourceView> m_ResourceView;
    std::unique_ptr<NavigationView> m_NavigationView;
    std::unique_ptr<StationView> m_StationView;

    // Modal Views (shown based on GameState flags)
    std::unique_ptr<AchievementView> m_AchievementView;
    std::unique_ptr<StatisticsView> m_StatisticsView;
    std::unique_ptr<ResearchView> m_ResearchView;
    std::unique_ptr<MilestoneView> m_MilestoneView;
    std::unique_ptr<BuyablesView> m_BuyablesView;
    std::unique_ptr<ChallengeView> m_ChallengeView;
    std::unique_ptr<EssenceShopView> m_EssenceShopView;
    std::unique_ptr<SingularityShopView> m_SingularityShopView;
    std::unique_ptr<SpaceshipView> m_SpaceshipView;
    std::unique_ptr<CombatView> m_CombatView;
    std::unique_ptr<GatchaView> m_GatchaView;
    std::unique_ptr<SkillTreeView> m_SkillTreeView;
    std::unique_ptr<EnhancementView> m_EnhancementView;
    std::unique_ptr<SpecializedSkillsView> m_SpecializedSkillsView;

    // Tutorial System (overlays everything when active)
    std::unique_ptr<TutorialOverlay> m_TutorialOverlay;

    /**
     * Render achievement and milestone notifications (always check)
     */
    void RenderNotifications(GameState* state, Renderer* renderer);

    /**
     * Render active quantum event banner
     */
    void RenderActiveEvent(GameState* state, Renderer* renderer);
};

} // namespace UI
