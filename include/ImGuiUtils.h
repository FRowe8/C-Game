#pragma once

#include "imgui.h"
#include "Types.h" // Assumed to define 'Color'

// Use 'inline' to avoid "multiple definition" linker errors
inline ImVec4 ToImVec4(const Color& color) {
    return ImVec4(color.r, color.g, color.b, color.a);
}

inline ImU32 ToImU32(const Color& color) {
    return ImGui::GetColorU32(ToImVec4(color));
}

// ADD THIS NEW FUNCTION:
inline ImVec2 ToImVec2(const Vec2& vec) {
    return ImVec2(vec.x, vec.y);
}

/**
 * Enhanced button with hover pulse and click shrink animations
 * Provides visual feedback for better "game feel"
 * @param label Button text
 * @param size Button size (use ImVec2(0,0) for auto-size)
 * @return true if button was clicked
 */
inline bool AnimatedButton(const char* label, const ImVec2& size = ImVec2(0,0)) {
    ImGui::PushID(label);

    // Get time for animations
    float time = ImGui::GetTime();

    // Check if button is hovered or active
    ImVec2 cursorPos = ImGui::GetCursorScreenPos();
    ImVec2 buttonSize = size;
    if (buttonSize.x == 0 || buttonSize.y == 0) {
        ImVec2 textSize = ImGui::CalcTextSize(label);
        ImGuiStyle& style = ImGui::GetStyle();
        if (buttonSize.x == 0) buttonSize.x = textSize.x + style.FramePadding.x * 2.0f;
        if (buttonSize.y == 0) buttonSize.y = textSize.y + style.FramePadding.y * 2.0f;
    }

    ImVec2 mousePos = ImGui::GetIO().MousePos;
    bool isHovered = (mousePos.x >= cursorPos.x && mousePos.x <= cursorPos.x + buttonSize.x &&
                     mousePos.y >= cursorPos.y && mousePos.y <= cursorPos.y + buttonSize.y);
    bool isClicked = isHovered && ImGui::IsMouseDown(0);

    // Calculate animation factors
    float pulseScale = 1.0f;
    float shrinkScale = 1.0f;

    if (isHovered && !isClicked) {
        // Pulse animation on hover (subtle breathing effect)
        pulseScale = 1.0f + sinf(time * 6.0f) * 0.02f; // ±2% scale
    }

    if (isClicked) {
        // Shrink effect on click
        shrinkScale = 0.95f; // 95% size when pressed
    }

    float finalScale = pulseScale * shrinkScale;

    // Apply scaling via padding adjustment
    if (finalScale != 1.0f) {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec2 originalPadding = style.FramePadding;
        style.FramePadding = ImVec2(
            originalPadding.x * finalScale,
            originalPadding.y * finalScale
        );

        bool clicked = ImGui::Button(label, ImVec2(size.x * finalScale, size.y * finalScale));

        style.FramePadding = originalPadding;
        ImGui::PopID();
        return clicked;
    } else {
        bool clicked = ImGui::Button(label, size);
        ImGui::PopID();
        return clicked;
    }
}

/**
 * Colored animated button variant
 */
inline bool ColoredAnimatedButton(const char* label, const Color& color, const ImVec2& size = ImVec2(0,0)) {
    ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(color));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.r * 1.2f, color.g * 1.2f, color.b * 1.2f, color.a));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.r * 0.8f, color.g * 0.8f, color.b * 0.8f, color.a));

    bool result = AnimatedButton(label, size);

    ImGui::PopStyleColor(3);
    return result;
}