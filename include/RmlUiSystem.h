#pragma once

#include "Types.h"
#include <SDL_events.h>
#include <string>

class Renderer;
struct SDL_Window;

// Runtime coordinator for the RmlUi HUD.
// When RMLUI_ENABLED is not defined, this behaves as a no-op stub so builds remain functional
// until the library is linked and documents are authored.
class RmlUiSystem {
public:
    RmlUiSystem();
    ~RmlUiSystem();

    bool Initialize(SDL_Window* window, Renderer* renderer);
    void Shutdown();

    void BeginFrame();
    void Render();

    bool ProcessEvent(const SDL_Event& event);
    void OnResize(int width, int height);

    bool IsInitialized() const { return m_Initialized; }

    // Data binding helpers - update UI elements with game state
    void UpdateResource(const std::string& resourceName, f64 value);
    void UpdateResourceFormatted(const std::string& resourceName, const std::string& formattedValue);
    void SetElementText(const std::string& elementId, const std::string& text);
    void SetElementVisible(const std::string& elementId, bool visible);
    void ActivateView(const std::string& viewId);

    // Toast notification system
    enum class ToastType {
        Info,
        Success,
        Warning,
        Achievement
    };
    void ShowToast(const std::string& title, const std::string& message, ToastType type = ToastType::Info, f32 duration = 4.0f);

    // Game state integration
    void UpdateStations(class GameState* gameState);
    void UpdateResearch(class GameState* gameState);
    void UpdateBuyables(class GameState* gameState);
    void UpdateCombat(class GameState* gameState);
    void UpdateMenu(class GameState* gameState);
    void SetGameState(class GameState* gameState) { m_GameState = gameState; }

private:
    bool m_Initialized = false;
    SDL_Window* m_Window = nullptr;
    class GameState* m_GameState = nullptr;

#ifdef RMLUI_ENABLED
    // Forward declarations to avoid leaking RmlUi headers broadly.
    struct RmlUiBackend;
    Scope<RmlUiBackend> m_Backend;

    // Internal helpers
    void InstallEventListeners();
#endif
};

