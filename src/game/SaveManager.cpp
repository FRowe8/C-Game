#include "SaveManager.h"
#include "GameState.h"
#include "Log.h"
#include "Platform.h"
#include <fstream>

SaveManager::SaveManager()
    : m_GameState(nullptr)
    , m_AutosaveEnabled(true)
    , m_AutosaveInterval(60.0) // Autosave every 60 seconds
    , m_TimeSinceAutosave(0.0)
    , m_LastSaveTimestamp(0)
{
}

void SaveManager::Initialize(GameState* gameState) {
    m_GameState = gameState;
    m_TimeSinceAutosave = 0.0;
}

void SaveManager::Update(f64 deltaTime) {
    if (!m_AutosaveEnabled || !m_GameState) {
        return;
    }

    m_TimeSinceAutosave += deltaTime;

    if (m_TimeSinceAutosave >= m_AutosaveInterval) {
        TriggerAutosave();
    }
}

bool SaveManager::Save(const std::string& filepath) {
    if (!m_GameState) {
        Log::Error("SaveManager: No GameState reference set!");
        return false;
    }

    // Delegate to GameState's existing Save implementation for now
    // TODO: Implement proper JSON serialization using nlohmann/json
    bool success = m_GameState->Save(filepath);

    if (success) {
        m_LastSaveFilepath = filepath;
        m_LastSaveTimestamp = static_cast<i64>(Platform::GetTime());
        Log::Infof("Game saved successfully to: ", filepath);
    }

    return success;
}

bool SaveManager::Load(const std::string& filepath) {
    if (!m_GameState) {
        Log::Error("SaveManager: No GameState reference set!");
        return false;
    }

    // Delegate to GameState's existing Load implementation for now
    // TODO: Implement proper JSON deserialization using nlohmann/json
    bool success = m_GameState->Load(filepath);

    if (success) {
        m_LastSaveFilepath = filepath;
        Log::Infof("Game loaded successfully from: ", filepath);
    } else {
        Log::Warningf("Failed to load game from: ", filepath);
    }

    return success;
}

void SaveManager::TriggerAutosave() {
    if (!m_LastSaveFilepath.empty()) {
        Log::Debug("Autosave triggered...");
        Save(m_LastSaveFilepath);
    }
    m_TimeSinceAutosave = 0.0;
}

// === Helper Methods (Stubbed for future implementation) ===

void SaveManager::WriteHeader(std::ofstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
}

void SaveManager::WriteResources(std::ofstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
}

void SaveManager::WriteTimeline(std::ofstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
}

void SaveManager::WriteStations(std::ofstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
}

void SaveManager::WriteStatistics(std::ofstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
}

void SaveManager::WriteAchievements(std::ofstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
}

void SaveManager::WriteResearch(std::ofstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
}

bool SaveManager::LoadVersion1(std::ifstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
    return false;
}

bool SaveManager::LoadVersion2(std::ifstream& file) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)file;
    return false;
}

std::string SaveManager::ExtractValue(const std::string& line, const std::string& key) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)line;
    (void)key;
    return "";
}

f64 SaveManager::ExtractNumberValue(const std::string& line, const std::string& key) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)line;
    (void)key;
    return 0.0;
}

bool SaveManager::ExtractBoolValue(const std::string& line, const std::string& key) {
    // TODO: Implement when refactoring to nlohmann/json
    (void)line;
    (void)key;
    return false;
}
