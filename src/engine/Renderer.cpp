#include "Renderer.h"
#include "Logger.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <cmath>
#include <cstring>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Undefine Windows macros that conflict with our code
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

Renderer::Renderer() {
}

Renderer::~Renderer() {
    Shutdown();
}
bool Renderer::Initialize(SDL_Window* window, SDL_GLContext glContext, int width, int height) {
    m_Window = window;
    m_GLContext = glContext;
    m_Width = width;
    m_Height = height;

    Log::Info("Initializing Renderer...");
    Log::Infof("OpenGL Version: ", glGetString(GL_VERSION));
    Log::Infof("GLSL Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Set up OpenGL state (no longer relying on legacy features)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // UpdateProjectionMatrix() is no longer strictly needed for this backend,
    // but the call is benign if the function is empty.
    // We remove its implementation for cleanliness.

    // Initialize ImGui
    InitializeImGui();

    Log::Info("Renderer initialized successfully");
    return true;
}

void Renderer::Shutdown() {
    ShutdownImGui();
    // Legacy bitmap font needs no cleanup
    // Add any necessary OpenGL/SDL cleanup here if needed later
}

void Renderer::InitializeImGui() {
    Log::Info("Initializing ImGui...");

    // Create ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    // Enable keyboard and gamepad navigation
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight(); // Alternative

    // Customize style for mobile-friendly UI
    ImGuiStyle& style = ImGui::GetStyle();
    style.FramePadding = ImVec2(8, 6);        // Bigger padding for touch
    style.ItemSpacing = ImVec2(12, 8);        // More spacing between items
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.TouchExtraPadding = ImVec2(4, 4);   // Extra padding for touch
    style.ScrollbarSize = 18.0f;              // Bigger scrollbars for touch
    style.GrabMinSize = 14.0f;                // Bigger grab handles

    // Setup platform/renderer backends
    // Use OpenGL 2.1 / ES 2.0 for maximum compatibility
    const char* glsl_version = "#version 100";  // OpenGL ES 2.0 / WebGL 1.0

#ifdef PLATFORM_WEB
    glsl_version = "#version 100";  // WebGL 1.0
#elif defined(PLATFORM_ANDROID) || defined(PLATFORM_IOS)
    glsl_version = "#version 100";  // OpenGL ES 2.0
#else
    glsl_version = "#version 130";  // OpenGL 3.0+ / Desktop
#endif

    ImGui_ImplSDL2_InitForOpenGL(m_Window, m_GLContext);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load default font
    io.Fonts->AddFontDefault();

    Log::Info("ImGui initialized successfully");
}

void Renderer::ShutdownImGui() {
    Log::Info("Shutting down ImGui...");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

void Renderer::BeginFrame() {
    glViewport(0, 0, m_Width, m_Height);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();
}

void Renderer::EndFrame() {
    // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap is handled by SDL
}

void Renderer::Clear(const Color& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::SetViewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
}

void Renderer::AddParticle(const Particle& particle) {
    m_Particles.push_back(particle);
}

void Renderer::UpdateParticles(f64 deltaTime) {
    for (auto it = m_Particles.begin(); it != m_Particles.end();) {
        it->life -= static_cast<f32>(deltaTime);
        if (it->life <= 0.0f) {
            it = m_Particles.erase(it);
        } else {
            it->position = it->position + it->velocity * static_cast<f32>(deltaTime);
            it->velocity = it->velocity * 0.98f; // Damping
            ++it;
        }
    }
}