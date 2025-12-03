#pragma once

#include <string>

namespace SteamIntegration {

// Initialize Steamworks (or stub). Returns true if initialization succeeded.
bool Initialize();

// Shut down Steamworks safely.
void Shutdown();

// Pump callbacks (should be called once per frame on Steam builds).
void RunCallbacks();

// Whether Steam integration is active (real or stub).
bool IsReady();

// Queue an achievement unlock with Steam.
void UnlockAchievement(const std::string& achievementId);

// Mirror the local save to Steam Cloud when available.
void MirrorSaveToCloud(const std::string& filepath, const std::string& slotName);

} // namespace SteamIntegration
