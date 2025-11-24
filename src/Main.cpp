// Define SDL_MAIN_HANDLED before including SDL.h on non-Emscripten platforms
// This tells SDL to NOT redefine main, we'll handle the entry point ourselves
#ifndef __EMSCRIPTEN__
#define SDL_MAIN_HANDLED
#endif

#include "Application.h"
#include "Logger.h"
#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <stdio.h>

// SDL_MAIN_HANDLED tells SDL we're handling main() ourselves
// This works correctly on all platforms

int main(int argc, char* argv[]) {
    printf("========================================\n");
    printf("=== MAIN() CALLED - argc: %d ===\n", argc);
    printf("========================================\n");

#ifdef __EMSCRIPTEN__
    printf("=== EMSCRIPTEN DETECTED IN MAIN ===\n");
#else
    // Tell SDL that we're handling main() ourselves
    SDL_SetMainReady();
#endif

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
    printf("=== Creating Application... ===\n");
    Application app(config);

    printf("=== Initializing Application... ===\n");
    if (!app.Initialize()) {
        printf("=== FAILED TO INITIALIZE ===\n");
        Log::Error("Failed to initialize application");
        return 1;
    }

    printf("=== Running Application... ===\n");
    app.Run();

    printf("=== Shutting down Application... ===\n");
    app.Shutdown();

    printf("=== Exiting main() ===\n");
    return 0;
}
