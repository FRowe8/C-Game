#include "VisualFXManager.h"
#include "Renderer.h"
#include "GameUtils.h"
#include "Log.h"
#include "imgui.h"
#include "ImGuiUtils.h"
#include <algorithm>
#include <cmath>

VisualFXManager::VisualFXManager()
    : m_ParticlesEnabled(true)
    , m_ParticleScale(1.0f)
    , m_AnomaliesEnabled(true)
    , m_AnomalyScale(1.0f)
    , m_PrestigeFlashActive(false)
    , m_PrestigeFlashTimer(0.0)
    , m_PrestigeFlashDuration(0.5)
    , m_ScreenShakeActive(false)
    , m_ScreenShakeIntensity(0.0f)
    , m_ScreenShakeTimer(0.0f)
    , m_ScreenShakeDuration(0.0f)
    , m_ComboCount(0)
{
}

void VisualFXManager::Initialize() {
    m_Particles.clear();
    m_Anomalies.clear();
    m_FloatingTexts.clear();
    m_ComboCount = 0;
    m_PrestigeFlashActive = false;
    m_ScreenShakeActive = false;
}

void VisualFXManager::Update(f64 deltaTime) {
    UpdateParticles(deltaTime);
    UpdateAnomalies(deltaTime);
    UpdateScreenEffects(deltaTime);
    UpdateFloatingTexts(deltaTime);
}

void VisualFXManager::Render(Renderer* renderer) {
    RenderParticles(renderer);
    RenderAnomalies(renderer);
    RenderFloatingTexts(renderer);
}

// === Particle System ===

void VisualFXManager::SpawnParticle(const Vec2& position, const Color& color, f64 lifetime) {
    if (!m_ParticlesEnabled) return;

    Particle p;
    p.position = position;
    p.velocity = Vec2(
        static_cast<f32>(GameUtils::RandomRange(-50.0, 50.0)),
        static_cast<f32>(GameUtils::RandomRange(-100.0, -50.0))
    );
    p.color = color;
    p.lifetime = 0.0f;
    p.maxLifetime = static_cast<f32>(lifetime);
    m_Particles.push_back(p);
}

void VisualFXManager::SpawnParticleBurst(const Vec2& position, const Color& color, i32 count) {
    if (!m_ParticlesEnabled) return;

    for (i32 i = 0; i < count; i++) {
        SpawnParticle(position, color, GameUtils::RandomRange(0.5, 1.5));
    }
}

void VisualFXManager::UpdateParticles(f64 deltaTime) {
    // Update and remove dead particles
    auto it = m_Particles.begin();
    while (it != m_Particles.end()) {
        it->lifetime += static_cast<f32>(deltaTime);
        it->position.x += it->velocity.x * static_cast<f32>(deltaTime);
        it->position.y += it->velocity.y * static_cast<f32>(deltaTime);

        // Apply gravity
        it->velocity.y += 200.0f * static_cast<f32>(deltaTime);

        // Fade out
        f32 alpha = 1.0f - (it->lifetime / it->maxLifetime);
        it->color.a = alpha;

        // Remove if dead
        if (it->lifetime >= it->maxLifetime) {
            it = m_Particles.erase(it);
        } else {
            ++it;
        }
    }

    // Limit particle count to prevent lag
    if (m_Particles.size() > 500) {
        m_Particles.erase(m_Particles.begin(), m_Particles.begin() + 100);
    }
}

void VisualFXManager::RenderParticles(Renderer* renderer) {
    (void)renderer; // Using ImGui directly

    if (!m_ParticlesEnabled) return;

    ImDrawList* bg_draw_list = ImGui::GetBackgroundDrawList();

    for (const auto& particle : m_Particles) {
        f32 size = (3.0f + (1.0f - particle.lifetime / particle.maxLifetime) * 3.0f) * m_ParticleScale;

        bg_draw_list->AddCircleFilled(
            ToImVec2(particle.position),
            size,
            ToImU32(particle.color)
        );
    }
}

// === Quantum Anomaly System ===

void VisualFXManager::SpawnQuantumAnomaly() {
    if (!m_AnomaliesEnabled) return;

    QuantumAnomaly anomaly;

    // Random position on screen (with margins)
    f32 screenWidth = ImGui::GetIO().DisplaySize.x;
    f32 screenHeight = ImGui::GetIO().DisplaySize.y;

    f32 margin = 100.0f;
    anomaly.position.x = static_cast<f32>(GameUtils::RandomRange(margin, screenWidth - margin));
    anomaly.position.y = static_cast<f32>(GameUtils::RandomRange(margin + 100.0f, screenHeight - margin - 100.0f));

    // Random size
    anomaly.radius = static_cast<f32>(GameUtils::RandomRange(30.0, 50.0)) * m_AnomalyScale;

    // Lifetime: 5-10 seconds
    anomaly.maxLifetime = static_cast<f32>(GameUtils::RandomRange(5.0, 10.0));
    anomaly.lifetime = 0.0f;

    // Reward based on rarity
    i32 rarity = rand() % 100;
    if (rarity < 60) {
        // Common: 10-30x production
        anomaly.rewardMultiplier = 10.0 + (rand() % 21);
        anomaly.color = Color(0.0f, 0.8f, 1.0f, 1.0f); // Cyan
    } else if (rarity < 85) {
        // Uncommon: 30-60x production
        anomaly.rewardMultiplier = 30.0 + (rand() % 31);
        anomaly.color = Color(0.2f, 1.0f, 0.2f, 1.0f); // Green
    } else if (rarity < 95) {
        // Rare: 60-100x production
        anomaly.rewardMultiplier = 60.0 + (rand() % 41);
        anomaly.color = Color(1.0f, 0.0f, 1.0f, 1.0f); // Magenta
    } else {
        // Legendary: 100-200x production
        anomaly.rewardMultiplier = 100.0 + (rand() % 101);
        anomaly.color = Color(1.0f, 0.8f, 0.0f, 1.0f); // Gold
    }

    anomaly.clicked = false;
    m_Anomalies.push_back(anomaly);

    Log::Infof("Quantum Anomaly spawned! Reward: ", anomaly.rewardMultiplier, "x");
}

f64 VisualFXManager::ClickQuantumAnomaly(const Vec2& clickPos) {
    for (auto& anomaly : m_Anomalies) {
        if (anomaly.clicked) continue;

        // Check if click is within anomaly circle
        f32 dx = clickPos.x - anomaly.position.x;
        f32 dy = clickPos.y - anomaly.position.y;
        f32 distSq = dx * dx + dy * dy;
        f32 radiusSq = anomaly.radius * anomaly.radius;

        if (distSq <= radiusSq) {
            // Clicked! Mark as clicked and return reward multiplier
            anomaly.clicked = true;

            // Visual feedback: particle burst
            SpawnParticleBurst(anomaly.position, anomaly.color, 30);

            // Increment combo
            IncrementCombo();

            Log::Infof("Anomaly clicked! Reward multiplier: ", anomaly.rewardMultiplier, "x");

            return anomaly.rewardMultiplier;
        }
    }

    return 0.0; // Missed
}

void VisualFXManager::UpdateAnomalies(f64 deltaTime) {
    // Update existing anomalies
    for (auto it = m_Anomalies.begin(); it != m_Anomalies.end();) {
        it->lifetime += deltaTime;

        // Remove if expired or clicked
        if (it->lifetime >= it->maxLifetime || it->clicked) {
            // If expired without clicking, it's a miss
            if (!it->clicked) {
                Log::Debug("Anomaly expired without being clicked");
            }
            it = m_Anomalies.erase(it);
        } else {
            ++it;
        }
    }
}

void VisualFXManager::RenderAnomalies(Renderer* renderer) {
    (void)renderer; // Using ImGui directly

    if (!m_AnomaliesEnabled) return;

    ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

    for (const auto& anomaly : m_Anomalies) {
        if (anomaly.clicked) continue; // Don't render clicked anomalies

        f32 lifeRatio = 1.0f - (anomaly.lifetime / anomaly.maxLifetime);

        // Pulsing effect
        f32 pulse = 1.0f + 0.2f * std::sin(static_cast<f32>(anomaly.lifetime * 3.0));
        f32 currentRadius = anomaly.radius * pulse;

        ImVec2 anomalyPos(anomaly.position.x, anomaly.position.y);

        // 1. Outer glow
        Color glowColor = anomaly.color;
        glowColor.a = 0.3f;
        draw_list->AddCircleFilled(anomalyPos, currentRadius * 1.3f, ToImU32(glowColor), 32);

        // 2. Main circle
        draw_list->AddCircleFilled(anomalyPos, currentRadius, ToImU32(anomaly.color), 32);

        // 3. Border
        Color borderColor = Color::White();
        borderColor.a = 0.8f;
        draw_list->AddCircle(anomalyPos, currentRadius, ToImU32(borderColor), 32, 2.0f);

        // 4. Show reward multiplier text above it
        std::string rewardText = std::to_string(static_cast<i32>(anomaly.rewardMultiplier)) + "x";

        ImVec2 textPos(anomalyPos.x - 15.0f, anomalyPos.y - currentRadius - 20.0f);
        draw_list->AddText(textPos, ToImU32(Color::White()), rewardText.c_str());

        // 5. Lifetime bar below it
        f32 barWidth = anomaly.radius * 2.0f;
        f32 barHeight = 4.0f;

        ImVec2 barPosStart(
            anomalyPos.x - barWidth / 2.0f,
            anomalyPos.y + currentRadius + 10.0f
        );
        ImVec2 barPosEnd(
            barPosStart.x + barWidth,
            barPosStart.y + barHeight
        );

        // Background bar
        draw_list->AddRectFilled(barPosStart, barPosEnd, ToImU32(Color(0.2f, 0.2f, 0.2f, 0.8f)));

        // Fill bar
        ImVec2 barFillEnd(barPosStart.x + barWidth * lifeRatio, barPosStart.y + barHeight);
        draw_list->AddRectFilled(barPosStart, barFillEnd, ToImU32(anomaly.color));
    }
}

bool VisualFXManager::ShouldSpawnAnomaly() const {
    // Anomaly spawning logic is handled by TimeManager
    // This just checks if we have room for more
    return m_Anomalies.size() < 3; // Max 3 anomalies at once
}

// === Screen Effects ===

f64 VisualFXManager::GetPrestigeFlashAlpha() const {
    if (!m_PrestigeFlashActive) return 0.0;

    // Fade from 0.3 to 0 over duration
    f64 progress = m_PrestigeFlashTimer / m_PrestigeFlashDuration;
    return 0.3 * progress;
}

void VisualFXManager::TriggerPrestigeFlash() {
    m_PrestigeFlashActive = true;
    m_PrestigeFlashTimer = m_PrestigeFlashDuration;
}

void VisualFXManager::TriggerScreenShake(f32 intensity, f32 duration) {
    m_ScreenShakeActive = true;
    m_ScreenShakeIntensity = intensity;
    m_ScreenShakeTimer = duration;
    m_ScreenShakeDuration = duration;
}

Vec2 VisualFXManager::GetScreenShakeOffset() const {
    if (!m_ScreenShakeActive) return Vec2(0.0f, 0.0f);

    // Random offset based on intensity
    f32 progress = m_ScreenShakeTimer / m_ScreenShakeDuration;
    f32 currentIntensity = m_ScreenShakeIntensity * progress;

    return Vec2(
        static_cast<f32>(GameUtils::RandomRange(-currentIntensity, currentIntensity)),
        static_cast<f32>(GameUtils::RandomRange(-currentIntensity, currentIntensity))
    );
}

void VisualFXManager::UpdateScreenEffects(f64 deltaTime) {
    // Update prestige flash
    if (m_PrestigeFlashActive) {
        m_PrestigeFlashTimer -= deltaTime;
        if (m_PrestigeFlashTimer <= 0.0) {
            m_PrestigeFlashActive = false;
            m_PrestigeFlashTimer = 0.0;
        }
    }

    // Update screen shake
    if (m_ScreenShakeActive) {
        m_ScreenShakeTimer -= deltaTime;
        if (m_ScreenShakeTimer <= 0.0f) {
            m_ScreenShakeActive = false;
            m_ScreenShakeTimer = 0.0f;
        }
    }
}

// === Combo Display ===

void VisualFXManager::IncrementCombo() {
    m_ComboCount++;
}

f32 VisualFXManager::GetComboAlpha() const {
    // Combo fades based on TimeManager's combo time remaining
    // For now, just return full alpha if combo is active
    return m_ComboCount > 0 ? 1.0f : 0.0f;
}

// === Floating Combat Text ===

void VisualFXManager::SpawnFloatingText(const std::string& text, const Vec2& position, FloatingTextType type) {
    FloatingText ft;
    ft.text = text;
    ft.position = position;
    ft.type = type;
    ft.lifetime = 0.0f;
    ft.maxLifetime = 1.5f; // Default 1.5 seconds

    // Set properties based on type
    switch (type) {
        case FloatingTextType::Damage:
            ft.color = Color(1.0f, 1.0f, 0.8f, 1.0f); // Light yellow
            ft.size = 1.0f;
            ft.velocity = Vec2(0.0f, -50.0f); // Float upward
            break;

        case FloatingTextType::CriticalHit:
            ft.color = Color(1.0f, 0.3f, 0.0f, 1.0f); // Orange-red
            ft.size = 1.5f; // 50% larger
            ft.velocity = Vec2(0.0f, -80.0f); // Float faster
            ft.maxLifetime = 2.0f; // Last longer
            break;

        case FloatingTextType::Healing:
            ft.color = Color(0.2f, 1.0f, 0.3f, 1.0f); // Bright green
            ft.size = 1.2f;
            ft.velocity = Vec2(0.0f, -60.0f);
            break;

        case FloatingTextType::Miss:
            ft.color = Color(0.6f, 0.6f, 0.6f, 1.0f); // Gray
            ft.size = 0.8f; // Smaller
            ft.velocity = Vec2(0.0f, -30.0f); // Float slower
            ft.maxLifetime = 1.0f; // Shorter
            break;

        case FloatingTextType::Blocked:
            ft.color = Color(0.3f, 0.6f, 1.0f, 1.0f); // Blue
            ft.size = 1.0f;
            ft.velocity = Vec2(0.0f, -40.0f);
            break;

        case FloatingTextType::ResourceGain:
            ft.color = Color(0.2f, 1.0f, 1.0f, 1.0f); // Cyan
            ft.size = 1.0f;
            ft.velocity = Vec2(0.0f, -70.0f);
            break;

        case FloatingTextType::XPGain:
            ft.color = Color(0.8f, 0.4f, 1.0f, 1.0f); // Purple
            ft.size = 1.1f;
            ft.velocity = Vec2(0.0f, -55.0f);
            break;

        case FloatingTextType::LevelUp:
            ft.color = Color(1.0f, 0.9f, 0.2f, 1.0f); // Gold
            ft.size = 2.0f; // Double size
            ft.velocity = Vec2(0.0f, -100.0f);
            ft.maxLifetime = 2.5f; // Last longest
            break;
    }

    m_FloatingTexts.push_back(ft);
}

void VisualFXManager::SpawnDamageText(f64 damage, const Vec2& position, bool isCritical) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%.0f", damage);

    if (isCritical) {
        std::string critText = std::string(buffer) + "!";
        SpawnFloatingText(critText, position, FloatingTextType::CriticalHit);
    } else {
        SpawnFloatingText(buffer, position, FloatingTextType::Damage);
    }
}

void VisualFXManager::SpawnHealText(f64 healing, const Vec2& position) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "+%.0f HP", healing);
    SpawnFloatingText(buffer, position, FloatingTextType::Healing);
}

void VisualFXManager::SpawnMissText(const Vec2& position) {
    SpawnFloatingText("MISS", position, FloatingTextType::Miss);
}

void VisualFXManager::SpawnBlockedText(const Vec2& position) {
    SpawnFloatingText("BLOCKED", position, FloatingTextType::Blocked);
}

void VisualFXManager::SpawnResourceText(f64 amount, const std::string& resourceName, const Vec2& position) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "+%.0f %s", amount, resourceName.c_str());
    SpawnFloatingText(buffer, position, FloatingTextType::ResourceGain);
}

void VisualFXManager::SpawnXPText(i32 xp, const Vec2& position) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "+%d XP", xp);
    SpawnFloatingText(buffer, position, FloatingTextType::XPGain);
}

void VisualFXManager::SpawnLevelUpText(i32 newLevel, const Vec2& position) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "LEVEL %d!", newLevel);
    SpawnFloatingText(buffer, position, FloatingTextType::LevelUp);
}

void VisualFXManager::UpdateFloatingTexts(f64 deltaTime) {
    for (auto& text : m_FloatingTexts) {
        text.lifetime += static_cast<f32>(deltaTime);

        // Move text upward
        text.position.x += text.velocity.x * static_cast<f32>(deltaTime);
        text.position.y += text.velocity.y * static_cast<f32>(deltaTime);

        // Fade out near end of lifetime
        f32 alpha = 1.0f;
        if (text.lifetime > text.maxLifetime * 0.7f) {
            f32 fadeProgress = (text.lifetime - text.maxLifetime * 0.7f) / (text.maxLifetime * 0.3f);
            alpha = 1.0f - fadeProgress;
        }
        text.color.a = alpha;
    }

    // Remove expired texts
    m_FloatingTexts.erase(
        std::remove_if(m_FloatingTexts.begin(), m_FloatingTexts.end(),
            [](const FloatingText& text) {
                return text.lifetime >= text.maxLifetime;
            }),
        m_FloatingTexts.end()
    );
}

void VisualFXManager::RenderFloatingTexts(Renderer* renderer) {
    ImDrawList* draw_list = ImGui::GetForegroundDrawList();

    for (const auto& text : m_FloatingTexts) {
        ImVec2 textPos(text.position.x, text.position.y);

        // Calculate text size
        f32 fontSize = 20.0f * text.size;
        ImFont* font = ImGui::GetFont();
        ImVec2 textSize = font->CalcTextSizeA(fontSize, FLT_MAX, 0.0f, text.text.c_str());

        // Center text horizontally
        textPos.x -= textSize.x * 0.5f;

        // Draw text with outline for better visibility
        ImU32 outlineColor = IM_COL32(0, 0, 0, static_cast<int>(text.color.a * 255));
        ImU32 textColor = IM_COL32(
            static_cast<int>(text.color.r * 255),
            static_cast<int>(text.color.g * 255),
            static_cast<int>(text.color.b * 255),
            static_cast<int>(text.color.a * 255)
        );

        // Draw outline (4 directions)
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                if (dx == 0 && dy == 0) continue;
                ImVec2 outlinePos(textPos.x + dx, textPos.y + dy);
                draw_list->AddText(font, fontSize, outlinePos, outlineColor, text.text.c_str());
            }
        }

        // Draw main text
        draw_list->AddText(font, fontSize, textPos, textColor, text.text.c_str());
    }
}
