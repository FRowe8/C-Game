#include "FontManager.h"
#include "Logger.h"
#include <SDL.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#include <GL/gl.h>
#elif defined(PLATFORM_MACOS)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

FontManager::FontManager()
    : m_Initialized(false) {
}

FontManager::~FontManager() {
    Shutdown();
}

bool FontManager::Initialize() {
    if (m_Initialized) {
        return true;
    }

    if (TTF_Init() != 0) {
        Log::Errorf("Failed to initialize SDL_ttf: ", TTF_GetError());
        return false;
    }

    Log::Info("FontManager initialized");
    m_Initialized = true;
    return true;
}

void FontManager::Shutdown() {
    if (!m_Initialized) {
        return;
    }

    // Close all loaded fonts
    for (auto& pair : m_Fonts) {
        if (pair.second.font) {
            TTF_CloseFont(pair.second.font);
        }
    }
    m_Fonts.clear();

    TTF_Quit();
    m_Initialized = false;
    Log::Info("FontManager shut down");
}

bool FontManager::LoadFont(const std::string& name, const std::string& filepath, int pointSize) {
    if (!m_Initialized) {
        Log::Error("FontManager not initialized");
        return false;
    }

    // Check if font is already loaded
    if (m_Fonts.find(name) != m_Fonts.end()) {
        Log::Infof("Font '", name.c_str(), "' already loaded");
        return true;
    }

    TTF_Font* font = TTF_OpenFont(filepath.c_str(), pointSize);
    if (!font) {
        Log::Errorf("Failed to load font '", filepath.c_str(), "': ", TTF_GetError());
        return false;
    }

    FontData data;
    data.font = font;
    data.pointSize = pointSize;
    m_Fonts[name] = data;

    Log::Infof("Loaded font '", name.c_str(), "' from '", filepath.c_str(), "' at ", pointSize, " points");
    return true;
}

u32 FontManager::RenderText(const std::string& fontName, const std::string& text,
                            const Color& color, int& outWidth, int& outHeight) {
    if (!m_Initialized) {
        Log::Error("FontManager not initialized");
        return 0;
    }

    if (text.empty()) {
        outWidth = 0;
        outHeight = 0;
        return 0;
    }

    // Find the font
    auto it = m_Fonts.find(fontName);
    if (it == m_Fonts.end()) {
        Log::Errorf("Font '", fontName.c_str(), "' not found");
        return 0;
    }

    TTF_Font* font = it->second.font;

    // Convert color to SDL format
    SDL_Color sdlColor;
    sdlColor.r = static_cast<u8>(color.r * 255.0f);
    sdlColor.g = static_cast<u8>(color.g * 255.0f);
    sdlColor.b = static_cast<u8>(color.b * 255.0f);
    sdlColor.a = static_cast<u8>(color.a * 255.0f);

    // Render text to surface (blended for best quality)
    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), sdlColor);
    if (!surface) {
        Log::Errorf("Failed to render text: ", TTF_GetError());
        return 0;
    }

    outWidth = surface->w;
    outHeight = surface->h;

    // Convert surface to OpenGL texture
    u32 textureId = SurfaceToTexture(surface);

    // Free the surface
    SDL_FreeSurface(surface);

    return textureId;
}

void FontManager::FreeTexture(u32 textureId) {
    if (textureId != 0) {
        glDeleteTextures(1, &textureId);
    }
}

void FontManager::GetTextSize(const std::string& fontName, const std::string& text,
                              int& outWidth, int& outHeight) {
    if (!m_Initialized) {
        outWidth = 0;
        outHeight = 0;
        return;
    }

    auto it = m_Fonts.find(fontName);
    if (it == m_Fonts.end()) {
        outWidth = 0;
        outHeight = 0;
        return;
    }

    TTF_Font* font = it->second.font;
    TTF_SizeText(font, text.c_str(), &outWidth, &outHeight);
}

u32 FontManager::SurfaceToTexture(SDL_Surface* surface) {
    if (!surface) {
        return 0;
    }

    // Generate texture
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Determine format
    GLenum format = GL_RGBA;
    if (surface->format->BytesPerPixel == 3) {
        format = GL_RGB;
    } else if (surface->format->BytesPerPixel == 4) {
        format = GL_RGBA;
    }

    // Upload texture data
    glTexImage2D(GL_TEXTURE_2D, 0, format, surface->w, surface->h,
                 0, format, GL_UNSIGNED_BYTE, surface->pixels);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    return textureId;
}
