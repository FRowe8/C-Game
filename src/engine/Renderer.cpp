#include "Renderer.h"
#include "Logger.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <cmath>
#include <cstring>

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
#ifdef DrawText
#undef DrawText
#endif

Renderer::Renderer() {
}

Renderer::~Renderer() {
    Shutdown();
}

bool Renderer::Initialize(int width, int height) {
    m_Width = width;
    m_Height = height;

    Log::Info("Initializing Renderer...");
    Log::Infof("OpenGL Version: ", glGetString(GL_VERSION));
    Log::Infof("GLSL Version: ", glGetString(GL_SHADING_LANGUAGE_VERSION));

    // Set up OpenGL state
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    UpdateProjectionMatrix();

    Log::Info("Renderer initialized successfully");
    return true;
}

void Renderer::Shutdown() {
    // Using immediate mode rendering, no resources to clean up
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
}

void Renderer::EndFrame() {
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

void Renderer::DrawText(const std::string& text, const Vec2& position, const Color& color, f32 size) {
    // Improved placeholder text rendering - makes characters distinguishable
    // Different character types have different sizes/colors for readability
    f32 charWidth = size * 0.8f;  // Increased from 0.6f for better visibility
    f32 charHeight = size;
    f32 x = position.x;

    for (char c : text) {
        if (c == ' ') {
            x += charWidth * 0.5f;
            continue;
        }

        // Vary appearance based on character type for better readability
        f32 width = charWidth;
        f32 height = charHeight;
        Color charColor = color;

        // Make different character types visually distinct
        if (c >= '0' && c <= '9') {
            // Numbers: brighter and full size
            charColor = color * 1.3f;
            charColor.a = 1.0f;
        } else if (c >= 'A' && c <= 'Z') {
            // Uppercase: full height
            height = charHeight;
        } else if (c >= 'a' && c <= 'z') {
            // Lowercase: slightly shorter
            height = charHeight * 0.75f;
        } else if (c == '.' || c == ',' || c == ':') {
            // Punctuation: small and lower
            height = charHeight * 0.3f;
            width = charWidth * 0.4f;
        } else {
            // Other characters: medium size
            height = charHeight * 0.6f;
            width = charWidth * 0.6f;
        }

        // Draw character rectangle with full opacity
        Rect charRect(x, position.y + (charHeight - height), width, height);
        DrawRect(charRect, charColor, true);

        // Add subtle white outline for better contrast
        DrawRect(charRect, Color(1.0f, 1.0f, 1.0f, 0.4f), false);

        x += charWidth;
    }
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
