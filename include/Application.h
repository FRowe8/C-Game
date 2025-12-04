#pragma once

#include "Types.h"
#include <string>

struct SDL_Window;
typedef void* SDL_GLContext;

class Renderer;
class Input;
class GameState;
class RmlUiSystem;

struct ApplicationConfig {
    std::string title = "Quantum Idle";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;
    bool vsync = true;
    int targetFPS = 60;
};

class Application {
public:
    Application(const ApplicationConfig& config);
    ~Application();

    bool Initialize();
    void Run();
    void RunFrame(); // Single frame execution for Emscripten
    void Shutdown();

    void Quit() { m_Running = false; }
    bool IsRunning() const { return m_Running; }

    Renderer* GetRenderer() { return m_Renderer.get(); }
    Input* GetInput() { return m_Input.get(); }
    GameState* GetGameState() { return m_GameState.get(); }

    f64 GetDeltaTime() const { return m_DeltaTime; }
    f64 GetTime() const { return m_Time; }
    u64 GetFrameCount() const { return m_FrameCount; }

    int GetWindowWidth() const { return m_Config.windowWidth; }
    int GetWindowHeight() const { return m_Config.windowHeight; }

private:
    void ProcessEvents();
    void Update(f64 deltaTime);
    void Render();

    ApplicationConfig m_Config;
    SDL_Window* m_Window = nullptr;
    SDL_GLContext m_GLContext = nullptr;

    Scope<Renderer> m_Renderer;
    Scope<Input> m_Input;
    Scope<GameState> m_GameState;
    Scope<RmlUiSystem> m_RmlUi;

    bool m_Running = false;
    bool m_Initialized = false;

    f64 m_DeltaTime = 0.0;
    f64 m_Time = 0.0;
    u64 m_FrameCount = 0;
    u64 m_LastFrameTime = 0;
};
