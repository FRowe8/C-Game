#include "SteamIntegration.h"
#include "Logger.h"

#include <fstream>
#include <sstream>

namespace SteamIntegration {
namespace {
    bool g_Initialized = false;
    bool g_StubMode = true; // Placeholder until Steamworks SDK is wired up
}

bool Initialize() {
    // In a full Steam build, initialization would load SteamAPI and return false on failure.
    g_Initialized = true;
    Log::Info("Steam integration initialized in stub mode (no SteamAPI linkage)");
    return true;
}

void Shutdown() {
    if (!g_Initialized) return;
    Log::Info("Steam integration shutdown");
    g_Initialized = false;
}

void RunCallbacks() {
    if (!g_Initialized) return;
    // SteamAPI_RunCallbacks would be invoked here when the SDK is linked.
}

bool IsReady() {
    return g_Initialized;
}

void UnlockAchievement(const std::string& achievementId) {
    if (!g_Initialized) return;
    Log::Infof("[Steam] Unlock achievement requested: ", achievementId, g_StubMode ? " (stub)" : "");
    // Future: SteamUserStats()->SetAchievement and StoreStats.
}

void MirrorSaveToCloud(const std::string& filepath, const std::string& slotName) {
    if (!g_Initialized) return;

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        Log::Warningf("[Steam] Could not mirror save to cloud (file missing): ", filepath);
        return;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    const auto saveData = buffer.str();

    Log::Infof("[Steam] Prepared cloud save for slot ", slotName, " (", saveData.size(), " bytes)" , g_StubMode ? " [stub]" : "");
    // Future: SteamRemoteStorage()->FileWrite with slotName + .json.
}

} // namespace SteamIntegration
