#pragma once

#include <cfloat>
#include <functional>
#include <string>
#include <unordered_map>

#include "imgui.h"
#include "ImGuiUtils.h"
#include "../src/game/ui/UITheme.h"

namespace MobileUI {
namespace Detail {
    struct TapFlashState {
        float until = 0.0f;
    };

    inline float GetScale() {
        return UITheme::CalculateScale(ImGui::GetIO().DisplaySize);
    }

    inline float TouchTargetSize() {
        return 44.0f * GetScale();
    }

    inline std::unordered_map<ImGuiID, TapFlashState>& FlashMap() {
        static std::unordered_map<ImGuiID, TapFlashState> states;
        return states;
    }
}

inline void RenderTapFlashOverlay(ImGuiID id, const ImVec2& min, const ImVec2& max, const ImVec4& color) {
    float now = ImGui::GetTime();
    auto& map = Detail::FlashMap();
    auto it = map.find(id);
    if (it != map.end() && it->second.until > now) {
        float t = (it->second.until - now) / 0.18f;
        float alpha = ImClamp(t, 0.0f, 1.0f) * 0.35f;
        ImGui::GetForegroundDrawList()->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(color.x, color.y, color.z, alpha)), UITheme::NavigationButtonRounding());
    }
}

inline void TriggerTapFlash(ImGuiID id, float duration = 0.18f) {
    Detail::FlashMap()[id].until = ImGui::GetTime() + duration;
}

inline void RenderCard(const char* id,
                       const char* title,
                       const std::function<void()>& content,
                       const ImVec4& background = UITheme::ColorPanelBg,
                       const ImVec4& accent = UITheme::ColorAccent,
                       const char* icon = nullptr,
                       const ImVec2& size = ImVec2(0, 0)) {
    const float minHeight = Detail::TouchTargetSize();
    ImVec2 cardSize = size;
    if (cardSize.y < minHeight) {
        cardSize.y = minHeight;
    }

    ImGui::PushID(id);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, background);
    ImGui::PushStyleColor(ImGuiCol_Border, UITheme::ColorBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, UITheme::BorderThickness());
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, UITheme::NavigationButtonRounding());
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(UITheme::ContentPadding(), UITheme::ContentPadding() * 0.5f));

    if (ImGui::BeginChild("Card", cardSize, true, ImGuiWindowFlags_NoScrollbar)) {
        if (title && *title) {
            if (icon && *icon) {
                ImGui::TextColored(accent, "%s %s", icon, title);
            } else {
                ImGui::TextColored(accent, "%s", title);
            }
            ImGui::Separator();
        }
        content();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
    ImGui::PopID();
}

inline bool RenderTouchButton(const char* id,
                              const char* label,
                              const ImVec4& color = UITheme::ColorAccent,
                              const char* icon = nullptr,
                              const ImVec2& requestedSize = ImVec2(0, 0)) {
    const float scale = Detail::GetScale();
    const float minSize = Detail::TouchTargetSize();

    std::string displayLabel = icon && *icon ? (std::string(icon) + "  " + label) : std::string(label);
    ImVec2 size = requestedSize;
    if (size.y < minSize) size.y = minSize;
    if (size.x < minSize * 2.0f && requestedSize.x == 0.0f) {
        size.x = 0.0f; // Let ImGui size the width but respect minimum height
    }

    ImGui::PushID(id);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(color.x, color.y, color.z, 0.75f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(color.x * 1.1f, color.y * 1.1f, color.z * 1.1f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(color.x * 0.9f, color.y * 0.9f, color.z * 0.9f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, UITheme::NavigationButtonRounding());
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UITheme::FramePadding().x + UITheme::TouchPadding().x,
                                                           UITheme::FramePadding().y + UITheme::TouchPadding().y));

    bool pressed = AnimatedButton(displayLabel.c_str(), size);
    ImGuiID imguiId = ImGui::GetID(displayLabel.c_str());
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    if (pressed) {
        TriggerTapFlash(imguiId);
    }
    RenderTapFlashOverlay(imguiId, min, max, color);

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
    ImGui::PopID();
    (void)scale;
    return pressed;
}

inline void RenderProgressBar(const char* id,
                              float value,
                              const ImVec4& fillColor = UITheme::ColorPrimary,
                              const ImVec4& bgColor = UITheme::ColorPanelBg,
                              const char* label = nullptr) {
    const float minHeight = Detail::TouchTargetSize();

    ImGui::PushID(id);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, bgColor);
    ImGui::PushStyleColor(ImGuiCol_PlotHistogram, fillColor);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, UITheme::NavigationButtonRounding());
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(UITheme::ContentPadding() * 0.5f, UITheme::ContentPadding() * 0.35f));

    ImVec2 size = ImVec2(-FLT_MIN, minHeight);
    ImGui::ProgressBar(value, size, label);

    ImGuiID imguiId = ImGui::GetID(id);
    RenderTapFlashOverlay(imguiId, ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), fillColor);

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
    ImGui::PopID();
}

inline bool LongPressInfoToggle(const char* id,
                                const char* infoText,
                                float holdDuration = 0.45f,
                                const ImVec4& accent = UITheme::ColorAccent) {
    struct PressState { float start = 0.0f; bool visible = false; };
    static std::unordered_map<ImGuiID, PressState> states;

    ImGui::PushID(id);
    bool tapped = RenderTouchButton("info", "Info", accent, "\xef\x81\x9a", ImVec2(Detail::TouchTargetSize(), 0));
    ImGuiID btnId = ImGui::GetID("info");

    (void)tapped; // Tapping still gives immediate feedback via flash overlay

    auto& state = states[btnId];
    if (ImGui::IsItemActive()) {
        if (state.start <= 0.0f) {
            state.start = ImGui::GetTime();
        } else if (ImGui::GetTime() - state.start >= holdDuration) {
            state.visible = !state.visible;
            state.start = 0.0f;
        }
    } else {
        state.start = 0.0f;
    }

    if (state.visible && infoText && *infoText) {
        ImGui::SetNextWindowBgAlpha(0.95f);
        if (ImGui::BeginTooltip()) {
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 32.0f);
            ImGui::TextWrapped("%s", infoText);
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    ImGui::PopID();
    return state.visible;
}

} // namespace MobileUI
