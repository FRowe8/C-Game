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

    // --- Layout Constants ---
    const float TopBarHeight       = 60.0f;
    const float BottomBarHeight    = 80.0f;
    const float SidebarWidth       = 250.0f;
    const float WindowRounding     = 8.0f;
    const float ItemSpacing        = 12.0f;
    const float FramePaddingX      = 10.0f;
    const float FramePaddingY      = 6.0f;

    // --- Helper to Apply Style ---
    inline void SetupStyle() {
        ImGuiStyle& style = ImGui::GetStyle();
        
        style.WindowRounding    = WindowRounding;
        style.FrameRounding     = 6.0f;
        style.PopupRounding     = WindowRounding;
        style.ScrollbarRounding = 12.0f;
        style.GrabRounding      = 12.0f;
        
        style.WindowPadding     = ImVec2(15, 15);
        style.FramePadding      = ImVec2(FramePaddingX, FramePaddingY);
        style.ItemSpacing       = ImVec2(ItemSpacing, 8);
        
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