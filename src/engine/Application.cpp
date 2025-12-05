#include "Application.h"
#include "Renderer.h"
#include "Input.h"
#include "GameState.h"
#include "Logger.h"
#include "RmlUiSystem.h"
#include "imgui_impl_sdl2.h"  // For ImGui SDL2 event processing
#include "Platform.h"
#include "SteamIntegration.h"
#include <SDL.h>
#include <SDL_opengl.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

// Global pointer for Emscripten callback
static Application* g_AppInstance = nullptr;

// Emscripten main loop callback
void EmscriptenMainLoop() {
    if (g_AppInstance) {
        g_AppInstance->RunFrame();
    }
}
#endif

Application::Application(const ApplicationConfig& config)
    : m_Config(config) {
}

Application::~Application() {
    Shutdown();
}

bool Application::Initialize() {
    Log::Init();
    Log::Infof("Initializing ", m_Config.title, "...");
    Log::Infof("Platform: ", Platform::GetPlatformName());
    Log::Infof("CPU Count: ", Platform::GetCPUCount());

    // Initialize optional platform services early so subsystems can hook in
    SteamIntegration::Initialize();

#ifdef __EMSCRIPTEN__
    printf("=== EMSCRIPTEN BUILD - Starting initialization ===\n");

    // Mount IndexedDB filesystem for persistent saves
    printf("=== Mounting IndexedDB filesystem ===\n");
    EM_ASM(
        // Create the /idbfs directory
        FS.mkdir('/idbfs');
        // Mount IndexedDB filesystem
        FS.mount(IDBFS, {}, '/idbfs');
        // Sync from IndexedDB to memory (load existing saves)
        FS.syncfs(true, function(err) {
            if (err) {
                console.error('Error loading saves from IndexedDB:', err);
            } else {
                console.log('Successfully loaded saves from IndexedDB');
            }
        });
    );
    printf("=== IndexedDB filesystem mounted ===\n");
#endif

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        Log::Errorf("Failed to initialize SDL: ", SDL_GetError());
        return false;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

#ifdef __EMSCRIPTEN__
    // For web: Request OpenGL ES 2.0 (WebGL 1) to avoid legacy emulation
    // This prevents Emscripten from needing to emulate desktop OpenGL
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    Log::Info("Requesting OpenGL ES 2.0 context for WebGL");
#else
    // For desktop: Use OpenGL 2.1 compatibility profile
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    Log::Info("Requesting OpenGL 2.1 context for desktop");
#endif

    // Create window
    u32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (m_Config.fullscreen) {
        windowFlags |= SDL_WINDOW_FULLSCREEN;
    }

    m_Window = SDL_CreateWindow(
        m_Config.title.c_str(),
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        m_Config.windowWidth,
        m_Config.windowHeight,
        windowFlags
    );

    if (!m_Window) {
        Log::Errorf("Failed to create window: ", SDL_GetError());
        return false;
    }

    // Create OpenGL context
    m_GLContext = SDL_GL_CreateContext(m_Window);
    if (!m_GLContext) {
        Log::Errorf("Failed to create OpenGL context: ", SDL_GetError());
        return false;
    }

    // Set VSync (skip on Emscripten - browser controls timing)
#ifndef __EMSCRIPTEN__
    SDL_GL_SetSwapInterval(m_Config.vsync ? 1 : 0);
#endif

    // Initialize subsystems
    m_Renderer = CreateScope<Renderer>();
    if (!m_Renderer->Initialize(m_Window, m_GLContext, m_Config.windowWidth, m_Config.windowHeight)) {
        Log::Error("Failed to initialize renderer");
        return false;
    }

#ifdef __EMSCRIPTEN__
    printf("=== Renderer initialized: %dx%d ===\n", m_Config.windowWidth, m_Config.windowHeight);
#endif

    m_Input = CreateScope<Input>();
    // Set window size for touch coordinate scaling
    m_Input->SetWindowSize(static_cast<f32>(m_Config.windowWidth), static_cast<f32>(m_Config.windowHeight));

    m_GameState = CreateScope<GameState>();
    m_GameState->Initialize();

    m_RmlUi = CreateScope<RmlUiSystem>();
    m_RmlUi->Initialize(m_Window, m_Renderer.get());
    m_RmlUi->SetGameState(m_GameState.get());

    // Populate initial game data in UI
    m_RmlUi->UpdateStations(m_GameState.get());
    m_RmlUi->UpdateResearch(m_GameState.get());
    m_RmlUi->UpdateBuyables(m_GameState.get());
    m_RmlUi->UpdateCombat(m_GameState.get());

    m_Initialized = true;
    m_Running = true;
    m_LastFrameTime = Platform::GetTicks();

#ifdef __EMSCRIPTEN__
    printf("=== Application initialized successfully ===\n");
#endif

    Log::Info("Application initialized successfully");
    return true;
}

void Application::Run() {
    if (!m_Initialized) {
        Log::Error("Cannot run application - not initialized");
        return;
    }

    Log::Info("Starting main loop...");

#ifdef __EMSCRIPTEN__
    // For web, use Emscripten's main loop
    g_AppInstance = this;
    // 0 = Use browser's requestAnimationFrame, 1 = Simulate infinite loop
    emscripten_set_main_loop(EmscriptenMainLoop, 0, 1);
#else
    // For desktop, use traditional loop
    while (m_Running) {
        RunFrame();
    }
    Log::Info("Main loop ended");
#endif
}

void Application::RunFrame() {
    // Calculate delta time
    u64 currentTime = Platform::GetTicks();
    m_DeltaTime = (currentTime - m_LastFrameTime) / 1000.0;
    m_LastFrameTime = currentTime;

    // Cap delta time to avoid huge jumps
    if (m_DeltaTime > 0.1) {
        m_DeltaTime = 0.1;
    }

    m_Time += m_DeltaTime;
    m_FrameCount++;

#ifdef __EMSCRIPTEN__
    // Log every 60 frames (about once per second at 60fps)
    if (m_FrameCount % 60 == 1) {
        printf("=== Frame %llu, DeltaTime: %.3f, Running: %d ===\n",
               m_FrameCount, m_DeltaTime, m_Running);
    }
#endif

    ProcessEvents();
    Update(m_DeltaTime);
    Render();

    // Frame rate limiting (if not using VSync)
    if (!m_Config.vsync && m_Config.targetFPS > 0) {
        u64 frameTime = Platform::GetTicks() - currentTime;
        u64 targetFrameTime = 1000 / m_Config.targetFPS;
        if (frameTime < targetFrameTime) {
            SDL_Delay(static_cast<u32>(targetFrameTime - frameTime));
        }
    }
}

void Application::ProcessEvents() {
    // Update input state first (clears previous frame's input)
    m_Input->Update();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        // Let ImGui process the event first
        bool rmlHandled = m_RmlUi ? m_RmlUi->ProcessEvent(event) : false;
        if (!rmlHandled) {
            ImGui_ImplSDL2_ProcessEvent(&event);
        }

        if (event.type == SDL_QUIT) {
            m_Running = false;
        }

        if (event.type == SDL_WINDOWEVENT &&
            (event.window.event == SDL_WINDOWEVENT_RESIZED || event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)) {
            if (m_RmlUi) {
                m_RmlUi->OnResize(event.window.data1, event.window.data2);
            }
        }

        // Let input system process the event
        m_Input->ProcessEvent(event);
    }
}

void Application::Update(f64 deltaTime) {
    // Pump platform callbacks (Steam, etc.) before updating game logic
    SteamIntegration::RunCallbacks();

    // Update game state
    m_GameState->Update(deltaTime, m_Input.get(), m_Renderer.get());

    // Update particles
    m_Renderer->UpdateParticles(deltaTime);
}

void Application::Render() {
    m_Renderer->BeginFrame();

    if (m_RmlUi && m_RmlUi->IsInitialized()) {
        m_RmlUi->BeginFrame();

        // Update RmlUi with current game state
        m_RmlUi->UpdateResource("qubits", m_GameState->GetResource(QuantumResource::Qubits));
        m_RmlUi->UpdateResource("coherence", m_GameState->GetCoherence());
        m_RmlUi->UpdateResource("entanglement", m_GameState->GetResource(QuantumResource::Entanglement));
        m_RmlUi->UpdateResource("photons", static_cast<f64>(m_GameState->GetTimeline().photons));
        m_RmlUi->UpdateResource("singularities", static_cast<f64>(m_GameState->GetTimeline().singularities));
    }

    m_Renderer->Clear(Color::DarkBackground()); // Modern dark cyberpunk background

    // Render game
    m_GameState->Render(m_Renderer.get());

    if (m_RmlUi && m_RmlUi->IsInitialized()) {
        m_RmlUi->Render();
    }

    m_Renderer->EndFrame();

    // Swap buffers
    SDL_GL_SwapWindow(m_Window);
}

void Application::Shutdown() {
    if (!m_Initialized) return;

    Log::Info("Shutting down application...");

    // CRITICAL: Save game state before shutdown
    if (m_GameState) {
        std::string savePath = Platform::GetSaveDirectory() + "quantum_save.json";
        if (m_GameState->Save(savePath)) {
            Log::Info("Final save completed successfully");
        } else {
            Log::Error("Failed to save game state on shutdown");
        }
    }

#ifdef __EMSCRIPTEN__
    // Sync filesystem to IndexedDB before shutdown (save to browser storage)
    printf("=== Syncing saves to IndexedDB ===\n");
    EM_ASM(
        FS.syncfs(false, function(err) {
            if (err) {
                console.error('Error saving to IndexedDB:', err);
            } else {
                console.log('Successfully saved to IndexedDB');
            }
        });
    );
#endif

    m_GameState.reset();
    m_Input.reset();
    if (m_RmlUi) {
        m_RmlUi->Shutdown();
    }
    m_RmlUi.reset();
    m_Renderer.reset();

    SteamIntegration::Shutdown();

    if (m_GLContext) {
        SDL_GL_DeleteContext(m_GLContext);
        m_GLContext = nullptr;
    }

    if (m_Window) {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }

    SDL_Quit();
    Log::Shutdown();

    m_Initialized = false;
}
