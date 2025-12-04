#pragma once

#include "Types.h"
#include <SDL_events.h>

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

private:
    bool m_Initialized = false;
    SDL_Window* m_Window = nullptr;

#ifdef RMLUI_ENABLED
    // Forward declarations to avoid leaking RmlUi headers broadly.
    struct RmlUiBackend;
    Scope<RmlUiBackend> m_Backend;
#endif
};

