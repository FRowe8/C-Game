#include "Spaceship.h"
#include "GameState.h"
#include "SpecializedSkills.h"
#include "Renderer.h"
#include "Logger.h"
#include "imgui.h"
#include <cstdlib>
#include <cmath>

// ShipPart Implementation
ShipPart::ShipPart()
    : name("Empty Slot"), description("No part installed"),
      slot(PartSlot::Hull), rarity(PartRarity::Common),
      powerBonus(0), combatBonus(0), dropRateBonus(0),
      repairContribution(0), installed(false), tier(1),
      enhancementLevel(0), stars(1) {
}

ShipPart::ShipPart(PartSlot slotType, PartRarity rarityTier)
    : slot(slotType), rarity(rarityTier), installed(false),
      enhancementLevel(0), stars(1) {

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

// Enhancement helper methods
f64 ShipPart::GetEnhancementMultiplier() const {
    // Each enhancement level adds 10% to base stats
    return 1.0 + (enhancementLevel * 0.10);
}

f64 ShipPart::GetStarMultiplier() const {
    // Each star beyond 1 adds 25% to base stats
    return 1.0 + ((stars - 1) * 0.25);
}

f64 ShipPart::GetTotalMultiplier() const {
    // Total multiplier is enhancement * star multipliers
    return GetEnhancementMultiplier() * GetStarMultiplier();
}

// Spaceship Implementation
Spaceship::Spaceship()
    : m_RepairProgress(0.0), m_TotalPowerBonus(0.0),
      m_TotalCombatBonus(0.0), m_TotalDropRateBonus(0.0),
      m_TotalPartsCollected(0), m_LegendaryPartsCollected(0),
      m_SelectedInventoryIndex(-1), m_ScrollOffset(0.0f),
      m_GameState(nullptr) {

    // Initialize installed parts to nullptr
    for (i32 i = 0; i < static_cast<i32>(PartSlot::COUNT); i++) {
        m_InstalledParts[i] = nullptr;
    }
}

void Spaceship::Initialize(GameState* gameState) {
    m_GameState = gameState;
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

    // Award Command skill XP for part installation
    if (m_GameState) {
        m_GameState->GetSpecializedSkills().AddExperience(SkillCategory::Command, SkillXP::INSTALL_PART);
    }

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
            // Apply enhancement and star multipliers to bonuses
            f64 multiplier = m_InstalledParts[i]->GetTotalMultiplier();
            m_TotalPowerBonus += m_InstalledParts[i]->powerBonus * multiplier;
            m_TotalCombatBonus += m_InstalledParts[i]->combatBonus * multiplier;
            m_TotalDropRateBonus += m_InstalledParts[i]->dropRateBonus * multiplier;
        }
    }
}

void Spaceship::UpdateStatistics(const ShipPart& part) {
    m_TotalPartsCollected++;
    if (part.rarity == PartRarity::Legendary) {
        m_LegendaryPartsCollected++;
    }
}

void Spaceship::RenderShipPanel() {
    // Note: Removed Renderer*, panelX, etc. arguments. ImGui handles them.

    // --- Ship Status ---

    // Title (Use ImGui::Text and large font if desired)
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "SPACESHIP STATUS"); // Neon Cyan

    // --- Repair Progress Bar ---
    ImGui::Separator();
    ImGui::Text("Repair Progress:");

    // Get progress and color
    float progress = static_cast<float>(m_RepairProgress / 100.0f);
    ImVec4 progressColor;
    if (m_RepairProgress >= 100.0) {
        progressColor = ImVec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
    } else if (m_RepairProgress >= 25.0) {
        progressColor = ImVec4(1.0f, 0.7f, 0.0f, 1.0f); // Orange
    } else {
        progressColor = ImVec4(0.8f, 0.3f, 0.3f, 1.0f); // Red
    }

    char overlay[32];
    snprintf(overlay, sizeof(overlay), "%.1f%%", m_RepairProgress);

    // Use ImGui::ProgressBar
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, progressColor);
    ImGui::ProgressBar(progress, ImVec2(-1.0f, 30.0f), overlay);
    ImGui::PopStyleColor();

    // Status text
    const char* statusText = m_RepairProgress >= 100.0f ? "STATUS: FULLY OPERATIONAL" :
                            m_RepairProgress >= 25.0f ? "STATUS: TRAVEL READY" :
                            "STATUS: CRITICAL DAMAGE";
    ImGui::TextColored(progressColor, "%s", statusText);
    ImGui::Spacing();

    // --- Part Slots (2x2 Grid) ---
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "INSTALLED PARTS");

    const char* slotNames[] = {"HULL", "ENGINE", "WEAPONS", "SHIELDS"};

    // Calculate button width for 2 columns
    float totalWidth = ImGui::GetContentRegionAvail().x;
    float slotSize = (totalWidth - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

    for (i32 row = 0; row < 2; row++) {
        for (i32 col = 0; col < 2; col++) {
            i32 slotIndex = row * 2 + col;
            ShipPart* installedPart = m_InstalledParts[slotIndex];

            // Start new column for the second item in the row
            if (col > 0) ImGui::SameLine();

            ImGui::BeginChild(slotNames[slotIndex], ImVec2(slotSize, slotSize), true, ImGuiWindowFlags_None);

            // Draw slot header (HULL, ENGINE, etc.)
            ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s", slotNames[slotIndex]);

            if (installedPart != nullptr) {
                // Apply rarity color to text
                Color c = installedPart->GetRarityColor();
                ImVec4 rarityColor(c.r, c.g, c.b, c.a);

                ImGui::TextColored(rarityColor, "%s", installedPart->name.c_str());
                ImGui::TextColored(rarityColor, "(%s)", installedPart->GetRarityName());

                // Bonuses
                ImGui::Spacing();
                ImGui::Text("+%.0f%% Power", installedPart->powerBonus * installedPart->GetTotalMultiplier());
                if (installedPart->combatBonus > 0) {
                    ImGui::Text("+%.0f%% Combat", installedPart->combatBonus * installedPart->GetTotalMultiplier());
                }

                // Tooltip on hover
                if (ImGui::IsItemHovered()) {
                    ImGui::SetTooltip("%s\nTier %d, Level +%d\nContribution: %.1f",
                        installedPart->description.c_str(), installedPart->tier,
                        installedPart->enhancementLevel, installedPart->repairContribution);
                }

                // === DRAG SOURCE: Drag installed part to uninstall ===
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                    // Set payload (slot index)
                    ImGui::SetDragDropPayload("SHIP_PART_INSTALLED", &slotIndex, sizeof(i32));
                    // Preview
                    ImGui::TextColored(rarityColor, "%s", installedPart->name.c_str());
                    ImGui::Text("(Drag to remove)");
                    ImGui::EndDragDropSource();
                }

            } else {
                ImGui::Spacing();
                ImGui::TextDisabled("EMPTY");
                ImGui::Spacing();
                ImGui::TextDisabled("Drag part here");
            }

            // === DROP TARGET: Drop inventory part to install ===
            if (ImGui::BeginDragDropTarget()) {
                // Accept parts from inventory
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHIP_PART_INVENTORY")) {
                    i32 inventoryIndex = *static_cast<const i32*>(payload->Data);
                    // Check if part matches this slot
                    if (inventoryIndex >= 0 && inventoryIndex < static_cast<i32>(m_Inventory.size())) {
                        ShipPart& draggedPart = m_Inventory[inventoryIndex];
                        if (draggedPart.slot == static_cast<PartSlot>(slotIndex)) {
                            InstallPart(inventoryIndex);
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::EndChild();
        }
    }

    // --- Total Bonuses Summary ---
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "TOTAL BONUSES:");

    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Production: +%.0f%%", m_TotalPowerBonus);
    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.2f, 1.0f), "Combat: +%.0f%%", m_TotalCombatBonus);
    ImGui::TextColored(ImVec4(0.5f, 0.5f, 1.0f, 1.0f), "Drop Rate: +%.0f%%", m_TotalDropRateBonus);
}

// You might want to call this something like RenderInventoryList()
void Spaceship::RenderInventoryPanel() {
    // Note: Removed Renderer*, panelX, etc. arguments. ImGui handles them.

    // --- Title ---
    ImGui::TextColored(ImVec4(0.0f, 0.8f, 1.0f, 1.0f), "INVENTORY (%zu parts)", m_Inventory.size());
    ImGui::Separator();

    // === DROP TARGET: Drop installed part here to uninstall ===
    ImGui::TextDisabled("Drag installed parts here to uninstall");
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SHIP_PART_INSTALLED")) {
            i32 slotIndex = *static_cast<const i32*>(payload->Data);
            UninstallPart(static_cast<PartSlot>(slotIndex));
            Log::Info("Part uninstalled via drag-and-drop");
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::Separator();

    // Use BeginChild for a fixed-size, scrollable inventory area
    ImGui::BeginChild("InventoryScrollRegion", ImVec2(0, 0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);

    if (m_Inventory.empty()) {
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No parts in inventory");
        ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f), "Observe research stations to find parts!");
    } else {
        for (i32 i = 0; i < static_cast<i32>(m_Inventory.size()); i++) {
            ShipPart& part = m_Inventory[i]; // Use reference for click feedback

            // Apply rarity color to the current ImGui element
            Color c = part.GetRarityColor();
            ImVec4 rarityColor(c.r, c.g, c.b, c.a);

            // === Create selectable region for drag-and-drop ===
            ImGui::PushID(i);
            bool isSelected = (m_SelectedInventoryIndex == i);

            // Use Selectable to make the whole item clickable/draggable
            ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(rarityColor.x * 0.3f, rarityColor.y * 0.3f, rarityColor.z * 0.3f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(rarityColor.x * 0.5f, rarityColor.y * 0.5f, rarityColor.z * 0.5f, 0.7f));
            ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(rarityColor.x * 0.7f, rarityColor.y * 0.7f, rarityColor.z * 0.7f, 0.9f));

            if (ImGui::Selectable("##item", isSelected, ImGuiSelectableFlags_None, ImVec2(0, 60))) {
                m_SelectedInventoryIndex = i;
            }

            ImGui::PopStyleColor(3);

            // === DRAG SOURCE: Drag inventory item ===
            if (!part.installed && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                // Set payload (inventory index)
                ImGui::SetDragDropPayload("SHIP_PART_INVENTORY", &i, sizeof(i32));
                // Preview
                ImGui::TextColored(rarityColor, "%s", part.name.c_str());
                ImGui::Text("(%s slot)", part.GetSlotName());
                ImGui::EndDragDropSource();
            }

            // Draw item content on top of selectable
            ImGui::SameLine();
            ImVec2 itemPos = ImGui::GetCursorPos();
            itemPos.x = ImGui::GetStyle().ItemSpacing.x;
            ImGui::SetCursorPos(itemPos);

            ImGui::BeginGroup();

            // Item Header
            ImGui::PushStyleColor(ImGuiCol_Text, rarityColor);
            ImGui::Text("%s - %s", part.name.c_str(), part.GetSlotName());
            ImGui::PopStyleColor();

            // Status and buttons
            if (part.installed) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 180.0f);
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "INSTALLED");
                ImGui::SameLine();
                if (ImGui::SmallButton("Uninstall")) {
                    UninstallPart(part.slot);
                }
            } else {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x - 180.0f);
                ImGui::TextDisabled("Available");
                ImGui::SameLine();
                if (ImGui::SmallButton("Install")) {
                    InstallPart(i);
                }
                ImGui::SameLine();
                ImGui::TextDisabled("[Drag]");
            }

            // Stats line
            ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.6f, 1.0f),
                               "Pwr:+%.0f%% Cbt:+%.0f%% Drop:+%.0f%% (Lvl +%d)",
                               part.powerBonus * part.GetTotalMultiplier(),
                               part.combatBonus * part.GetTotalMultiplier(),
                               part.dropRateBonus * part.GetTotalMultiplier(),
                               part.enhancementLevel);

            ImGui::EndGroup();

            // Tooltip for full description
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s\nTier %d | Rarity: %s\n\nDrag to ship slot to install",
                    part.description.c_str(), part.tier, part.GetRarityName());
            }

            ImGui::PopID();
            ImGui::Separator(); // Visual separator between parts
        }
    }

    ImGui::EndChild();
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
