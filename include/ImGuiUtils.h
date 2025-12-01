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