#include "RmlUiSystem.h"
#include "Logger.h"
#include "Renderer.h"
#include "GameState.h"
#include "Research.h"
#include <SDL.h>
#include <sstream>
#include <iomanip>

#ifdef RMLUI_ENABLED
#include <RmlUi/Core.h>
#include <RmlUi/Debugger.h>
#include "RmlUi_Platform_SDL.h"
#include "RmlUi_Renderer_GL3.h"

// Event listener for navigation button clicks
class NavigationEventListener : public Rml::EventListener {
public:
    explicit NavigationEventListener(RmlUiSystem* system) : m_System(system) {}

    void ProcessEvent(Rml::Event& event) override {
        if (event.GetType() == "click") {
            Rml::Element* element = event.GetTargetElement();
            if (element) {
                std::string dataView = element->GetAttribute<Rml::String>("data-view", "");
                if (!dataView.empty()) {
                    // Activate the corresponding view
                    m_System->ActivateView("view-" + dataView);
                }
            }
        }
    }

private:
    RmlUiSystem* m_System;
};

// Event listener for tooltip display
class TooltipEventListener : public Rml::EventListener {
public:
    explicit TooltipEventListener(Rml::Context* context) : m_Context(context) {}

    void ProcessEvent(Rml::Event& event) override {
        Rml::Element* element = event.GetTargetElement();
        if (!element) return;

        Rml::ElementDocument* document = m_Context->GetDocument(0);
        if (!document) return;

        Rml::Element* tooltip = document->GetElementById("global-tooltip");
        if (!tooltip) return;

        if (event.GetType() == "mouseenter" || event.GetType() == "mouseover") {
            // Get tooltip text from data attribute
            std::string tooltipText = element->GetAttribute<Rml::String>("data-tooltip", "");
            if (!tooltipText.empty()) {
                // Set tooltip content
                Rml::Element* content = tooltip->GetFirstChild();
                if (content) {
                    content->SetInnerRML(tooltipText);
                }

                // Position tooltip near mouse
                auto mousePos = event.GetParameter<Rml::Vector2i>("mouse_x", Rml::Vector2i(0, 0));
                tooltip->SetProperty("left", std::to_string(mousePos.x + 10) + "px");
                tooltip->SetProperty("top", std::to_string(mousePos.y + 10) + "px");

                // Show tooltip
                tooltip->SetClass("visible", true);
            }
        } else if (event.GetType() == "mouseleave" || event.GetType() == "mouseout") {
            // Hide tooltip
            tooltip->SetClass("visible", false);
        }
    }

private:
    Rml::Context* m_Context;
};

// Event listener for keyboard navigation
class KeyboardNavigationListener : public Rml::EventListener {
public:
    explicit KeyboardNavigationListener(RmlUiSystem* system) : m_System(system) {}

    void ProcessEvent(Rml::Event& event) override {
        if (event.GetType() == "keydown") {
            auto keyIdentifier = event.GetParameter<Rml::Input::KeyIdentifier>("key_identifier", Rml::Input::KI_UNKNOWN);

            // Number keys 1-5 for quick navigation
            switch (keyIdentifier) {
                case Rml::Input::KI_1:
                    m_System->ActivateView("view-stations");
                    break;
                case Rml::Input::KI_2:
                    m_System->ActivateView("view-research");
                    break;
                case Rml::Input::KI_3:
                    m_System->ActivateView("view-upgrades");
                    break;
                case Rml::Input::KI_4:
                    m_System->ActivateView("view-combat");
                    break;
                case Rml::Input::KI_5:
                    m_System->ActivateView("view-menu");
                    break;
                default:
                    break;
            }
        }
    }

private:
    RmlUiSystem* m_System;
};

// Event listener for station action buttons
class StationActionListener : public Rml::EventListener {
public:
    explicit StationActionListener(RmlUiSystem* system, GameState* gameState)
        : m_System(system), m_GameState(gameState) {}

    void ProcessEvent(Rml::Event& event) override {
        if (event.GetType() == "click") {
            Rml::Element* element = event.GetTargetElement();
            if (!element || !m_GameState) return;

            // Get station index from data attribute
            if (!element->HasAttribute("data-station-id")) return;

            i32 stationId = element->GetAttribute<int>("data-station-id", -1);
            std::string action = element->GetAttribute<Rml::String>("data-action", "");

            if (stationId < 0 || stationId >= static_cast<i32>(m_GameState->GetStations().size())) return;

            auto& stations = m_GameState->GetStations();
            auto& station = stations[stationId];

            if (action == "observe") {
                station.Observe(m_GameState);
                m_System->UpdateStations(m_GameState);
            } else if (action == "upgrade") {
                if (m_GameState->SpendResource(QuantumResource::Qubits, station.upgradeCost)) {
                    station.Upgrade();
                    m_System->UpdateStations(m_GameState);
                    m_System->ShowToast("Upgraded!",
                        station.name + " upgraded to level " + std::to_string(station.level),
                        RmlUiSystem::ToastType::Success);
                }
            }
        }
    }

private:
    RmlUiSystem* m_System;
    GameState* m_GameState;
};

struct RmlUiSystem::RmlUiBackend {
    Rml::Context* context = nullptr;
    bool debuggerInitialized = false;
    Scope<SystemInterface_SDL> systemInterface;
    Scope<RenderInterface_GL3> renderInterface;
    Scope<NavigationEventListener> navigationListener;
    Scope<TooltipEventListener> tooltipListener;
    Scope<KeyboardNavigationListener> keyboardListener;
    Scope<StationActionListener> stationActionListener;
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

        // Install event listeners for interactive elements
        InstallEventListeners();
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

// ========== Toast Notifications ==========

void RmlUiSystem::ShowToast(const std::string& title, const std::string& message, ToastType type, f32 duration) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    Rml::Element* container = document->GetElementById("toast-container");
    if (!container) return;

    // Determine toast class and icon based on type
    std::string toastClass = "toast";
    std::string icon = "ℹ️";

    switch (type) {
        case ToastType::Success:
            toastClass += " success";
            icon = "✅";
            break;
        case ToastType::Warning:
            toastClass += " warning";
            icon = "⚠️";
            break;
        case ToastType::Achievement:
            toastClass += " achievement";
            icon = "🏆";
            break;
        default:
            break;
    }

    // Create toast HTML structure
    std::string toastHTML =
        "<div class=\"" + toastClass + "\">"
        "  <div class=\"toast-header\">"
        "    <span class=\"toast-icon\">" + icon + "</span>"
        "    <span class=\"toast-title\">" + title + "</span>"
        "  </div>"
        "  <div class=\"toast-message\">" + message + "</div>"
        "</div>";

    // Append the toast to the container
    container->SetInnerRML(container->GetInnerRML() + toastHTML);

    // TODO: Add auto-removal after duration (would need timer system)
    // For now, toasts will stay until manually cleared or document reloads

    Log::Infof("Toast notification: ", title);
#else
    (void)title;
    (void)message;
    (void)type;
    (void)duration;
#endif
}

// ========== Game State Integration ==========

void RmlUiSystem::UpdateStations(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context || !gameState) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    // Find the stations grid container
    Rml::Element* stationsGrid = nullptr;
    Rml::ElementList elements;
    document->GetElementsByClassName(elements, "stations-grid");
    if (!elements.empty()) {
        stationsGrid = elements[0];
    }

    if (!stationsGrid) return;

    // Build station cards HTML
    std::stringstream html;
    const auto& stations = gameState->GetStations();

    for (size_t i = 0; i < stations.size(); ++i) {
        const auto& station = stations[i];

        if (!station.unlocked) continue;

        // Format numbers
        char prodBuffer[32];
        if (station.currentProduction >= 1e6) {
            snprintf(prodBuffer, sizeof(prodBuffer), "%.2fM", station.currentProduction / 1e6);
        } else if (station.currentProduction >= 1e3) {
            snprintf(prodBuffer, sizeof(prodBuffer), "%.2fK", station.currentProduction / 1e3);
        } else {
            snprintf(prodBuffer, sizeof(prodBuffer), "%.1f", station.currentProduction);
        }

        char superposBuffer[32];
        if (station.superpositionValue >= 1e6) {
            snprintf(superposBuffer, sizeof(superposBuffer), "%.2fM", station.superpositionValue / 1e6);
        } else if (station.superpositionValue >= 1e3) {
            snprintf(superposBuffer, sizeof(superposBuffer), "%.2fK", station.superpositionValue / 1e3);
        } else {
            snprintf(superposBuffer, sizeof(superposBuffer), "%.1f", station.superpositionValue);
        }

        char costBuffer[32];
        if (station.upgradeCost >= 1e6) {
            snprintf(costBuffer, sizeof(costBuffer), "%.2fM", station.upgradeCost / 1e6);
        } else if (station.upgradeCost >= 1e3) {
            snprintf(costBuffer, sizeof(costBuffer), "%.2fK", station.upgradeCost / 1e3);
        } else {
            snprintf(costBuffer, sizeof(costBuffer), "%.0f", station.upgradeCost);
        }

        html << "<div class=\"station-card\">";
        html << "  <h3 class=\"station-name\">" << station.name << " (Lv." << station.level << ")</h3>";
        html << "  <p class=\"station-desc\">" << station.description << "</p>";
        html << "  <p class=\"station-stat\">Production: " << prodBuffer << "/s</p>";
        html << "  <p class=\"station-stat\">Superposition: " << superposBuffer << "</p>";

        if (station.superpositionValue > 0) {
            html << "  <button class=\"btn-primary\" data-station-id=\"" << i << "\" data-action=\"observe\">Observe (" << static_cast<i32>(station.superpositionProbability * 100) << "%)</button>";
        }

        html << "  <button class=\"btn-primary\" data-station-id=\"" << i << "\" data-action=\"upgrade\">Upgrade (Cost: " << costBuffer << ")</button>";
        html << "</div>";
    }

    // Update the stations grid
    stationsGrid->SetInnerRML(html.str());

    // Reattach event listeners to new buttons
    if (m_Backend->stationActionListener) {
        Rml::ElementList buttons;
        stationsGrid->GetElementsByTagName(buttons, "button");
        for (Rml::Element* button : buttons) {
            if (button->HasAttribute("data-station-id")) {
                button->AddEventListener(Rml::EventId::Click, m_Backend->stationActionListener.get());
            }
        }
    }
#else
    (void)gameState;
#endif
}

void RmlUiSystem::UpdateResearch(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context || !gameState) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    // Find the research grid container
    Rml::Element* researchGrid = nullptr;
    Rml::ElementList elements;
    document->GetElementsByClassName(elements, "research-grid");
    if (!elements.empty()) {
        researchGrid = elements[0];
    }

    if (!researchGrid) return;

    // Build research nodes HTML
    std::stringstream html;
    auto& researchTree = gameState->GetResearchTree();

    // Get available and researched nodes
    auto researchedNodes = researchTree.GetResearchedNodes();
    auto availableNodes = researchTree.GetAvailableResearch(gameState->GetTimeline().completedResets);

    // Show researched nodes first
    for (const auto* node : researchedNodes) {
        html << "<div class=\"research-node unlocked\">";
        html << "  <h3 class=\"research-name\">" << node->name << "</h3>";
        html << "  <p class=\"research-desc\">" << node->description << "</p>";
        html << "  <p class=\"research-status\">✓ Researched</p>";
        html << "</div>";
    }

    // Then show available research
    for (const auto* node : availableNodes) {
        if (node->researched) continue; // Already shown above

        char costBuffer[64];
        snprintf(costBuffer, sizeof(costBuffer), "Cost: %.0f Qubits", node->qubitCost);

        html << "<div class=\"research-node\">";
        html << "  <h3 class=\"research-name\">" << node->name << "</h3>";
        html << "  <p class=\"research-desc\">" << node->description << "</p>";
        html << "  <p class=\"research-cost\">" << costBuffer << "</p>";

        // Add purchase button
        html << "  <button class=\"btn-primary\" data-research-id=\"" << static_cast<i32>(node->id) << "\" data-action=\"research\">Research</button>";
        html << "</div>";
    }

    // Update the research grid
    researchGrid->SetInnerRML(html.str());

    // TODO: Attach event listeners for research buttons
#else
    (void)gameState;
#endif
}

// ========== Event Listener Installation ==========

#ifdef RMLUI_ENABLED
void RmlUiSystem::InstallEventListeners() {
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    // Create navigation event listener
    m_Backend->navigationListener = CreateScope<NavigationEventListener>(this);

    // Attach to all navigation buttons
    Rml::ElementList navButtons;
    document->GetElementsByClassName(navButtons, "nav-btn");

    for (Rml::Element* button : navButtons) {
        button->AddEventListener(Rml::EventId::Click, m_Backend->navigationListener.get());
    }

    Log::Infof("Installed click listeners on ", navButtons.size(), " navigation buttons");

    // Create tooltip event listener
    m_Backend->tooltipListener = CreateScope<TooltipEventListener>(m_Backend->context);

    // Attach to all elements with data-tooltip attribute
    Rml::ElementList allElements;
    document->GetElementsByTagName(allElements, "*");

    i32 tooltipCount = 0;
    for (Rml::Element* element : allElements) {
        if (element->HasAttribute("data-tooltip")) {
            element->AddEventListener(Rml::EventId::Mouseover, m_Backend->tooltipListener.get());
            element->AddEventListener(Rml::EventId::Mouseout, m_Backend->tooltipListener.get());
            tooltipCount++;
        }
    }

    Log::Infof("Installed tooltip listeners on ", tooltipCount, " elements");

    // Create keyboard navigation listener
    m_Backend->keyboardListener = CreateScope<KeyboardNavigationListener>(this);

    // Attach to document for global keyboard shortcuts
    document->AddEventListener(Rml::EventId::Keydown, m_Backend->keyboardListener.get());

    Log::Info("Installed keyboard navigation shortcuts (1-5 for views)");

    // Create station action listener (will be attached when stations are created)
    if (m_GameState) {
        m_Backend->stationActionListener = CreateScope<StationActionListener>(this, m_GameState);
        Log::Info("Station action listener created");
    }
}
#endif

