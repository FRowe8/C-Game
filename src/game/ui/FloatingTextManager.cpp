#include "FloatingTextManager.h"
#include "UITheme.h"
#include "GameUtils.h"
#include "Renderer.h"
#include <cmath>
#include <algorithm>

namespace UI {

// =============================================================================
// FloatingText Implementation
// =============================================================================

FloatingText::FloatingText(const std::string& txt, const Vec2& pos, const ImVec4& col, float life)
    : text(txt)
    , position(pos)
    , velocity(0.0f, -50.0f) // Move upward at 50 pixels/sec
    , color(col)
    , lifetime(life)
    , timeRemaining(life)
    , scale(0.0f) // Start at 0 for pop-in effect
{
}

void FloatingText::Update(float deltaTime) {
    // Update position
    position.x += velocity.x * deltaTime;
    position.y += velocity.y * deltaTime;

    // Update lifetime
    timeRemaining -= deltaTime;

    // Pop-in animation (first 0.1s)
    if (lifetime - timeRemaining < 0.1f) {
        scale = (lifetime - timeRemaining) / 0.1f; // 0 to 1 over 0.1s
    } else {
        scale = 1.0f;
    }

    // Slow down velocity over time (ease out)
    velocity.y *= 0.98f;
}

// =============================================================================
// FloatingTextManager Implementation
// =============================================================================

FloatingTextManager::FloatingTextManager() {
}

void FloatingTextManager::SpawnText(const std::string& text, const Vec2& position, const ImVec4& color, float lifetime) {
    m_Texts.emplace_back(text, position, color, lifetime);
}

void FloatingTextManager::SpawnResourceText(QuantumResource resourceType, f64 amount, const Vec2& position,
                                           GameUtils::NumberFormat format) {
    ImVec4 color = GetResourceColor(resourceType);
    std::string text = FormatResourceText(resourceType, amount, format);
    SpawnText(text, position, color, 2.0f);
}

void FloatingTextManager::Update(float deltaTime) {
    // Update all texts
    for (auto& text : m_Texts) {
        text.Update(deltaTime);
    }

    // Remove dead texts
    m_Texts.erase(
        std::remove_if(m_Texts.begin(), m_Texts.end(),
            [](const FloatingText& text) { return !text.IsAlive(); }),
        m_Texts.end()
    );
}

void FloatingTextManager::Render(Renderer* renderer) {
    (void)renderer; // Unused - using ImGui

    ImDrawList* drawList = ImGui::GetForegroundDrawList();

    for (const auto& text : m_Texts) {
        // Calculate fade-out alpha
        float alpha = text.timeRemaining / text.lifetime;
        if (alpha > 1.0f) alpha = 1.0f;
        if (alpha < 0.0f) alpha = 0.0f;

        // Calculate scale (pop-in + slight pulse)
        float scale = text.scale * (1.0f + sinf(text.timeRemaining * 3.0f) * 0.05f);

        // Apply alpha to color
        ImVec4 color = text.color;
        color.w *= alpha;

        // Calculate text size for centering
        ImVec2 textSize = ImGui::CalcTextSize(text.text.c_str());
        textSize.x *= scale;
        textSize.y *= scale;

        ImVec2 renderPos(
            text.position.x - textSize.x * 0.5f,
            text.position.y - textSize.y * 0.5f
        );

        // Draw text with outline for better visibility
        ImU32 colorU32 = ImGui::ColorConvertFloat4ToU32(color);
        ImU32 outlineColor = ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, color.w));

        // Outline (draw 4 times offset by 1 pixel)
        drawList->AddText(ImVec2(renderPos.x - 1, renderPos.y), outlineColor, text.text.c_str());
        drawList->AddText(ImVec2(renderPos.x + 1, renderPos.y), outlineColor, text.text.c_str());
        drawList->AddText(ImVec2(renderPos.x, renderPos.y - 1), outlineColor, text.text.c_str());
        drawList->AddText(ImVec2(renderPos.x, renderPos.y + 1), outlineColor, text.text.c_str());

        // Main text
        drawList->AddText(renderPos, colorU32, text.text.c_str());
    }
}

void FloatingTextManager::Clear() {
    m_Texts.clear();
}

ImVec4 FloatingTextManager::GetResourceColor(QuantumResource type) const {
    switch (type) {
        case QuantumResource::Qubits:
            return ImVec4(0.4f, 0.7f, 1.0f, 1.0f); // Light blue
        case QuantumResource::Coherence:
            return UITheme::ColorAccent; // Cyan
        case QuantumResource::Entanglement:
            return ImVec4(0.9f, 0.5f, 1.0f, 1.0f); // Purple
        default:
            return UITheme::ColorText; // White
    }
}

std::string FloatingTextManager::FormatResourceText(QuantumResource type, f64 amount,
                                                   GameUtils::NumberFormat format) const {
    std::string prefix = (amount > 0) ? "+" : "";
    std::string formatted = prefix + GameUtils::FormatNumber(amount, format);

    switch (type) {
        case QuantumResource::Qubits:
            return formatted + " Qubits";
        case QuantumResource::Coherence:
            return formatted + " Coherence";
        case QuantumResource::Entanglement:
            return formatted + " Entanglement";
        default:
            return formatted;
    }
}

} // namespace UI
