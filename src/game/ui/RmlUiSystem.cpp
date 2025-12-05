#include "RmlUiSystem.h"
#include "Logger.h"
#include "Renderer.h"
#include "GameState.h"
#include "Research.h"
#include "Buyables.h"
#include "CombatSystem.h"
#include "Enemy.h"
#include "Spaceship.h"
#include <SDL.h>
#include <sstream>
#include <iomanip>
#include <algorithm>

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

// Event listener for research purchase buttons
class ResearchActionListener : public Rml::EventListener {
public:
    explicit ResearchActionListener(RmlUiSystem* system, GameState* gameState)
        : m_System(system), m_GameState(gameState) {}

    void ProcessEvent(Rml::Event& event) override {
        if (event.GetType() == "click") {
            Rml::Element* element = event.GetTargetElement();
            if (!element || !m_GameState) return;

            // Get research ID from data attribute
            if (!element->HasAttribute("data-research-id")) return;

            i32 researchIdInt = element->GetAttribute<int>("data-research-id", -1);
            std::string action = element->GetAttribute<Rml::String>("data-action", "");

            if (researchIdInt < 0 || action != "research") return;

            ResearchID researchId = static_cast<ResearchID>(researchIdInt);
            auto& researchTree = m_GameState->GetResearchTree();

            // Attempt to purchase research
            if (researchTree.Research(researchId)) {
                auto* node = researchTree.GetNode(researchId);
                if (node) {
                    m_System->ShowToast("Research Complete!",
                        node->name + " has been researched",
                        RmlUiSystem::ToastType::Achievement);
                }
                // Refresh both research and stations (bonuses may have changed)
                m_System->UpdateResearch(m_GameState);
                m_System->UpdateStations(m_GameState);
            } else {
                m_System->ShowToast("Cannot Research",
                    "Insufficient resources or prerequisites not met",
                    RmlUiSystem::ToastType::Warning);
            }
        }
    }

private:
    RmlUiSystem* m_System;
    GameState* m_GameState;
};

// Event listener for buyable purchase buttons
class BuyableActionListener : public Rml::EventListener {
public:
    explicit BuyableActionListener(RmlUiSystem* system, GameState* gameState)
        : m_System(system), m_GameState(gameState) {}

    void ProcessEvent(Rml::Event& event) override {
        if (event.GetType() == "click") {
            Rml::Element* element = event.GetTargetElement();
            if (!element || !m_GameState) return;

            // Get buyable ID from data attribute
            if (!element->HasAttribute("data-buyable-id")) return;

            std::string buyableId = element->GetAttribute<Rml::String>("data-buyable-id", "");
            std::string action = element->GetAttribute<Rml::String>("data-action", "");

            if (buyableId.empty() || action != "buyable") return;

            auto& buyableManager = m_GameState->GetBuyableManager();

            // Attempt to purchase buyable
            if (buyableManager.Purchase(buyableId, m_GameState)) {
                // Find the buyable to get its name for the toast
                const auto& buyables = buyableManager.GetBuyables();
                auto it = std::find_if(buyables.begin(), buyables.end(),
                    [&buyableId](const BuyableUpgrade& b) { return b.id == buyableId; });

                if (it != buyables.end()) {
                    m_System->ShowToast("Purchase Successful!",
                        it->name + " purchased",
                        RmlUiSystem::ToastType::Success);
                }
                // Refresh the buyables view
                m_System->UpdateBuyables(m_GameState);
            } else {
                m_System->ShowToast("Cannot Purchase",
                    "Insufficient resources",
                    RmlUiSystem::ToastType::Warning);
            }
        }
    }

private:
    RmlUiSystem* m_System;
    GameState* m_GameState;
};

// Event listener for combat action buttons
class CombatActionListener : public Rml::EventListener {
public:
    explicit CombatActionListener(RmlUiSystem* system, GameState* gameState)
        : m_System(system), m_GameState(gameState) {}

    void ProcessEvent(Rml::Event& event) override {
        if (event.GetType() == "click") {
            Rml::Element* element = event.GetTargetElement();
            if (!element || !m_GameState) return;

            std::string action = element->GetAttribute<Rml::String>("data-action", "");
            if (action.empty()) return;

            auto& combat = m_GameState->GetCombatSystem();

            if (action == "combat-attack") {
                combat.PlayerAttack();
                m_System->UpdateCombat(m_GameState);
            } else if (action == "combat-defend") {
                combat.PlayerDefend();
                m_System->UpdateCombat(m_GameState);
            } else if (action == "combat-special") {
                combat.PlayerSpecialAttack();
                m_System->UpdateCombat(m_GameState);
            } else if (action == "start-combat") {
                // Start combat with a test enemy (level 1 Scout)
                // In a real game, this would come from zone/encounter selection
                Enemy* testEnemy = new Enemy(EnemyType::Scout, EnemyTier::Tier1, 1);
                combat.StartCombat(testEnemy, m_GameState->GetPlayerLevel(),
                    &m_GameState->GetSpaceship(), nullptr, m_GameState);
                m_System->ShowToast("Combat Started!",
                    std::string("Engaging ") + testEnemy->GetName(),
                    RmlUiSystem::ToastType::Info);
                m_System->UpdateCombat(m_GameState);
            } else if (action == "end-combat") {
                combat.EndCombat();
                m_System->UpdateCombat(m_GameState);
            } else if (action == "toggle-auto") {
                combat.SetAutoBattle(!combat.IsAutoBattle());
                m_System->UpdateCombat(m_GameState);
            } else if (action == "battle-speed") {
                // Cycle through speeds: 1x -> 2x -> 4x -> 1x
                f32 currentSpeed = combat.GetBattleSpeed();
                f32 newSpeed = currentSpeed >= 4.0f ? 1.0f : currentSpeed * 2.0f;
                combat.SetBattleSpeed(newSpeed);
                m_System->UpdateCombat(m_GameState);
            }
        }
    }

private:
    RmlUiSystem* m_System;
    GameState* m_GameState;
};

// Event listener for menu action buttons
class MenuActionListener : public Rml::EventListener {
public:
    explicit MenuActionListener(RmlUiSystem* system, GameState* gameState)
        : m_System(system), m_GameState(gameState) {}

    void ProcessEvent(Rml::Event& event) override {
        if (event.GetType() == "click") {
            Rml::Element* element = event.GetTargetElement();
            if (!element || !m_GameState) return;

            std::string action = element->GetAttribute<Rml::String>("data-action", "");
            if (action.empty()) return;

            if (action == "save-game") {
                if (m_GameState->Save("savegame.dat")) {
                    m_System->ShowToast("Game Saved", "Progress saved successfully", RmlUiSystem::ToastType::Success);
                } else {
                    m_System->ShowToast("Save Failed", "Could not save game", RmlUiSystem::ToastType::Warning);
                }
            } else if (action == "load-game") {
                if (m_GameState->Load("savegame.dat")) {
                    m_System->ShowToast("Game Loaded", "Save file loaded successfully", RmlUiSystem::ToastType::Success);
                    // Refresh all views with loaded data
                    m_System->UpdateStations(m_GameState);
                    m_System->UpdateResearch(m_GameState);
                    m_System->UpdateBuyables(m_GameState);
                    m_System->UpdateCombat(m_GameState);
                    m_System->UpdateMenu(m_GameState);
                } else {
                    m_System->ShowToast("Load Failed", "No save file found", RmlUiSystem::ToastType::Warning);
                }
            } else if (action == "export-save") {
                m_System->ShowToast("Export Save", "Feature coming soon", RmlUiSystem::ToastType::Info);
            } else if (action == "import-save") {
                m_System->ShowToast("Import Save", "Feature coming soon", RmlUiSystem::ToastType::Info);
            } else if (action == "prestige-reset") {
                // TODO: Add confirmation dialog
                m_GameState->PerformPrestige();
                m_System->ShowToast("Prestige Reset!", "Starting fresh with bonuses", RmlUiSystem::ToastType::Achievement);
                // Refresh all views
                m_System->UpdateStations(m_GameState);
                m_System->UpdateResearch(m_GameState);
                m_System->UpdateBuyables(m_GameState);
                m_System->UpdateCombat(m_GameState);
                m_System->UpdateMenu(m_GameState);
            } else if (action == "toggle-notifications") {
                // Toggle setting (placeholder for now)
                m_System->UpdateMenu(m_GameState);
            } else if (action == "toggle-autosave") {
                // Toggle setting (placeholder for now)
                m_System->UpdateMenu(m_GameState);
            } else if (action == "toggle-tooltips") {
                // Toggle setting (placeholder for now)
                m_System->UpdateMenu(m_GameState);
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
    Scope<ResearchActionListener> researchActionListener;
    Scope<BuyableActionListener> buyableActionListener;
    Scope<CombatActionListener> combatActionListener;
    Scope<MenuActionListener> menuActionListener;
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
    m_HudDocument = m_Backend->context->LoadDocument("assets/ui/rml/hud.rml");
    if (m_HudDocument) {
        m_HudDocument->Show();
        Log::Info("RmlUi HUD document loaded successfully");

        // Install event listeners for interactive elements
        InstallEventListeners();
    } else {
        Log::Warning("Failed to load HUD document, but continuing initialization");
    }

    // Load overlay/panel document
    m_PanelDocument = m_Backend->context->LoadDocument("assets/ui/rml/panels.rml");
    if (m_PanelDocument) {
        m_PanelDocument->Show();
        Log::Info("RmlUi panel document loaded successfully");
    } else {
        Log::Warning("Failed to load panel document");
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

void RmlUiSystem::UpdateUIResource(const std::string& resourceName, f64 value) {
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

    UpdateUIResourceFormatted(resourceName, buffer);
#else
    (void)resourceName;
    (void)value;
#endif
}

void RmlUiSystem::UpdateUIResourceFormatted(const std::string& resourceName, const std::string& formattedValue) {
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

void RmlUiSystem::UpdateUIProgress(const std::string& elementId, f32 ratio) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context) return;

    ratio = std::clamp(ratio, 0.0f, 1.0f);

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    Rml::Element* element = document->GetElementById(elementId);
    if (element) {
        int percent = static_cast<int>(ratio * 100.0f);
        element->SetProperty("width", std::to_string(percent) + "%");
    }
#else
    (void)elementId;
    (void)ratio;
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

    // Attach event listeners to research buttons
    if (m_Backend->researchActionListener) {
        Rml::ElementList buttons;
        researchGrid->GetElementsByTagName(buttons, "button");
        for (Rml::Element* button : buttons) {
            if (button->HasAttribute("data-research-id")) {
                button->AddEventListener(Rml::EventId::Click, m_Backend->researchActionListener.get());
            }
        }
    }
#else
    (void)gameState;
#endif
}

void RmlUiSystem::UpdateBuyables(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context || !gameState) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    // Find the buyables grid container
    Rml::Element* buyablesGrid = nullptr;
    Rml::ElementList elements;
    document->GetElementsByClassName(elements, "buyables-grid");
    if (!elements.empty()) {
        buyablesGrid = elements[0];
    }

    if (!buyablesGrid) return;

    // Build buyable cards HTML
    std::stringstream html;
    auto& buyableManager = gameState->GetBuyableManager();
    const auto& buyables = buyableManager.GetBuyables();
    f64 currentQubits = gameState->GetResource(QuantumResource::Qubits);

    for (const auto& buyable : buyables) {
        // Determine if maxed or can afford
        bool isMaxed = buyable.IsMaxed();
        bool canAfford = !isMaxed && buyable.CanAfford(currentQubits);

        std::string cardClass = isMaxed ? "buyable-card maxed" : "buyable-card";

        html << "<div class=\"" << cardClass << "\">";
        html << "  <h3 class=\"buyable-name\">" << buyable.name << "</h3>";
        html << "  <p class=\"buyable-desc\">" << buyable.description << "</p>";

        if (isMaxed) {
            html << "  <div class=\"buyable-maxed\">✓ Maxed Out</div>";
        } else {
            // Show stats (cost and progress)
            f64 cost = buyable.GetCurrentCost();
            char costBuffer[64];
            if (cost >= 1e9) {
                snprintf(costBuffer, sizeof(costBuffer), "%.2fB", cost / 1e9);
            } else if (cost >= 1e6) {
                snprintf(costBuffer, sizeof(costBuffer), "%.2fM", cost / 1e6);
            } else if (cost >= 1e3) {
                snprintf(costBuffer, sizeof(costBuffer), "%.2fK", cost / 1e3);
            } else {
                snprintf(costBuffer, sizeof(costBuffer), "%.0f", cost);
            }

            html << "  <div class=\"buyable-stats\">";
            html << "    <span class=\"buyable-cost\">Cost: " << costBuffer << " Qubits</span>";
            html << "    <span class=\"buyable-progress\">" << buyable.GetProgressString() << "</span>";
            html << "  </div>";

            // Add purchase button
            html << "  <button class=\"btn-primary\" data-buyable-id=\"" << buyable.id << "\" data-action=\"buyable\">Purchase</button>";
        }

        html << "</div>";
    }

    // Update the buyables grid
    buyablesGrid->SetInnerRML(html.str());

    // Attach event listeners for buyable purchase buttons
    if (m_Backend->buyableActionListener) {
        Rml::ElementList buttons;
        buyablesGrid->GetElementsByTagName(buttons, "button");

        i32 attachedCount = 0;
        for (Rml::Element* button : buttons) {
            if (button->GetAttribute<Rml::String>("data-action", "") == "buyable") {
                button->AddEventListener(Rml::EventId::Click, m_Backend->buyableActionListener.get());
                attachedCount++;
            }
        }

        if (attachedCount > 0) {
            Log::Debugf("Attached buyable action listeners to ", attachedCount, " buttons");
        }
    }
#else
    (void)gameState;
#endif
}

void RmlUiSystem::UpdateCombat(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context || !gameState) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    auto& combat = gameState->GetCombatSystem();
    bool inCombat = combat.IsInCombat();
    Enemy* enemy = combat.GetCurrentEnemy();

    // Helper lambda to update text content
    auto updateText = [&](const std::string& bindAttr, const std::string& value) {
        Rml::ElementList elements;
        document->GetElementsByTagName(elements, "*");
        for (Rml::Element* elem : elements) {
            if (elem->GetAttribute<Rml::String>("data-bind", "") == bindAttr) {
                elem->SetInnerRML(value);
            }
        }
    };

    // Update player stats
    updateText("player-level", std::to_string(gameState->GetPlayerLevel()));
    updateText("player-hp", std::to_string(static_cast<i32>(combat.GetPlayerCurrentHP())));
    updateText("player-max-hp", std::to_string(static_cast<i32>(combat.GetPlayerMaxHP())));
    updateText("player-attack", std::to_string(static_cast<i32>(combat.GetPlayerAttack())));
    updateText("player-defense", std::to_string(static_cast<i32>(combat.GetPlayerDefense())));
    updateText("player-speed", std::to_string(static_cast<i32>(combat.GetPlayerSpeed())));

    // Update player HP bar
    f64 playerHPPercent = combat.GetPlayerMaxHP() > 0 ?
        (combat.GetPlayerCurrentHP() / combat.GetPlayerMaxHP()) * 100.0 : 0.0;
    Rml::ElementList playerHPBars;
    document->GetElementsByClassName(playerHPBars, "player-hp");
    for (Rml::Element* bar : playerHPBars) {
        char styleBuffer[32];
        snprintf(styleBuffer, sizeof(styleBuffer), "width: %.1f%%;", playerHPPercent);
        bar->SetAttribute("style", styleBuffer);
    }

    // Update enemy stats
    if (inCombat && enemy) {
        updateText("enemy-name", enemy->GetName());
        updateText("enemy-level", std::to_string(enemy->GetLevel()));
        updateText("enemy-hp", std::to_string(static_cast<i32>(enemy->GetCurrentHealth())));
        updateText("enemy-max-hp", std::to_string(static_cast<i32>(enemy->GetMaxHealth())));
        updateText("enemy-attack", std::to_string(static_cast<i32>(enemy->GetAttack())));
        updateText("enemy-defense", std::to_string(static_cast<i32>(enemy->GetDefense())));
        updateText("enemy-speed", std::to_string(static_cast<i32>(enemy->GetSpeed())));

        // Update enemy HP bar
        f64 enemyHPPercent = enemy->GetHealthPercent();
        Rml::ElementList enemyHPBars;
        document->GetElementsByClassName(enemyHPBars, "enemy-hp");
        for (Rml::Element* bar : enemyHPBars) {
            char styleBuffer[32];
            snprintf(styleBuffer, sizeof(styleBuffer), "width: %.1f%%;", enemyHPPercent);
            bar->SetAttribute("style", styleBuffer);
        }
    } else {
        updateText("enemy-name", "No Enemy");
        updateText("enemy-level", "-");
        updateText("enemy-hp", "-");
        updateText("enemy-max-hp", "-");
        updateText("enemy-attack", "-");
        updateText("enemy-defense", "-");
        updateText("enemy-speed", "-");
    }

    // Update combat status
    std::string statusText;
    switch (combat.GetState()) {
        case CombatState::NotInCombat:
            statusText = "Not in combat";
            break;
        case CombatState::PlayerTurn:
            statusText = "Your Turn - Choose an action!";
            break;
        case CombatState::EnemyTurn:
            statusText = "Enemy Turn...";
            break;
        case CombatState::Victory:
            statusText = "Victory! +" + std::to_string(combat.GetXPEarned()) + " XP";
            break;
        case CombatState::Defeat:
            statusText = "Defeat - Try again";
            break;
    }
    updateText("combat-status", statusText);

    // Update auto-battle and battle speed
    updateText("auto-battle", combat.IsAutoBattle() ? "ON" : "OFF");
    char speedBuffer[16];
    snprintf(speedBuffer, sizeof(speedBuffer), "%.0fx", combat.GetBattleSpeed());
    updateText("battle-speed", speedBuffer);

    // Build combat action buttons
    Rml::Element* actionsContainer = nullptr;
    Rml::ElementList actionElements;
    document->GetElementsByClassName(actionElements, "combat-actions");
    if (!actionElements.empty()) {
        actionsContainer = actionElements[0];
    }

    if (actionsContainer) {
        std::stringstream html;

        if (combat.GetState() == CombatState::PlayerTurn) {
            html << "<button class=\"btn-primary\" data-action=\"combat-attack\">⚔️ Attack</button>";
            html << "<button class=\"btn-secondary\" data-action=\"combat-defend\">🛡️ Defend</button>";
            html << "<button class=\"btn-primary\" data-action=\"combat-special\">✨ Special</button>";
        } else if (combat.GetState() == CombatState::NotInCombat) {
            html << "<button class=\"btn-primary\" data-action=\"start-combat\">Start Combat</button>";
        } else if (combat.GetState() == CombatState::Victory || combat.GetState() == CombatState::Defeat) {
            html << "<button class=\"btn-primary\" data-action=\"end-combat\">Continue</button>";
        }

        actionsContainer->SetInnerRML(html.str());

        // Attach event listeners to combat action buttons
        if (m_Backend->combatActionListener) {
            Rml::ElementList buttons;
            actionsContainer->GetElementsByTagName(buttons, "button");
            for (Rml::Element* button : buttons) {
                button->AddEventListener(Rml::EventId::Click, m_Backend->combatActionListener.get());
            }
        }
    }

    // Attach event listeners to combat control buttons
    if (m_Backend->combatActionListener) {
        Rml::ElementList controlButtons;
        document->GetElementsByClassName(controlButtons, "combat-controls");
        if (!controlButtons.empty()) {
            Rml::Element* controls = controlButtons[0];
            Rml::ElementList buttons;
            controls->GetElementsByTagName(buttons, "button");
            for (Rml::Element* button : buttons) {
                button->AddEventListener(Rml::EventId::Click, m_Backend->combatActionListener.get());
            }
        }
    }

    // Update combat log
    Rml::Element* logContainer = nullptr;
    Rml::ElementList logElements;
    document->GetElementsByClassName(logElements, "log-entries");
    if (!logElements.empty()) {
        logContainer = logElements[0];
    }

    if (logContainer) {
        std::stringstream html;
        const auto& log = combat.GetCombatLog();

        // Show last 10 entries
        size_t startIdx = log.size() > 10 ? log.size() - 10 : 0;
        for (size_t i = startIdx; i < log.size(); i++) {
            html << "<div class=\"log-entry\">" << log[i].message << "</div>";
        }

        logContainer->SetInnerRML(html.str());
    }

#else
    (void)gameState;
#endif
}

void RmlUiSystem::UpdateMenu(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_Initialized || !m_Backend || !m_Backend->context || !gameState) return;

    Rml::ElementDocument* document = m_Backend->context->GetDocument(0);
    if (!document) return;

    // Helper lambda to update text content
    auto updateText = [&](const std::string& bindAttr, const std::string& value) {
        Rml::ElementList elements;
        document->GetElementsByTagName(elements, "*");
        for (Rml::Element* elem : elements) {
            if (elem->GetAttribute<Rml::String>("data-bind", "") == bindAttr) {
                elem->SetInnerRML(value);
            }
        }
    };

    // Update prestige info
    i32 prestigeCount = gameState->GetTimeline().completedResets;
    updateText("prestige-count", std::to_string(prestigeCount));

    // Calculate next prestige gain (simplified formula)
    f64 currentQubits = gameState->GetResource(QuantumResource::Qubits);
    i32 photonGain = static_cast<i32>(std::sqrt(currentQubits / 1000.0));
    updateText("prestige-gain", "+" + std::to_string(photonGain));

    // Update statistics (using current qubits as total for now)
    char qubitsBuffer[64];
    if (currentQubits >= 1e9) {
        snprintf(qubitsBuffer, sizeof(qubitsBuffer), "%.2fB", currentQubits / 1e9);
    } else if (currentQubits >= 1e6) {
        snprintf(qubitsBuffer, sizeof(qubitsBuffer), "%.2fM", currentQubits / 1e6);
    } else if (currentQubits >= 1e3) {
        snprintf(qubitsBuffer, sizeof(qubitsBuffer), "%.2fK", currentQubits / 1e3);
    } else {
        snprintf(qubitsBuffer, sizeof(qubitsBuffer), "%.0f", currentQubits);
    }
    updateText("total-qubits", qubitsBuffer);

    // Play time placeholder (no time tracking available in GameState yet)
    updateText("play-time", "0:00:00");

    // Research completed count
    auto& researchTree = gameState->GetResearchTree();
    i32 researchCount = static_cast<i32>(researchTree.GetResearchedNodes().size());
    updateText("research-count", std::to_string(researchCount));

    // Battles won (placeholder - would come from combat stats)
    updateText("battles-won", "0");

    // Settings states (placeholder - would come from settings manager)
    updateText("notifications-state", "ON");
    updateText("autosave-state", "ON");
    updateText("tooltips-state", "ON");

    // Game version
    updateText("game-version", "1.0.0");

    // Attach event listeners to all menu buttons
    if (m_Backend->menuActionListener) {
        Rml::ElementList buttons;
        document->GetElementsByClassName(buttons, "menu-btn");
        for (Rml::Element* button : buttons) {
            button->AddEventListener(Rml::EventId::Click, m_Backend->menuActionListener.get());
        }

        // Also attach to toggle buttons
        Rml::ElementList toggleButtons;
        document->GetElementsByClassName(toggleButtons, "btn-toggle");
        for (Rml::Element* button : toggleButtons) {
            button->AddEventListener(Rml::EventId::Click, m_Backend->menuActionListener.get());
        }
    }

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

    // Create game action listeners (will be attached when elements are created)
    if (m_GameState) {
        m_Backend->stationActionListener = CreateScope<StationActionListener>(this, m_GameState);
        m_Backend->researchActionListener = CreateScope<ResearchActionListener>(this, m_GameState);
        m_Backend->buyableActionListener = CreateScope<BuyableActionListener>(this, m_GameState);
        m_Backend->combatActionListener = CreateScope<CombatActionListener>(this, m_GameState);
        m_Backend->menuActionListener = CreateScope<MenuActionListener>(this, m_GameState);
        Log::Info("Game action listeners created (stations, research, buyables, combat, menu)");
    }
}

void RmlUiSystem::UpdateAchievements(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_PanelDocument || !gameState) return;

    Rml::Element* container = m_PanelDocument->GetElementById("achievements-list");
    if (!container) return;

    std::ostringstream body;
    for (const auto& achievement : gameState->GetAchievements()) {
        f64 progressRatio = (achievement.target > 0.0) ? achievement.progress / achievement.target : 0.0;
        progressRatio = std::clamp(progressRatio, 0.0, 1.0);

        body << "<div class=\"achievement-card\">";
        body << "<div class=\"title\">" << achievement.name << "</div>";
        body << "<div class=\"desc\">" << achievement.description << "</div>";
        body << "<div class=\"status-badge" << (achievement.unlocked ? "" : " locked") << "\">";
        body << (achievement.unlocked ? "Unlocked" : "Locked") << "</div>";
        if (!achievement.unlocked) {
            body << "<div class=\"progress-bar\"><div class=\"progress-fill\" style=\"width: "
                 << static_cast<int>(progressRatio * 100.0) << "%;\"></div></div>";
        }
        body << "</div>";
    }

    container->SetInnerRML(body.str());
#else
    (void)gameState;
#endif
}

void RmlUiSystem::UpdateStatistics(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_PanelDocument || !gameState) return;

    Rml::Element* grid = m_PanelDocument->GetElementById("statistics-list");
    if (!grid) return;

    const auto& stats = gameState->GetStatistics();
    auto formatNumber = [](f64 value) {
        std::ostringstream ss;
        if (value >= 1'000'000.0) {
            ss << std::fixed << std::setprecision(2) << value / 1'000'000.0 << "M";
        } else if (value >= 1'000.0) {
            ss << std::fixed << std::setprecision(2) << value / 1'000.0 << "K";
        } else {
            ss << std::fixed << std::setprecision(0) << value;
        }
        return ss.str();
    };

    std::ostringstream body;
    auto addRow = [&body](const std::string& label, const std::string& value) {
        body << "<div class=\"stat-row\">";
        body << "<span class=\"stat-label\">" << label << "</span>";
        body << "<span class=\"stat-value\">" << value << "</span>";
        body << "</div>";
    };

    addRow("Total Qubits", formatNumber(stats.totalQubitsEarned));
    addRow("Total Coherence", formatNumber(stats.totalCoherenceEarned));
    addRow("Total Entanglement", formatNumber(stats.totalEntanglementEarned));
    addRow("Observations", std::to_string(stats.totalObservations));
    addRow("Upgrades Purchased", std::to_string(stats.totalUpgrades));
    addRow("Prestiges", std::to_string(stats.totalPrestigesPerformed));
    addRow("Session Qubits", formatNumber(stats.sessionQubits));
    addRow("Session Observations", std::to_string(stats.sessionObservations));
    addRow("Highest Qubits", formatNumber(stats.highestQubits));
    addRow("Fastest Prestige (s)", stats.fastestPrestige > 0 ? std::to_string(static_cast<int>(stats.fastestPrestige)) : "-" );

    grid->SetInnerRML(body.str());
#else
    (void)gameState;
#endif
}

void RmlUiSystem::UpdateSingularityShop(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_PanelDocument || !gameState) return;

    Rml::Element* list = m_PanelDocument->GetElementById("singularity-upgrades");
    if (!list) return;

    auto& manager = gameState->GetSingularityShopManager();
    auto& upgrades = manager.GetUpgrades();

    std::ostringstream body;
    for (const auto& upgrade : upgrades) {
        body << "<div class=\"upgrade-card\">";
        body << "<div class=\"title\">" << upgrade.name << "</div>";
        body << "<div class=\"desc\">" << upgrade.description << "</div>";
        body << "<div class=\"upgrade-meta\">";
        body << "<span>Cost: " << static_cast<int>(upgrade.GetCurrentCost()) << " S</span>";
        body << "<span>" << upgrade.GetProgressString() << "</span>";
        body << "</div>";
        body << "</div>";
    }

    list->SetInnerRML(body.str());
#else
    (void)gameState;
#endif
}

void RmlUiSystem::UpdateSpaceship(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_PanelDocument || !gameState) return;

    Rml::Element* repairBar = m_PanelDocument->GetElementById("ship-repair");
    Rml::Element* repairText = m_PanelDocument->GetElementById("ship-repair-text");
    Rml::Element* stats = m_PanelDocument->GetElementById("ship-stats");
    Rml::Element* inventory = m_PanelDocument->GetElementById("ship-inventory");
    if (!repairBar || !repairText || !stats || !inventory) return;

    auto& ship = gameState->GetSpaceship();
    f64 repair = ship.GetRepairProgress();
    repairBar->SetAttribute("style", "width: " + std::to_string(static_cast<int>(repair)) + "%;");
    repairText->SetInnerRML(std::to_string(static_cast<int>(repair)) + "%");

    std::ostringstream statsBody;
    statsBody << "<div class=\"stat-row\"><span class=\"stat-label\">Power Bonus</span><span class=\"stat-value\">+"
              << std::fixed << std::setprecision(1) << ship.GetTotalPowerBonus() << "%</span></div>";
    statsBody << "<div class=\"stat-row\"><span class=\"stat-label\">Combat Bonus</span><span class=\"stat-value\">+"
              << std::fixed << std::setprecision(1) << ship.GetTotalCombatBonus() << "%</span></div>";
    statsBody << "<div class=\"stat-row\"><span class=\"stat-label\">Drop Rate</span><span class=\"stat-value\">+"
              << std::fixed << std::setprecision(1) << ship.GetTotalDropRateBonus() << "%</span></div>";
    stats->SetInnerRML(statsBody.str());

    std::ostringstream invBody;
    for (const auto& part : ship.GetInventory()) {
        invBody << "<div class=\"inventory-card\">";
        invBody << "<div class=\"title\">" << part.name << "</div>";
        invBody << "<div class=\"meta\">" << part.GetSlotName() << " · " << part.GetRarityName() << "</div>";
        invBody << "<div class=\"desc\">" << part.description << "</div>";
        invBody << "</div>";
    }
    inventory->SetInnerRML(invBody.str());
#else
    (void)gameState;
#endif
}

void RmlUiSystem::SyncPanels(GameState* gameState) {
#ifdef RMLUI_ENABLED
    if (!m_PanelDocument || !gameState) return;

    auto setVisible = [&](const std::string& id, bool visible) {
        if (auto* element = m_PanelDocument->GetElementById(id)) {
            element->SetClass("hidden", !visible);
        }
    };

    GameMode mode = gameState->GetGameMode();
    setVisible("panel-achievements", mode == GameMode::Achievements);
    setVisible("panel-statistics", mode == GameMode::Statistics);
    setVisible("panel-singularity", mode == GameMode::SingularityShop);
    setVisible("panel-spaceship", mode == GameMode::Spaceship);
#else
    (void)gameState;
#endif
}
#endif

