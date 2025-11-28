#pragma once

#include "Types.h"
#include <vector>
#include <string>

// Forward declarations for SDL/ImGui
struct SDL_Window;
typedef void* SDL_GLContext;

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

    // Legacy 2D rendering primitives (DEPRECATED - use ImGui instead)
    // These will be removed once UI migration to ImGui is complete
    void DrawRect(const Rect& rect, const Color& color, bool filled = true);
    void DrawCircle(const Vec2& center, f32 radius, const Color& color, bool filled = true);
    void DrawLine(const Vec2& start, const Vec2& end, const Color& color, f32 thickness = 1.0f);
    void DrawText(const std::string& text, const Vec2& position, const Color& color, f32 size = 16.0f);
    void DrawProgressBar(const Vec2& position, f32 width, f32 height,
                        f64 current, f64 max,
                        const Color& fillColor, const Color& bgColor = Color(0.2f, 0.2f, 0.2f, 1.0f),
                        bool showPercentage = true, f32 textSize = 12.0f);

    // Particle system (will be migrated to ImGui custom rendering later)
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
    void RenderParticles();

    int GetWidth() const { return m_Width; }
    int GetHeight() const { return m_Height; }

    // Access to SDL/GL context for ImGui
    SDL_Window* GetWindow() const { return m_Window; }
    SDL_GLContext GetGLContext() const { return m_GLContext; }

private:
    void InitializeImGui();
    void ShutdownImGui();

    SDL_Window* m_Window = nullptr;
    SDL_GLContext m_GLContext = nullptr;

    int m_Width = 0;
    int m_Height = 0;

    std::vector<Particle> m_Particles;

    // Legacy projection matrix for immediate mode rendering (DEPRECATED)
    f32 m_ProjectionMatrix[16];
    void UpdateProjectionMatrix();
};
