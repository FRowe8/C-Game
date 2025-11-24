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
    glEnable(GL_TEXTURE_2D);

    UpdateProjectionMatrix();

    // Initialize font manager
    m_FontManager = CreateScope<FontManager>();
    if (!m_FontManager->Initialize()) {
        Log::Error("Failed to initialize FontManager");
        return false;
    }

    // Try to load a default font from common locations
    bool fontLoaded = false;
    const char* fontPaths[] = {
        "assets/fonts/Roboto-Regular.ttf",  // Project font
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",  // Linux
        "C:/Windows/Fonts/arial.ttf",  // Windows
        "/System/Library/Fonts/Helvetica.ttc",  // macOS
    };

    for (const char* path : fontPaths) {
        if (m_FontManager->LoadFont("default", path, 18)) {
            fontLoaded = true;
            Log::Infof("Loaded default font from: ", path);
            break;
        }
    }

    if (!fontLoaded) {
        Log::Warning("Could not load TrueType font, text may not render correctly");
    }

    Log::Info("Renderer initialized successfully");
    return true;
}

void Renderer::Shutdown() {
    if (m_FontManager) {
        m_FontManager->Shutdown();
        m_FontManager.reset();
    }
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

// Simple 5x7 bitmap font for reliable cross-platform text rendering
static const unsigned char font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Space
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // !
    {0x00, 0x07, 0x00, 0x07, 0x00}, // "
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // #
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // $
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x36, 0x49, 0x55, 0x22, 0x50}, // &
    {0x00, 0x05, 0x03, 0x00, 0x00}, // '
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // (
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // )
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // *
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // +
    {0x00, 0x50, 0x30, 0x00, 0x00}, // ,
    {0x08, 0x08, 0x08, 0x08, 0x08}, // -
    {0x00, 0x60, 0x60, 0x00, 0x00}, // .
    {0x20, 0x10, 0x08, 0x04, 0x02}, // /
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x00, 0x56, 0x36, 0x00, 0x00}, // ;
    {0x08, 0x14, 0x22, 0x41, 0x00}, // <
    {0x14, 0x14, 0x14, 0x14, 0x14}, // =
    {0x00, 0x41, 0x22, 0x14, 0x08}, // >
    {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // @
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x7F, 0x41, 0x41, 0x00}, // [
    {0x02, 0x04, 0x08, 0x10, 0x20}, // backslash
    {0x00, 0x41, 0x41, 0x7F, 0x00}, // ]
    {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
    {0x40, 0x40, 0x40, 0x40, 0x40}, // _
    {0x00, 0x01, 0x02, 0x04, 0x00}, // `
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x0C, 0x52, 0x52, 0x52, 0x3E}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x20, 0x40, 0x44, 0x3D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0x7C, 0x14, 0x14, 0x14, 0x08}, // p
    {0x08, 0x14, 0x14, 0x18, 0x7C}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x0C, 0x50, 0x50, 0x50, 0x3C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}, // z
};

void Renderer::DrawText(const std::string& text, const Vec2& position, const Color& color, f32 size) {
    // Pixel-perfect bitmap font rendering
    f32 charWidth = size * 0.7f;
    f32 spacing = charWidth + 3.0f;
    f32 pixelSize = size / 7.0f;
    f32 x = position.x;

    for (char c : text) {
        if (c == ' ') {
            x += spacing * 0.6f;
            continue;
        }

        int charIndex = static_cast<int>(c) - 32;
        if (charIndex < 0 || charIndex > 90) {
            x += spacing;
            continue;
        }

        const unsigned char* charData = font5x7[charIndex];

        // Draw character column by column
        for (int col = 0; col < 5; col++) {
            unsigned char colBits = charData[col];
            for (int row = 0; row < 7; row++) {
                if (colBits & (1 << row)) {
                    f32 px = x + col * pixelSize;
                    f32 py = position.y + row * pixelSize;
                    Rect pixel(px, py, pixelSize * 1.4f, pixelSize * 1.4f);
                    DrawRect(pixel, color, true);
                }
            }
        }

        x += spacing;
    }
}

void Renderer::DrawTexture(u32 textureId, const Rect& destRect) {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, textureId);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f); glVertex2f(destRect.x, destRect.y);
        glTexCoord2f(1.0f, 0.0f); glVertex2f(destRect.x + destRect.width, destRect.y);
        glTexCoord2f(1.0f, 1.0f); glVertex2f(destRect.x + destRect.width, destRect.y + destRect.height);
        glTexCoord2f(0.0f, 1.0f); glVertex2f(destRect.x, destRect.y + destRect.height);
    glEnd();

    glBindTexture(GL_TEXTURE_2D, 0);
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
