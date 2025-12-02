#pragma once

#include <string>
#include <cstdint>

// Platform detection macros are defined by CMake
// PLATFORM_WINDOWS, PLATFORM_LINUX, PLATFORM_MACOS, PLATFORM_IOS, PLATFORM_ANDROID

namespace Platform {

// File system
std::string GetSaveDirectory();
std::string GetAssetPath(const std::string& relativePath);
bool FileExists(const std::string& path);
bool CreateDirectory(const std::string& path);

// Time
double GetTime(); // Returns time in seconds since app start
uint64_t GetTicks(); // Returns milliseconds

// System info
std::string GetPlatformName();
int GetCPUCount();

// Display
struct DisplayInfo {
    int width;
    int height;
    float dpiScale;
};
DisplayInfo GetDisplayInfo();

// File system sync (Critical for Web/Emscripten to persist to IndexedDB)
void SyncFileSystem();

} // namespace Platform
