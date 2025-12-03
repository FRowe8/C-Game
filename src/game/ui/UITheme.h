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

    // Modern card visuals
    const ImVec4 ColorCardBg           = ImVec4(0.08f, 0.08f, 0.12f, 0.98f);
    const ImVec4 ColorCardBgAlt        = ImVec4(0.10f, 0.10f, 0.16f, 0.98f);
    const ImVec4 ColorCardHeader       = ImVec4(0.14f, 0.10f, 0.20f, 0.98f);
    const ImVec4 ColorCardBorder       = ImVec4(0.25f, 0.70f, 0.90f, 0.50f);
    const ImVec4 ColorBadgeBg          = ImVec4(0.10f, 0.70f, 0.85f, 1.00f);
    const ImVec4 ColorBadgeText        = ImVec4(0.95f, 0.98f, 1.00f, 1.00f);
    const ImVec4 ColorStatusInfo       = ImVec4(0.05f, 0.60f, 0.90f, 1.00f);
    const ImVec4 ColorStatusNeutral    = ImVec4(0.50f, 0.55f, 0.65f, 1.00f);
    const ImVec4 ColorStatusSuccess    = ColorSuccess;
    const ImVec4 ColorStatusWarning    = ColorWarning;
    const ImVec4 ColorStatusDanger     = ColorDanger;

    const ImVec4 ColorText         = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);
    const ImVec4 ColorTextDim      = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);

    struct TypographyScale {
        float size;
        float weight;
    };

    // --- Layout Constants (scaled for mobile/desktop) ---
    struct LayoutMetrics {
        float topBarHeight = 60.0f;
        float bottomBarHeight = 80.0f;
        float sidebarWidth = 250.0f;
        float windowRounding = 10.0f;
        float cardRounding = 12.0f;
        float frameRounding = 10.0f;
        float grabRounding = 12.0f;
        float scrollbarRounding = 12.0f;
        float itemSpacing = 12.0f;
        float framePaddingX = 12.0f;
        float framePaddingY = 10.0f;
        float fontScale = 1.0f;
        float contentPadding = 18.0f;
        float safeAreaPadding = 14.0f;
        float cardPadding = 16.0f;
        float touchPadding = 8.0f;
        float navButtonHeight = 56.0f;
        float buttonRounding = 12.0f;
        float borderThickness = 1.25f;
        float scrollbarSize = 18.0f;
        float progressThickness = 6.0f;
        float shadowSoftness = 18.0f;
        float touchMinSize = 44.0f;
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
        layout.windowRounding = 10.0f * scale;
        layout.cardRounding = 12.0f * scale;
        layout.frameRounding = 10.0f * scale;
        layout.grabRounding = 12.0f * scale;
        layout.scrollbarRounding = 12.0f * scale;
        layout.itemSpacing = 12.0f * scale;
        layout.framePaddingX = 12.0f * scale;
        layout.framePaddingY = 10.0f * scale;
        layout.contentPadding = 18.0f * scale;
        layout.safeAreaPadding = 14.0f * scale;
        layout.cardPadding = 16.0f * scale;
        layout.touchPadding = 8.0f * scale;
        layout.navButtonHeight = 56.0f * scale;
        layout.buttonRounding = 12.0f * scale;
        layout.borderThickness = 1.25f * scale;
        layout.scrollbarSize = 18.0f * scale;
        layout.progressThickness = 6.0f * scale;
        layout.shadowSoftness = 18.0f * scale;
        layout.touchMinSize = 44.0f * scale;
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
    inline float CardRounding() { return GetLayout().cardRounding; }
    inline float CardPadding() { return GetLayout().cardPadding; }
    inline float ProgressThickness() { return GetLayout().progressThickness; }
    inline float ShadowSoftness() { return GetLayout().shadowSoftness; }
    inline float TouchMinSize() { return GetLayout().touchMinSize; }
    inline ImVec2 FramePadding() { return ImVec2(GetLayout().framePaddingX, GetLayout().framePaddingY); }
    inline ImVec2 TouchPadding() { return ImVec2(GetLayout().touchPadding, GetLayout().touchPadding); }
    inline ImVec2 TouchMinSizeVec2() { return ImVec2(GetLayout().touchMinSize, GetLayout().touchMinSize); }

    inline TypographyScale HeaderText() { return TypographyScale{ 28.0f * GetLayout().fontScale, 800.0f }; }
    inline TypographyScale SubtitleText() { return TypographyScale{ 22.0f * GetLayout().fontScale, 700.0f }; }
    inline TypographyScale BodyText() { return TypographyScale{ 18.0f * GetLayout().fontScale, 600.0f }; }
    inline TypographyScale CaptionText() { return TypographyScale{ 15.0f * GetLayout().fontScale, 500.0f }; }

    // --- Helper to Apply Style ---
    inline void SetupStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        ImGuiIO& io = ImGui::GetIO();

        const float scale = CalculateScale(io.DisplaySize);
        ApplyLayoutScale(scale);
        io.FontGlobalScale = GetLayout().fontScale;
        const float shadowAlpha = ImClamp(GetLayout().shadowSoftness / 24.0f, 0.35f, 0.90f);

        style.WindowRounding    = GetLayout().windowRounding;
        style.ChildRounding     = GetLayout().cardRounding;
        style.FrameRounding     = GetLayout().frameRounding;
        style.PopupRounding     = GetLayout().windowRounding;
        style.ScrollbarRounding = GetLayout().scrollbarRounding;
        style.GrabRounding      = GetLayout().grabRounding;

        style.WindowPadding     = ImVec2(GetLayout().contentPadding, GetLayout().contentPadding);
        style.FramePadding      = FramePadding();
        style.ItemSpacing       = ImVec2(GetLayout().itemSpacing, 10.0f * scale);
        style.TouchExtraPadding = TouchPadding();
        style.ScrollbarSize     = GetLayout().scrollbarSize;
        style.GrabMinSize       = GetLayout().touchMinSize;
        style.WindowBorderSize  = GetLayout().borderThickness;
        style.ChildBorderSize   = GetLayout().borderThickness * 0.75f;
        style.PopupBorderSize   = GetLayout().borderThickness;
        style.FrameBorderSize   = GetLayout().borderThickness * 0.75f;

        style.Colors[ImGuiCol_WindowBg]       = ColorPanelBg;
        style.Colors[ImGuiCol_Border]         = ColorBorder;
        style.Colors[ImGuiCol_Text]           = ColorText;
        style.Colors[ImGuiCol_FrameBg]        = ImVec4(ColorCardBg.x, ColorCardBg.y, ColorCardBg.z, 0.9f);
        style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(ColorCardBgAlt.x, ColorCardBgAlt.y, ColorCardBgAlt.z, 0.95f);
        style.Colors[ImGuiCol_FrameBgActive]  = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.6f);

        style.Colors[ImGuiCol_ChildBg]        = ColorCardBg;
        style.Colors[ImGuiCol_PopupBg]        = ColorCardBgAlt;
        style.Colors[ImGuiCol_TitleBg]        = ColorCardHeader;
        style.Colors[ImGuiCol_TitleBgActive]  = ColorCardHeader;
        style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.35f);

        // Buttons
        style.Colors[ImGuiCol_Button]         = ImVec4(ColorPrimary.x, ColorPrimary.y, ColorPrimary.z, 0.6f);
        style.Colors[ImGuiCol_ButtonHovered]  = ImVec4(ColorPrimary.x, ColorPrimary.y, ColorPrimary.z, 0.8f);
        style.Colors[ImGuiCol_ButtonActive]   = ColorPrimary;
        
        // Headers (Collapsing headers, etc)
        style.Colors[ImGuiCol_Header]         = ImVec4(ColorCardHeader.x, ColorCardHeader.y, ColorCardHeader.z, 0.85f);
        style.Colors[ImGuiCol_HeaderHovered]  = ImVec4(ColorCardHeader.x, ColorCardHeader.y, ColorCardHeader.z, 0.95f);
        style.Colors[ImGuiCol_HeaderActive]   = ColorAccent;

        // Tabs and cards
        style.Colors[ImGuiCol_Tab]            = ColorCardHeader;
        style.Colors[ImGuiCol_TabHovered]     = ImVec4(ColorBadgeBg.x, ColorBadgeBg.y, ColorBadgeBg.z, 0.95f);
        style.Colors[ImGuiCol_TabActive]      = ColorAccent;
        style.Colors[ImGuiCol_TabUnfocused]   = ColorCardHeader;
        style.Colors[ImGuiCol_TabUnfocusedActive] = ColorCardHeader;
        style.Colors[ImGuiCol_Separator]      = ColorCardBorder;
        style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.8f);
        style.Colors[ImGuiCol_SeparatorActive] = ColorAccent;

        // Badges & status accents
        style.Colors[ImGuiCol_CheckMark]      = ColorBadgeBg;
        style.Colors[ImGuiCol_SliderGrab]     = ColorAccent;
        style.Colors[ImGuiCol_SliderGrabActive] = ColorPrimary;
        style.Colors[ImGuiCol_PlotHistogram]  = ColorStatusWarning;
        style.Colors[ImGuiCol_PlotHistogramHovered] = ColorStatusDanger;
        style.Colors[ImGuiCol_PlotLines]      = ColorStatusInfo;
        style.Colors[ImGuiCol_PlotLinesHovered] = ColorStatusSuccess;

        // Scrollbars
        style.Colors[ImGuiCol_ScrollbarBg]    = ImVec4(0,0,0,0.2f);
        style.Colors[ImGuiCol_ScrollbarGrab]  = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.4f);
        style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.6f);
        style.Colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, 0.8f);

        // Shadows / overlays
        style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(ColorBgDark.x, ColorBgDark.y, ColorBgDark.z, shadowAlpha);
        style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(ColorAccent.x, ColorAccent.y, ColorAccent.z, shadowAlpha * 0.5f);
        style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(ColorBgDark.x, ColorBgDark.y, ColorBgDark.z, shadowAlpha * 0.75f);
    }
}
