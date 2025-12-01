#pragma once

#include "Types.h"
#include <string>
#include <vector>

// Forward declarations
class GameState;
struct ResearchNode;
struct Achievement;
struct ResearchStation;
struct GameStatistics;
struct QuantumTimeline;

/**
 * SaveManager - Centralized save/load system
 *
 * Handles all game persistence including:
 * - Saving and loading game state
 * - Autosave functionality
 * - Save file versioning
 * - Save data migration
 *
 * Current implementation uses manual JSON formatting.
 * TODO: Replace with nlohmann/json for robust parsing (see IMPROVEMENTS.md)
 *
 * This class extracts save/load logic from GameState to establish
 * proper separation of concerns.
 */
class SaveManager {
public:
    SaveManager();
    ~SaveManager() = default;

    // Initialize with game state reference
    void Initialize(GameState* gameState);

    // Update (handles autosave)
    void Update(f64 deltaTime);

    // === Save/Load Operations ===

    // Save game to file (returns true on success)
    bool Save(const std::string& filepath);

    // Load game from file (returns true on success)
    bool Load(const std::string& filepath);

    // === Autosave ===

    void EnableAutosave(bool enabled) { m_AutosaveEnabled = enabled; }
    bool IsAutosaveEnabled() const { return m_AutosaveEnabled; }

    void SetAutosaveInterval(f64 seconds) { m_AutosaveInterval = seconds; }
    f64 GetAutosaveInterval() const { return m_AutosaveInterval; }

    f64 GetTimeSinceLastAutosave() const { return m_TimeSinceAutosave; }

    // Manually trigger autosave
    void TriggerAutosave();

    // === Save File Info ===

    std::string GetLastSaveFilepath() const { return m_LastSaveFilepath; }
    i64 GetLastSaveTimestamp() const { return m_LastSaveTimestamp; }

    // === Save Version ===

    i32 GetCurrentSaveVersion() const { return 2; }

private:
    // Game state reference (we don't own this)
    GameState* m_GameState;

    // Autosave settings
    bool m_AutosaveEnabled;
    f64 m_AutosaveInterval;      // Autosave every N seconds (default 60s)
    f64 m_TimeSinceAutosave;

    // Save file tracking
    std::string m_LastSaveFilepath;
    i64 m_LastSaveTimestamp;

    // === Helper Methods ===

    // Save individual sections (for cleaner code)
    void WriteHeader(std::ofstream& file);
    void WriteResources(std::ofstream& file);
    void WriteTimeline(std::ofstream& file);
    void WriteStations(std::ofstream& file);
    void WriteStatistics(std::ofstream& file);
    void WriteAchievements(std::ofstream& file);
    void WriteResearch(std::ofstream& file);

    // Load helpers
    bool LoadVersion1(std::ifstream& file);
    bool LoadVersion2(std::ifstream& file);

    // Parse JSON value from line (simple utility)
    std::string ExtractValue(const std::string& line, const std::string& key);
    f64 ExtractNumberValue(const std::string& line, const std::string& key);
    bool ExtractBoolValue(const std::string& line, const std::string& key);
};
