#pragma once

#include "Types.h"
#include <vector>
#include <string>

class Renderer {
public:
    Renderer();
    ~Renderer();

    bool Initialize(int width, int height);
    void Shutdown();

    void BeginFrame();
    void EndFrame();

    void Clear(const Color& color);
    void SetViewport(int x, int y, int width, int height);

    // 2D rendering primitives
    void DrawRect(const Rect& rect, const Color& color, bool filled = true);
    void DrawCircle(const Vec2& center, f32 radius, const Color& color, bool filled = true);
    void DrawLine(const Vec2& start, const Vec2& end, const Color& color, f32 thickness = 1.0f);

    // Text rendering (uses bitmap font)
    void DrawText(const std::string& text, const Vec2& position, const Color& color, f32 size = 16.0f);

    // Particle system
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

private:
    void InitializeShaders();

    int m_Width = 0;
    int m_Height = 0;

    std::vector<Particle> m_Particles;

    // Projection matrix for 2D rendering
    f32 m_ProjectionMatrix[16];
    void UpdateProjectionMatrix();
};
