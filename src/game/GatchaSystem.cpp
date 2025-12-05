#include "GatchaSystem.h"
#include "ImGuiUtils.h"
#include "Renderer.h" // Still needed for GameState dependency
#include "Logger.h"
#include "GameState.h"
#include "GameUtils.h"
#include "imgui.h"
#ifdef RMLUI_ENABLED
#include <RmlUi/Core.h>
#endif
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <algorithm>
#include <ctime> // For rand() seed if needed
#include <sstream>

// --- Local Helpers ---

ImVec4 GetRarityColorImVec4(PartRarity rarity) {
    switch (rarity) {
        case PartRarity::Common: return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        case PartRarity::Uncommon: return ImVec4(0.3f, 0.8f, 0.3f, 1.0f);
        case PartRarity::Rare: return ImVec4(0.3f, 0.8f, 1.0f, 1.0f);
        case PartRarity::Epic: return ImVec4(0.8f, 0.3f, 1.0f, 1.0f);
        case PartRarity::Legendary: return ImVec4(1.0f, 0.7f, 0.0f, 1.0f);
        default: return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

#ifdef RMLUI_ENABLED
class GatchaSystem::RmlSummonListener : public Rml::EventListener {
public:
    RmlSummonListener(GatchaSystem* system, GameState* gameState)
        : m_System(system), m_GameState(gameState) {}

    void ProcessEvent(Rml::Event& event) override {
        Rml::Element* element = event.GetTargetElement();
        if (!element || !m_System) return;

        std::string action = element->GetAttribute<Rml::String>("data-action", "");
        if (action == "select-banner") {
            std::string banner = element->GetAttribute<Rml::String>("data-banner", "basic");
            if (banner == "advanced") {
                m_System->SetSelectedBanner(SummonBanner::Advanced);
            } else if (banner == "elite") {
                m_System->SetSelectedBanner(SummonBanner::Elite);
            } else {
                m_System->SetSelectedBanner(SummonBanner::Basic);
            }
        } else if (action == "summon-single") {
            m_System->ExecuteSummon(1, m_GameState);
        } else if (action == "summon-ten") {
            m_System->ExecuteSummon(10, m_GameState);
        } else if (action == "claim-results") {
            m_System->ClaimResults(m_GameState);
        } else if (action == "close-summon" && m_GameState) {
            m_GameState->SetActiveModal(ActiveModal::None);
            m_System->HideSummonDocument();
        }

        m_System->UpdateSummonDocument(m_GameState);
    }

private:
    GatchaSystem* m_System;
    GameState* m_GameState;
};
#endif


// --- GatchaSystem Implementation ---

GatchaSystem::GatchaSystem()
    : m_StellarShards(0), m_SummonTickets(0),
      m_TotalSummons(0), m_LegendaryPulls(0),
      m_IsAnimating(false), m_AnimationTimer(0.0),
      m_CurrentRevealIndex(0),
      m_SelectedBanner(SummonBanner::Basic) {

    m_ShardTelemetry.fill(0);
    m_TicketTelemetry.fill(0);

    // Seed the random number generator if not done globally
    // srand(static_cast<unsigned int>(time(NULL)));
}

// --- COST CALCULATIONS (Placeholder implementations) ---

i32 GatchaSystem::GetSingleSummonCost(SummonBanner banner) const {
    switch (banner) {
        case SummonBanner::Basic: return 100; // Credits
        case SummonBanner::Advanced: return 15; // Shards (aligned to new earn rates)
        case SummonBanner::Elite: return 1; // Tickets
        default: return 999;
    }
}

i32 GatchaSystem::GetTenSummonCost(SummonBanner banner) const {
    // 10x is usually 10x cost, sometimes with a small discount (9x cost)
    return GetSingleSummonCost(banner) * 10;
}

// --- PITY HELPERS AND GETTERS ---

// Private helper to get a writable reference to the correct pity tracker
PityTracker& GatchaSystem::GetPityTrackerMutable(SummonBanner banner) {
    switch (banner) {
        case SummonBanner::Advanced: return m_AdvancedPity;
        case SummonBanner::Elite: return m_ElitePity;
        case SummonBanner::Basic:
        default: return m_BasicPity;
    }
}

// Public const getter to retrieve pity status for rendering/info
const PityTracker& GatchaSystem::GetPityTracker(SummonBanner banner) const {
    switch (banner) {
        case SummonBanner::Advanced: return m_AdvancedPity;
        case SummonBanner::Elite: return m_ElitePity;
        case SummonBanner::Basic:
        default: return m_BasicPity;
    }
}

void GatchaSystem::ResetPity(SummonBanner banner) {
    PityTracker& pity = GetPityTrackerMutable(banner);
    pity.pullsSinceRare = 0;
    pity.pullsSinceEpic = 0;
    pity.pullsSinceLegendary = 0;
}

// --- PART GENERATION HELPERS (Simplified Logic) ---

PartRarity GatchaSystem::RollRarity(SummonBanner banner) {
    i32 roll = rand() % 10000; // 0 to 9999

    switch (banner) {
        case SummonBanner::Elite:
            // Guaranteed Rare+
            if (roll < 100) return PartRarity::Legendary; // 1.00%
            if (roll < 600) return PartRarity::Epic;      // 5.00%
            return PartRarity::Rare;                      // 94.00%

        case SummonBanner::Advanced:
            if (roll < 50) return PartRarity::Legendary; // 0.50%
            if (roll < 350) return PartRarity::Epic;     // 3.00%
            if (roll < 1550) return PartRarity::Rare;    // 12.00%
            if (roll < 4550) return PartRarity::Uncommon; // 30.00%
            return PartRarity::Common;                   // 54.50%

        case SummonBanner::Basic:
        default:
            if (roll < 10) return PartRarity::Legendary; // 0.10%
            if (roll < 110) return PartRarity::Epic;     // 1.00%
            if (roll < 710) return PartRarity::Rare;     // 6.00%
            if (roll < 3210) return PartRarity::Uncommon; // 25.00%
            return PartRarity::Common;                   // 67.90%
    }
}

size_t GatchaSystem::GetSourceIndex(CurrencySource source) {
    return static_cast<size_t>(source);
}

const char* GatchaSystem::GetSourceLabel(CurrencySource source) {
    switch (source) {
        case CurrencySource::LevelUps: return "Level Ups";
        case CurrencySource::Combat: return "Combat Wins";
        case CurrencySource::Stations: return "Stations";
        case CurrencySource::Prestige: return "Prestige";
        case CurrencySource::Milestones: return "Milestones";
        default: return "Other";
    }
}

void GatchaSystem::AddStellarShards(i32 amount, CurrencySource source) {
    if (amount <= 0) return;
    m_StellarShards += amount;
    m_ShardTelemetry[GetSourceIndex(source)] += amount;
}

void GatchaSystem::AddSummonTickets(i32 amount, CurrencySource source) {
    if (amount <= 0) return;
    m_SummonTickets += amount;
    m_TicketTelemetry[GetSourceIndex(source)] += amount;
}

void GatchaSystem::IncrementPity(SummonBanner banner) {
    PityTracker& pity = GetPityTrackerMutable(banner);
    pity.pullsSinceRare++;
    pity.pullsSinceEpic++;
    pity.pullsSinceLegendary++;
}

bool GatchaSystem::CheckAndConsumePity(SummonBanner banner, PartRarity& outRarity) {
    const PityTracker& pity = GetPityTracker(banner);

    // Legendary Pity (Only on Advanced)
    if (banner == SummonBanner::Advanced && pity.pullsSinceLegendary >= 100) {
        GetPityTrackerMutable(banner).pullsSinceLegendary = 0;
        GetPityTrackerMutable(banner).pullsSinceEpic = 0; // Epic pity resets on Legendary
        GetPityTrackerMutable(banner).pullsSinceRare = 0; // Rare pity resets on Legendary
        outRarity = PartRarity::Legendary;
        Log::Info("Legendary Pity Hit!");
        return true;
    }

    // Epic Pity (Only on Advanced)
    if (banner == SummonBanner::Advanced && pity.pullsSinceEpic >= 30) {
        GetPityTrackerMutable(banner).pullsSinceEpic = 0;
        GetPityTrackerMutable(banner).pullsSinceRare = 0; // Rare pity resets on Epic
        outRarity = PartRarity::Epic;
        Log::Info("Epic Pity Hit!");
        return true;
    }

    // Rare Pity (All banners - Guaranteed at 10)
    if (pity.pullsSinceRare >= 10) {
        GetPityTrackerMutable(banner).pullsSinceRare = 0;
        outRarity = PartRarity::Rare;
        Log::Info("Rare Pity Hit!");
        return true;
    }

    return false;
}

ShipPart GatchaSystem::GeneratePartForBanner(SummonBanner banner, bool forcedRarity, PartRarity rarity) {
    // In a real system, rates are complex. Here we use a simple wrapper.
    PartRarity finalRarity = forcedRarity ? rarity : RollRarity(banner);

    // ShipPartGenerator::GeneratePart handles the actual part creation
    return ShipPartGenerator::GeneratePart(finalRarity);
}

// --- SUMMON MECHANICS (Fixes Linker Errors) ---

SummonResult GatchaSystem::PerformSingleSummon(SummonBanner banner) {
    // Cost is deducted by the button handler in the UI, but should be done here in a robust system.

    // 1. Increment Pity and Total Summons
    IncrementPity(banner);
    m_TotalSummons++;

    PartRarity finalRarity = PartRarity::Common;
    bool isPity = false;

    // 2. Check Pity first (and force the rarity if pity is hit)
    if (CheckAndConsumePity(banner, finalRarity)) {
        isPity = true;
    } else {
        // 3. If no pity, roll the rarity normally
        finalRarity = RollRarity(banner);
    }

    // 4. Generate the part based on the final rarity
    ShipPart newPart = GeneratePartForBanner(banner, true, finalRarity);

    // 5. Update legendary stats if applicable
    if (newPart.rarity == PartRarity::Legendary) {
        m_LegendaryPulls++;
    }

    Log::Infof("Single Summon Result: ", newPart.GetRarityName(), " ", newPart.name, isPity ? " (PITY)" : "");

    // 6. Return the result
    return SummonResult(newPart, isPity, false);
}

std::vector<SummonResult> GatchaSystem::PerformTenSummon(SummonBanner banner) {
    std::vector<SummonResult> results;

    // Cost is deducted by the button handler in the UI, but should be done here in a robust system.

    for (i32 i = 0; i < 10; ++i) {
        // Perform a single summon for each of the 10 pulls
        SummonResult result = PerformSingleSummon(banner);

        // Add an animation delay between reveals (0.1s steps)
        result.revealDelay = 0.1 * i;

        // Add the 10th-pull guarantee logic here if needed (e.g., guarantee Rare+ for the 10th slot)

        // This is a 10x summon, so we need to generate a bonus 11th result or
        // implement a stronger guarantee on the 10th pull. For now, we stick to 10 pulls.

        results.push_back(result);
    }

    Log::Infof("Performed 10x Summon for ", results.size(), " results.");

    return results;
}

// --- ANIMATION / UI STATE MANAGEMENT ---

// void GatchaSystem::ClearResults() {
//     // NOTE: The inventory addition logic is now placed in HandleClick
//     m_CurrentResults.clear();
//     m_IsAnimating = false;
//     m_AnimationTimer = 0.0;
//     m_CurrentRevealIndex = 0;
// }

void GatchaSystem::Update(f64 deltaTime) {
    if (!m_IsAnimating) return;

    m_AnimationTimer += deltaTime;

    // Check if it's time to reveal the next part
    if (m_CurrentRevealIndex < static_cast<i32>(m_CurrentResults.size())) {
        if (m_AnimationTimer >= m_CurrentResults[m_CurrentRevealIndex].revealDelay) {

            // Play sound effect / particle burst for reveal (TODO: Implement SFX)
            Log::Infof("Revealed part #%d: %s", m_CurrentRevealIndex + 1, m_CurrentResults[m_CurrentRevealIndex].part.GetRarityName());

            m_CurrentRevealIndex++;
        }
    }

    // After all parts are revealed, the animation continues to show results until dismissed by click/button.
}

void GatchaSystem::ClaimResults(GameState* state) {
    if (state) {
        for (const auto& result : m_CurrentResults) {
            state->GetSpaceship().AddPart(result.part);
        }
    }
    ClearResults();
}

void GatchaSystem::HideSummonDocument() {
#ifdef RMLUI_ENABLED
    if (m_SummonDocument) {
        m_SummonDocument->Hide();
    }
#endif
}

#ifdef RMLUI_ENABLED
static std::string GetRarityClass(const ShipPart& part) {
    switch (part.rarity) {
        case PartRarity::Legendary: return "legendary";
        case PartRarity::Epic: return "epic";
        case PartRarity::Rare: return "rare";
        case PartRarity::Uncommon: return "uncommon";
        default: return "common";
    }
}

void GatchaSystem::ExecuteSummon(i32 count, GameState* state) {
    if (count != 1 && count != 10) return;

    i32 cost = (count == 1) ? GetSingleSummonCost(m_SelectedBanner) : GetTenSummonCost(m_SelectedBanner);

    switch (m_SelectedBanner) {
        case SummonBanner::Basic: {
            if (!state || state->GetPlayerCredits() < cost) return;
            state->DeductPlayerCredits(cost);
            break;
        }
        case SummonBanner::Advanced: {
            if (m_StellarShards < cost) return;
            m_StellarShards -= cost;
            break;
        }
        case SummonBanner::Elite: {
            if (m_SummonTickets < cost) return;
            m_SummonTickets -= cost;
            break;
        }
    }

    if (count == 1) {
        m_CurrentResults.clear();
        m_CurrentResults.push_back(PerformSingleSummon(m_SelectedBanner));
    } else {
        m_CurrentResults = PerformTenSummon(m_SelectedBanner);
    }

    m_IsAnimating = true;
    m_AnimationTimer = 0.0;
    m_CurrentRevealIndex = 0;
}

void GatchaSystem::InitializeSummonDocument(GameState* state) {
    Rml::Context* context = Rml::GetContext("main");
    if (!context) return;

    if (!m_SummonDocument) {
        m_SummonDocument = context->LoadDocument("assets/ui/rml/summon.rml");
        if (m_SummonDocument) {
            m_SummonDocument->Show();

            Rml::ElementList actionButtons;
            m_SummonDocument->GetElementsByClassName(actionButtons, "summon-action");

            if (!m_SummonListener) {
                m_SummonListener = CreateScope<RmlSummonListener>(this, state);
            }

            for (Rml::Element* button : actionButtons) {
                button->AddEventListener(Rml::EventId::Click, m_SummonListener.get());
            }
        }
    }

    if (m_SummonDocument) {
        m_SummonDocument->Show();
    }
}

void GatchaSystem::UpdateSummonDocument(GameState* state) {
    if (!m_SummonDocument) return;

    auto setText = [this](const std::string& id, const std::string& text) {
        if (Rml::Element* element = m_SummonDocument->GetElementById(id)) {
            element->SetInnerRML(text);
        }
    };

    // Banner highlighting
    if (Rml::Element* basic = m_SummonDocument->GetElementById("banner-basic")) {
        basic->SetClass("active", m_SelectedBanner == SummonBanner::Basic);
    }
    if (Rml::Element* adv = m_SummonDocument->GetElementById("banner-advanced")) {
        adv->SetClass("active", m_SelectedBanner == SummonBanner::Advanced);
    }
    if (Rml::Element* elite = m_SummonDocument->GetElementById("banner-elite")) {
        elite->SetClass("active", m_SelectedBanner == SummonBanner::Elite);
    }

    setText("currency-basic", std::to_string(state ? state->GetPlayerCredits() : 0));
    setText("currency-advanced", std::to_string(m_StellarShards));
    setText("currency-elite", std::to_string(m_SummonTickets));

    const PityTracker& pity = GetPityTracker(m_SelectedBanner);
    setText("pity-rare", std::to_string(pity.pullsSinceRare) + " / 10");
    if (m_SelectedBanner == SummonBanner::Advanced) {
        setText("pity-epic", std::to_string(pity.pullsSinceEpic) + " / 30");
        setText("pity-legendary", std::to_string(pity.pullsSinceLegendary) + " / 100");
    } else {
        setText("pity-epic", "-");
        setText("pity-legendary", "-");
    }

    setText("cost-single", std::to_string(GetSingleSummonCost(m_SelectedBanner)));
    setText("cost-ten", std::to_string(GetTenSummonCost(m_SelectedBanner)));

    if (Rml::Element* results = m_SummonDocument->GetElementById("summon-results")) {
        std::string html;
        if (m_CurrentResults.empty()) {
            html = "<div class='summon-empty'>No recent summons</div>";
        } else {
            for (const auto& result : m_CurrentResults) {
                html += "<div class='result-card " + GetRarityClass(result.part) + "'>";
                html += "<div class='result-name'>" + result.part.name + "</div>";
                html += "<div class='result-rarity'>" + std::string(result.part.GetRarityName()) + "</div>";
                if (result.isPityDrop) {
                    html += "<div class='result-badge'>Pity</div>";
                }
                html += "</div>";
            }
        }
        results->SetInnerRML(html);
    }
}
#endif

// --- IMGUI RENDERING IMPLEMENTATIONS ---

void GatchaSystem::RenderBannerSelection(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    (void)renderer; (void)panelX; (void)panelY; (void)panelWidth;

    ImGui::TextColored(ToImVec4(Color::NeonCyan()), "SELECT BANNER:");
    ImGui::Spacing();

    // Use RadioButton to select m_SelectedBanner
    if (ImGui::RadioButton("Basic (Credits)", m_SelectedBanner == SummonBanner::Basic)) {
        m_SelectedBanner = SummonBanner::Basic;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Advanced (Shards)", m_SelectedBanner == SummonBanner::Advanced)) {
        m_SelectedBanner = SummonBanner::Advanced;
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Elite (Tickets)", m_SelectedBanner == SummonBanner::Elite)) {
        m_SelectedBanner = SummonBanner::Elite;
    }
}

void GatchaSystem::RenderSummonButtons(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth) {
    (void)renderer; (void)panelX; (void)panelY; (void)panelWidth;

    i32 cost1 = GetSingleSummonCost(m_SelectedBanner);
    i32 cost10 = GetTenSummonCost(m_SelectedBanner);

    const char* currencyName = "";
    i32 currentCurrency = 0;

    // Determine currency name and current amount based on the selected banner
    switch (m_SelectedBanner) {
        case SummonBanner::Basic:
            currencyName = "Credits";
            // Check if GameState getter exists and use it
            currentCurrency = state ? state->GetPlayerCredits() : 0;
            break;
        case SummonBanner::Advanced:
            currencyName = "Stellar Shards";
            currentCurrency = GetStellarShards();
            break;
        case SummonBanner::Elite:
            currencyName = "Tickets";
            currentCurrency = GetSummonTickets();
            break;
    }

    ImGui::Text("Available Currency: %d %s", currentCurrency, currencyName);
    ImGui::Spacing();

    RenderCurrencyBreakdown(panelWidth);
    ImGui::Spacing();

    // --- Single Summon Button ---
    bool canSummon1 = currentCurrency >= cost1;
    if (!canSummon1) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

    std::string text1 = "Summon x1 (" + std::to_string(cost1) + " " + currencyName + ")";
    if (ImGui::Button(text1.c_str(), ImVec2(ImGui::GetContentRegionAvail().x * 0.49f, 50.0f)) && canSummon1) {
        // Deduct cost and initiate animation
        if (state) state->DeductPlayerCredits(cost1); // Assuming deduct function exists

        m_CurrentResults.clear();
        m_CurrentResults.push_back(PerformSingleSummon(m_SelectedBanner));

        m_IsAnimating = true;
        m_AnimationTimer = 0.0;
        m_CurrentRevealIndex = 0;
    }

    if (!canSummon1) ImGui::PopStyleVar();

    // --- Ten Summon Button ---
    ImGui::SameLine();
    bool canSummon10 = currentCurrency >= cost10;
    if (!canSummon10) ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);

    std::string text10 = "Summon x10 (" + std::to_string(cost10) + " " + currencyName + " + Guaranteed Rare+)";
    if (ImGui::Button(text10.c_str(), ImVec2(ImGui::GetContentRegionAvail().x, 50.0f)) && canSummon10) {
        // Deduct cost and initiate animation
        if (state) state->DeductPlayerCredits(cost10); // Assuming deduct function exists

        m_CurrentResults = PerformTenSummon(m_SelectedBanner);

        m_IsAnimating = true;
        m_AnimationTimer = 0.0;
        m_CurrentRevealIndex = 0;
    }

    if (!canSummon10) ImGui::PopStyleVar();
}

void GatchaSystem::RenderCurrencyBreakdown(f32 panelWidth) const {
    ImGui::TextColored(ToImVec4(Color::NeonCyan()), "EARNED THIS RUN:");
    ImGui::Columns(3, "CurrencyBreakdown", false);
    ImGui::SetColumnWidth(0, panelWidth * 0.35f);

    for (size_t i = 0; i < static_cast<size_t>(CurrencySource::COUNT); i++) {
        CurrencySource source = static_cast<CurrencySource>(i);
        ImGui::Text("%s", GetSourceLabel(source)); ImGui::NextColumn();
        ImGui::Text("Shards: %d", m_ShardTelemetry[i]); ImGui::NextColumn();
        ImGui::Text("Tickets: %d", m_TicketTelemetry[i]); ImGui::NextColumn();
    }

    ImGui::Columns(1);
}

void GatchaSystem::RenderPityCounters(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    (void)renderer; (void)panelX; (void)panelY; (void)panelWidth;

    const PityTracker& pity = GetPityTracker(m_SelectedBanner);

    ImGui::TextColored(ToImVec4(Color::NeonCyan()), "PITY STATUS:");
    ImGui::Indent();

    // Rare Pity (Guaranteed at 10)
    ImGui::Text("Pulls since Rare: %d / 10", pity.pullsSinceRare);

    // Advanced specific pity
    if (m_SelectedBanner == SummonBanner::Advanced) {
        ImGui::Text("Pulls since Epic: %d / 30", pity.pullsSinceEpic);
        ImGui::Text("Pulls since Legendary: %d / 100", pity.pullsSinceLegendary);
    } else if (m_SelectedBanner == SummonBanner::Elite) {
        ImGui::TextDisabled("Elite banner has no long-term pity (Guarantees Rare+).");
    }

    ImGui::Unindent();
}

void GatchaSystem::RenderRateInfo(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    (void)renderer; (void)panelX; (void)panelY; (void)panelWidth;

    ImGui::TextColored(ToImVec4(Color::NeonCyan()), "SUMMON RATES:");
    ImGui::Columns(2, "RatesTable", true);
    ImGui::SetColumnWidth(0, 200.0f);

    auto DisplayRate = [this](PartRarity rarity) -> f32 {
        // Placeholder rates to match RollRarity logic (needs refactoring to share rate data)
        float rate = 0.0f;

        switch (m_SelectedBanner) {
            case SummonBanner::Elite:
                if (rarity == PartRarity::Legendary) rate = 1.0f;
                else if (rarity == PartRarity::Epic) rate = 5.0f;
                else if (rarity == PartRarity::Rare) rate = 94.0f;
                else rate = 0.0f;
                break;

            case SummonBanner::Advanced:
                if (rarity == PartRarity::Legendary) rate = 0.5f;
                else if (rarity == PartRarity::Epic) rate = 3.0f;
                else if (rarity == PartRarity::Rare) rate = 12.0f;
                else if (rarity == PartRarity::Uncommon) rate = 30.0f;
                else rate = 54.5f;
                break;

            case SummonBanner::Basic:
            default:
                if (rarity == PartRarity::Legendary) rate = 0.1f;
                else if (rarity == PartRarity::Epic) rate = 1.0f;
                else if (rarity == PartRarity::Rare) rate = 6.0f;
                else if (rarity == PartRarity::Uncommon) rate = 25.0f;
                else rate = 67.9f;
                break;
        }
        return rate;
    };

    ImGui::Text("Legendary (5-Star):"); ImGui::NextColumn();
    ImGui::TextColored(GetRarityColorImVec4(PartRarity::Legendary), "%.2f%%", DisplayRate(PartRarity::Legendary)); ImGui::NextColumn();

    ImGui::Text("Epic (4-Star):"); ImGui::NextColumn();
    ImGui::TextColored(GetRarityColorImVec4(PartRarity::Epic), "%.2f%%", DisplayRate(PartRarity::Epic)); ImGui::NextColumn();

    ImGui::Text("Rare (3-Star):"); ImGui::NextColumn();
    ImGui::TextColored(GetRarityColorImVec4(PartRarity::Rare), "%.2f%%", DisplayRate(PartRarity::Rare)); ImGui::NextColumn();

    ImGui::Text("Uncommon/Common (1-2 Star):"); ImGui::NextColumn();
    ImGui::TextColored(GetRarityColorImVec4(PartRarity::Common), "%.2f%%", DisplayRate(PartRarity::Common) + DisplayRate(PartRarity::Uncommon)); ImGui::NextColumn();

    ImGui::Columns(1);
}

void GatchaSystem::RenderSummonAnimation(Renderer* renderer, GameState* state) {
    (void)renderer;

    ImGui::TextColored(ToImVec4(Color::NeonCyan()), "--- REVEALING RESULTS ---");
    ImGui::Text("Total Summons: %zu", m_CurrentResults.size());

    // Use a fixed column size for the grid display
    ImGui::Columns(5, "SummonGrid", false);

    for (size_t i = 0; i < m_CurrentResults.size(); ++i) {
        const auto& result = m_CurrentResults[i];
        float columnWidth = ImGui::GetContentRegionAvail().x;

        // Use a fixed size button/child area for the reveal box
        ImGui::BeginChild(reinterpret_cast<const char*>(i), ImVec2(columnWidth, 120.0f), true);

        // Only reveal results up to the current animation index
        if (i < static_cast<size_t>(m_CurrentRevealIndex)) {

            ImVec4 rarityColor = GetRarityColorImVec4(result.part.rarity);
            const char* rarityText = result.part.GetRarityName();

            // Display Rarity Name
            ImGui::PushStyleColor(ImGuiCol_Text, rarityColor);
            ImGui::Text("%s", rarityText);
            ImGui::PopStyleColor();

            // Display Part Name
            ImGui::TextWrapped("%s", result.part.name.c_str());

            if (result.isPityDrop) {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "PITY DROP!");
            }

        } else {
            // Unrevealed state (Fading text to suggest tapping/waiting)
            float alpha = 0.2f + 0.8f * (sinf(static_cast<float>(m_AnimationTimer) * 5.0f) * 0.5f + 0.5f);
            ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, alpha), "Tapping...");
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, alpha), "Wait...");
        }

        ImGui::EndChild();
        ImGui::NextColumn();

        // Start a new row after 5 columns
        if ((i + 1) % 5 == 0) {
            ImGui::Columns(1);
            ImGui::Spacing();
            ImGui::Columns(5, "SummonGrid", false);
        }
    }

    ImGui::Columns(1); // Stop columns
    ImGui::Separator();

    // Add a button to skip or dismiss the animation
    if (static_cast<size_t>(m_CurrentRevealIndex) >= m_CurrentResults.size()) {
        if (ImGui::Button("Add to Inventory and Continue", ImVec2(-1, 50.0f))) {
            // Finalize results and clear animation
            if (state) {
                for (const auto& result : m_CurrentResults) {
                    state->GetSpaceship().AddPart(result.part);
                }
            }
            ClearResults(); // Resets animation state
        }
    } else {
        // Option to skip waiting for reveal
        if (ImGui::Button("Quick Reveal All", ImVec2(-1, 50.0f))) {
            m_CurrentRevealIndex = static_cast<i32>(m_CurrentResults.size());
        }
    }
}

// In src/game/GatchaSystem.cpp

// In src/game/GatchaSystem.cpp

void GatchaSystem::RenderSummonUI(Renderer* renderer, GameState* state) {
    (void)renderer;

#ifdef RMLUI_ENABLED
    InitializeSummonDocument(state);
    UpdateSummonDocument(state);
    return;
#endif

    ImGui::SetNextWindowSize(ImVec2(800, 700), ImGuiCond_Once);

    ImVec2 centerPos(
        ImGui::GetIO().DisplaySize.x * 0.5f,
        ImGui::GetIO().DisplaySize.y * 0.5f
    );
    ImGui::SetNextWindowPos(centerPos, ImGuiCond_Once, ImVec2(0.5f, 0.5f));

    bool showGatcha = state->IsGatchaUIVisible();

    // Use ImGuiWindowFlags_NoTitleBar to enable custom close button logic
    if (ImGui::Begin("Summon System (Gatcha)", &showGatcha, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {

        // Update GameState visibility if the user closes the window using the ImGui 'x' (if enabled)
        if (!showGatcha) {
            state->SetGatchaUIVisible(false);
        }

        // Add Manual Close Button (to replicate the fixed X button behavior)
        f32 closeBtnSize = 30.0f;
        f32 panelWidth = ImGui::GetWindowWidth();
        ImGui::SetCursorPos(ImVec2(panelWidth - closeBtnSize - 15.0f, 15.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.3f, 0.1f, 0.1f, 0.8f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.2f, 0.2f, 1.0f));

        if (ImGui::Button("X##CloseGatcha", ImVec2(closeBtnSize, closeBtnSize))) {
            state->SetGatchaUIVisible(false);
        }
        ImGui::PopStyleColor(2);

        // Reset cursor back below the title/close button area
        ImGui::SetCursorPosY(50.0f);
        ImGui::TextColored(ToImVec4(Color::NeonPink()), "SUMMON SYSTEM (GALACTIC PARTS EXCHANGE)");
        ImGui::Separator();

        f32 panelX = 0.0f;
        f32 panelY = 0.0f;
        f32 panelContentWidth = ImGui::GetContentRegionAvail().x; // Usable width

        if (m_IsAnimating) {
            RenderSummonAnimation(renderer, state);
        } else {
            RenderBannerSelection(renderer, panelX, panelY, panelContentWidth);
            ImGui::Spacing();
            ImGui::Separator();

            RenderSummonButtons(renderer, state, panelX, panelY, panelContentWidth);
            ImGui::Spacing();
            ImGui::Separator();

            RenderRateInfo(renderer, panelX, panelY, panelContentWidth);
            ImGui::Spacing();
            ImGui::Separator();

            RenderPityCounters(renderer, panelX, panelY, panelContentWidth);
        }

        ImGui::End();
    } else {
        // If ImGui::Begin returns false (e.g., due to outside click on some configurations),
        // we must still update the state flag.
        state->SetGatchaUIVisible(false);
    }
}

void GatchaSystem::HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state) {
    // This function is largely redundant now that all interaction is via ImGui buttons/widgets.
    // However, we keep a minimalist check for animation dismissal/completion.

    (void)mouseX; (void)mouseY; (void)mousePressed; (void)state;

    // The core logic for adding parts to inventory is now in RenderSummonAnimation's "Continue" button.
    // This avoids accidental double-adds or out-of-sync state changes.
}

void GatchaSystem::SaveToJson(std::ofstream& file) const {
    file << "\"stellarShards\":" << m_StellarShards << ",\n";
    file << "\"summonTickets\":" << m_SummonTickets << ",\n";
    file << "\"totalSummons\":" << m_TotalSummons << ",\n";
    file << "\"legendaryPulls\":" << m_LegendaryPulls << ",\n";

    file << "\"currencySources\":{\n";
    file << "\"shards\":[";
    for (size_t i = 0; i < m_ShardTelemetry.size(); ++i) {
        file << m_ShardTelemetry[i];
        if (i < m_ShardTelemetry.size() - 1) file << ",";
    }
    file << "],\n";

    file << "\"tickets\":[";
    for (size_t i = 0; i < m_TicketTelemetry.size(); ++i) {
        file << m_TicketTelemetry[i];
        if (i < m_TicketTelemetry.size() - 1) file << ",";
    }
    file << "]\n";
    file << "},\n";

    file << "\"basicPity\":{";
    file << "\"rare\":" << m_BasicPity.pullsSinceRare << ",";
    file << "\"epic\":" << m_BasicPity.pullsSinceEpic << ",";
    file << "\"legendary\":" << m_BasicPity.pullsSinceLegendary;
    file << "},\n";

    file << "\"advancedPity\":{";
    file << "\"rare\":" << m_AdvancedPity.pullsSinceRare << ",";
    file << "\"epic\":" << m_AdvancedPity.pullsSinceEpic << ",";
    file << "\"legendary\":" << m_AdvancedPity.pullsSinceLegendary;
    file << "},\n";

    file << "\"elitePity\":{";
    file << "\"rare\":" << m_ElitePity.pullsSinceRare << ",";
    file << "\"epic\":" << m_ElitePity.pullsSinceEpic << ",";
    file << "\"legendary\":" << m_ElitePity.pullsSinceLegendary;
    file << "}\n";
}

void GatchaSystem::LoadFromJson(const std::string& line) {
    if (line.find("\"stellarShards\"") != std::string::npos) {
        m_StellarShards = static_cast<i32>(GameUtils::ParseJsonNumber(line, "stellarShards"));
    } else if (line.find("\"summonTickets\"") != std::string::npos) {
        m_SummonTickets = static_cast<i32>(GameUtils::ParseJsonNumber(line, "summonTickets"));
    } else if (line.find("\"totalSummons\"") != std::string::npos) {
        m_TotalSummons = static_cast<i32>(GameUtils::ParseJsonNumber(line, "totalSummons"));
    } else if (line.find("\"legendaryPulls\"") != std::string::npos) {
        m_LegendaryPulls = static_cast<i32>(GameUtils::ParseJsonNumber(line, "legendaryPulls"));
    } else if (line.find("\"basicPity\"") != std::string::npos ||
               line.find("\"advancedPity\"") != std::string::npos ||
               line.find("\"elitePity\"") != std::string::npos) {
        // no-op: handled by subsequent lines
    } else if (line.find("\"rare\"") != std::string::npos && line.find("basicPity") != std::string::npos) {
        m_BasicPity.pullsSinceRare = static_cast<i32>(GameUtils::ParseJsonNumber(line, "rare"));
        m_BasicPity.pullsSinceEpic = static_cast<i32>(GameUtils::ParseJsonNumber(line, "epic"));
        m_BasicPity.pullsSinceLegendary = static_cast<i32>(GameUtils::ParseJsonNumber(line, "legendary"));
    } else if (line.find("\"rare\"") != std::string::npos && line.find("advancedPity") != std::string::npos) {
        m_AdvancedPity.pullsSinceRare = static_cast<i32>(GameUtils::ParseJsonNumber(line, "rare"));
        m_AdvancedPity.pullsSinceEpic = static_cast<i32>(GameUtils::ParseJsonNumber(line, "epic"));
        m_AdvancedPity.pullsSinceLegendary = static_cast<i32>(GameUtils::ParseJsonNumber(line, "legendary"));
    } else if (line.find("\"rare\"") != std::string::npos && line.find("elitePity") != std::string::npos) {
        m_ElitePity.pullsSinceRare = static_cast<i32>(GameUtils::ParseJsonNumber(line, "rare"));
        m_ElitePity.pullsSinceEpic = static_cast<i32>(GameUtils::ParseJsonNumber(line, "epic"));
        m_ElitePity.pullsSinceLegendary = static_cast<i32>(GameUtils::ParseJsonNumber(line, "legendary"));
    } else if (line.find("\"currencySources\"") != std::string::npos) {
        // parent header, nothing to do
    } else if (line.find("\"shards\"") != std::string::npos) {
        size_t start = line.find('[');
        size_t end = line.find(']');
        if (start != std::string::npos && end != std::string::npos) {
            std::string values = line.substr(start + 1, end - start - 1);
            std::istringstream iss(values);
            std::string val;
            size_t idx = 0;
            while (std::getline(iss, val, ',') && idx < m_ShardTelemetry.size()) {
                m_ShardTelemetry[idx++] = std::stoi(val);
            }
        }
    } else if (line.find("\"tickets\"") != std::string::npos) {
        size_t start = line.find('[');
        size_t end = line.find(']');
        if (start != std::string::npos && end != std::string::npos) {
            std::string values = line.substr(start + 1, end - start - 1);
            std::istringstream iss(values);
            std::string val;
            size_t idx = 0;
            while (std::getline(iss, val, ',') && idx < m_TicketTelemetry.size()) {
                m_TicketTelemetry[idx++] = std::stoi(val);
            }
        }
    }
}