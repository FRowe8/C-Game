#include "CombatSystem.h"
#include "Renderer.h"
#include "Logger.h"
#include "Spaceship.h"
#include "GameState.h"
#include <cstdlib>
#include <cmath>

CombatSystem::CombatSystem()
    : m_State(CombatState::NotInCombat), m_CurrentEnemy(nullptr),
      m_PlayerLevel(1), m_PlayerMaxHP(100.0), m_PlayerCurrentHP(100.0),
      m_PlayerAttack(20.0), m_PlayerDefense(10.0), m_PlayerSpeed(15.0),
      m_ShipPowerBonus(0.0), m_ShipCombatBonus(0.0),
      m_TurnTimer(0.0), m_TurnDelay(1.5), m_PlayerDefending(false),
      m_AutoBattle(false), m_BattleSpeed(1.0f),
      m_CreditsEarned(0), m_XPEarned(0), m_PartDropped(false),
      m_CombatTime(0.0) {
}

void CombatSystem::StartCombat(Enemy* enemy, i32 playerLevel, const Spaceship* ship, GameState* state) {
    if (!enemy) {
        Log::Error("Cannot start combat: null enemy");
        return;
    }

    m_CurrentEnemy = enemy;
    m_PlayerLevel = playerLevel;
    m_CombatLog.clear();
    m_CombatTime = 0.0;
    m_TurnTimer = 0.0;
    m_PlayerDefending = false;
    m_CreditsEarned = 0;
    m_XPEarned = 0;
    m_PartDropped = false;

    // Calculate player stats based on level and ship bonuses
    if (ship) {
        m_ShipPowerBonus = ship->GetTotalPowerBonus();
        m_ShipCombatBonus = ship->GetTotalCombatBonus();
    } else {
        m_ShipPowerBonus = 0.0;
        m_ShipCombatBonus = 0.0;
    }

    // Get skill tree bonuses
    f64 skillDamageMultiplier = 1.0;
    f64 skillHPMultiplier = 1.0;
    if (state) {
        skillDamageMultiplier = state->GetSkillTree().GetDamageMultiplier();
        skillHPMultiplier = state->GetSkillTree().GetMaxHPMultiplier();
    }

    // Base stats scale with level
    f64 levelScale = 1.0 + (playerLevel * 0.1); // 10% per level

    m_PlayerMaxHP = 100.0 * levelScale * (1.0 + m_ShipPowerBonus / 100.0) * skillHPMultiplier;
    m_PlayerCurrentHP = m_PlayerMaxHP;
    m_PlayerAttack = 20.0 * levelScale * (1.0 + (m_ShipCombatBonus + m_ShipPowerBonus) / 100.0) * skillDamageMultiplier;
    m_PlayerDefense = 10.0 * levelScale * (1.0 + m_ShipPowerBonus / 100.0);
    m_PlayerSpeed = 15.0 + playerLevel * 0.3;

    // Determine who goes first based on speed
    if (m_PlayerSpeed >= enemy->GetSpeed()) {
        m_State = CombatState::PlayerTurn;
        AddLog("Combat started! You attack first!", Color::NeonCyan());
    } else {
        m_State = CombatState::EnemyTurn;
        AddLog("Combat started! Enemy is faster!", Color(1.0f, 0.6f, 0.3f, 1.0f));
    }

    Log::Infof("Combat started: Player (Lvl ", playerLevel, ") vs ", enemy->GetName(), " (Lvl ", enemy->GetLevel(), ")");
}

void CombatSystem::EndCombat() {
    m_State = CombatState::NotInCombat;
    m_CurrentEnemy = nullptr;
    m_TurnTimer = 0.0;
    m_PlayerDefending = false;

    Log::Info("Combat ended");
}

void CombatSystem::Update(f64 deltaTime) {
    if (!IsInCombat()) return;

    m_CombatTime += deltaTime;

    // Apply battle speed multiplier
    f64 scaledDelta = deltaTime * m_BattleSpeed;

    // Handle victory/defeat states (wait before ending)
    if (m_State == CombatState::Victory || m_State == CombatState::Defeat) {
        m_TurnTimer += scaledDelta;
        if (m_TurnTimer >= 3.0) { // Show result for 3 seconds
            // Combat ends, but don't call EndCombat() yet - let caller handle it
        }
        return;
    }

    // Turn timer
    m_TurnTimer += scaledDelta;

    if (m_TurnTimer >= m_TurnDelay) {
        m_TurnTimer = 0.0;

        // Execute turn based on state
        if (m_State == CombatState::PlayerTurn) {
            if (m_AutoBattle) {
                PlayerAttack(); // Auto-attack
            }
            // Otherwise wait for player input
        } else if (m_State == CombatState::EnemyTurn) {
            ExecuteEnemyTurn();
        }
    }
}

void CombatSystem::PlayerAttack() {
    if (m_State != CombatState::PlayerTurn || !m_CurrentEnemy) return;

    bool isCrit = RollCritical();
    f64 damage = CalculateDamage(m_PlayerAttack, m_CurrentEnemy->GetDefense(), isCrit);

    m_CurrentEnemy->TakeDamage(damage);

    char msg[128];
    if (isCrit) {
        snprintf(msg, sizeof(msg), "You strike for %.0f damage! CRITICAL HIT!", damage);
        AddLog(msg, Color(1.0f, 0.9f, 0.0f, 1.0f));
    } else {
        snprintf(msg, sizeof(msg), "You attack for %.0f damage", damage);
        AddLog(msg, Color(0.7f, 1.0f, 0.7f, 1.0f));
    }

    m_PlayerDefending = false;
    CheckVictoryDefeat();

    if (m_State != CombatState::Victory) {
        m_State = CombatState::EnemyTurn;
    }
}

void CombatSystem::PlayerDefend() {
    if (m_State != CombatState::PlayerTurn) return;

    m_PlayerDefending = true;
    AddLog("You brace for impact! Defense increased.", Color(0.5f, 0.7f, 1.0f, 1.0f));

    m_State = CombatState::EnemyTurn;
}

void CombatSystem::PlayerSpecialAttack() {
    if (m_State != CombatState::PlayerTurn || !m_CurrentEnemy) return;

    // Special attack: 2x damage, always crits
    f64 damage = CalculateDamage(m_PlayerAttack * 2.0, m_CurrentEnemy->GetDefense(), true);
    m_CurrentEnemy->TakeDamage(damage);

    char msg[128];
    snprintf(msg, sizeof(msg), "SPECIAL ATTACK! You unleash %.0f devastating damage!", damage);
    AddLog(msg, Color(1.0f, 0.3f, 1.0f, 1.0f));

    m_PlayerDefending = false;
    CheckVictoryDefeat();

    if (m_State != CombatState::Victory) {
        m_State = CombatState::EnemyTurn;
    }
}

void CombatSystem::PlayerSkip() {
    if (m_State != CombatState::PlayerTurn) return;

    AddLog("You skip your turn", Color(0.6f, 0.6f, 0.6f, 1.0f));
    m_PlayerDefending = false;
    m_State = CombatState::EnemyTurn;
}

void CombatSystem::ExecutePlayerTurn() {
    // This is called when it becomes player's turn
    m_State = CombatState::PlayerTurn;
    m_TurnTimer = 0.0;
}

void CombatSystem::ExecuteEnemyTurn() {
    if (!m_CurrentEnemy || !m_CurrentEnemy->IsAlive()) return;

    bool isCrit = RollCritical();
    f64 enemyAttack = m_CurrentEnemy->GetAttack();

    // Apply defending bonus
    f64 effectiveDefense = m_PlayerDefense;
    if (m_PlayerDefending) {
        effectiveDefense *= 1.5; // 50% more defense when defending
    }

    f64 damage = CalculateDamage(enemyAttack, effectiveDefense, isCrit);

    m_PlayerCurrentHP -= damage;
    if (m_PlayerCurrentHP < 0.0) m_PlayerCurrentHP = 0.0;

    char msg[128];
    if (isCrit) {
        snprintf(msg, sizeof(msg), "%s strikes you for %.0f damage! CRITICAL!", m_CurrentEnemy->GetName(), damage);
        AddLog(msg, Color(1.0f, 0.4f, 0.4f, 1.0f));
    } else {
        snprintf(msg, sizeof(msg), "%s attacks for %.0f damage", m_CurrentEnemy->GetName(), damage);
        AddLog(msg, Color(1.0f, 0.7f, 0.5f, 1.0f));
    }

    if (m_PlayerDefending) {
        AddLog("Your defense absorbed some damage!", Color(0.5f, 0.7f, 1.0f, 1.0f));
    }

    m_PlayerDefending = false;
    CheckVictoryDefeat();

    if (m_State != CombatState::Defeat) {
        m_State = CombatState::PlayerTurn;
    }
}

f64 CombatSystem::CalculateDamage(f64 attack, f64 defense, bool isCritical) {
    // Damage formula: attack * (100 / (100 + defense))
    f64 damageReduction = 100.0 / (100.0 + defense);
    f64 baseDamage = attack * damageReduction;

    // Critical hits deal 2x damage
    if (isCritical) {
        baseDamage *= 2.0;
    }

    // Add random variance (±10%)
    f64 variance = 0.9 + (rand() % 20) / 100.0;
    baseDamage *= variance;

    return baseDamage;
}

bool CombatSystem::RollCritical() const {
    // 15% base critical chance
    return (rand() % 100) < 15;
}

void CombatSystem::CheckVictoryDefeat() {
    if (!m_CurrentEnemy) return;

    if (!m_CurrentEnemy->IsAlive()) {
        m_State = CombatState::Victory;
        AddLog("VICTORY! Enemy defeated!", Color(0.2f, 1.0f, 0.2f, 1.0f));
        GrantRewards();
    } else if (m_PlayerCurrentHP <= 0.0) {
        m_State = CombatState::Defeat;
        AddLog("DEFEAT! You were destroyed...", Color(1.0f, 0.2f, 0.2f, 1.0f));
    }
}

void CombatSystem::GrantRewards() {
    if (!m_CurrentEnemy) return;

    m_CreditsEarned = m_CurrentEnemy->GetCreditReward();
    m_XPEarned = m_CurrentEnemy->GetXPReward();

    // Roll for part drop
    f64 dropRoll = (rand() % 100) / 100.0;
    m_PartDropped = (dropRoll <= m_CurrentEnemy->GetPartDropChance());

    char msg[256];
    snprintf(msg, sizeof(msg), "Earned: %d Credits | %d XP%s",
             m_CreditsEarned, m_XPEarned,
             m_PartDropped ? " | PART DROPPED!" : "");
    AddLog(msg, Color(1.0f, 0.9f, 0.3f, 1.0f));

    Log::Infof("Victory! Rewards: ", m_CreditsEarned, " credits, ", m_XPEarned, " XP",
               m_PartDropped ? ", Part dropped" : "");
}

void CombatSystem::AddLog(const std::string& message, const Color& color) {
    m_CombatLog.push_back(CombatLogEntry(message, color, m_CombatTime));

    // Keep only last 15 messages
    if (m_CombatLog.size() > 15) {
        m_CombatLog.erase(m_CombatLog.begin());
    }
}

void CombatSystem::RenderCombatUI(Renderer* renderer) {
    if (!IsInCombat() || !m_CurrentEnemy) return;

    f32 screenWidth = static_cast<f32>(renderer->GetWidth());
    f32 screenHeight = static_cast<f32>(renderer->GetHeight());

    // Combat background overlay
    Rect bgOverlay(0, 0, screenWidth, screenHeight);
    renderer->DrawRect(bgOverlay, Color(0.0f, 0.0f, 0.05f, 0.7f), true);

    // Main combat panel (centered)
    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    f32 panelX = (screenWidth - panelWidth) * 0.5f;
    f32 panelY = (screenHeight - panelHeight) * 0.5f;

    Rect panelRect(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panelRect, Color(0.05f, 0.05f, 0.1f, 0.95f), true);
    renderer->DrawRect(panelRect, Color::NeonCyan() * 0.7f, false);

    f32 yOffset = panelY + 15.0f;

    // Title
    renderer->DrawText("COMBAT", Vec2(panelX + panelWidth * 0.5f - 60.0f, yOffset), Color::NeonCyan(), 24.0f);
    yOffset += 35.0f;

    // Player section (left side)
    f32 sectionWidth = (panelWidth - 60.0f) * 0.5f;
    f32 playerX = panelX + 20.0f;

    renderer->DrawText("YOUR SHIP", Vec2(playerX, yOffset), Color(0.5f, 1.0f, 0.5f, 1.0f), 18.0f);
    yOffset += 25.0f;

    char playerInfo[128];
    snprintf(playerInfo, sizeof(playerInfo), "Level %d", m_PlayerLevel);
    renderer->DrawText(playerInfo, Vec2(playerX, yOffset), Color::White(), 14.0f);
    yOffset += 25.0f;

    // Player HP bar
    f32 barWidth = sectionWidth - 10.0f;
    f32 barHeight = 30.0f;
    Rect playerHPBg(playerX, yOffset, barWidth, barHeight);
    f32 playerHPPercent = m_PlayerMaxHP > 0.0 ? (m_PlayerCurrentHP / m_PlayerMaxHP) : 0.0;
    Rect playerHPFill(playerX, yOffset, barWidth * playerHPPercent, barHeight);

    renderer->DrawRect(playerHPBg, Color(0.2f, 0.2f, 0.2f, 1.0f), true);
    Color playerHPColor = playerHPPercent > 0.5 ? Color(0.2f, 1.0f, 0.2f, 1.0f) :
                          playerHPPercent > 0.25 ? Color(1.0f, 0.8f, 0.0f, 1.0f) :
                          Color(1.0f, 0.2f, 0.2f, 1.0f);
    renderer->DrawRect(playerHPFill, playerHPColor, true);

    char hpText[64];
    snprintf(hpText, sizeof(hpText), "%.0f / %.0f HP", m_PlayerCurrentHP, m_PlayerMaxHP);
    renderer->DrawText(hpText, Vec2(playerX + barWidth * 0.5f - 50.0f, yOffset + 8.0f), Color::White(), 14.0f);
    yOffset += 40.0f;

    // Player stats
    snprintf(playerInfo, sizeof(playerInfo), "ATK: %.0f | DEF: %.0f", m_PlayerAttack, m_PlayerDefense);
    renderer->DrawText(playerInfo, Vec2(playerX, yOffset), Color(0.7f, 0.7f, 0.9f, 1.0f), 12.0f);
    yOffset += 25.0f;

    // Enemy section (right side)
    f32 enemyX = panelX + panelWidth * 0.5f + 10.0f;
    yOffset = panelY + 50.0f;

    m_CurrentEnemy->Render(renderer, enemyX, yOffset, sectionWidth, 180.0f);

    // Combat log (bottom section)
    yOffset = panelY + 260.0f;
    renderer->DrawText("Combat Log:", Vec2(panelX + 20.0f, yOffset), Color::NeonCyan(), 14.0f);
    yOffset += 22.0f;

    // Draw log entries (last 8 visible)
    i32 startIndex = static_cast<i32>(m_CombatLog.size()) - 8;
    if (startIndex < 0) startIndex = 0;

    for (i32 i = startIndex; i < static_cast<i32>(m_CombatLog.size()); i++) {
        const CombatLogEntry& entry = m_CombatLog[i];
        renderer->DrawText(entry.message.c_str(), Vec2(panelX + 25.0f, yOffset), entry.color, 11.0f);
        yOffset += 16.0f;
    }

    // Action buttons (if player's turn)
    if (m_State == CombatState::PlayerTurn) {
        f32 btnY = panelY + panelHeight - 70.0f;
        f32 btnWidth = 150.0f;
        f32 btnHeight = 50.0f;
        f32 btnSpacing = 20.0f;
        f32 btnStartX = panelX + 30.0f;

        // Attack button
        Rect attackBtn(btnStartX, btnY, btnWidth, btnHeight);
        renderer->DrawRect(attackBtn, Color(0.2f, 0.6f, 0.2f, 0.8f), true);
        renderer->DrawRect(attackBtn, Color(0.3f, 1.0f, 0.3f, 1.0f), false);
        renderer->DrawText("ATTACK", Vec2(btnStartX + 35.0f, btnY + 16.0f), Color::White(), 16.0f);

        // Defend button
        btnStartX += btnWidth + btnSpacing;
        Rect defendBtn(btnStartX, btnY, btnWidth, btnHeight);
        renderer->DrawRect(defendBtn, Color(0.2f, 0.4f, 0.6f, 0.8f), true);
        renderer->DrawRect(defendBtn, Color(0.3f, 0.7f, 1.0f, 1.0f), false);
        renderer->DrawText("DEFEND", Vec2(btnStartX + 35.0f, btnY + 16.0f), Color::White(), 16.0f);

        // Special button
        btnStartX += btnWidth + btnSpacing;
        Rect specialBtn(btnStartX, btnY, btnWidth, btnHeight);
        renderer->DrawRect(specialBtn, Color(0.6f, 0.2f, 0.6f, 0.8f), true);
        renderer->DrawRect(specialBtn, Color(1.0f, 0.3f, 1.0f, 1.0f), false);
        renderer->DrawText("SPECIAL", Vec2(btnStartX + 32.0f, btnY + 16.0f), Color::White(), 16.0f);
    }

    // Auto-battle toggle (bottom left)
    f32 autoX = panelX + 20.0f;
    f32 autoY = panelY + panelHeight - 25.0f;
    Color autoColor = m_AutoBattle ? Color(0.2f, 1.0f, 0.2f, 1.0f) : Color(0.6f, 0.6f, 0.6f, 1.0f);
    char autoText[64];
    snprintf(autoText, sizeof(autoText), "[Auto: %s] [Speed: %.0fx]", m_AutoBattle ? "ON" : "OFF", m_BattleSpeed);
    renderer->DrawText(autoText, Vec2(autoX, autoY), autoColor, 12.0f);

    // Victory/Defeat overlay
    if (m_State == CombatState::Victory) {
        Rect victoryOverlay(panelX, panelY, panelWidth, panelHeight);
        renderer->DrawRect(victoryOverlay, Color(0.0f, 0.3f, 0.0f, 0.7f), true);
        renderer->DrawText("VICTORY!", Vec2(panelX + panelWidth * 0.5f - 80.0f, panelY + panelHeight * 0.5f - 20.0f),
                         Color(0.3f, 1.0f, 0.3f, 1.0f), 32.0f);
    } else if (m_State == CombatState::Defeat) {
        Rect defeatOverlay(panelX, panelY, panelWidth, panelHeight);
        renderer->DrawRect(defeatOverlay, Color(0.3f, 0.0f, 0.0f, 0.7f), true);
        renderer->DrawText("DEFEAT", Vec2(panelX + panelWidth * 0.5f - 70.0f, panelY + panelHeight * 0.5f - 20.0f),
                         Color(1.0f, 0.3f, 0.3f, 1.0f), 32.0f);
    }
}

void CombatSystem::HandleClick(f32 mouseX, f32 mouseY, bool mousePressed) {
    if (!IsInCombat() || m_State != CombatState::PlayerTurn) return;

    f32 screenWidth = 1920.0f; // TODO: Get from renderer
    f32 screenHeight = 1080.0f;

    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    f32 panelX = (screenWidth - panelWidth) * 0.5f;
    f32 panelY = (screenHeight - panelHeight) * 0.5f;

    f32 btnY = panelY + panelHeight - 70.0f;
    f32 btnWidth = 150.0f;
    f32 btnHeight = 50.0f;
    f32 btnSpacing = 20.0f;

    // Attack button
    Rect attackBtn(panelX + 30.0f, btnY, btnWidth, btnHeight);
    if (mouseX >= attackBtn.x && mouseX <= attackBtn.x + attackBtn.width &&
        mouseY >= attackBtn.y && mouseY <= attackBtn.y + attackBtn.height) {
        if (mousePressed) {
            PlayerAttack();
        }
        return;
    }

    // Defend button
    Rect defendBtn(panelX + 30.0f + btnWidth + btnSpacing, btnY, btnWidth, btnHeight);
    if (mouseX >= defendBtn.x && mouseX <= defendBtn.x + defendBtn.width &&
        mouseY >= defendBtn.y && mouseY <= defendBtn.y + defendBtn.height) {
        if (mousePressed) {
            PlayerDefend();
        }
        return;
    }

    // Special button
    Rect specialBtn(panelX + 30.0f + (btnWidth + btnSpacing) * 2, btnY, btnWidth, btnHeight);
    if (mouseX >= specialBtn.x && mouseX <= specialBtn.x + specialBtn.width &&
        mouseY >= specialBtn.y && mouseY <= specialBtn.y + specialBtn.height) {
        if (mousePressed) {
            PlayerSpecialAttack();
        }
        return;
    }
}
