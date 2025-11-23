#include "Platform.h"
#include <SDL.h>
#include <sys/stat.h>
#include <cstring>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <shlobj.h>
// Undefine Windows macros that conflict with our function names
#ifdef CreateDirectory
#undef CreateDirectory
#endif
#elif defined(PLATFORM_LINUX) || defined(PLATFORM_MACOS)
#include <unistd.h>
#include <pwd.h>
#include <sys/types.h>
#endif

namespace Platform {

std::string GetSaveDirectory() {
    std::string saveDir;

#ifdef PLATFORM_WINDOWS
    char path[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_APPDATA, NULL, 0, path))) {
        saveDir = std::string(path) + "\\QuantumIdle\\";
    }
#elif defined(PLATFORM_MACOS)
    const char* home = getenv("HOME");
    if (home) {
        saveDir = std::string(home) + "/Library/Application Support/QuantumIdle/";
    }
#elif defined(PLATFORM_LINUX)
    const char* home = getenv("HOME");
    if (home) {
        saveDir = std::string(home) + "/.quantumidle/";
    }
#else
    saveDir = "./saves/";
#endif

    CreateDirectory(saveDir);
    return saveDir;
}

std::string GetAssetPath(const std::string& relativePath) {
    // For now, assets are relative to executable
    return "assets/" + relativePath;
}

bool FileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool CreateDirectory(const std::string& path) {
#ifdef PLATFORM_WINDOWS
    return CreateDirectoryA(path.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS;
#else
    return mkdir(path.c_str(), 0755) == 0 || errno == EEXIST;
#endif
}

double GetTime() {
    return SDL_GetTicks() / 1000.0;
}

uint64_t GetTicks() {
    return SDL_GetTicks64();
}

std::string GetPlatformName() {
#ifdef PLATFORM_WINDOWS
    return "Windows";
#elif defined(PLATFORM_MACOS)
    return "macOS";
#elif defined(PLATFORM_LINUX)
    return "Linux";
#elif defined(PLATFORM_IOS)
    return "iOS";
#elif defined(PLATFORM_ANDROID)
    return "Android";
#else
    return "Unknown";
#endif
}

int GetCPUCount() {
    return SDL_GetCPUCount();
}

DisplayInfo GetDisplayInfo() {
    DisplayInfo info;
    SDL_DisplayMode mode;
    if (SDL_GetCurrentDisplayMode(0, &mode) == 0) {
        info.width = mode.w;
        info.height = mode.h;
    } else {
        info.width = 1920;
        info.height = 1080;
    }

    float ddpi, hdpi, vdpi;
    if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) == 0) {
        info.dpiScale = ddpi / 96.0f;
    } else {
        info.dpiScale = 1.0f;
    }

    return info;
}

} // namespace Platform
