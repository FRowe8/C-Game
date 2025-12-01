#include "EnhancementSystem.h"
#include "Renderer.h"
#include "ImGuiUtils.h"
#include "Logger.h"
#include "GameState.h"
#include "imgui.h" // ADDED: Necessary for ImGui integration
#include <cstdlib>
#include <cmath>
#include <sstream> // ADDED: For string manipulation in UI

EnhancementSystem::EnhancementSystem()
    : m_SelectedInventoryIndex(-1), m_ShowEnhancementUI(false), m_ShowConfirmDismantle(false) {
}

void EnhancementSystem::Initialize() {
    Log::Info("Initializing enhancement system...");

    // Give starting materials for testing
    m_Materials.techScraps = 100;
    m_Materials.nanoAlloy = 20;
    m_Materials.quantumCore = 5;
    m_Materials.universalShards = 10;

    Log::Info("Enhancement system initialized");
}

// ===== Material Management (Untouched) =====

void EnhancementSystem::AddMaterial(MaterialType type, i32 amount) {
    switch (type) {
        case MaterialType::TechScraps:
            m_Materials.techScraps += amount;
            break;
        case MaterialType::NanoAlloy:
            m_Materials.nanoAlloy += amount;
            break;
        case MaterialType::QuantumCore:
            m_Materials.quantumCore += amount;
            break;
        case MaterialType::UniversalShards:
            m_Materials.universalShards += amount;
            break;
        default:
            break;
    }

    Log::Infof("Added ", amount, " ", GetMaterialName(type));
}

bool EnhancementSystem::SpendMaterial(MaterialType type, i32 amount) {
    i32 current = GetMaterial(type);
    if (current < amount) return false;

    switch (type) {
        case MaterialType::TechScraps:
            m_Materials.techScraps -= amount;
            break;
        case MaterialType::NanoAlloy:
            m_Materials.nanoAlloy -= amount;
            break;
        case MaterialType::QuantumCore:
            m_Materials.quantumCore -= amount;
            break;
        case MaterialType::UniversalShards:
            m_Materials.universalShards -= amount;
            break;
        default:
            return false;
    }

    return true;
}

i32 EnhancementSystem::GetMaterial(MaterialType type) const {
    switch (type) {
        case MaterialType::TechScraps:
            return m_Materials.techScraps;
        case MaterialType::NanoAlloy:
            return m_Materials.nanoAlloy;
        case MaterialType::QuantumCore:
            return m_Materials.quantumCore;
        case MaterialType::UniversalShards:
            return m_Materials.universalShards;
        default:
            return 0;
    }
}

// ===== Enhancement System (Untouched) =====

f64 EnhancementSystem::GetSuccessRate(i32 currentLevel) const {
    if (currentLevel < 0) return 0.0;
    if (currentLevel >= 15) return 0.0; // Max level

    // Success rates:
    // +0 to +5: 100%
    // +6 to +10: 80%
    // +11 to +12: 60%
    // +13 to +14: 40%
    // +15: 20%

    if (currentLevel <= 5) return 1.0;
    if (currentLevel <= 10) return 0.8;
    if (currentLevel <= 12) return 0.6;
    if (currentLevel <= 14) return 0.4;
    return 0.2;
}

EnhancementCost EnhancementSystem::GetEnhancementCost(const ShipPart& part) const {
    EnhancementCost cost;

    i32 level = part.enhancementLevel;

    // Base costs scale with level
    cost.credits = 1000 * (level + 1) * (level + 1);
    cost.techScraps = 5 + level * 2;
    cost.nanoAlloy = (level >= 5) ? (level - 4) * 2 : 0;
    cost.quantumCore = (level >= 10) ? (level - 9) : 0;

    // Rarity multiplier
    f64 rarityMult = 1.0;
    switch (part.rarity) {
        case PartRarity::Common: rarityMult = 1.0; break;
        case PartRarity::Uncommon: rarityMult = 1.5; break;
        case PartRarity::Rare: rarityMult = 2.0; break;
        case PartRarity::Epic: rarityMult = 3.0; break;
        case PartRarity::Legendary: rarityMult = 5.0; break;
    }

    cost.credits = static_cast<i32>(cost.credits * rarityMult);
    cost.techScraps = static_cast<i32>(cost.techScraps * rarityMult);
    cost.nanoAlloy = static_cast<i32>(cost.nanoAlloy * rarityMult);
    cost.quantumCore = static_cast<i32>(cost.quantumCore * rarityMult);

    cost.successRate = GetSuccessRate(level);

    return cost;
}

bool EnhancementSystem::CanEnhance(const ShipPart& part, GameState* state) const {
    if (part.enhancementLevel >= 15) return false; // Max level

    EnhancementCost cost = GetEnhancementCost(part);

    // Check materials
    if (m_Materials.techScraps < cost.techScraps) return false;
    if (m_Materials.nanoAlloy < cost.nanoAlloy) return false;
    if (m_Materials.quantumCore < cost.quantumCore) return false;

    // Check credits
    if (state && state->GetResource(QuantumResource::Qubits) < cost.credits) return false;

    return true;
}

EnhancementResult EnhancementSystem::EnhancePart(ShipPart& part, GameState* state) {
    if (part.enhancementLevel >= 15) {
        return EnhancementResult::MaxLevel;
    }

    EnhancementCost cost = GetEnhancementCost(part);

    // Spend materials
    if (!SpendMaterial(MaterialType::TechScraps, cost.techScraps)) return EnhancementResult::Failure;
    if (!SpendMaterial(MaterialType::NanoAlloy, cost.nanoAlloy)) return EnhancementResult::Failure;
    if (!SpendMaterial(MaterialType::QuantumCore, cost.quantumCore)) return EnhancementResult::Failure;

    // Spend credits
    if (state) {
        if (!state->SpendResource(QuantumResource::Qubits, cost.credits)) {
            return EnhancementResult::Failure;
        }
    }

    // Roll for success
    f64 roll = static_cast<f64>(rand()) / RAND_MAX;

    if (roll < cost.successRate) {
        // Success!
        part.enhancementLevel++;
        Log::Infof("Enhancement SUCCESS! ", part.name, " is now +", part.enhancementLevel);
        return EnhancementResult::Success;
    } else {
        // Failure - materials lost but item intact
        Log::Info("Enhancement failed - materials lost");
        return EnhancementResult::Failure;
    }
}

// ===== Star Ascension (Untouched) =====

AscensionCost EnhancementSystem::GetAscensionCost(const ShipPart& part) const {
    AscensionCost cost;

    i32 currentStars = part.stars;
    i32 nextStar = currentStars + 1;

    if (nextStar > 6) return cost; // Max stars

    // Costs scale with star level
    cost.credits = 10000 * nextStar * nextStar;
    cost.universalShards = 10 + (nextStar - 2) * 10; // 10, 20, 30, 40, 50
    cost.requiresDuplicate = (nextStar <= 3); // Stars 2-3 need duplicates, 4+ can use shards

    return cost;
}

bool EnhancementSystem::CanAscend(const ShipPart& part, GameState* state, bool hasDuplicate) const {
    if (part.stars >= 6) return false;

    AscensionCost cost = GetAscensionCost(part);

    // Check if duplicate required
    if (cost.requiresDuplicate && !hasDuplicate) return false;

    // Check materials
    if (!hasDuplicate && m_Materials.universalShards < cost.universalShards) return false;

    // Check credits
    if (state && state->GetResource(QuantumResource::Qubits) < cost.credits) return false;

    return true;
}

bool EnhancementSystem::AscendPart(ShipPart& part, GameState* state, bool useDuplicate) {
    if (part.stars >= 6) return false;

    AscensionCost cost = GetAscensionCost(part);

    // Spend materials
    if (!useDuplicate) {
        if (!SpendMaterial(MaterialType::UniversalShards, cost.universalShards)) {
            return false;
        }
    }

    // Spend credits
    if (state) {
        if (!state->SpendResource(QuantumResource::Qubits, cost.credits)) {
            return false;
        }
    }

    // Upgrade!
    part.stars++;
    Log::Infof("Star ascension SUCCESS! ", part.name, " is now ", part.stars, " stars!");

    return true;
}

// ===== Dismantling (Untouched) =====

MaterialInventory EnhancementSystem::GetDismantleReward(const ShipPart& part) const {
    MaterialInventory reward;

    // Base rewards by rarity
    switch (part.rarity) {
        case PartRarity::Common:
            reward.techScraps = 2;
            break;
        case PartRarity::Uncommon:
            reward.techScraps = 5;
            reward.nanoAlloy = 1;
            break;
        case PartRarity::Rare:
            reward.techScraps = 10;
            reward.nanoAlloy = 3;
            break;
        case PartRarity::Epic:
            reward.techScraps = 20;
            reward.nanoAlloy = 10;
            reward.quantumCore = 2;
            break;
        case PartRarity::Legendary:
            reward.techScraps = 50;
            reward.nanoAlloy = 25;
            reward.quantumCore = 10;
            reward.universalShards = 5;
            break;
    }

    // Bonus materials from enhancement level
    reward.techScraps += part.enhancementLevel * 2;
    reward.nanoAlloy += part.enhancementLevel / 2;

    // Bonus from stars
    reward.universalShards += (part.stars - 1) * 2;

    return reward;
}

void EnhancementSystem::DismantlePart(const ShipPart& part) {
    MaterialInventory reward = GetDismantleReward(part);

    AddMaterial(MaterialType::TechScraps, reward.techScraps);
    AddMaterial(MaterialType::NanoAlloy, reward.nanoAlloy);
    AddMaterial(MaterialType::QuantumCore, reward.quantumCore);
    AddMaterial(MaterialType::UniversalShards, reward.universalShards);

    Log::Infof("Dismantled ", part.name, " for materials");
}

// ===== Rendering (MIGRATED TO IMGUI) =====

void EnhancementSystem::RenderEnhancementUI(Renderer* renderer, GameState* state) {
    if (!m_ShowEnhancementUI) return;

    // Window dimensions
    f32 panelWidth = 900.0f;
    f32 panelHeight = 700.0f;

    // 1. Setup position and size (Centered)
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(displaySize.x * 0.5f, displaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));

    // 2. Setup styles (matching old background/border)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(Color(0.05f, 0.05f, 0.1f, 0.95f)));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color(0.8f, 0.6f, 0.2f, 0.8f))); // Gold border
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);

    // Use the NoTitleBar flag since we are drawing a custom title and close button
    if (ImGui::Begin("##PartEnhancementPanel", &m_ShowEnhancementUI, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {

        // --- Title (Replaces line 320) ---
        ImGui::SetCursorPosX(panelWidth * 0.5f - ImGui::CalcTextSize("PART ENHANCEMENT").x * 0.5f);
        ImGui::TextColored(ToImVec4(Color(1.0f, 0.8f, 0.3f, 1.0f)), "PART ENHANCEMENT");

        // --- Close button (Replaces lines 322-330) ---
        f32 closeBtnSize = 44.0f;
        ImGui::SetCursorPos(ImVec2(panelWidth - closeBtnSize - 10.0f, 10.0f));

        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(Color(0.3f, 0.1f, 0.1f, 0.8f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(Color(0.5f, 0.2f, 0.2f, 1.0f)));

        // This will toggle m_ShowEnhancementUI to false when clicked
        if (ImGui::Button("X##CloseEnhancement", ImVec2(closeBtnSize, closeBtnSize))) {
            m_ShowEnhancementUI = false;
        }
        ImGui::PopStyleColor(2);

        ImGui::Separator();
        ImGui::Spacing();

        // --- Material Display (Left) ---
        // We will allocate the left 2/5 of the panel for materials
        f32 materialWidth = panelWidth * 0.35f;
        f32 detailWidth = panelWidth * 0.60f;

        ImGui::BeginGroup();
        {
            ImGui::BeginChild("##MaterialDisplayChild", ImVec2(materialWidth, 180.0f), true, ImGuiWindowFlags_NoScrollbar);
            RenderMaterialDisplay(renderer, 0, 0, materialWidth); // ImGui handles layout, args are mostly ignored now
            ImGui::EndChild();
        }
        ImGui::EndGroup();

        ImGui::SameLine();

        // --- Part Details (Right) ---
        ImGui::BeginGroup();
        {
            ImGui::BeginChild("##PartDetailsChild", ImVec2(detailWidth, 350.0f), true);
            RenderPartDetails(renderer, state, 0, 0, detailWidth); // ImGui handles layout, args are mostly ignored now
            ImGui::EndChild();

            // Enhancement Options (below Part Details)
            RenderEnhancementOptions(renderer, state, 0, 0, detailWidth);
        }
        ImGui::EndGroup();


        // --- Close hint (Replaces line 338) ---
        ImGui::SetCursorPosY(panelHeight - 35.0f);
        ImGui::SetCursorPosX(panelWidth - ImGui::CalcTextSize("Press ESC to close").x - 20.0f);
        ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "Press ESC to close");

        ImGui::End();
    }
    // 3. Pop styles
    ImGui::PopStyleVar(2); // Pop WindowRounding, WindowBorderSize
    ImGui::PopStyleColor(2); // Pop Border, WindowBg
}

void EnhancementSystem::RenderMaterialDisplay(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    (void)renderer; // No longer needed
    (void)panelX; (void)panelY; (void)panelWidth; // No longer used for absolute positioning

    ImGui::TextColored(ToImVec4(Color::White()), "MATERIALS:"); // Replaces line 345
    ImGui::Spacing();

    // Display each material
    const char* materialNames[] = {"Tech Scraps", "Nano-Alloy", "Quantum Core", "Universal Shards"};
    i32 materialCounts[] = {m_Materials.techScraps, m_Materials.nanoAlloy, m_Materials.quantumCore, m_Materials.universalShards};
    Color materialColors[] = {Color(0.7f, 0.7f, 0.7f, 1.0f), Color(0.3f, 0.8f, 1.0f, 1.0f),
                              Color(0.8f, 0.3f, 1.0f, 1.0f), Color(1.0f, 0.9f, 0.3f, 1.0f)};

    for (i32 i = 0; i < 4; i++) {
        std::stringstream ss;
        ss << materialNames[i] << ": " << materialCounts[i];

        ImGui::TextColored(ToImVec4(materialColors[i]), "%s", ss.str().c_str()); // Replaces line 357

        // This simulates the two-column layout from the original code (i % 2) * 220.0f
        if (i % 2 == 0 && i < 3) {
             // Use SameLine to place the next item on the same row (column 2)
            ImGui::SameLine(panelWidth * 0.5f);
        }
    }
}

void EnhancementSystem::RenderPartDetails(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth) {
    (void)renderer; // No longer needed
    (void)state;
    (void)panelX; (void)panelY; (void)panelWidth;

    ImGui::TextColored(ToImVec4(Color(0.8f, 0.8f, 0.8f, 1.0f)), "SELECT PART TO ENHANCE"); // Replaces line 368
    ImGui::Separator();

    // TODO: Display selected part details, stats, enhancement level, stars
    ImGui::Text("Part List / Details Placeholder...");
}

void EnhancementSystem::RenderEnhancementOptions(Renderer* renderer, GameState* state, f32 panelX, f32 panelY, f32 panelWidth) {
    (void)state;
    (void)renderer;
    (void)panelX; (void)panelY; (void)panelWidth;

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Text("Enhancement Options Placeholder...");

    // TODO: Render enhancement buttons, success rates, costs
}

void EnhancementSystem::HandleClick(f32 mouseX, f32 mouseY, bool mousePressed, GameState* state) {
    (void)mouseX;
    (void)mouseY;
    (void)mousePressed;
    (void)state;

    // TODO: Handle UI clicks for enhancement/ascension/dismantle
    // NOTE: This function should be removed once all UI interaction
    // is handled by ImGui::Button and ImGui::Checkbox calls in the Render functions.
}

// ===== Helpers (Untouched) =====

Color EnhancementSystem::GetMaterialColor(MaterialType type) const {
    switch (type) {
        case MaterialType::TechScraps: return Color(0.7f, 0.7f, 0.7f, 1.0f);
        case MaterialType::NanoAlloy: return Color(0.3f, 0.8f, 1.0f, 1.0f);
        case MaterialType::QuantumCore: return Color(0.8f, 0.3f, 1.0f, 1.0f);
        case MaterialType::UniversalShards: return Color(1.0f, 0.9f, 0.3f, 1.0f);
        default: return Color::White();
    }
}

const char* EnhancementSystem::GetMaterialName(MaterialType type) const {
    switch (type) {
        case MaterialType::TechScraps: return "Tech Scraps";
        case MaterialType::NanoAlloy: return "Nano-Alloy";
        case MaterialType::QuantumCore: return "Quantum Core";
        case MaterialType::UniversalShards: return "Universal Shards";
        default: return "Unknown";
    }
}

// ===== Serialization (Untouched) =====

std::string EnhancementSystem::SaveToJson() const {
    char buffer[512];
    snprintf(buffer, sizeof(buffer),
        "{\"techScraps\":%d,\"nanoAlloy\":%d,\"quantumCore\":%d,\"universalShards\":%d}",
        m_Materials.techScraps, m_Materials.nanoAlloy, m_Materials.quantumCore, m_Materials.universalShards);
    return std::string(buffer);
}

void EnhancementSystem::LoadFromJson(const std::string& line) {
    (void)line;
    // TODO: Parse JSON and load materials
}