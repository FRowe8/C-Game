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

// ========== Data Binding Helpers ==========

void RmlUiSystem::UpdateResource(const std::string& resourceName, f64 value) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    // Format the value (simple formatting for now)
    char buffer[64];
    if (value >= 1e12) {
        snprintf(buffer, sizeof(buffer), "%.2fT", value / 1e12);
    } else if (value >= 1e9) {
        snprintf(buffer, sizeof(buffer), "%.2fB", value / 1e9);
    } else if (value >= 1e6) {
        snprintf(buffer, sizeof(buffer), "%.2fM", value / 1e6);
    } else if (value >= 1e3) {
        snprintf(buffer, sizeof(buffer), "%.2fK", value / 1e3);
    } else if (value >= 10) {
        snprintf(buffer, sizeof(buffer), "%.1f", value);
    } else {
        snprintf(buffer, sizeof(buffer), "%.2f", value);
    }

    UpdateResourceFormatted(resourceName, buffer);
#else
    (void)resourceName;
    (void)value;
#endif
}

void RmlUiSystem::UpdateResourceFormatted(const std::string& resourceName, const std::string& formattedValue) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    // Find element with data-bind attribute
    Rml::ElementList elements;
    document->GetElementsByTagName(elements, "*");

    for (Rml::Element* element : elements) {
        if (element->GetAttribute<Rml::String>("data-bind", "") == resourceName) {
            element->SetInnerRML(formattedValue);
        }
    }
#else
    (void)resourceName;
    (void)formattedValue;
#endif
}

void RmlUiSystem::SetElementText(const std::string& elementId, const std::string& text) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    Rml::Element* element = document->GetElementById(elementId);
    if (element) {
        element->SetInnerRML(text);
    }
#else
    (void)elementId;
    (void)text;
#endif
}

void RmlUiSystem::SetElementVisible(const std::string& elementId, bool visible) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    Rml::Element* element = document->GetElementById(elementId);
    if (element) {
        element->SetProperty("display", visible ? "block" : "none");
    }
#else
    (void)elementId;
    (void)visible;
#endif
}

void RmlUiSystem::ActivateView(const std::string& viewId) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    // Deactivate all view panels
    Rml::ElementList panels;
    document->GetElementsByClassName(panels, "view-panel");
    for (Rml::Element* panel : panels) {
        panel->SetClass("active", false);
    }

    // Activate the requested view
    Rml::Element* targetView = document->GetElementById(viewId);
    if (targetView) {
        targetView->SetClass("active", true);
    }

    // Update navigation button states
    Rml::ElementList navButtons;
    document->GetElementsByClassName(navButtons, "nav-btn");
    for (Rml::Element* button : navButtons) {
        std::string dataView = button->GetAttribute<Rml::String>("data-view", "");
        bool isActive = ("view-" + dataView == viewId);
        button->SetClass("active", isActive);
    }
#else
    (void)viewId;
#endif
}

