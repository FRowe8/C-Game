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

    // Set up OpenGL state (still needed for legacy rendering and ImGui)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    UpdateProjectionMatrix();

    // Initialize ImGui
    InitializeImGui();

    Log::Info("Renderer initialized successfully");
    return true;
}

void Renderer::Shutdown() {
    ShutdownImGui();
    // Legacy bitmap font needs no cleanup
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

void Renderer::UpdateProjectionMatrix() {
    // Simple orthographic projection matrix
    f32 left = 0.0f;
    f32 right = static_cast<f32>(m_Width);
    f32 bottom = static_cast<f32>(m_Height);
    f32 top = 0.0f;
    f32 near = -1.0f;
    f32 far = 1.0f;

    memset(m_ProjectionMatrix, 0, sizeof(m_ProjectionMatrix));
    m_ProjectionMatrix[0] = 2.0f / (right - left);
    m_ProjectionMatrix[5] = 2.0f / (top - bottom);
    m_ProjectionMatrix[10] = -2.0f / (far - near);
    m_ProjectionMatrix[12] = -(right + left) / (right - left);
    m_ProjectionMatrix[13] = -(top + bottom) / (top - bottom);
    m_ProjectionMatrix[14] = -(far + near) / (far - near);
    m_ProjectionMatrix[15] = 1.0f;
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

void Renderer::DrawRect(const Rect& rect, const Color& color, bool filled) {
    // Use immediate mode for simplicity (can optimize later with batching)
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_ProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor4f(color.r, color.g, color.b, color.a);

    if (filled) {
        glBegin(GL_QUADS);
        glVertex2f(rect.x, rect.y);
        glVertex2f(rect.x + rect.width, rect.y);
        glVertex2f(rect.x + rect.width, rect.y + rect.height);
        glVertex2f(rect.x, rect.y + rect.height);
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        glVertex2f(rect.x, rect.y);
        glVertex2f(rect.x + rect.width, rect.y);
        glVertex2f(rect.x + rect.width, rect.y + rect.height);
        glVertex2f(rect.x, rect.y + rect.height);
        glEnd();
    }
}

void Renderer::DrawCircle(const Vec2& center, f32 radius, const Color& color, bool filled) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_ProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor4f(color.r, color.g, color.b, color.a);

    int segments = 32;
    if (filled) {
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(center.x, center.y);
        for (int i = 0; i <= segments; i++) {
            f32 angle = 2.0f * M_PI * i / segments;
            f32 x = center.x + radius * cosf(angle);
            f32 y = center.y + radius * sinf(angle);
            glVertex2f(x, y);
        }
        glEnd();
    } else {
        glBegin(GL_LINE_LOOP);
        for (int i = 0; i < segments; i++) {
            f32 angle = 2.0f * M_PI * i / segments;
            f32 x = center.x + radius * cosf(angle);
            f32 y = center.y + radius * sinf(angle);
            glVertex2f(x, y);
        }
        glEnd();
    }
}

void Renderer::DrawLine(const Vec2& start, const Vec2& end, const Color& color, f32 thickness) {
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(m_ProjectionMatrix);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glColor4f(color.r, color.g, color.b, color.a);
    glLineWidth(thickness);

    glBegin(GL_LINES);
    glVertex2f(start.x, start.y);
    glVertex2f(end.x, end.y);
    glEnd();

    glLineWidth(1.0f);
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

void Renderer::RenderParticles() {
    for (const auto& particle : m_Particles) {
        f32 alpha = particle.life / particle.maxLife;
        Color color = particle.color;
        color.a *= alpha;

        DrawCircle(particle.position, particle.size, color, true);
    }
}
