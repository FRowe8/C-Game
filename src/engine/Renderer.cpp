#include "Renderer.h"
#include "Logger.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <cmath>
#include <cstring>

#ifdef RMLUI_ENABLED
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/RenderInterface.h>
#endif

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

    Log::Info("Renderer initialized successfully");
    return true;
}

void Renderer::Shutdown() {
    // Legacy bitmap font needs no cleanup
    // Add any necessary OpenGL/SDL cleanup here if needed later
}

void Renderer::SetRmlUiContext(Rml::Context* context, Rml::RenderInterface* renderInterface) {
    m_RmlContext = context;
    m_RmlRenderInterface = renderInterface;
}

void Renderer::BeginFrame() {
    glViewport(0, 0, m_Width, m_Height);
}

void Renderer::EndFrame() {
#ifdef RMLUI_ENABLED
    if (m_RmlContext && m_RmlRenderInterface) {
        m_RmlRenderInterface->BeginFrame();
        m_RmlContext->Render();
        m_RmlRenderInterface->EndFrame();
    }
#endif

    // Swap is handled by SDL via Application
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