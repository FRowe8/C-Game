// Define SDL_MAIN_HANDLED to avoid SDL2main.lib dependency
// This is needed when using MinGW with MSVC-compiled SDL2 libraries
#define SDL_MAIN_HANDLED
#include <SDL.h>

#include "Application.h"
#include "Logger.h"
#include <cstdlib>
#include <ctime>

int main(int argc, char* argv[]) {
    // Initialize SDL's main handling
    SDL_SetMainReady();

    // Seed random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    // Configure application
    ApplicationConfig config;
    config.title = "Quantum Idle - A Cross-Platform Idle Game";
    config.windowWidth = 1280;
    config.windowHeight = 720;
    config.fullscreen = false;
    config.vsync = true;
    config.targetFPS = 60;

    // Create and run application
    Application app(config);

    if (!app.Initialize()) {
        Log::Error("Failed to initialize application");
        return 1;
    }

    app.Run();
    app.Shutdown();

    return 0;
}
