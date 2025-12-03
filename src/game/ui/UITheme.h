#pragma once
#include "imgui.h"

namespace UITheme {
    // --- Colors (Cyberpunk / Quantum Theme) ---
    const ImVec4 ColorBgDark       = ImVec4(0.05f, 0.05f, 0.08f, 1.00f); // Deep Space
    const ImVec4 ColorPanelBg      = ImVec4(0.10f, 0.10f, 0.15f, 0.95f); // Panel Background
    const ImVec4 ColorBorder       = ImVec4(0.00f, 0.80f, 0.90f, 0.50f); // Cyan Border
    const ImVec4 ColorAccent       = ImVec4(0.00f, 0.90f, 1.00f, 1.00f); // Neon Cyan
    const ImVec4 ColorAccentHover  = ImVec4(0.20f, 0.95f, 1.00f, 1.00f);
    const ImVec4 ColorAccentActive = ImVec4(0.00f, 0.70f, 0.80f, 1.00f);
    
    const ImVec4 ColorPrimary      = ImVec4(0.60f, 0.20f, 0.90f, 1.00f); // Quantum Purple
    const ImVec4 ColorSuccess      = ImVec4(0.20f, 0.80f, 0.20f, 1.00f); // Green
    const ImVec4 ColorWarning      = ImVec4(1.00f, 0.80f, 0.00f, 1.00f); // Gold
    const ImVec4 ColorDanger       = ImVec4(0.90f, 0.20f, 0.20f, 1.00f); // Red
    
    const ImVec4 ColorText         = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    const ImVec4 ColorTextDim      = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

    // --- Layout Constants (scaled for mobile/desktop) ---
    struct LayoutMetrics {
        float topBarHeight = 60.0f;
        float bottomBarHeight = 80.0f;
        float sidebarWidth = 250.0f;
        float windowRounding = 8.0f;
        float itemSpacing = 12.0f;
        float framePaddingX = 10.0f;
        float framePaddingY = 6.0f;
        float fontScale = 1.0f;
        float contentPadding = 16.0f;
        float safeAreaPadding = 12.0f;
        float touchPadding = 6.0f;
        float navButtonHeight = 56.0f;
        float buttonRounding = 10.0f;
        float borderThickness = 1.0f;
        float scrollbarSize = 16.0f;
    };

    inline LayoutMetrics& GetLayout() {
        static LayoutMetrics metrics{};
        return metrics;
    }

    inline float CalculateScale(const ImVec2& displaySize) {
        const float shortSide = (displaySize.x < displaySize.y) ? displaySize.x : displaySize.y;

        // Heuristic: aggressively scale up on smaller screens to keep touch targets usable.
        if (shortSide <= 540.0f) {
            return 1.45f; // Small phones
        }
        if (shortSide <= 720.0f) {
            return 1.30f; // Large phones / small tablets
        }
        if (shortSide <= 900.0f) {
            return 1.15f; // Small laptops / medium tablets
        }
        return 1.0f; // Desktop baseline
    }

    inline void ApplyLayoutScale(float scale) {
        auto& layout = GetLayout();
        layout.fontScale = scale;
        layout.topBarHeight = 60.0f * scale;
        layout.bottomBarHeight = 80.0f * scale;
        layout.sidebarWidth = 250.0f * scale;
        layout.windowRounding = 8.0f * scale;
        layout.itemSpacing = 12.0f * scale;
        layout.framePaddingX = 10.0f * scale;
        layout.framePaddingY = 6.0f * scale;
        layout.contentPadding = 16.0f * scale;
        layout.safeAreaPadding = 12.0f * scale;
        layout.touchPadding = 6.0f * scale;
        layout.navButtonHeight = 56.0f * scale;
        layout.buttonRounding = 10.0f * scale;
        layout.borderThickness = 1.0f * scale;
        layout.scrollbarSize = 16.0f * scale;
    }

    inline float TopBarHeight() { return GetLayout().topBarHeight; }
    inline float BottomBarHeight() { return GetLayout().bottomBarHeight; }
    inline float ItemSpacing() { return GetLayout().itemSpacing; }
    inline float ContentPadding() { return GetLayout().contentPadding; }
    inline float SafeAreaPadding() { return GetLayout().safeAreaPadding; }
    inline float NavigationButtonHeight() { return GetLayout().navButtonHeight; }
    inline float NavigationButtonRounding() { return GetLayout().buttonRounding; }
    inline float BorderThickness() { return GetLayout().borderThickness; }
    inline float ScrollbarSize() { return GetLayout().scrollbarSize; }
    inline ImVec2 FramePadding() { return ImVec2(GetLayout().framePaddingX, GetLayout().framePaddingY); }
    inline ImVec2 TouchPadding() { return ImVec2(GetLayout().touchPadding, GetLayout().touchPadding); }

    // --- Helper to Apply Style ---
    inline void SetupStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImGuiIO& io = ImGui::GetIO();

        const float scale = CalculateScale(io.DisplaySize);
        ApplyLayoutScale(scale);
        io.FontGlobalScale = GetLayout().fontScale;

        style.WindowRounding    = GetLayout().windowRounding;
        style.FrameRounding     = 6.0f * scale;
        style.PopupRounding     = GetLayout().windowRounding;
        style.ScrollbarRounding = 12.0f * scale;
        style.GrabRounding      = 12.0f * scale;

        style.WindowPadding     = ImVec2(GetLayout().contentPadding, GetLayout().contentPadding);
        style.FramePadding      = FramePadding();
        style.ItemSpacing       = ImVec2(GetLayout().itemSpacing, 8.0f * scale);
        style.TouchExtraPadding = TouchPadding();
        style.ScrollbarSize     = GetLayout().scrollbarSize;
        style.GrabMinSize       = 18.0f * scale;
        style.WindowBorderSize  = GetLayout().borderThickness;
        style.ChildBorderSize   = GetLayout().borderThickness * 0.75f;
        style.PopupBorderSize   = GetLayout().borderThickness;
        style.FrameBorderSize   = GetLayout().borderThickness * 0.75f;

        style.Colors[ImGuiCol_WindowBg]       = ColorPanelBg;
        style.Colors[ImGuiCol_Border]         = ColorBorder;
        style.Colors[ImGuiCol_Text]           = ColorText;
        
        // Buttons
        style.Colors[ImGuiCol_Button]         = ImVec4(ColorPrimary.x, ColorPrimary.y, ColorPrimary.z, 0.6f);
        style.Colors[ImGuiCol_ButtonHovered]  = ImVec4(ColorPrimary.x, ColorPrimary.y, ColorPrimary.z, 0.8f);
        style.Colors[ImGuiCol_ButtonActive]   = ColorPrimary;
        
        // Headers (Collapsing headers, etc)
        style.Colors[ImGuiCol_Header]         = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.3f);
        style.Colors[ImGuiCol_HeaderHovered]  = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.5f);
        style.Colors[ImGuiCol_HeaderActive]   = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.7f);
        
        // Scrollbars
        style.Colors[ImGuiCol_ScrollbarBg]    = ImVec4(0,0,0,0.2f);
        style.Colors[ImGuiCol_ScrollbarGrab]  = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.4f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.6f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.8f);
    }
}