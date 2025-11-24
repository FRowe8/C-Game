#include "Application.h"
#include "Logger.h"
#include <SDL.h>
#include <cstdlib>
#include <ctime>
#include <stdio.h>

// On Windows and Emscripten, undef SDL's main macro to use our own entry point
// SDL's main macro interferes with Emscripten's module initialization
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
    #undef main
#endif

int main(int argc, char* argv[]) {
    printf("========================================\n");
    printf("=== MAIN() CALLED - argc: %d ===\n", argc);
    printf("========================================\n");

#ifdef __EMSCRIPTEN__
    printf("=== EMSCRIPTEN DETECTED IN MAIN ===\n");
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
