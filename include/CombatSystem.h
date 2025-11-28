#pragma once

#include "Types.h"
#include "Enemy.h"
#include "Spaceship.h"
#include <string>
#include <vector>

// Forward declarations
class Renderer;
class GameState;

// Combat state
enum class CombatState {
    NotInCombat,
    PlayerTurn,
    EnemyTurn,
    Victory,
    Defeat
};

// Combat log entry
struct CombatLogEntry {
    std::string message;
    Color color;
    f64 timestamp;

    CombatLogEntry(const std::string& msg, const Color& col, f64 time)
        : message(msg), color(col), timestamp(time) {}
};

// Combat system - handles all battle logic
class CombatSystem {
public:
    CombatSystem();

    // Combat flow
    void StartCombat(Enemy* enemy, i32 playerLevel, const Spaceship* ship);
    void EndCombat();
    void Update(f64 deltaTime);

    // Player actions
    void PlayerAttack();
    void PlayerDefend(); // +50% defense this turn, lower damage
    void PlayerSpecialAttack(); // High damage, costs energy
    void PlayerSkip(); // Skip turn (for testing)

    // State
    CombatState GetState() const { return m_State; }
    bool IsInCombat() const { return m_State != CombatState::NotInCombat; }
    bool IsPlayerTurn() const { return m_State == CombatState::PlayerTurn; }

    // Combat stats (derived from ship and level)
    f64 GetPlayerMaxHP() const { return m_PlayerMaxHP; }
    f64 GetPlayerCurrentHP() const { return m_PlayerCurrentHP; }
    f64 GetPlayerAttack() const { return m_PlayerAttack; }
    f64 GetPlayerDefense() const { return m_PlayerDefense; }
    f64 GetPlayerSpeed() const { return m_PlayerSpeed; }

    // Current enemy
    Enemy* GetCurrentEnemy() const { return m_CurrentEnemy; }

    // Auto-battle
    void SetAutoBattle(bool enabled) { m_AutoBattle = enabled; }
    bool IsAutoBattle() const { return m_AutoBattle; }
    void SetBattleSpeed(f32 speed) { m_BattleSpeed = speed; } // 1x, 2x, 4x
    f32 GetBattleSpeed() const { return m_BattleSpeed; }

    // Rewards (after victory)
    i32 GetCreditsEarned() const { return m_CreditsEarned; }
    i32 GetXPEarned() const { return m_XPEarned; }
    bool GetPartDropped() const { return m_PartDropped; }

    // Combat log
    const std::vector<CombatLogEntry>& GetCombatLog() const { return m_CombatLog; }
    void ClearCombatLog() { m_CombatLog.clear(); }

    // Rendering
    void RenderCombatUI(Renderer* renderer);

    // UI interaction
    void HandleClick(f32 mouseX, f32 mouseY, bool mousePressed);

private:
    // Combat state
    CombatState m_State;
    Enemy* m_CurrentEnemy;

    // Player stats (calculated at combat start)
    i32 m_PlayerLevel;
    f64 m_PlayerMaxHP;
    f64 m_PlayerCurrentHP;
    f64 m_PlayerAttack;
    f64 m_PlayerDefense;
    f64 m_PlayerSpeed;

    // Combat bonuses from ship
    f64 m_ShipPowerBonus;
    f64 m_ShipCombatBonus;

    // Turn system
    f64 m_TurnTimer;
    f64 m_TurnDelay; // Time between turns
    bool m_PlayerDefending; // Is player defending this turn?

    // Auto-battle
    bool m_AutoBattle;
    f32 m_BattleSpeed;

    // Rewards
    i32 m_CreditsEarned;
    i32 m_XPEarned;
    bool m_PartDropped;

    // Combat log
    std::vector<CombatLogEntry> m_CombatLog;
    f64 m_CombatTime;

    // Combat actions
    void ExecutePlayerTurn();
    void ExecuteEnemyTurn();
    f64 CalculateDamage(f64 attack, f64 defense, bool isCritical = false);
    bool RollCritical() const; // 15% base crit chance
    void CheckVictoryDefeat();
    void GrantRewards();

    // Logging
    void AddLog(const std::string& message, const Color& color = Color::White());
};
