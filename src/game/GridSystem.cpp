#include "GridSystem.h"
#include "logger.h"
#include <algorithm>
#include <fstream>

GridSystem::GridSystem() {
    Initialize();
}

void GridSystem::Initialize() {
    // Initialize all slots as empty and unlocked
    for (int i = 0; i < 9; i++) {
        m_Grid[i].crewID = -1;
        m_Grid[i].locked = false;
        m_Grid[i].bonusMultiplier = 1.0;
    }

    // Set position-specific bonuses
    // Center (1,1) = 1.5x
    GetSlot(GridPos(1, 1)).bonusMultiplier = 1.5;

    // Corners = 1.2x
    GetSlot(GridPos(0, 0)).bonusMultiplier = 1.2;
    GetSlot(GridPos(0, 2)).bonusMultiplier = 1.2;
    GetSlot(GridPos(2, 0)).bonusMultiplier = 1.2;
    GetSlot(GridPos(2, 2)).bonusMultiplier = 1.2;

    // Edges = 1.1x
    GetSlot(GridPos(0, 1)).bonusMultiplier = 1.1;
    GetSlot(GridPos(1, 0)).bonusMultiplier = 1.1;
    GetSlot(GridPos(1, 2)).bonusMultiplier = 1.1;
    GetSlot(GridPos(2, 1)).bonusMultiplier = 1.1;

    // Initialize formation bonuses
    InitializeFormations();

    Log::Info("GridSystem initialized (3x3)");
}

void GridSystem::Clear() {
    for (int i = 0; i < 9; i++) {
        m_Grid[i].crewID = -1;
    }
    Log::Info("GridSystem cleared");
}

// === Crew Placement ===

bool GridSystem::PlaceCrew(i32 crewID, const GridPos& pos) {
    if (!IsValidPos(pos)) {
        Log::Warningf("Invalid grid position: (", pos.row, ", ", pos.col, ")");
        return false;
    }

    GridSlot& slot = GetSlot(pos);

    if (slot.locked) {
        Log::Warning("Cannot place crew in locked slot");
        return false;
    }

    if (slot.crewID != -1) {
        Log::Warningf("Slot already occupied by crew ", slot.crewID);
        return false;
    }

    slot.crewID = crewID;
    Log::Infof("Placed crew ", crewID, " at (", pos.row, ", ", pos.col, ")");
    return true;
}

bool GridSystem::PlaceCrew(i32 crewID, i32 row, i32 col) {
    return PlaceCrew(crewID, GridPos(row, col));
}

bool GridSystem::RemoveCrew(const GridPos& pos) {
    if (!IsValidPos(pos)) {
        return false;
    }

    GridSlot& slot = GetSlot(pos);
    if (slot.crewID == -1) {
        Log::Warning("No crew to remove at this position");
        return false;
    }

    i32 removedCrewID = slot.crewID;
    slot.crewID = -1;

    Log::Infof("Removed crew ", removedCrewID, " from (", pos.row, ", ", pos.col, ")");
    return true;
}

bool GridSystem::RemoveCrew(i32 row, i32 col) {
    return RemoveCrew(GridPos(row, col));
}

bool GridSystem::MoveCrew(const GridPos& from, const GridPos& to) {
    if (!IsValidPos(from) || !IsValidPos(to)) {
        Log::Warning("Invalid grid positions for move");
        return false;
    }

    GridSlot& fromSlot = GetSlot(from);
    GridSlot& toSlot = GetSlot(to);

    if (fromSlot.crewID == -1) {
        Log::Warning("No crew to move at source position");
        return false;
    }

    if (toSlot.locked) {
        Log::Warning("Destination slot is locked");
        return false;
    }

    if (toSlot.crewID != -1) {
        Log::Warning("Destination slot is already occupied");
        return false;
    }

    i32 crewID = fromSlot.crewID;
    fromSlot.crewID = -1;
    toSlot.crewID = crewID;

    Log::Infof("Moved crew ", crewID, " from (", from.row, ", ", from.col, ") to (", to.row, ", ", to.col, ")");
    return true;
}

void GridSystem::SwapCrew(const GridPos& pos1, const GridPos& pos2) {
    if (!IsValidPos(pos1) || !IsValidPos(pos2)) {
        Log::Warning("Invalid grid positions for swap");
        return;
    }

    GridSlot& slot1 = GetSlot(pos1);
    GridSlot& slot2 = GetSlot(pos2);

    i32 temp = slot1.crewID;
    slot1.crewID = slot2.crewID;
    slot2.crewID = temp;

    Log::Infof("Swapped crew at (", pos1.row, ", ", pos1.col, ") and (", pos2.row, ", ", pos2.col, ")");
}

// === Grid State ===

i32 GridSystem::GetCrewAt(const GridPos& pos) const {
    if (!IsValidPos(pos)) {
        return -1;
    }
    return GetSlot(pos).crewID;
}

i32 GridSystem::GetCrewAt(i32 row, i32 col) const {
    return GetCrewAt(GridPos(row, col));
}

bool GridSystem::IsSlotEmpty(const GridPos& pos) const {
    return GetCrewAt(pos) == -1;
}

bool GridSystem::IsSlotLocked(const GridPos& pos) const {
    if (!IsValidPos(pos)) {
        return true; // Invalid positions are "locked"
    }
    return GetSlot(pos).locked;
}

GridPos GridSystem::FindCrew(i32 crewID) const {
    for (int i = 0; i < 9; i++) {
        if (m_Grid[i].crewID == crewID) {
            return GridPos::FromIndex(i);
        }
    }
    return GridPos(-1, -1); // Not found
}

// === Slot Management ===

void GridSystem::LockSlot(const GridPos& pos) {
    if (IsValidPos(pos)) {
        GetSlot(pos).locked = true;
        Log::Infof("Locked slot (", pos.row, ", ", pos.col, ")");
    }
}

void GridSystem::UnlockSlot(const GridPos& pos) {
    if (IsValidPos(pos)) {
        GetSlot(pos).locked = false;
        Log::Infof("Unlocked slot (", pos.row, ", ", pos.col, ")");
    }
}

void GridSystem::SetSlotBonus(const GridPos& pos, f64 multiplier) {
    if (IsValidPos(pos)) {
        GetSlot(pos).bonusMultiplier = multiplier;
    }
}

f64 GridSystem::GetSlotBonus(const GridPos& pos) const {
    if (!IsValidPos(pos)) {
        return 1.0;
    }
    return GetSlot(pos).bonusMultiplier;
}

// === Adjacency ===

std::vector<GridPos> GridSystem::GetAdjacentPositions(const GridPos& pos, bool includeDiagonal) const {
    std::vector<GridPos> adjacent;

    if (!IsValidPos(pos)) {
        return adjacent;
    }

    // Cardinal directions (up, down, left, right)
    GridPos candidates[8] = {
        GridPos(pos.row - 1, pos.col),     // Up
        GridPos(pos.row + 1, pos.col),     // Down
        GridPos(pos.row, pos.col - 1),     // Left
        GridPos(pos.row, pos.col + 1),     // Right
        GridPos(pos.row - 1, pos.col - 1), // Top-left
        GridPos(pos.row - 1, pos.col + 1), // Top-right
        GridPos(pos.row + 1, pos.col - 1), // Bottom-left
        GridPos(pos.row + 1, pos.col + 1)  // Bottom-right
    };

    i32 checkCount = includeDiagonal ? 8 : 4;

    for (i32 i = 0; i < checkCount; i++) {
        if (candidates[i].IsValid()) {
            adjacent.push_back(candidates[i]);
        }
    }

    return adjacent;
}

std::vector<i32> GridSystem::GetAdjacentCrew(const GridPos& pos, bool includeDiagonal) const {
    std::vector<i32> crewList;
    std::vector<GridPos> adjPositions = GetAdjacentPositions(pos, includeDiagonal);

    for (const auto& adjPos : adjPositions) {
        i32 crewID = GetCrewAt(adjPos);
        if (crewID != -1) {
            crewList.push_back(crewID);
        }
    }

    return crewList;
}

i32 GridSystem::GetAdjacentCount(const GridPos& pos, bool includeDiagonal) const {
    return static_cast<i32>(GetAdjacentCrew(pos, includeDiagonal).size());
}

// === Formation Detection ===

bool GridSystem::HasFormation(FormationType type) const {
    switch (type) {
        case FormationType::HorizontalLine:
            // Check all 3 rows
            for (i32 row = 0; row < 3; row++) {
                if (GetCrewAt(row, 0) != -1 && GetCrewAt(row, 1) != -1 && GetCrewAt(row, 2) != -1) {
                    return true;
                }
            }
            return false;

        case FormationType::VerticalLine:
            // Check all 3 columns
            for (i32 col = 0; col < 3; col++) {
                if (GetCrewAt(0, col) != -1 && GetCrewAt(1, col) != -1 && GetCrewAt(2, col) != -1) {
                    return true;
                }
            }
            return false;

        case FormationType::DiagonalLine:
            // Check both diagonals
            bool diagonal1 = (GetCrewAt(0, 0) != -1 && GetCrewAt(1, 1) != -1 && GetCrewAt(2, 2) != -1);
            bool diagonal2 = (GetCrewAt(0, 2) != -1 && GetCrewAt(1, 1) != -1 && GetCrewAt(2, 0) != -1);
            return diagonal1 || diagonal2;

        case FormationType::Cross:
            // Center + 4 cardinal adjacent
            return GetCrewAt(1, 1) != -1 &&
                   GetCrewAt(0, 1) != -1 &&
                   GetCrewAt(2, 1) != -1 &&
                   GetCrewAt(1, 0) != -1 &&
                   GetCrewAt(1, 2) != -1;

        case FormationType::Square:
            // Check all possible 2x2 squares
            for (i32 row = 0; row < 2; row++) {
                for (i32 col = 0; col < 2; col++) {
                    if (GetCrewAt(row, col) != -1 &&
                        GetCrewAt(row, col + 1) != -1 &&
                        GetCrewAt(row + 1, col) != -1 &&
                        GetCrewAt(row + 1, col + 1) != -1) {
                        return true;
                    }
                }
            }
            return false;

        case FormationType::Corners:
            // All 4 corners filled
            return GetCrewAt(0, 0) != -1 &&
                   GetCrewAt(0, 2) != -1 &&
                   GetCrewAt(2, 0) != -1 &&
                   GetCrewAt(2, 2) != -1;

        case FormationType::Perimeter:
            // All 8 outer slots filled (not center)
            return GetCrewAt(0, 0) != -1 && GetCrewAt(0, 1) != -1 && GetCrewAt(0, 2) != -1 &&
                   GetCrewAt(1, 0) != -1 && GetCrewAt(1, 2) != -1 &&
                   GetCrewAt(2, 0) != -1 && GetCrewAt(2, 1) != -1 && GetCrewAt(2, 2) != -1;

        case FormationType::Center:
            // Center slot filled
            return GetCrewAt(1, 1) != -1;

        default:
            return false;
    }
}

std::vector<FormationType> GridSystem::GetActiveFormations() const {
    std::vector<FormationType> activeFormations;

    for (i32 i = 0; i < static_cast<i32>(FormationType::COUNT); i++) {
        FormationType type = static_cast<FormationType>(i);
        if (HasFormation(type)) {
            activeFormations.push_back(type);
        }
    }

    return activeFormations;
}

FormationBonus GridSystem::GetFormationBonus(FormationType type) const {
    for (const auto& bonus : m_FormationBonuses) {
        if (bonus.type == type) {
            return bonus;
        }
    }
    return FormationBonus(); // No bonus
}

// === Total Bonuses ===

f64 GridSystem::GetTotalDamageMultiplier() const {
    f64 multiplier = 1.0;

    // Add formation bonuses
    std::vector<FormationType> formations = GetActiveFormations();
    for (const auto& formationType : formations) {
        FormationBonus bonus = GetFormationBonus(formationType);
        multiplier *= bonus.damageMultiplier;
    }

    // Add position bonuses (average across filled slots)
    i32 filledSlots = GetFilledSlots();
    if (filledSlots > 0) {
        f64 totalPosBonus = 0.0;
        for (int i = 0; i < 9; i++) {
            if (m_Grid[i].crewID != -1) {
                totalPosBonus += m_Grid[i].bonusMultiplier;
            }
        }
        multiplier *= (totalPosBonus / filledSlots);
    }

    return multiplier;
}

f64 GridSystem::GetTotalDefenseMultiplier() const {
    f64 multiplier = 1.0;

    std::vector<FormationType> formations = GetActiveFormations();
    for (const auto& formationType : formations) {
        FormationBonus bonus = GetFormationBonus(formationType);
        multiplier *= bonus.defenseMultiplier;
    }

    return multiplier;
}

f64 GridSystem::GetTotalResourceMultiplier() const {
    f64 multiplier = 1.0;

    std::vector<FormationType> formations = GetActiveFormations();
    for (const auto& formationType : formations) {
        FormationBonus bonus = GetFormationBonus(formationType);
        multiplier *= bonus.resourceMultiplier;
    }

    return multiplier;
}

// === Statistics ===

i32 GridSystem::GetFilledSlots() const {
    i32 count = 0;
    for (int i = 0; i < 9; i++) {
        if (m_Grid[i].crewID != -1) {
            count++;
        }
    }
    return count;
}

i32 GridSystem::GetEmptySlots() const {
    return 9 - GetFilledSlots();
}

i32 GridSystem::GetUnlockedSlots() const {
    i32 count = 0;
    for (int i = 0; i < 9; i++) {
        if (!m_Grid[i].locked) {
            count++;
        }
    }
    return count;
}

// === Grid Layout ===

void GridSystem::PrintGrid() const {
    Log::Info("=== Grid Layout ===");
    for (i32 row = 0; row < 3; row++) {
        std::string line = "";
        for (i32 col = 0; col < 3; col++) {
            i32 crewID = GetCrewAt(row, col);
            if (crewID == -1) {
                line += "[ ] ";
            } else {
                char buf[8];
                snprintf(buf, sizeof(buf), "[%d] ", crewID);
                line += buf;
            }
        }
        Log::Info(line);
    }
    Log::Info("===================");
}

// === Save/Load ===

void GridSystem::SaveToJson(std::ofstream& file) const {
    file << "    \"gridSystem\": {\n";
    file << "        \"grid\": [\n";

    for (int i = 0; i < 9; i++) {
        file << "            {\"crewID\": " << m_Grid[i].crewID
             << ", \"locked\": " << (m_Grid[i].locked ? "true" : "false")
             << ", \"bonusMultiplier\": " << m_Grid[i].bonusMultiplier << "}";
        if (i < 8) file << ",";
        file << "\n";
    }

    file << "        ]\n";
    file << "    }";
}

void GridSystem::LoadFromJson(const std::string& line) {
    // Simple manual JSON parsing (to be replaced with nlohmann/json later)
    // For now, stub this out
    (void)line;
}

// === Private Helpers ===

void GridSystem::InitializeFormations() {
    m_FormationBonuses.clear();

    // Define formation bonuses
    m_FormationBonuses.push_back(FormationBonus(
        FormationType::HorizontalLine, 1.2, 1.0, 1.0,
        "Horizontal Line: +20% damage"
    ));

    m_FormationBonuses.push_back(FormationBonus(
        FormationType::VerticalLine, 1.0, 1.2, 1.0,
        "Vertical Line: +20% defense"
    ));

    m_FormationBonuses.push_back(FormationBonus(
        FormationType::DiagonalLine, 1.15, 1.15, 1.0,
        "Diagonal Line: +15% damage and defense"
    ));

    m_FormationBonuses.push_back(FormationBonus(
        FormationType::Cross, 1.3, 1.0, 1.0,
        "Cross Formation: +30% damage"
    ));

    m_FormationBonuses.push_back(FormationBonus(
        FormationType::Square, 1.0, 1.3, 1.0,
        "Square Formation: +30% defense"
    ));

    m_FormationBonuses.push_back(FormationBonus(
        FormationType::Corners, 1.1, 1.1, 1.2,
        "Four Corners: +10% damage/defense, +20% resources"
    ));

    m_FormationBonuses.push_back(FormationBonus(
        FormationType::Perimeter, 1.5, 1.5, 1.0,
        "Full Perimeter: +50% damage and defense"
    ));

    m_FormationBonuses.push_back(FormationBonus(
        FormationType::Center, 1.0, 1.0, 1.1,
        "Center: +10% resources"
    ));

    Log::Infof("Initialized ", static_cast<i32>(m_FormationBonuses.size()), " formation bonuses");
}

GridSlot& GridSystem::GetSlot(const GridPos& pos) {
    return m_Grid[pos.ToIndex()];
}

const GridSlot& GridSystem::GetSlot(const GridPos& pos) const {
    return m_Grid[pos.ToIndex()];
}
