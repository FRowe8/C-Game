#include "RmlUiSystem.h"
#include "Logger.h"
#include "Renderer.h"
#include <SDL.h>

#ifdef RMLUI_ENABLED
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>

// Include RmlUi backend headers
#if defined(__EMSCRIPTEN__)
    // For web builds with Emscripten
    #include <RmlUi_Backend.h>
#else
    // For desktop builds
    #include <RmlUi_Backend.h>
#endif

struct RmlUiSystem::RmlUiBackend {
    Rml::Context* context = nullptr;
    bool debuggerInitialized = false;
};
#endif

RmlUiSystem::RmlUiSystem() = default;
RmlUiSystem::~RmlUiSystem() { Shutdown(); }

bool RmlUiSystem::Initialize(SDL_Window* window, Renderer* renderer) {
    m_Window = window;
    (void)renderer;

#ifdef RMLUI_ENABLED
    Log::Info("Initializing RmlUi runtime");

    // Create backend
    m_Backend = CreateScope<RmlUiBackend>();

    // Initialize the backend - this sets up the system and render interfaces
    if (!Backend::Initialize(window)) {
        Log::Error("Failed to initialize RmlUi backend");
        return false;
    }

    // Get window dimensions
    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    // Create the main context
    m_Backend->context = Rml::CreateContext("main", Rml::Vector2i(width, height));
    if (!m_Backend->context) {
        Log::Error("Failed to create RmlUi context");
        Backend::Shutdown();
        return false;
    }

    // Initialize debugger
    if (Rml::Debugger::Initialise(m_Backend->context)) {
        m_Backend->debuggerInitialized = true;
        Log::Info("RmlUi debugger initialized (press F8 to toggle)");
    }

    // Load fonts (if any custom fonts are needed, add them here)
    // Rml::LoadFontFace("assets/fonts/Inter-Regular.ttf");

    // Load the initial HUD document
    if (Rml::ElementDocument* document = m_Backend->context->LoadDocument("assets/ui/rml/hud.rml")) {
        document->Show();
        Log::Info("RmlUi HUD document loaded successfully");
    } else {
        Log::Warn("Failed to load HUD document, but continuing initialization");
    }

    m_Initialized = true;
    Log::Info("RmlUi initialized successfully");
    return true;
#else
    Log::Info("RmlUiSystem stub active (RMLUI_ENABLED not set); skipping initialization");
    m_Initialized = false;
    return true;
#endif
}

void RmlUiSystem::Shutdown() {
#ifdef RMLUI_ENABLED
    if (m_Backend) {
        if (m_Backend->debuggerInitialized) {
            Rml::Debugger::Shutdown();
        }
        if (m_Backend->context) {
            m_Backend->context->RemoveReference();
            m_Backend->context = nullptr;
        }
        m_Backend.reset();
    }

    Backend::Shutdown();

    Log::Info("RmlUi shutdown complete");
#endif
    m_Initialized = false;
    m_Window = nullptr;
}

void RmlUiSystem::BeginFrame() {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    // Begin frame - prepares the backend for rendering
    Backend::BeginFrame();

    // Update context - this processes animations, transitions, etc.
    m_Backend->context->Update();
#endif
}

void RmlUiSystem::Render() {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    // Render all documents in the context
    m_Backend->context->Render();

    // Present the render buffer
    Backend::PresentRenderBuffer();
#endif
}

bool RmlUiSystem::ProcessEvent(const SDL_Event& event) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return false;

    // Handle window resize
    if (event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                                          event.window.event == SDL_WINDOWEVENT_RESIZED)) {
        OnResize(event.window.data1, event.window.data2);
    }

    // Route SDL events to RmlUi backend
    return Backend::ProcessSDLEvent(event, m_Backend->context);
#else
    (void)event;
    return false;
#endif
}

void RmlUiSystem::OnResize(int width, int height) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    // Update context dimensions
    m_Backend->context->SetDimensions(Rml::Vector2i(width, height));
    Log::Info("RmlUi context resized to {}x{}", width, height);
#else
    (void)width;
    (void)height;
#endif
}

