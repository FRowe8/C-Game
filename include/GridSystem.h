#pragma once

#include "Types.h"
#include <string>
#include <vector>

// Forward declarations
class Crew;

// Grid position
struct GridPos {
    i32 row;    // 0-2
    i32 col;    // 0-2

    GridPos() : row(0), col(0) {}
    GridPos(i32 r, i32 c) : row(r), col(c) {}

    bool IsValid() const {
        return row >= 0 && row < 3 && col >= 0 && col < 3;
    }

    bool operator==(const GridPos& other) const {
        return row == other.row && col == other.col;
    }

    // Get index in flat array (0-8)
    i32 ToIndex() const {
        return row * 3 + col;
    }

    static GridPos FromIndex(i32 index) {
        return GridPos(index / 3, index % 3);
    }
};

// Grid slot state
struct GridSlot {
    i32 crewID;             // -1 = empty
    bool locked;            // Some slots may be locked initially
    f64 bonusMultiplier;    // Position-specific bonus (center = 1.5x, corners = 1.2x, etc.)

    GridSlot()
        : crewID(-1), locked(false), bonusMultiplier(1.0) {}
};

// Formation pattern (e.g., diagonal line, square, cross)
enum class FormationType {
    None,
    HorizontalLine,     // 3 crew in a row
    VerticalLine,       // 3 crew in a column
    DiagonalLine,       // 3 crew diagonally
    Cross,              // Center + 4 adjacent
    Square,             // 2x2 square
    Corners,            // All 4 corners
    Perimeter,          // All 8 outer slots
    Center,             // Center slot filled
    COUNT
};

// Formation bonus
struct FormationBonus {
    FormationType type;
    f64 damageMultiplier;
    f64 defenseMultiplier;
    f64 resourceMultiplier;
    std::string description;

    FormationBonus()
        : type(FormationType::None),
          damageMultiplier(1.0),
          defenseMultiplier(1.0),
          resourceMultiplier(1.0),
          description("") {}

    FormationBonus(FormationType _type, f64 _damage, f64 _defense, f64 _resource, const std::string& _desc)
        : type(_type),
          damageMultiplier(_damage),
          defenseMultiplier(_defense),
          resourceMultiplier(_resource),
          description(_desc) {}
};

// Grid System - 3x3 Crew positioning with synergy bonuses
class GridSystem {
public:
    GridSystem();

    void Initialize();
    void Clear();

    // === Crew Placement ===
    bool PlaceCrew(i32 crewID, const GridPos& pos);
    bool PlaceCrew(i32 crewID, i32 row, i32 col);
    bool RemoveCrew(const GridPos& pos);
    bool RemoveCrew(i32 row, i32 col);
    bool MoveCrew(const GridPos& from, const GridPos& to);
    void SwapCrew(const GridPos& pos1, const GridPos& pos2);

    // === Grid State ===
    i32 GetCrewAt(const GridPos& pos) const;
    i32 GetCrewAt(i32 row, i32 col) const;
    bool IsSlotEmpty(const GridPos& pos) const;
    bool IsSlotLocked(const GridPos& pos) const;
    GridPos FindCrew(i32 crewID) const; // Returns GridPos(-1, -1) if not found

    // === Slot Management ===
    void LockSlot(const GridPos& pos);
    void UnlockSlot(const GridPos& pos);
    void SetSlotBonus(const GridPos& pos, f64 multiplier);
    f64 GetSlotBonus(const GridPos& pos) const;

    // === Adjacency ===
    std::vector<GridPos> GetAdjacentPositions(const GridPos& pos, bool includeDiagonal = false) const;
    std::vector<i32> GetAdjacentCrew(const GridPos& pos, bool includeDiagonal = false) const;
    i32 GetAdjacentCount(const GridPos& pos, bool includeDiagonal = false) const;

    // === Formation Detection ===
    bool HasFormation(FormationType type) const;
    std::vector<FormationType> GetActiveFormations() const;
    FormationBonus GetFormationBonus(FormationType type) const;

    // === Total Bonuses ===
    f64 GetTotalDamageMultiplier() const;
    f64 GetTotalDefenseMultiplier() const;
    f64 GetTotalResourceMultiplier() const;

    // === Statistics ===
    i32 GetFilledSlots() const;
    i32 GetEmptySlots() const;
    i32 GetTotalSlots() const { return 9; }
    i32 GetUnlockedSlots() const;

    // === Grid Layout ===
    const GridSlot* GetGrid() const { return m_Grid; }
    void PrintGrid() const; // Debug: print grid to console

    // === Save/Load ===
    void SaveToJson(std::ofstream& file) const;
    void LoadFromJson(const std::string& line);

private:
    GridSlot m_Grid[9]; // 3x3 grid (row-major order)

    // Formation definitions
    void InitializeFormations();
    std::vector<FormationBonus> m_FormationBonuses;

    // Helper: Check if position is valid
    bool IsValidPos(const GridPos& pos) const {
        return pos.IsValid();
    }

    // Helper: Get slot reference
    GridSlot& GetSlot(const GridPos& pos);
    const GridSlot& GetSlot(const GridPos& pos) const;
};
