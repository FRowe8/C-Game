#include "Application.h"
#include "Renderer.h"
#include "Input.h"
#include "GameState.h"
#include "Logger.h"
#include "Platform.h"
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

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        Log::Errorf("Failed to initialize SDL: ", SDL_GetError());
        return false;
    }

    // Set OpenGL attributes
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

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

    // Set VSync
    SDL_GL_SetSwapInterval(m_Config.vsync ? 1 : 0);

    // Initialize subsystems
    m_Renderer = CreateScope<Renderer>();
    if (!m_Renderer->Initialize(m_Config.windowWidth, m_Config.windowHeight)) {
        Log::Error("Failed to initialize renderer");
        return false;
    }

    m_Input = CreateScope<Input>();

    m_GameState = CreateScope<GameState>();
    m_GameState->Initialize();

    m_Initialized = true;
    m_Running = true;
    m_LastFrameTime = Platform::GetTicks();

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
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            m_Running = false;
        }

        // Let input system process the event
        m_Input->ProcessEvent(event);
    }

    // Update input state
    m_Input->Update();
}

void Application::Update(f64 deltaTime) {
    // Update game state
    m_GameState->Update(deltaTime, m_Input.get(), m_Renderer.get());

    // Update particles
    m_Renderer->UpdateParticles(deltaTime);
}

void Application::Render() {
    m_Renderer->BeginFrame();
    m_Renderer->Clear(Color(0.05f, 0.05f, 0.1f, 1.0f)); // Dark blue background

    // Render game
    m_GameState->Render(m_Renderer.get());

    // Render particles
    m_Renderer->RenderParticles();

    m_Renderer->EndFrame();

    // Swap buffers
    SDL_GL_SwapWindow(m_Window);
}

void Application::Shutdown() {
    if (!m_Initialized) return;

    Log::Info("Shutting down application...");

    m_GameState.reset();
    m_Input.reset();
    m_Renderer.reset();

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
