#pragma once

#include "Types.h"
#include "Spaceship.h"
#include <vector>
#include <string>
#include <array>

// Forward declarations
namespace Rml { class ElementDocument; }
class Renderer;
class GameState;

// Summon banner types (different costs and rates)
enum class SummonBanner {
    Basic,      // Costs Credits, standard rates
    Advanced,   // Costs Stellar Shards, better rates
    Elite       // Costs Tickets, guaranteed Rare+
};

// Currency source attribution for telemetry and balancing
enum class CurrencySource {
    LevelUps,
    Combat,
    Stations,
    Prestige,
    Milestones,
    COUNT
};

// Summon result for a single pull
struct SummonResult {
    ShipPart part;
    bool isPityDrop;        // Was this from pity?
    bool isBonusDrop;       // Was this a bonus from 10x?
    f32 revealDelay;        // Animation delay before revealing

    SummonResult() : isPityDrop(false), isBonusDrop(false), revealDelay(0.0f) {}
    SummonResult(const ShipPart& p, bool pity = false, bool bonus = false)
        : part(p), isPityDrop(pity), isBonusDrop(bonus), revealDelay(0.0f) {}
};

// Pity tracking for each banner
struct PityTracker {
    i32 pullsSinceRare;       // Guaranteed Rare at 10 (Basic/Advanced)
    i32 pullsSinceEpic;       // Guaranteed Epic at 30 (Advanced)
    i32 pullsSinceLegendary;  // Guaranteed Legendary at 100 (Advanced)

    PityTracker() : pullsSinceRare(0), pullsSinceEpic(0), pullsSinceLegendary(0) {}
};

// Gatcha/Summon system
class GatchaSystem {
public:
    GatchaSystem();

    // Summon operations
    SummonResult PerformSingleSummon(SummonBanner banner);
    std::vector<SummonResult> PerformTenSummon(SummonBanner banner);

    // Cost calculations
    i32 GetSingleSummonCost(SummonBanner banner) const;
    i32 GetTenSummonCost(SummonBanner banner) const;

    // Currency
    void AddStellarShards(i32 amount, CurrencySource source = CurrencySource::LevelUps);
    void AddSummonTickets(i32 amount, CurrencySource source = CurrencySource::LevelUps);
    i32 GetStellarShards() const { return m_StellarShards; }
    i32 GetSummonTickets() const { return m_SummonTickets; }
    const std::array<i32, static_cast<size_t>(CurrencySource::COUNT)>& GetShardBreakdown() const { return m_ShardTelemetry; }
    const std::array<i32, static_cast<size_t>(CurrencySource::COUNT)>& GetTicketBreakdown() const { return m_TicketTelemetry; }

    // Pity information
    const PityTracker& GetPityTracker(SummonBanner banner) const;
    void ResetPity(SummonBanner banner);

    // Statistics
    i32 GetTotalSummons() const { return m_TotalSummons; }
    i32 GetLegendaryCount() const { return m_LegendaryPulls; }

    // UI State
    bool IsAnimating() const { return m_IsAnimating; }
    const std::vector<SummonResult>& GetCurrentResults() const { return m_CurrentResults; }
    void ClearResults() { m_CurrentResults.clear(); m_IsAnimating = false; }

    // UI helpers
    void SetSelectedBanner(SummonBanner banner) { m_SelectedBanner = banner; }
    SummonBanner GetSelectedBanner() const { return m_SelectedBanner; }
    void ClaimResults(GameState* state);
    void HideSummonDocument();

    // Rendering
    void RenderSummonUI(Renderer* renderer, GameState* state);
    void Update(f64 deltaTime);

    // Input handling
    void HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state);

    // Save/Load
    void SaveToJson(std::ostream& file) const;
    void LoadFromJson(const std::string& line);

private:
    // Currencies
    i32 m_StellarShards;    // Premium currency (earned from achievements, milestones)
    i32 m_SummonTickets;    // Special tickets (earned from raids, events)
    std::array<i32, static_cast<size_t>(CurrencySource::COUNT)> m_ShardTelemetry;
    std::array<i32, static_cast<size_t>(CurrencySource::COUNT)> m_TicketTelemetry;

    // Pity tracking (one per banner)
    PityTracker m_BasicPity;
    PityTracker m_AdvancedPity;
    PityTracker m_ElitePity;

    // Statistics
    i32 m_TotalSummons;
    i32 m_LegendaryPulls;

    // Animation state
    bool m_IsAnimating;
    f64 m_AnimationTimer;
    std::vector<SummonResult> m_CurrentResults;
    i32 m_CurrentRevealIndex;

    // UI state
    SummonBanner m_SelectedBanner;

#ifdef RMLUI_ENABLED
    class RmlSummonListener;
    class Rml::ElementDocument* m_SummonDocument = nullptr;
    Scope<RmlSummonListener> m_SummonListener;
#endif

    // Helper methods
    ShipPart GeneratePartForBanner(SummonBanner banner, bool forcedRarity = false, PartRarity rarity = PartRarity::Common);
    PartRarity RollRarity(SummonBanner banner);
    void IncrementPity(SummonBanner banner);
    PityTracker& GetPityTrackerMutable(SummonBanner banner);
    bool CheckAndConsumePity(SummonBanner banner, PartRarity& outRarity);

    // UI rendering helpers
    void RenderBannerSelection(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth);
    void RenderSummonButtons(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth);
    void RenderPityCounters(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth);
    void RenderSummonAnimation(Renderer* renderer, GameState* state);
    void RenderRateInfo(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth);
    void RenderCurrencyBreakdown(f32 panelWidth) const;

    static const char* GetSourceLabel(CurrencySource source);
    static size_t GetSourceIndex(CurrencySource source);

#ifdef RMLUI_ENABLED
    void UpdateSummonDocument(GameState* state);
    void InitializeSummonDocument(GameState* state);
    void ExecuteSummon(i32 count, GameState* state);
#endif
};
