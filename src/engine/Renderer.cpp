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

void Renderer::DrawText(const std::string& text, const Vec2& position, const Color& color, f32 size) {
    if (!m_FontManager || text.empty()) {
        return;
    }

    // Render text to texture
    int textWidth, textHeight;
    u32 textureId = m_FontManager->RenderText("default", text, color, textWidth, textHeight);

    if (textureId == 0) {
        // Fallback: draw a simple rectangle placeholder
        Rect rect(position.x, position.y, size * text.length() * 0.6f, size);
        DrawRect(rect, color * 0.3f, false);
        return;
    }

    // Scale texture to requested size (size parameter is target height)
    f32 scale = size / textHeight;
    f32 scaledWidth = textWidth * scale;
    f32 scaledHeight = size;

    // Draw the texture
    Rect destRect(position.x, position.y, scaledWidth, scaledHeight);
    DrawTexture(textureId, destRect);

    // Clean up texture
    m_FontManager->FreeTexture(textureId);
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
