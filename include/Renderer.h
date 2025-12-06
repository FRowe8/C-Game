#pragma once

#include "Types.h"
#include <vector>
#include <string>

// Forward declarations for SDL/RmlUi
struct SDL_Window;
typedef void* SDL_GLContext;
namespace Rml {
    class Context;
    class RenderInterface;
}

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(SDL_Window* window, SDL_GLContext glContext, int width, int height);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Clear(const Color& color);
    void SetViewport(int x, int y, int width, int height);

    // --- DEPRECATED LEGACY DRAWING FUNCTIONS REMOVED ---

    // Particle system (keeping signatures, implementation moves to GameState/ImGui)
    struct Particle {
        Vec2 position;
        Vec2 velocity;
        Color color;
        f32 life;
        f32 maxLife;
        f32 size;
    };

    void AddParticle(const Particle& particle);
    void UpdateParticles(f64 deltaTime);
    void RenderParticles(); // This will now use ImGui::GetBackgroundDrawList

    void SetRmlUiContext(Rml::Context* context, Rml::RenderInterface* renderInterface);

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    SDL_Window* GetWindow() const { return m_Window; }
    SDL_GLContext GetGLContext() const { return m_GLContext; }

private:
    SDL_Window* m_Window = nullptr;
    SDL_GLContext m_GLContext = nullptr;

    int m_Width = 0;
    int m_Height = 0;

    std::vector<Particle> m_Particles;

    Rml::Context* m_RmlContext = nullptr;
    Rml::RenderInterface* m_RmlRenderInterface = nullptr;

    // Legacy projection matrix removed
    // f32 m_ProjectionMatrix[16];
    void UpdateProjectionMatrix(); // Implementation removed below
};