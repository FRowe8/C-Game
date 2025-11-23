#include "Application.h"
#include "Logger.h"
#include <SDL.h>
#include <cstdlib>
#include <ctime>

// Use SDL's main macro for cross-platform compatibility
#undef main

int main(int argc, char* argv[]) {

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
