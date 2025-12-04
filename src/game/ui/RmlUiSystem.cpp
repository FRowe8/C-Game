#include "RmlUiSystem.h"
#include "Logger.h"
#include "Renderer.h"
#include <SDL.h>

RmlUiSystem::RmlUiSystem() = default;
RmlUiSystem::~RmlUiSystem() { Shutdown(); }

bool RmlUiSystem::Initialize(SDL_Window* window, Renderer* renderer) {
    m_Window = window;
    (void)renderer;

#ifdef RMLUI_ENABLED
    Log::Info("Initializing RmlUi runtime (RMLUI_ENABLED)");
    // Real backend wiring will be added once the library is linked.
    m_Initialized = true;
    return true;
#else
    Log::Info("RmlUiSystem stub active (RMLUI_ENABLED not set); skipping initialization");
    m_Initialized = false;
    return true;
#endif
}

void RmlUiSystem::Shutdown() {
#ifdef RMLUI_ENABLED
    m_Backend.reset();
#endif
    m_Initialized = false;
    m_Window = nullptr;
}

void RmlUiSystem::BeginFrame() {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend) return;
    // TODO: Pump context updates and data bindings
#endif
}

void RmlUiSystem::Render() {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend) return;
    // TODO: Render active documents before ImGui
#endif
}

bool RmlUiSystem::ProcessEvent(const SDL_Event& event) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend) return false;

    if (event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                                          event.window.event == SDL_WINDOWEVENT_RESIZED)) {
        OnResize(event.window.data1, event.window.data2);
    }

    // TODO: Route mouse, keyboard, and text events into RmlUi context
    return false;
#else
    (void)event;
    return false;
#endif
}

void RmlUiSystem::OnResize(int width, int height) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend) return;
    // TODO: call context->SetDimensions and update pixel density
#else
    (void)width;
    (void)height;
#endif
}

