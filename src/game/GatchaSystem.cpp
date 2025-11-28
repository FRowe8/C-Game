#include "GatchaSystem.h"
#include "Renderer.h"
#include "Logger.h"
#include "GameState.h"
#include <cstdlib>
#include <cmath>
#include <fstream>

GatchaSystem::GatchaSystem()
    : m_StellarShards(0), m_SummonTickets(0),
      m_TotalSummons(0), m_LegendaryPulls(0),
      m_IsAnimating(false), m_AnimationTimer(0.0),
      m_CurrentRevealIndex(0),
      m_SelectedBanner(SummonBanner::Basic),
      m_ScrollOffset(0.0f) {
}

i32 GatchaSystem::GetSingleSummonCost(SummonBanner banner) const {
    switch (banner) {
        case SummonBanner::Basic: return 1000; // Credits (Qubits)
        case SummonBanner::Advanced: return 10; // Stellar Shards
        case SummonBanner::Elite: return 1; // Summon Tickets
    }
    return 0;
}

i32 GatchaSystem::GetTenSummonCost(SummonBanner banner) const {
    // 10x summons give a discount (9x cost instead of 10x)
    return GetSingleSummonCost(banner) * 9;
}

const PityTracker& GatchaSystem::GetPityTracker(SummonBanner banner) const {
    switch (banner) {
        case SummonBanner::Basic: return m_BasicPity;
        case SummonBanner::Advanced: return m_AdvancedPity;
        case SummonBanner::Elite: return m_ElitePity;
    }
    return m_BasicPity;
}

PityTracker& GatchaSystem::GetPityTrackerMutable(SummonBanner banner) {
    switch (banner) {
        case SummonBanner::Basic: return m_BasicPity;
        case SummonBanner::Advanced: return m_AdvancedPity;
        case SummonBanner::Elite: return m_ElitePity;
    }
    return m_BasicPity;
}

void GatchaSystem::ResetPity(SummonBanner banner) {
    PityTracker& pity = GetPityTrackerMutable(banner);
    pity.pullsSinceRare = 0;
    pity.pullsSinceEpic = 0;
    pity.pullsSinceLegendary = 0;
}

PartRarity GatchaSystem::RollRarity(SummonBanner banner) {
    i32 roll = rand() % 1000; // 0-999 for finer control

    switch (banner) {
        case SummonBanner::Basic:
            // Basic: 60% Common, 25% Uncommon, 10% Rare, 4% Epic, 1% Legendary
            if (roll < 600) return PartRarity::Common;
            if (roll < 850) return PartRarity::Uncommon;
            if (roll < 950) return PartRarity::Rare;
            if (roll < 990) return PartRarity::Epic;
            return PartRarity::Legendary;

        case SummonBanner::Advanced:
            // Advanced: 40% Common, 30% Uncommon, 18% Rare, 10% Epic, 2% Legendary
            if (roll < 400) return PartRarity::Common;
            if (roll < 700) return PartRarity::Uncommon;
            if (roll < 880) return PartRarity::Rare;
            if (roll < 980) return PartRarity::Epic;
            return PartRarity::Legendary;

        case SummonBanner::Elite:
            // Elite: Guaranteed Rare+, 50% Rare, 30% Epic, 20% Legendary
            if (roll < 500) return PartRarity::Rare;
            if (roll < 800) return PartRarity::Epic;
            return PartRarity::Legendary;
    }

    return PartRarity::Common;
}

bool GatchaSystem::CheckAndConsumePity(SummonBanner banner, PartRarity& outRarity) {
    PityTracker& pity = GetPityTrackerMutable(banner);

    // Check pity in order: Legendary > Epic > Rare
    if (banner == SummonBanner::Advanced && pity.pullsSinceLegendary >= 100) {
        outRarity = PartRarity::Legendary;
        pity.pullsSinceLegendary = 0;
        pity.pullsSinceEpic = 0;
        pity.pullsSinceRare = 0;
        Log::Info("PITY ACTIVATED: Guaranteed Legendary!");
        return true;
    }

    if (banner == SummonBanner::Advanced && pity.pullsSinceEpic >= 30) {
        outRarity = PartRarity::Epic;
        pity.pullsSinceEpic = 0;
        pity.pullsSinceRare = 0;
        Log::Info("PITY ACTIVATED: Guaranteed Epic!");
        return true;
    }

    if ((banner == SummonBanner::Basic || banner == SummonBanner::Advanced) &&
        pity.pullsSinceRare >= 10) {
        outRarity = PartRarity::Rare;
        pity.pullsSinceRare = 0;
        Log::Info("PITY ACTIVATED: Guaranteed Rare!");
        return true;
    }

    return false;
}

void GatchaSystem::IncrementPity(SummonBanner banner) {
    PityTracker& pity = GetPityTrackerMutable(banner);
    pity.pullsSinceRare++;
    pity.pullsSinceEpic++;
    pity.pullsSinceLegendary++;
}

ShipPart GatchaSystem::GeneratePartForBanner(SummonBanner banner, bool forcedRarity, PartRarity rarity) {
    PartRarity finalRarity;

    if (forcedRarity) {
        finalRarity = rarity;
    } else {
        // Check pity first
        if (!CheckAndConsumePity(banner, finalRarity)) {
            // Normal roll
            finalRarity = RollRarity(banner);
        }
    }

    // Generate the part
    ShipPart part = ShipPartGenerator::GeneratePart(finalRarity);

    // Update pity counters if we got a high rarity naturally
    if (!forcedRarity) {
        if (finalRarity >= PartRarity::Legendary) {
            GetPityTrackerMutable(banner).pullsSinceLegendary = 0;
            GetPityTrackerMutable(banner).pullsSinceEpic = 0;
            GetPityTrackerMutable(banner).pullsSinceRare = 0;
        } else if (finalRarity >= PartRarity::Epic) {
            GetPityTrackerMutable(banner).pullsSinceEpic = 0;
            GetPityTrackerMutable(banner).pullsSinceRare = 0;
        } else if (finalRarity >= PartRarity::Rare) {
            GetPityTrackerMutable(banner).pullsSinceRare = 0;
        } else {
            // Increment pity for lower rarities
            IncrementPity(banner);
        }
    }

    return part;
}

SummonResult GatchaSystem::PerformSingleSummon(SummonBanner banner) {
    ShipPart part = GeneratePartForBanner(banner, false, PartRarity::Common);

    m_TotalSummons++;
    if (part.rarity == PartRarity::Legendary) {
        m_LegendaryPulls++;
    }

    bool isPity = false; // TODO: Track if this was from pity
    return SummonResult(part, isPity, false);
}

std::vector<SummonResult> GatchaSystem::PerformTenSummon(SummonBanner banner) {
    std::vector<SummonResult> results;

    // 10x summon gives 11th pull as bonus (10 + 1 free)
    for (i32 i = 0; i < 11; i++) {
        bool isBonus = (i == 10);
        ShipPart part = GeneratePartForBanner(banner, false, PartRarity::Common);

        m_TotalSummons++;
        if (part.rarity == PartRarity::Legendary) {
            m_LegendaryPulls++;
        }

        SummonResult result(part, false, isBonus);
        result.revealDelay = i * 0.3f; // Stagger reveals by 0.3s each
        results.push_back(result);
    }

    return results;
}

void GatchaSystem::Update(f64 deltaTime) {
    if (!m_IsAnimating) return;

    m_AnimationTimer += deltaTime;

    // Reveal parts one by one based on delay
    for (size_t i = m_CurrentRevealIndex; i < m_CurrentResults.size(); i++) {
        if (m_AnimationTimer >= m_CurrentResults[i].revealDelay) {
            m_CurrentRevealIndex = static_cast<i32>(i) + 1;

            // Play sound effect / particle burst for reveal
            Log::Infof("Revealed: ", m_CurrentResults[i].part.GetRarityName(), " ", m_CurrentResults[i].part.name);
        } else {
            break;
        }
    }

    // Animation complete when all revealed
    if (m_CurrentRevealIndex >= static_cast<i32>(m_CurrentResults.size())) {
        // Keep showing results for 3 more seconds
        if (m_AnimationTimer >= m_CurrentResults.back().revealDelay + 3.0) {
            m_IsAnimating = false;
            m_AnimationTimer = 0.0;
            m_CurrentRevealIndex = 0;
        }
    }
}

void GatchaSystem::RenderBannerSelection(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 yOffset = panelY + 60.0f;

    renderer->DrawText("SELECT BANNER:", Vec2(panelX + 20.0f, yOffset), Color::NeonCyan(), 18.0f);
    yOffset += 30.0f;

    f32 bannerWidth = (panelWidth - 60.0f) / 3.0f;
    f32 bannerHeight = 80.0f;
    f32 bannerSpacing = 10.0f;

    const char* bannerNames[] = {"BASIC", "ADVANCED", "ELITE"};
    Color bannerColors[] = {
        Color(0.5f, 0.7f, 1.0f, 1.0f),  // Blue
        Color(0.8f, 0.3f, 1.0f, 1.0f),  // Purple
        Color(1.0f, 0.7f, 0.0f, 1.0f)   // Gold
    };

    for (i32 i = 0; i < 3; i++) {
        f32 bannerX = panelX + 20.0f + i * (bannerWidth + bannerSpacing);
        Rect bannerRect(bannerX, yOffset, bannerWidth, bannerHeight);

        bool selected = (static_cast<i32>(m_SelectedBanner) == i);
        Color bgColor = selected ? (bannerColors[i] * 0.4f) : (bannerColors[i] * 0.2f);
        renderer->DrawRect(bannerRect, bgColor, true);

        Color borderColor = selected ? bannerColors[i] : (bannerColors[i] * 0.6f);
        renderer->DrawRect(bannerRect, borderColor, false);

        if (selected) {
            Rect glowRect(bannerX - 2.0f, yOffset - 2.0f, bannerWidth + 4.0f, bannerHeight + 4.0f);
            renderer->DrawRect(glowRect, bannerColors[i] * 0.8f, false);
        }

        f32 textWidth = strlen(bannerNames[i]) * 7.5f;
        Vec2 textPos(bannerX + (bannerWidth - textWidth) * 0.5f, yOffset + bannerHeight * 0.5f - 8.0f);
        renderer->DrawText(bannerNames[i], textPos, Color::White(), 16.0f);
    }
}

void GatchaSystem::RenderSummonButtons(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 yOffset = panelY + 170.0f;

    // Show costs and currency
    i32 singleCost = GetSingleSummonCost(m_SelectedBanner);
    i32 tenCost = GetTenSummonCost(m_SelectedBanner);

    const char* currencyName = "";
    i32 currentAmount = 0;

    switch (m_SelectedBanner) {
        case SummonBanner::Basic:
            currencyName = "Credits";
            currentAmount = static_cast<i32>(state->GetResource(QuantumResource::Qubits));
            break;
        case SummonBanner::Advanced:
            currencyName = "Stellar Shards";
            currentAmount = m_StellarShards;
            break;
        case SummonBanner::Elite:
            currencyName = "Summon Tickets";
            currentAmount = m_SummonTickets;
            break;
    }

    char currencyText[128];
    snprintf(currencyText, sizeof(currencyText), "You have: %d %s", currentAmount, currencyName);
    renderer->DrawText(currencyText, Vec2(panelX + 20.0f, yOffset), Color(1.0f, 0.9f, 0.3f, 1.0f), 16.0f);
    yOffset += 30.0f;

    // Single summon button
    f32 btnWidth = (panelWidth - 50.0f) * 0.5f;
    f32 btnHeight = 70.0f;
    f32 btnX1 = panelX + 20.0f;

    bool canAffordSingle = currentAmount >= singleCost;
    Color singleColor = canAffordSingle ? Color(0.3f, 0.8f, 0.3f, 1.0f) : Color(0.4f, 0.4f, 0.4f, 1.0f);

    Rect singleBtn(btnX1, yOffset, btnWidth, btnHeight);
    renderer->DrawRect(singleBtn, singleColor * 0.3f, true);
    renderer->DrawRect(singleBtn, singleColor, false);

    renderer->DrawText("SUMMON x1", Vec2(btnX1 + btnWidth * 0.5f - 50.0f, yOffset + 15.0f), Color::White(), 18.0f);
    char costText[64];
    snprintf(costText, sizeof(costText), "Cost: %d", singleCost);
    renderer->DrawText(costText, Vec2(btnX1 + btnWidth * 0.5f - 40.0f, yOffset + 40.0f), Color(0.8f, 0.8f, 0.8f, 1.0f), 14.0f);

    // Ten summon button
    f32 btnX2 = btnX1 + btnWidth + 10.0f;
    bool canAffordTen = currentAmount >= tenCost;
    Color tenColor = canAffordTen ? Color(0.3f, 0.5f, 1.0f, 1.0f) : Color(0.4f, 0.4f, 0.4f, 1.0f);

    Rect tenBtn(btnX2, yOffset, btnWidth, btnHeight);
    renderer->DrawRect(tenBtn, tenColor * 0.3f, true);
    renderer->DrawRect(tenBtn, tenColor, false);

    renderer->DrawText("SUMMON x10", Vec2(btnX2 + btnWidth * 0.5f - 60.0f, yOffset + 10.0f), Color::White(), 18.0f);
    snprintf(costText, sizeof(costText), "Cost: %d (Save %d!)", tenCost, singleCost);
    renderer->DrawText(costText, Vec2(btnX2 + btnWidth * 0.5f - 80.0f, yOffset + 35.0f), Color(1.0f, 0.9f, 0.3f, 1.0f), 12.0f);
    renderer->DrawText("+1 BONUS PULL!", Vec2(btnX2 + btnWidth * 0.5f - 60.0f, yOffset + 52.0f), Color(0.3f, 1.0f, 0.3f, 1.0f), 12.0f);
}

void GatchaSystem::RenderPityCounters(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 yOffset = panelY + 280.0f;

    renderer->DrawText("PITY PROGRESS:", Vec2(panelX + 20.0f, yOffset), Color::NeonCyan(), 16.0f);
    yOffset += 25.0f;

    const PityTracker& pity = GetPityTracker(m_SelectedBanner);

    // Show relevant pity counters based on banner
    if (m_SelectedBanner == SummonBanner::Basic || m_SelectedBanner == SummonBanner::Advanced) {
        char pityText[128];
        snprintf(pityText, sizeof(pityText), "Rare in %d pulls (Guaranteed at 10)", 10 - pity.pullsSinceRare);
        Color rareColor = (10 - pity.pullsSinceRare <= 3) ? Color(0.3f, 1.0f, 1.0f, 1.0f) : Color(0.7f, 0.7f, 0.7f, 1.0f);
        renderer->DrawText(pityText, Vec2(panelX + 25.0f, yOffset), rareColor, 13.0f);
        yOffset += 20.0f;
    }

    if (m_SelectedBanner == SummonBanner::Advanced) {
        char pityText[128];
        snprintf(pityText, sizeof(pityText), "Epic in %d pulls (Guaranteed at 30)", 30 - pity.pullsSinceEpic);
        Color epicColor = (30 - pity.pullsSinceEpic <= 5) ? Color(0.8f, 0.3f, 1.0f, 1.0f) : Color(0.7f, 0.7f, 0.7f, 1.0f);
        renderer->DrawText(pityText, Vec2(panelX + 25.0f, yOffset), epicColor, 13.0f);
        yOffset += 20.0f;

        snprintf(pityText, sizeof(pityText), "Legendary in %d pulls (Guaranteed at 100)", 100 - pity.pullsSinceLegendary);
        Color legendaryColor = (100 - pity.pullsSinceLegendary <= 10) ? Color(1.0f, 0.7f, 0.0f, 1.0f) : Color(0.7f, 0.7f, 0.7f, 1.0f);
        renderer->DrawText(pityText, Vec2(panelX + 25.0f, yOffset), legendaryColor, 13.0f);
        yOffset += 20.0f;
    }

    if (m_SelectedBanner == SummonBanner::Elite) {
        renderer->DrawText("No pity needed - All pulls are Rare or better!", Vec2(panelX + 25.0f, yOffset),
                         Color(1.0f, 0.7f, 0.0f, 1.0f), 13.0f);
        yOffset += 20.0f;
    }
}

void GatchaSystem::RenderRateInfo(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 yOffset = panelY + 380.0f;

    renderer->DrawText("DROP RATES:", Vec2(panelX + 20.0f, yOffset), Color::NeonCyan(), 14.0f);
    yOffset += 22.0f;

    const char* rates[] = {"", "", ""};
    switch (m_SelectedBanner) {
        case SummonBanner::Basic:
            rates[0] = "Common: 60% | Uncommon: 25% | Rare: 10%";
            rates[1] = "Epic: 4% | Legendary: 1%";
            rates[2] = "";
            break;
        case SummonBanner::Advanced:
            rates[0] = "Common: 40% | Uncommon: 30% | Rare: 18%";
            rates[1] = "Epic: 10% | Legendary: 2%";
            rates[2] = "";
            break;
        case SummonBanner::Elite:
            rates[0] = "Guaranteed Rare or better!";
            rates[1] = "Rare: 50% | Epic: 30% | Legendary: 20%";
            rates[2] = "";
            break;
    }

    for (i32 i = 0; i < 3; i++) {
        if (strlen(rates[i]) > 0) {
            renderer->DrawText(rates[i], Vec2(panelX + 25.0f, yOffset), Color(0.8f, 0.8f, 0.8f, 1.0f), 11.0f);
            yOffset += 18.0f;
        }
    }
}

void GatchaSystem::RenderSummonAnimation(Renderer* renderer) {
    if (!m_IsAnimating || m_CurrentResults.empty()) return;

    f32 screenWidth = static_cast<f32>(renderer->GetWidth());
    f32 screenHeight = static_cast<f32>(renderer->GetHeight());

    // Dark overlay
    Rect overlay(0, 0, screenWidth, screenHeight);
    renderer->DrawRect(overlay, Color(0.0f, 0.0f, 0.0f, 0.9f), true);

    // Title
    renderer->DrawText("SUMMON RESULTS", Vec2(screenWidth * 0.5f - 120.0f, 50.0f), Color::NeonCyan(), 28.0f);

    // Display revealed parts in a grid
    f32 cardWidth = 180.0f;
    f32 cardHeight = 220.0f;
    f32 cardSpacing = 20.0f;
    i32 cardsPerRow = 5;

    f32 startX = (screenWidth - (cardWidth * cardsPerRow + cardSpacing * (cardsPerRow - 1))) * 0.5f;
    f32 startY = 120.0f;

    for (i32 i = 0; i < m_CurrentRevealIndex; i++) {
        const SummonResult& result = m_CurrentResults[i];

        i32 row = i / cardsPerRow;
        i32 col = i % cardsPerRow;

        f32 cardX = startX + col * (cardWidth + cardSpacing);
        f32 cardY = startY + row * (cardHeight + cardSpacing);

        Rect cardRect(cardX, cardY, cardWidth, cardHeight);

        // Card background with rarity color
        Color cardColor = result.part.GetRarityColor() * 0.3f;
        cardColor.a = 0.9f;
        renderer->DrawRect(cardRect, cardColor, true);

        // Rarity border
        renderer->DrawRect(cardRect, result.part.GetRarityColor(), false);

        // Bonus indicator
        if (result.isBonusDrop) {
            renderer->DrawText("BONUS!", Vec2(cardX + 5.0f, cardY + 5.0f), Color(0.3f, 1.0f, 0.3f, 1.0f), 12.0f);
        }

        // Pity indicator
        if (result.isPityDrop) {
            renderer->DrawText("PITY", Vec2(cardX + 5.0f, cardY + 5.0f), Color(1.0f, 0.7f, 0.0f, 1.0f), 12.0f);
        }

        // Part info
        renderer->DrawText(result.part.GetRarityName(), Vec2(cardX + 10.0f, cardY + 30.0f),
                         result.part.GetRarityColor(), 16.0f);

        // Part name (truncated if too long)
        std::string partName = result.part.name;
        if (partName.length() > 15) {
            partName = partName.substr(0, 12) + "...";
        }
        renderer->DrawText(partName.c_str(), Vec2(cardX + 10.0f, cardY + 50.0f), Color::White(), 13.0f);

        // Slot type
        renderer->DrawText(result.part.GetSlotName(), Vec2(cardX + 10.0f, cardY + 70.0f),
                         Color(0.7f, 0.7f, 0.7f, 1.0f), 12.0f);

        // Stats preview
        char stats[64];
        snprintf(stats, sizeof(stats), "+%.0f%% Power", result.part.powerBonus);
        renderer->DrawText(stats, Vec2(cardX + 10.0f, cardY + 90.0f), Color(0.6f, 1.0f, 0.6f, 1.0f), 11.0f);

        if (result.part.combatBonus > 0) {
            snprintf(stats, sizeof(stats), "+%.0f%% Combat", result.part.combatBonus);
            renderer->DrawText(stats, Vec2(cardX + 10.0f, cardY + 105.0f), Color(1.0f, 0.6f, 0.4f, 1.0f), 11.0f);
        }
    }

    // "Click to continue" text after all revealed
    if (m_CurrentRevealIndex >= static_cast<i32>(m_CurrentResults.size())) {
        f32 pulse = 0.5f + 0.5f * static_cast<f32>(sin(m_AnimationTimer * 3.0));
        Color pulseColor = Color::NeonCyan() * pulse;
        renderer->DrawText("Click anywhere to continue...", Vec2(screenWidth * 0.5f - 140.0f, screenHeight - 50.0f),
                         pulseColor, 16.0f);
    }
}

void GatchaSystem::RenderSummonUI(Renderer* renderer, GameState* state) {
    // Background overlay
    Rect bg(0, 0, static_cast<f32>(renderer->GetWidth()), static_cast<f32>(renderer->GetHeight()));
    renderer->DrawRect(bg, Color(0.0f, 0.0f, 0.05f, 0.85f), true);

    // Main panel
    f32 panelWidth = 950.0f;
    f32 panelHeight = 700.0f;
    f32 panelX = (static_cast<f32>(renderer->GetWidth()) - panelWidth) * 0.5f;
    f32 panelY = (static_cast<f32>(renderer->GetHeight()) - panelHeight) * 0.5f;

    Rect panelRect(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelRect, Color(0.05f, 0.05f, 0.1f, 0.95f), true);
    renderer->DrawRect(panelRect, Color::NeonCyan() * 0.7f, false);

    // Title
    renderer->DrawText("SUMMON SYSTEM", Vec2(panelX + panelWidth * 0.5f - 120.0f, panelY + 15.0f), Color::NeonCyan(), 24.0f);

    // Close button (top-right, 44px for mobile-first touch target)
    f32 closeBtnSize = 44.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;
    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);
    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color(1.0f, 0.3f, 0.3f, 1.0f), false);
    renderer->DrawText("X", Vec2(closeBtnX + 14.0f, closeBtnY + 10.0f), Color::White(), 20.0f);

    // Render UI components
    RenderBannerSelection(renderer, panelX, panelY, panelWidth);
    RenderSummonButtons(renderer, state, panelX, panelY, panelWidth);
    RenderPityCounters(renderer, panelX, panelY, panelWidth);
    RenderRateInfo(renderer, panelX, panelY, panelWidth);

    // Statistics
    f32 statsY = panelY + panelHeight - 60.0f;
    char statsText[256];
    snprintf(statsText, sizeof(statsText), "Total Summons: %d | Legendary Pulls: %d (%.1f%%)",
             m_TotalSummons, m_LegendaryPulls,
             m_TotalSummons > 0 ? (m_LegendaryPulls * 100.0 / m_TotalSummons) : 0.0);
    renderer->DrawText(statsText, Vec2(panelX + 20.0f, statsY), Color(1.0f, 0.9f, 0.3f, 1.0f), 13.0f);

    // Close button hint
    renderer->DrawText("Press ESC to close", Vec2(panelX + panelWidth - 150.0f, panelY + panelHeight - 30.0f),
                     Color(0.6f, 0.6f, 0.6f, 1.0f), 12.0f);

    // Render animation overlay if active
    if (m_IsAnimating) {
        RenderSummonAnimation(renderer);
    }
}

void GatchaSystem::HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state) {
    if (!mousePressed) return;

    // If animating, click clears results after all revealed
    if (m_IsAnimating) {
        if (m_CurrentRevealIndex >= static_cast<i32>(m_CurrentResults.size())) {
            // Add all parts to inventory
            for (const auto& result : m_CurrentResults) {
                state->GetSpaceship().AddPart(result.part);
            }
            ClearResults();
        }
        return;
    }

    f32 panelWidth = 950.0f;
    f32 panelHeight = 700.0f;
    f32 panelX = (1920.0f - panelWidth) * 0.5f; // TODO: Use actual screen width
    f32 panelY = (1080.0f - panelHeight) * 0.5f;

    // Banner selection
    f32 yOffset = panelY + 90.0f;
    f32 bannerWidth = (panelWidth - 60.0f) / 3.0f;
    f32 bannerHeight = 80.0f;
    f32 bannerSpacing = 10.0f;

    for (i32 i = 0; i < 3; i++) {
        f32 bannerX = panelX + 20.0f + i * (bannerWidth + bannerSpacing);
        Rect bannerRect(bannerX, yOffset, bannerWidth, bannerHeight);

        if (bannerRect.Contains(Vec2(mouseX, mouseY))) {
            m_SelectedBanner = static_cast<SummonBanner>(i);
            Log::Infof("Selected banner: ", i);
            return;
        }
    }

    // Summon buttons
    yOffset = panelY + 200.0f;
    f32 btnWidth = (panelWidth - 50.0f) * 0.5f;
    f32 btnHeight = 70.0f;

    Rect singleBtn(panelX + 20.0f, yOffset, btnWidth, btnHeight);
    Rect tenBtn(panelX + 30.0f + btnWidth, yOffset, btnWidth, btnHeight);

    i32 singleCost = GetSingleSummonCost(m_SelectedBanner);
    i32 tenCost = GetTenSummonCost(m_SelectedBanner);

    // Check if can afford
    bool canAffordSingle = false;
    bool canAffordTen = false;

    switch (m_SelectedBanner) {
        case SummonBanner::Basic:
            canAffordSingle = state->GetResource(QuantumResource::Qubits) >= singleCost;
            canAffordTen = state->GetResource(QuantumResource::Qubits) >= tenCost;
            break;
        case SummonBanner::Advanced:
            canAffordSingle = m_StellarShards >= singleCost;
            canAffordTen = m_StellarShards >= tenCost;
            break;
        case SummonBanner::Elite:
            canAffordSingle = m_SummonTickets >= singleCost;
            canAffordTen = m_SummonTickets >= tenCost;
            break;
    }

    if (singleBtn.Contains(Vec2(mouseX, mouseY)) && canAffordSingle) {
        // Deduct cost
        switch (m_SelectedBanner) {
            case SummonBanner::Basic:
                state->SpendResource(QuantumResource::Qubits, singleCost);
                break;
            case SummonBanner::Advanced:
                m_StellarShards -= singleCost;
                break;
            case SummonBanner::Elite:
                m_SummonTickets -= singleCost;
                break;
        }

        // Perform summon
        m_CurrentResults.clear();
        m_CurrentResults.push_back(PerformSingleSummon(m_SelectedBanner));
        m_IsAnimating = true;
        m_AnimationTimer = 0.0;
        m_CurrentRevealIndex = 0;

        Log::Info("Performed single summon");
    } else if (tenBtn.Contains(Vec2(mouseX, mouseY)) && canAffordTen) {
        // Deduct cost
        switch (m_SelectedBanner) {
            case SummonBanner::Basic:
                state->SpendResource(QuantumResource::Qubits, tenCost);
                break;
            case SummonBanner::Advanced:
                m_StellarShards -= tenCost;
                break;
            case SummonBanner::Elite:
                m_SummonTickets -= tenCost;
                break;
        }

        // Perform 10x summon
        m_CurrentResults = PerformTenSummon(m_SelectedBanner);
        m_IsAnimating = true;
        m_AnimationTimer = 0.0;
        m_CurrentRevealIndex = 0;

        Log::Info("Performed 10x summon");
    }
}

void GatchaSystem::SaveToJson(std::ofstream& file) const {
    file << "\"stellarShards\":" << m_StellarShards << ",\n";
    file << "\"summonTickets\":" << m_SummonTickets << ",\n";
    file << "\"totalSummons\":" << m_TotalSummons << ",\n";
    file << "\"legendaryPulls\":" << m_LegendaryPulls << ",\n";

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
    // TODO: Implement JSON loading (parsing would go here)
    Log::Info("GatchaSystem::LoadFromJson called");
}
