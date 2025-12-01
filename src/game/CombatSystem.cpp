#include "CombatSystem.h"
#include "Renderer.h"
#include "Logger.h"
#include "ImGuiUtils.h"
#include "Spaceship.h"
#include "GameState.h"
#include "imgui.h" // ADDED: Necessary for ImGui integration
#include <cstdlib>
#include <cmath>
#include <sstream>


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

void CombatSystem::HandleClick(float mouseX, float mouseY, bool mousePressed) const {
    // We are using ImGui buttons (ATTACK, DEFEND, SPECIAL) inside RenderCombatUI
    // to handle all player turn input.
    // However, this function is still called by GameState.cpp.
    // If you had active combat elements (like clicking the enemy sprite itself)
    // that were not ImGui buttons, that logic would go here.

    // For now, we will leave it empty as a placeholder to satisfy the linker.
    (void)mouseX;
    (void)mouseY;
    (void)mousePressed;

    if (!IsInCombat()) {
        return;
    }

    // Optional future logic:
    // If m_State == CombatState::PlayerTurn && !m_AutoBattle:
    //    if (Click hit enemy boundary) { PlayerAttack(); }
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

// ====================================================================
// NEW IMGUI RENDERING
// ====================================================================

void CombatSystem::RenderCombatUI(Renderer* renderer) {
    if (!IsInCombat() || !m_CurrentEnemy) return;

    // --- Panel setup (matching old dimensions) ---
    f32 screenWidth = static_cast<f32>(renderer->GetWidth());
    f32 screenHeight = static_cast<f32>(renderer->GetHeight());
    f32 panelWidth = 900.0f;
    f32 panelHeight = 600.0f;
    f32 panelX = (screenWidth - panelWidth) * 0.5f;
    f32 panelY = (screenHeight - panelHeight) * 0.5f;

    // 1. Setup position and size
    ImGui::SetNextWindowPos(ImVec2(panelX, panelY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight), ImGuiCond_Always);

    // 2. Setup styles
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ToImVec4(Color(0.05f, 0.05f, 0.1f, 0.95f)));
    ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color::NeonCyan() * 0.7f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));

    // 3. Begin the main combat window
    if (ImGui::Begin("##CombatPanel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar)) {

        // --- Header and Close Button ---
        ImGui::SetCursorPosX(panelWidth * 0.5f - ImGui::CalcTextSize("COMBAT").x * 0.5f);
        ImGui::TextColored(ToImVec4(Color::NeonCyan()), "COMBAT");

        // Close button (top right corner logic)
        f32 closeBtnSize = 44.0f;
        ImGui::SetCursorPos(ImVec2(panelWidth - closeBtnSize - 10.0f, 10.0f));
        ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(Color(0.3f, 0.1f, 0.1f, 0.8f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(Color(0.5f, 0.2f, 0.2f, 1.0f)));
        ImGui::PushStyleColor(ImGuiCol_Border, ToImVec4(Color(1.0f, 0.3f, 0.3f, 1.0f)));

        if (ImGui::Button("X##CloseCombat", ImVec2(closeBtnSize, closeBtnSize))) {
            // Note: The caller (GameState) must detect this and call EndCombat
            // We can return a flag or set a state, but for this file, just let the click register.
            // A more complete system would manage a `bool* p_open` passed to ImGui::Begin.
        }
        ImGui::PopStyleColor(3);

        ImGui::Separator();

        // --- Player and Enemy Sections (Side-by-Side) ---
        f32 sectionWidth = (panelWidth - 40.0f) * 0.5f; // Adjusted width for cleaner SameLine

        // Player Section (Left)
        ImGui::BeginGroup();
        {
            ImGui::TextColored(ToImVec4(Color(0.5f, 1.0f, 0.5f, 1.0f)), "YOUR SHIP");
            ImGui::TextColored(ToImVec4(Color::White()), "Level %d", m_PlayerLevel);
            ImGui::Spacing();

            // Player HP bar
            float playerHPPercent = m_PlayerMaxHP > 0.0 ? (m_PlayerCurrentHP / m_PlayerMaxHP) : 0.0f;
            Color playerHPColor = playerHPPercent > 0.5f ? Color(0.2f, 1.0f, 0.2f, 1.0f) :
                                  playerHPPercent > 0.25f ? Color(1.0f, 0.8f, 0.0f, 1.0f) :
                                  Color(1.0f, 0.2f, 0.2f, 1.0f);

            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ToImVec4(playerHPColor));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ToImVec4(Color(0.2f, 0.2f, 0.2f, 1.0f)));

            char hpText[64];
            snprintf(hpText, sizeof(hpText), "%.0f / %.0f HP", m_PlayerCurrentHP, m_PlayerMaxHP);

            ImGui::ProgressBar(playerHPPercent, ImVec2(sectionWidth, 30.0f), hpText);

            ImGui::PopStyleColor(2);
            ImGui::Spacing();

            // Player stats
            ImGui::TextColored(ToImVec4(Color(0.7f, 0.7f, 0.9f, 1.0f)), "ATK: %.0f | DEF: %.0f | SPD: %.1f", m_PlayerAttack, m_PlayerDefense, m_PlayerSpeed);
            if (m_PlayerDefending) {
                ImGui::TextColored(ToImVec4(Color(0.5f, 0.7f, 1.0f, 1.0f)), "DEFENDING");
            }
            ImGui::Spacing();
        }
        ImGui::EndGroup(); // End Player Section

        ImGui::SameLine(sectionWidth + 20.0f); // Position Enemy section immediately after Player section

        // Enemy Section (Right)
        ImGui::BeginGroup();
        {
            // Since m_CurrentEnemy->Render is assumed to be converted to use ImGui::BeginChild/ImGui::Text,
            // we pass coordinates relative to the screen to draw the enemy panel correctly.
            // Note: The parameters (x, y, w, h) are largely vestigial if the Enemy::Render fully uses ImGui layout,
            // but we keep the call structure for now.
            // We pass the start position of the right column group.
            m_CurrentEnemy->Render(renderer, panelX + sectionWidth + 20.0f, panelY + 50.0f, sectionWidth, 180.0f);
        }
        ImGui::EndGroup(); // End Enemy Section

        // --- Combat Log ---
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 20.0f); // Add separation
        ImGui::Separator();
        ImGui::TextColored(ToImVec4(Color::NeonCyan()), "Combat Log:");

        // Use a Child Window for the scrollable log region
        f32 logHeight = panelHeight - ImGui::GetCursorPosY() - 100.0f; // Calculate remaining space for log and buttons
        if (ImGui::BeginChild("LogRegion", ImVec2(0, logHeight), true)) {
            for (const auto& entry : m_CombatLog) {
                ImGui::TextColored(ToImVec4(entry.color), "%.1fs: %s", entry.timestamp, entry.message.c_str());
            }
            if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f); // Auto-scroll
        }
        ImGui::EndChild();
        ImGui::Spacing();

        // --- Action Buttons and Controls ---
        ImGui::Separator();

        // Action Buttons (Only visible on PlayerTurn)
        if (m_State == CombatState::PlayerTurn) {
            f32 btnWidth = 150.0f;
            f32 btnHeight = 50.0f;

            // ATTACK
            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(Color(0.2f, 0.6f, 0.2f, 0.8f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(Color(0.3f, 1.0f, 0.3f, 1.0f)));
            if (ImGui::Button("ATTACK", ImVec2(btnWidth, btnHeight))) {
                PlayerAttack();
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine();

            // DEFEND
            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(Color(0.2f, 0.4f, 0.6f, 0.8f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(Color(0.3f, 0.7f, 1.0f, 1.0f)));
            if (ImGui::Button("DEFEND", ImVec2(btnWidth, btnHeight))) {
                PlayerDefend();
            }
            ImGui::PopStyleColor(2);
            ImGui::SameLine();

            // SPECIAL
            ImGui::PushStyleColor(ImGuiCol_Button, ToImVec4(Color(0.6f, 0.2f, 0.6f, 0.8f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ToImVec4(Color(1.0f, 0.3f, 1.0f, 1.0f)));
            if (ImGui::Button("SPECIAL", ImVec2(btnWidth, btnHeight))) {
                PlayerSpecialAttack();
            }
            ImGui::PopStyleColor(2);
        } else {
            // Display waiting text if not player turn
            ImGui::TextColored(ToImVec4(Color(0.6f, 0.6f, 0.6f, 1.0f)), "Waiting for opponent...");
        }

        // --- Auto-battle and Speed Controls (Bottom Right) ---
        ImGui::SameLine(panelWidth - 300.0f);

        // Auto Battle Toggle
        if (ImGui::Checkbox("Auto Battle", &m_AutoBattle)) {
            Log::Infof("Auto Battle: %s", m_AutoBattle ? "ON" : "OFF");
        }

        ImGui::SameLine();

        // Battle Speed Control
        ImGui::SetNextItemWidth(100.0f);
        if (ImGui::SliderFloat("Speed", &m_BattleSpeed, 0.5f, 5.0f, "%.1fx")) {
            Log::Infof("Battle Speed set to %.1fx", m_BattleSpeed);
        }


        // --- Victory/Defeat Overlay (Final Layer) ---
        if (m_State == CombatState::Victory || m_State == CombatState::Defeat) {
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 p_min = ImGui::GetWindowPos();
            ImVec2 p_max = ImVec2(p_min.x + panelWidth, p_min.y + panelHeight);

            ImVec4 overlayColor;
            const char* message;
            ImVec4 messageColor;

            if (m_State == CombatState::Victory) {
                overlayColor = ToImVec4(Color(0.0f, 0.3f, 0.0f, 0.7f));
                message = "VICTORY!";
                messageColor = ToImVec4(Color(0.3f, 1.0f, 0.3f, 1.0f));
            } else { // Defeat
                overlayColor = ToImVec4(Color(0.3f, 0.0f, 0.0f, 0.7f));
                message = "DEFEAT";
                messageColor = ToImVec4(Color(1.0f, 0.3f, 0.3f, 1.0f));
            }

            // Draw translucent overlay (Full window size)
            drawList->AddRectFilled(p_min, p_max, ImGui::GetColorU32(overlayColor));

            // Draw message centered
            ImVec2 textSize = ImGui::CalcTextSize(message, NULL, true);
            ImVec2 textPos(p_min.x + (panelWidth - textSize.x) * 0.5f, p_min.y + (panelHeight - textSize.y) * 0.5f);

            // We need to use a larger font size for the dramatic title. Since ImGui doesn't easily change font size mid-drawlist,
            // we will draw the text larger directly to the screen (best effort for simple integration)
            // Note: True large text requires font loading/switching, so we'll just center normal text for now.
            ImGui::SetCursorPos(ImVec2((panelWidth - textSize.x) * 0.5f, (panelHeight - textSize.y) * 0.5f));
            ImGui::TextColored(messageColor, "%s", message);

            // Add rewards info below the victory/defeat message
            ImGui::SetCursorPos(ImVec2(panelWidth * 0.5f - 150.0f, (panelHeight - textSize.y) * 0.5f + textSize.y + 15.0f));
            ImGui::TextColored(ToImVec4(Color(1.0f, 0.9f, 0.3f, 1.0f)), "Rewards: %d Credits | %d XP%s",
                m_CreditsEarned, m_XPEarned, m_PartDropped ? " | Part Dropped" : "");
        }

    }
    ImGui::End();

    // 4. Pop styles
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(2);
}


