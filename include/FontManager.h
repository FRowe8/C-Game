#pragma once

#include "Core.h"
#include "Math.h"
#include <SDL_ttf.h>
#include <string>
#include <unordered_map>

// FontManager handles loading and rendering TrueType fonts
class FontManager {
public:
    FontManager();
    ~FontManager();

    // Initialize SDL_ttf
    bool Initialize();

    // Shutdown and clean up resources
    void Shutdown();

    // Load a font from file at specified size
    bool LoadFont(const std::string& name, const std::string& filepath, int pointSize);

    // Render text to a texture and return texture ID
    // Returns 0 if rendering fails
    u32 RenderText(const std::string& fontName, const std::string& text,
                   const Color& color, int& outWidth, int& outHeight);

    // Free a rendered text texture
    void FreeTexture(u32 textureId);

    // Get size of rendered text without actually rendering it
    void GetTextSize(const std::string& fontName, const std::string& text,
                     int& outWidth, int& outHeight);

private:
    struct FontData {
        TTF_Font* font;
        int pointSize;
    };

    bool m_Initialized;
    std::unordered_map<std::string, FontData> m_Fonts;

    // Helper to convert SDL_Surface to OpenGL texture
    u32 SurfaceToTexture(SDL_Surface* surface);
};
