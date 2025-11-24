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

// Simple bitmap font - each character is defined as a 5x7 pixel pattern
// 1 = draw pixel, 0 = empty space
static const unsigned char font5x7[][7] = {
    // Digits 0-9 (ASCII 48-57)
    {0x7E, 0x81, 0x89, 0x91, 0xA1, 0x81, 0x7E}, // 0
    {0x00, 0x41, 0xFF, 0x01, 0x00, 0x00, 0x00}, // 1
    {0x43, 0x85, 0x89, 0x91, 0x61, 0x00, 0x00}, // 2
    {0x42, 0x81, 0x91, 0x91, 0x6E, 0x00, 0x00}, // 3
    {0x18, 0x28, 0x48, 0xFF, 0x08, 0x00, 0x00}, // 4
    {0xF2, 0x91, 0x91, 0x91, 0x8E, 0x00, 0x00}, // 5
    {0x7E, 0x91, 0x91, 0x91, 0x4E, 0x00, 0x00}, // 6
    {0x80, 0x87, 0x88, 0x90, 0xE0, 0x00, 0x00}, // 7
    {0x6E, 0x91, 0x91, 0x91, 0x6E, 0x00, 0x00}, // 8
    {0x72, 0x89, 0x89, 0x89, 0x7E, 0x00, 0x00}, // 9
    // Uppercase A-Z (ASCII 65-90)
    {0x3F, 0x48, 0x88, 0x88, 0x48, 0x3F, 0x00}, // A
    {0xFF, 0x91, 0x91, 0x91, 0x6E, 0x00, 0x00}, // B
    {0x7E, 0x81, 0x81, 0x81, 0x42, 0x00, 0x00}, // C
    {0xFF, 0x81, 0x81, 0x81, 0x7E, 0x00, 0x00}, // D
    {0xFF, 0x91, 0x91, 0x91, 0x81, 0x00, 0x00}, // E
    {0xFF, 0x90, 0x90, 0x90, 0x80, 0x00, 0x00}, // F
    {0x7E, 0x81, 0x89, 0x89, 0x4E, 0x00, 0x00}, // G
    {0xFF, 0x10, 0x10, 0x10, 0xFF, 0x00, 0x00}, // H
    {0x00, 0x81, 0xFF, 0x81, 0x00, 0x00, 0x00}, // I
    {0x06, 0x01, 0x01, 0x01, 0xFE, 0x00, 0x00}, // J
    {0xFF, 0x18, 0x24, 0x42, 0x81, 0x00, 0x00}, // K
    {0xFF, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00}, // L
    {0xFF, 0x40, 0x30, 0x40, 0xFF, 0x00, 0x00}, // M
    {0xFF, 0x40, 0x20, 0x10, 0xFF, 0x00, 0x00}, // N
    {0x7E, 0x81, 0x81, 0x81, 0x7E, 0x00, 0x00}, // O
    {0xFF, 0x88, 0x88, 0x88, 0x70, 0x00, 0x00}, // P
    {0x7E, 0x81, 0x85, 0x82, 0x7D, 0x00, 0x00}, // Q
    {0xFF, 0x88, 0x8C, 0x8A, 0x71, 0x00, 0x00}, // R
    {0x62, 0x91, 0x91, 0x91, 0x8E, 0x00, 0x00}, // S
    {0x80, 0x80, 0xFF, 0x80, 0x80, 0x00, 0x00}, // T
    {0xFE, 0x01, 0x01, 0x01, 0xFE, 0x00, 0x00}, // U
    {0xF8, 0x04, 0x02, 0x04, 0xF8, 0x00, 0x00}, // V
    {0xFF, 0x02, 0x0C, 0x02, 0xFF, 0x00, 0x00}, // W
    {0xC3, 0x24, 0x18, 0x24, 0xC3, 0x00, 0x00}, // X
    {0xC0, 0x20, 0x1F, 0x20, 0xC0, 0x00, 0x00}, // Y
    {0x83, 0x85, 0x89, 0x91, 0xE1, 0x00, 0x00}, // Z
};

void Renderer::DrawText(const std::string& text, const Vec2& position, const Color& color, f32 size) {
    f32 pixelSize = size / 7.0f; // Each character is 7 pixels tall
    f32 charSpacing = size * 0.8f;
    f32 x = position.x;

    for (char c : text) {
        if (c == ' ') {
            x += charSpacing * 0.6f;
            continue;
        }

        // Get font pattern for this character
        const unsigned char* pattern = nullptr;
        if (c >= '0' && c <= '9') {
            pattern = font5x7[c - '0'];
        } else if (c >= 'A' && c <= 'Z') {
            pattern = font5x7[10 + (c - 'A')];
        } else if (c >= 'a' && c <= 'z') {
            // Use uppercase patterns for lowercase
            pattern = font5x7[10 + (c - 'a')];
        }

        // Draw the character pixel by pixel
        if (pattern) {
            for (int row = 0; row < 7; row++) {
                unsigned char rowData = pattern[row];
                for (int col = 0; col < 8; col++) {
                    if (rowData & (0x80 >> col)) {
                        f32 px = x + col * pixelSize;
                        f32 py = position.y + row * pixelSize;
                        Rect pixel(px, py, pixelSize * 1.2f, pixelSize * 1.2f);
                        DrawRect(pixel, color, true);
                    }
                }
            }
        } else {
            // For unsupported characters, draw a simple placeholder
            Rect charRect(x, position.y, size * 0.5f, size);
            DrawRect(charRect, color * 0.5f, false);
        }

        x += charSpacing;
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
