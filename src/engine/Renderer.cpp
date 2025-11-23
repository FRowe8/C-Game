#include "Renderer.h"
#include "Logger.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include <cmath>
#include <cstring>

#ifndef M_PI
#define M_PI 3.14159265358979323846
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

    SetupQuad();
    UpdateProjectionMatrix();

    Log::Info("Renderer initialized successfully");
    return true;
}

void Renderer::Shutdown() {
    if (m_QuadVAO) {
        glDeleteVertexArrays(1, &m_QuadVAO);
        m_QuadVAO = 0;
    }
    if (m_QuadVBO) {
        glDeleteBuffers(1, &m_QuadVBO);
        m_QuadVBO = 0;
    }
}

void Renderer::SetupQuad() {
    // Simple vertex format: position only, we'll transform in immediate mode
    f32 vertices[] = {
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f
    };

    glGenVertexArrays(1, &m_QuadVAO);
    glGenBuffers(1, &m_QuadVBO);

    glBindVertexArray(m_QuadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_QuadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(f32), (void*)0);

    glBindVertexArray(0);
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
    // Simple bitmap text rendering - draw each character as a small rect
    // This is a placeholder - in a real game you'd use a proper font rendering library
    f32 charWidth = size * 0.6f;
    f32 charHeight = size;
    f32 x = position.x;

    for (char c : text) {
        if (c == ' ') {
            x += charWidth;
            continue;
        }

        // Draw character as a simple rectangle (placeholder)
        Rect charRect(x, position.y, charWidth * 0.8f, charHeight);
        DrawRect(charRect, color * 0.3f, true);

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
