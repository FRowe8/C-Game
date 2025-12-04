#include "RmlUiSystem.h"
#include "Logger.h"
#include "Renderer.h"
#include <SDL.h>

#ifdef RMLUI_ENABLED
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_GL3.h"

struct RmlUiSystem::RmlUiBackend {
    Rml::Context* context = nullptr;
    bool debuggerInitialized = false;
    Scope<SystemInterface_SDL> systemInterface;
    Scope<RenderInterface_GL3> renderInterface;
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

    // Create and install the system interface
    m_Backend->systemInterface = CreateScope<SystemInterface_SDL>();
    m_Backend->systemInterface->SetWindow(window);
    Rml::SetSystemInterface(m_Backend->systemInterface.get());

    // Create and install the render interface
    m_Backend->renderInterface = CreateScope<RenderInterface_GL3>();
    if (!*m_Backend->renderInterface) {
        Log::Error("Failed to create RmlUi GL3 renderer");
        return false;
    }
    Rml::SetRenderInterface(m_Backend->renderInterface.get());

    // Initialize RmlUi
    if (!Rml::Initialise()) {
        Log::Error("Failed to initialize RmlUi");
        return false;
    }

    // Get window dimensions and set viewport
    int width, height;
    SDL_GetWindowSize(window, &width, &height);
    m_Backend->renderInterface->SetViewport(width, height);

    // Create the main context
    m_Backend->context = Rml::CreateContext("main", Rml::Vector2i(width, height));
    if (!m_Backend->context) {
        Log::Error("Failed to create RmlUi context");
        Rml::Shutdown();
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
        Log::Warning("Failed to load HUD document, but continuing initialization");
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
        // Context will be cleaned up by Rml::Shutdown()
        m_Backend->context = nullptr;

        Rml::Shutdown();

        m_Backend->renderInterface.reset();
        m_Backend->systemInterface.reset();
        m_Backend.reset();
    }

    Log::Info("RmlUi shutdown complete");
#endif
    m_Initialized = false;
    m_Window = nullptr;
}

void RmlUiSystem::BeginFrame() {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    // Update context - this processes animations, transitions, etc.
    m_Backend->context->Update();
#endif
}

void RmlUiSystem::Render() {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    // Prepare OpenGL state for RmlUi
    m_Backend->renderInterface->BeginFrame();

    // Render all documents in the context
    m_Backend->context->Render();

    // Restore OpenGL state after RmlUi
    m_Backend->renderInterface->EndFrame();
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

    // Route SDL events to RmlUi using the SDL input handler
    // Need to make a mutable copy since RmlSDL::InputEventHandler takes non-const reference
    SDL_Event mutableEvent = event;
    return RmlSDL::InputEventHandler(m_Backend->context, mutableEvent);
#else
    (void)event;
    return false;
#endif
}

void RmlUiSystem::OnResize(int width, int height) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    // Update renderer viewport
    m_Backend->renderInterface->SetViewport(width, height);

    // Update context dimensions
    m_Backend->context->SetDimensions(Rml::Vector2i(width, height));

    Log::Infof("RmlUi context resized to ", width, "x", height);
#else
    (void)width;
    (void)height;
#endif
}

