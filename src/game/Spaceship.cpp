#include "Spaceship.h"
#include "Renderer.h"
#include "Logger.h"
#include <cstdlib>
#include <cmath>

// ShipPart Implementation
ShipPart::ShipPart()
    : name("Empty Slot"), description("No part installed"),
      slot(PartSlot::Hull), rarity(PartRarity::Common),
      powerBonus(0), combatBonus(0), dropRateBonus(0),
      repairContribution(0), installed(false), tier(1) {
}

ShipPart::ShipPart(PartSlot slotType, PartRarity rarityTier)
    : slot(slotType), rarity(rarityTier), installed(false) {

    // Set tier based on rarity
    tier = static_cast<i32>(rarity) + 1; // 1-5

    // Generate random bonuses based on rarity
    f64 rarityMultiplier = 1.0;
    switch (rarity) {
        case PartRarity::Common:
            rarityMultiplier = 1.0 + (rand() % 50) / 100.0; // 1.0-1.5x
            repairContribution = 1.0 + (rand() % 2); // 1-2
            break;
        case PartRarity::Uncommon:
            rarityMultiplier = 1.5 + (rand() % 100) / 100.0; // 1.5-2.5x
            repairContribution = 2.0 + (rand() % 3); // 2-4
            break;
        case PartRarity::Rare:
            rarityMultiplier = 2.5 + (rand() % 150) / 100.0; // 2.5-4.0x
            repairContribution = 4.0 + (rand() % 3); // 4-6
            break;
        case PartRarity::Epic:
            rarityMultiplier = 4.0 + (rand() % 300) / 100.0; // 4.0-7.0x
            repairContribution = 6.0 + (rand() % 3); // 6-8
            break;
        case PartRarity::Legendary:
            rarityMultiplier = 7.0 + (rand() % 800) / 100.0; // 7.0-15.0x
            repairContribution = 8.0 + (rand() % 3); // 8-10
            break;
    }

    // Base bonuses (different per slot type)
    switch (slot) {
        case PartSlot::Hull:
            powerBonus = 10.0 * rarityMultiplier;
            combatBonus = 5.0 * rarityMultiplier;
            dropRateBonus = 0.0;
            name = "Reinforced Hull";
            description = "Increases production and durability";
            break;
        case PartSlot::Engine:
            powerBonus = 15.0 * rarityMultiplier;
            combatBonus = 0.0;
            dropRateBonus = 5.0 * rarityMultiplier;
            name = "Quantum Engine";
            description = "Boosts production and exploration speed";
            break;
        case PartSlot::Weapons:
            powerBonus = 5.0 * rarityMultiplier;
            combatBonus = 20.0 * rarityMultiplier;
            dropRateBonus = 0.0;
            name = "Plasma Cannon";
            description = "Devastating combat power";
            break;
        case PartSlot::Shields:
            powerBonus = 8.0 * rarityMultiplier;
            combatBonus = 8.0 * rarityMultiplier;
            dropRateBonus = 3.0 * rarityMultiplier;
            name = "Energy Shield";
            description = "Balanced protection and efficiency";
            break;
        default:
            break;
    }

    // Add rarity prefix to name
    const char* prefix = "";
    switch (rarity) {
        case PartRarity::Uncommon: prefix = "Advanced "; break;
        case PartRarity::Rare: prefix = "Superior "; break;
        case PartRarity::Epic: prefix = "Masterwork "; break;
        case PartRarity::Legendary: prefix = "Legendary "; break;
        default: break;
    }
    name = std::string(prefix) + name;
}

Color ShipPart::GetRarityColor() const {
    switch (rarity) {
        case PartRarity::Common: return Color(0.7f, 0.7f, 0.7f, 1.0f); // Gray
        case PartRarity::Uncommon: return Color(0.2f, 1.0f, 0.2f, 1.0f); // Green
        case PartRarity::Rare: return Color(0.3f, 0.5f, 1.0f, 1.0f); // Blue
        case PartRarity::Epic: return Color(0.8f, 0.2f, 1.0f, 1.0f); // Purple
        case PartRarity::Legendary: return Color(1.0f, 0.7f, 0.0f, 1.0f); // Gold
    }
    return Color::White();
}

const char* ShipPart::GetRarityName() const {
    switch (rarity) {
        case PartRarity::Common: return "Common";
        case PartRarity::Uncommon: return "Uncommon";
        case PartRarity::Rare: return "Rare";
        case PartRarity::Epic: return "Epic";
        case PartRarity::Legendary: return "LEGENDARY";
    }
    return "Unknown";
}

const char* ShipPart::GetSlotName() const {
    switch (slot) {
        case PartSlot::Hull: return "Hull";
        case PartSlot::Engine: return "Engine";
        case PartSlot::Weapons: return "Weapons";
        case PartSlot::Shields: return "Shields";
        default: return "Unknown";
    }
}

// Spaceship Implementation
Spaceship::Spaceship()
    : m_RepairProgress(0.0), m_TotalPowerBonus(0.0),
      m_TotalCombatBonus(0.0), m_TotalDropRateBonus(0.0),
      m_TotalPartsCollected(0), m_LegendaryPartsCollected(0),
      m_SelectedInventoryIndex(-1), m_ScrollOffset(0.0f) {

    // Initialize installed parts to nullptr
    for (i32 i = 0; i < static_cast<i32>(PartSlot::COUNT); i++) {
        m_InstalledParts[i] = nullptr;
    }
}

void Spaceship::Initialize() {
    Log::Info("Initializing spaceship system...");

    // Give player a starter common part for each slot
    for (i32 i = 0; i < static_cast<i32>(PartSlot::COUNT); i++) {
        ShipPart starterPart(static_cast<PartSlot>(i), PartRarity::Common);
        starterPart.name = "Salvaged " + std::string(starterPart.GetSlotName());
        starterPart.description = "Barely functional starter part";
        starterPart.powerBonus *= 0.5; // Half power for starters
        starterPart.repairContribution = 1.0;
        AddPart(starterPart);
    }

    Log::Info("Spaceship initialized with 4 salvaged parts");
}

void Spaceship::AddPart(const ShipPart& part) {
    m_Inventory.push_back(part);
    UpdateStatistics(part);

    Log::Infof("Acquired ", part.GetRarityName(), " ", part.name,
               " (+", part.powerBonus, "% power, +", part.repairContribution, " repair)");
}

bool Spaceship::InstallPart(i32 inventoryIndex) {
    if (inventoryIndex < 0 || inventoryIndex >= static_cast<i32>(m_Inventory.size())) {
        return false;
    }

    ShipPart& part = m_Inventory[inventoryIndex];
    PartSlot slot = part.slot;
    i32 slotIndex = static_cast<i32>(slot);

    // Uninstall existing part in this slot
    if (m_InstalledParts[slotIndex] != nullptr) {
        m_InstalledParts[slotIndex]->installed = false;
        m_InstalledParts[slotIndex] = nullptr;
    }

    // Install new part
    part.installed = true;
    m_InstalledParts[slotIndex] = &part;

    RecalculateBonuses();
    RecalculateRepairProgress();

    Log::Infof("Installed ", part.name, " in ", part.GetSlotName(), " slot");
    return true;
}

bool Spaceship::UninstallPart(PartSlot slot) {
    i32 slotIndex = static_cast<i32>(slot);
    if (m_InstalledParts[slotIndex] == nullptr) {
        return false;
    }

    m_InstalledParts[slotIndex]->installed = false;
    m_InstalledParts[slotIndex] = nullptr;

    RecalculateBonuses();
    RecalculateRepairProgress();
    return true;
}

ShipPart* Spaceship::GetInstalledPart(PartSlot slot) {
    return m_InstalledParts[static_cast<i32>(slot)];
}

void Spaceship::RecalculateRepairProgress() {
    f64 totalRepair = 0.0;
    i32 partsInstalled = 0;

    for (i32 i = 0; i < static_cast<i32>(PartSlot::COUNT); i++) {
        if (m_InstalledParts[i] != nullptr) {
            totalRepair += m_InstalledParts[i]->repairContribution;
            partsInstalled++;
        }
    }

    // Repair progress scales with part quality and number of parts
    // Need all 4 slots filled with good parts to reach 100%
    // Formula: (total repair value / max possible repair value) * 100
    // Max possible: 4 slots * 10 (legendary max) = 40
    m_RepairProgress = (totalRepair / 40.0) * 100.0;

    // Minimum 25% if all slots filled (even with worst parts)
    if (partsInstalled == static_cast<i32>(PartSlot::COUNT)) {
        if (m_RepairProgress < 25.0) {
            m_RepairProgress = 25.0;
        }
    }

    if (m_RepairProgress > 100.0) {
        m_RepairProgress = 100.0;
    }

    // Log milestones
    static bool reached25 = false;
    static bool reached50 = false;
    static bool reached100 = false;

    if (m_RepairProgress >= 25.0 && !reached25) {
        Log::Info("Ship is operational! Travel system unlocked.");
        reached25 = true;
    }
    if (m_RepairProgress >= 50.0 && !reached50) {
        Log::Info("Ship repair 50% complete!");
        reached50 = true;
    }
    if (m_RepairProgress >= 100.0 && !reached100) {
        Log::Info("Ship fully repaired! Maximum efficiency achieved.");
        reached100 = true;
    }
}

void Spaceship::RecalculateBonuses() {
    m_TotalPowerBonus = 0.0;
    m_TotalCombatBonus = 0.0;
    m_TotalDropRateBonus = 0.0;

    for (i32 i = 0; i < static_cast<i32>(PartSlot::COUNT); i++) {
        if (m_InstalledParts[i] != nullptr) {
            m_TotalPowerBonus += m_InstalledParts[i]->powerBonus;
            m_TotalCombatBonus += m_InstalledParts[i]->combatBonus;
            m_TotalDropRateBonus += m_InstalledParts[i]->dropRateBonus;
        }
    }
}

void Spaceship::UpdateStatistics(const ShipPart& part) {
    m_TotalPartsCollected++;
    if (part.rarity == PartRarity::Legendary) {
        m_LegendaryPartsCollected++;
    }
}

void Spaceship::RenderShipPanel(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth, f32 panelHeight) {
    // Draw panel background
    Rect panelRect(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelRect, Color(0.1f, 0.1f, 0.15f, 0.9f), true);

    f32 yOffset = panelY + 10.0f;

    // Title
    renderer->DrawText("SPACESHIP", Vec2(panelX + 10.0f, yOffset), Color::NeonCyan(), 20.0f);
    yOffset += 30.0f;

    // Repair progress bar
    renderer->DrawText("Repair Progress:", Vec2(panelX + 10.0f, yOffset), Color::White(), 16.0f);
    yOffset += 25.0f;

    f32 barWidth = panelWidth - 20.0f;
    f32 barHeight = 30.0f;
    Rect progressBarBg(panelX + 10.0f, yOffset, barWidth, barHeight);
    Rect progressBarFill(panelX + 10.0f, yOffset, barWidth * (m_RepairProgress / 100.0f), barHeight);

    renderer->DrawRect(progressBarBg, Color(0.2f, 0.2f, 0.2f, 1.0f), true);

    Color progressColor = m_RepairProgress >= 100.0f ? Color(0.0f, 1.0f, 0.0f, 1.0f) :
                          m_RepairProgress >= 25.0f ? Color(1.0f, 0.7f, 0.0f, 1.0f) :
                          Color(0.8f, 0.3f, 0.3f, 1.0f);
    renderer->DrawRect(progressBarFill, progressColor, true);

    char progressText[32];
    snprintf(progressText, sizeof(progressText), "%.1f%%", m_RepairProgress);
    renderer->DrawText(progressText, Vec2(panelX + panelWidth * 0.5f - 20.0f, yOffset + 7.0f),
                      Color::White(), 16.0f);
    yOffset += 40.0f;

    // Status text
    const char* statusText = m_RepairProgress >= 100.0f ? "STATUS: FULLY OPERATIONAL" :
                            m_RepairProgress >= 25.0f ? "STATUS: TRAVEL READY" :
                            "STATUS: CRITICAL DAMAGE";
    renderer->DrawText(statusText, Vec2(panelX + 10.0f, yOffset), progressColor, 14.0f);
    yOffset += 30.0f;

    // Part slots (4 slots in 2x2 grid)
    f32 slotSize = (panelWidth - 40.0f) * 0.5f;
    f32 slotSpacing = 10.0f;

    const char* slotNames[] = {"HULL", "ENGINE", "WEAPONS", "SHIELDS"};

    for (i32 row = 0; row < 2; row++) {
        for (i32 col = 0; col < 2; col++) {
            i32 slotIndex = row * 2 + col;
            f32 slotX = panelX + 10.0f + col * (slotSize + slotSpacing);
            f32 slotY = yOffset + row * (slotSize + slotSpacing);

            Rect slotRect(slotX, slotY, slotSize, slotSize);

            ShipPart* installedPart = m_InstalledParts[slotIndex];

            if (installedPart != nullptr) {
                // Draw slot with installed part
                Color slotColor = installedPart->GetRarityColor() * 0.3f;
                slotColor.a = 0.8f;
                renderer->DrawRect(slotRect, slotColor, true);

                // Part name
                renderer->DrawText(slotNames[slotIndex], Vec2(slotX + 5.0f, slotY + 5.0f),
                                 Color::White(), 12.0f);
                renderer->DrawText(installedPart->GetRarityName(), Vec2(slotX + 5.0f, slotY + 20.0f),
                                 installedPart->GetRarityColor(), 10.0f);

                // Bonuses
                char bonusText[64];
                snprintf(bonusText, sizeof(bonusText), "+%.0f%% Power", installedPart->powerBonus);
                renderer->DrawText(bonusText, Vec2(slotX + 5.0f, slotY + slotSize - 35.0f),
                                 Color(0.7f, 0.7f, 0.7f, 1.0f), 10.0f);

                if (installedPart->combatBonus > 0) {
                    snprintf(bonusText, sizeof(bonusText), "+%.0f%% Combat", installedPart->combatBonus);
                    renderer->DrawText(bonusText, Vec2(slotX + 5.0f, slotY + slotSize - 22.0f),
                                     Color(0.7f, 0.7f, 0.7f, 1.0f), 10.0f);
                }
            } else {
                // Draw empty slot
                renderer->DrawRect(slotRect, Color(0.2f, 0.2f, 0.2f, 0.6f), true);
                renderer->DrawText(slotNames[slotIndex], Vec2(slotX + 5.0f, slotY + 5.0f),
                                 Color(0.5f, 0.5f, 0.5f, 1.0f), 12.0f);
                renderer->DrawText("EMPTY", Vec2(slotX + 5.0f, slotY + slotSize * 0.5f - 7.0f),
                                 Color(0.4f, 0.4f, 0.4f, 1.0f), 14.0f);
            }
        }
    }
    yOffset += (slotSize + slotSpacing) * 2 + 10.0f;

    // Total bonuses summary
    renderer->DrawText("TOTAL BONUSES:", Vec2(panelX + 10.0f, yOffset), Color::NeonCyan(), 14.0f);
    yOffset += 20.0f;

    char bonusLine[128];
    snprintf(bonusLine, sizeof(bonusLine), "Production: +%.0f%%", m_TotalPowerBonus);
    renderer->DrawText(bonusLine, Vec2(panelX + 10.0f, yOffset), Color(0.2f, 1.0f, 0.2f, 1.0f), 12.0f);
    yOffset += 18.0f;

    snprintf(bonusLine, sizeof(bonusLine), "Combat: +%.0f%%", m_TotalCombatBonus);
    renderer->DrawText(bonusLine, Vec2(panelX + 10.0f, yOffset), Color(1.0f, 0.5f, 0.2f, 1.0f), 12.0f);
    yOffset += 18.0f;

    snprintf(bonusLine, sizeof(bonusLine), "Drop Rate: +%.0f%%", m_TotalDropRateBonus);
    renderer->DrawText(bonusLine, Vec2(panelX + 10.0f, yOffset), Color(0.5f, 0.5f, 1.0f, 1.0f), 12.0f);
}

void Spaceship::RenderInventoryPanel(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth, f32 panelHeight) {
    // Draw inventory panel background
    Rect panelRect(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelRect, Color(0.1f, 0.1f, 0.15f, 0.9f), true);

    f32 yOffset = panelY + 10.0f;

    // Title
    char title[64];
    snprintf(title, sizeof(title), "INVENTORY (%d parts)", GetInventoryCount());
    renderer->DrawText(title, Vec2(panelX + 10.0f, yOffset), Color::NeonCyan(), 16.0f);
    yOffset += 25.0f;

    // Render inventory items (scrollable list)
    f32 itemHeight = 60.0f;
    f32 maxHeight = panelHeight - 50.0f;
    i32 visibleItems = static_cast<i32>(maxHeight / itemHeight);

    for (i32 i = 0; i < static_cast<i32>(m_Inventory.size()) && i < visibleItems; i++) {
        const ShipPart& part = m_Inventory[i];

        f32 itemY = yOffset + i * itemHeight;
        Rect itemRect(panelX + 10.0f, itemY, panelWidth - 20.0f, itemHeight - 5.0f);

        Color bgColor = part.installed ?
                       Color(0.2f, 0.3f, 0.2f, 0.6f) :
                       Color(0.15f, 0.15f, 0.2f, 0.6f);
        renderer->DrawRect(itemRect, bgColor, true);

        // Rarity border
        Rect rarityBorder(panelX + 10.0f, itemY, 4.0f, itemHeight - 5.0f);
        renderer->DrawRect(rarityBorder, part.GetRarityColor(), true);

        // Part info
        f32 textX = panelX + 20.0f;
        renderer->DrawText(part.name.c_str(), Vec2(textX, itemY + 5.0f),
                         part.GetRarityColor(), 12.0f);
        renderer->DrawText(part.GetSlotName(), Vec2(textX, itemY + 20.0f),
                         Color(0.7f, 0.7f, 0.7f, 1.0f), 10.0f);

        char stats[128];
        snprintf(stats, sizeof(stats), "Pwr:+%.0f%% Cbt:+%.0f%% Drop:+%.0f%%",
                part.powerBonus, part.combatBonus, part.dropRateBonus);
        renderer->DrawText(stats, Vec2(textX, itemY + 35.0f),
                         Color(0.6f, 0.6f, 0.6f, 1.0f), 9.0f);

        if (part.installed) {
            renderer->DrawText("INSTALLED", Vec2(panelX + panelWidth - 90.0f, itemY + 20.0f),
                             Color(0.2f, 1.0f, 0.2f, 1.0f), 11.0f);
        } else {
            renderer->DrawText("[Click to Install]", Vec2(panelX + panelWidth - 120.0f, itemY + 20.0f),
                             Color(0.5f, 0.8f, 1.0f, 1.0f), 10.0f);
        }
    }

    if (m_Inventory.empty()) {
        renderer->DrawText("No parts in inventory", Vec2(panelX + 10.0f, yOffset + 20.0f),
                         Color(0.5f, 0.5f, 0.5f, 1.0f), 14.0f);
        renderer->DrawText("Observe research stations to find parts!",
                         Vec2(panelX + 10.0f, yOffset + 40.0f),
                         Color(0.6f, 0.6f, 0.6f, 1.0f), 12.0f);
    }
}

void Spaceship::HandleClick(f32 mouseX, f32 mouseY, bool mousePressed) {
    // TODO: Implement click handling for installing parts from inventory
    // Will be implemented when integrating with GameState UI
}

// ShipPartGenerator Implementation
namespace ShipPartGenerator {
    ShipPart GenerateRandomPart() {
        // Weighted random rarity selection
        i32 roll = rand() % 100;
        PartRarity rarity;

        if (roll < 60) {
            rarity = PartRarity::Common;
        } else if (roll < 85) {
            rarity = PartRarity::Uncommon;
        } else if (roll < 95) {
            rarity = PartRarity::Rare;
        } else if (roll < 99) {
            rarity = PartRarity::Epic;
        } else {
            rarity = PartRarity::Legendary;
        }

        return GeneratePart(rarity);
    }

    ShipPart GeneratePart(PartRarity rarity) {
        // Random slot type
        PartSlot slot = static_cast<PartSlot>(rand() % static_cast<i32>(PartSlot::COUNT));
        return ShipPart(slot, rarity);
    }

    f64 GetRarityDropChance(PartRarity rarity) {
        switch (rarity) {
            case PartRarity::Common: return 0.60;
            case PartRarity::Uncommon: return 0.25;
            case PartRarity::Rare: return 0.10;
            case PartRarity::Epic: return 0.04;
            case PartRarity::Legendary: return 0.01;
        }
        return 0.0;
    }
}
