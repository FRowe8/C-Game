#include "TimeManager.h"
#include "Platform.h"
#include "Logger.h"

TimeManager::TimeManager()
    : m_TotalTimePlayed(0.0)
    , m_TimeSinceLastSave(0.0)
    , m_TimeSinceLastPrestige(0.0)
    , m_OfflineCapHours(4.0)
    , m_OfflineMultiplier(0.5)
    , m_BoostActive(false)
    , m_BoostTimeRemaining(0.0)
    , m_BoostCooldownRemaining(0.0)
    , m_BoostDuration(30.0)
    , m_BoostCooldown(60.0)
    , m_TimeSinceLastEvent(0.0)
    , m_EventCooldown(120.0)
    , m_TimeSinceLastAnomaly(0.0)
    , m_AnomalySpawnInterval(45.0)
    , m_ComboCount(0)
    , m_ComboTimeRemaining(0.0)
    , m_ComboWindow(5.0)
    , m_PrestigeFlashTimer(0.0)
    , m_PrestigeFlashActive(false)
    , m_TimeScale(1.0)
{
}

void TimeManager::Initialize() {
    m_TotalTimePlayed = 0.0;
    m_TimeSinceLastSave = 0.0;
    m_TimeSinceLastPrestige = 0.0;
    m_TimeScale = 1.0;
}

void TimeManager::Update(f64 deltaTime) {
    // Apply time scale
    f64 effectiveDelta = deltaTime * m_TimeScale;

    // Update play time tracking
    m_TotalTimePlayed += effectiveDelta;
    m_TimeSinceLastSave += effectiveDelta;
    m_TimeSinceLastPrestige += effectiveDelta;

    // Update boost system
    if (m_BoostActive) {
        m_BoostTimeRemaining -= deltaTime;
        if (m_BoostTimeRemaining <= 0.0) {
            m_BoostActive = false;
            m_BoostTimeRemaining = 0.0;
            m_BoostCooldownRemaining = m_BoostCooldown;
        }
    } else if (m_BoostCooldownRemaining > 0.0) {
        m_BoostCooldownRemaining -= deltaTime;
        if (m_BoostCooldownRemaining < 0.0) {
            m_BoostCooldownRemaining = 0.0;
        }
    }

    // Update event timer
    m_TimeSinceLastEvent += effectiveDelta;

    // Update anomaly timer
    m_TimeSinceLastAnomaly += effectiveDelta;

    // Update combo timer
    if (m_ComboCount > 0) {
        m_ComboTimeRemaining -= deltaTime;
        if (m_ComboTimeRemaining <= 0.0) {
            ResetCombo();
        }
    }

    // Update prestige flash
    if (m_PrestigeFlashActive) {
        m_PrestigeFlashTimer -= deltaTime;
        if (m_PrestigeFlashTimer <= 0.0) {
            m_PrestigeFlashActive = false;
            m_PrestigeFlashTimer = 0.0;
        }
    }
}

// === Offline Progress ===

TimeManager::OfflineProgressData TimeManager::CalculateOfflineProgress(i64 lastSaveTimestamp) {
    OfflineProgressData result;
    result.secondsOffline = 0.0;
    result.resourcesGenerated = 0.0;
    result.hadProgress = false;

    i64 currentTime = static_cast<i64>(Platform::GetTime());
    if (lastSaveTimestamp == 0) {
        return result;
    }

    i64 timeOffline = currentTime - lastSaveTimestamp;
    if (timeOffline < 10) {
        return result; // Less than 10 seconds, ignore
    }

    f64 secondsOffline = static_cast<f64>(timeOffline);
    f64 maxOfflineTime = 3600.0 * m_OfflineCapHours;

    if (secondsOffline > maxOfflineTime) {
        secondsOffline = maxOfflineTime;
    }

    result.secondsOffline = secondsOffline;
    result.hadProgress = true;

    Log::Infof("You were away for ", secondsOffline / 60.0, " minutes");

    return result;
}

// === Time-Based Cooldowns ===

void TimeManager::ActivateBoost() {
    if (m_BoostActive || m_BoostCooldownRemaining > 0.0) {
        return; // Already active or on cooldown
    }

    m_BoostActive = true;
    m_BoostTimeRemaining = m_BoostDuration;
    Log::Infof("Boost activated! 2x production for ", m_BoostDuration, " seconds!");
}

void TimeManager::IncrementCombo() {
    m_ComboCount++;
    m_ComboTimeRemaining = m_ComboWindow;
}

void TimeManager::TriggerPrestigeFlash() {
    m_PrestigeFlashActive = true;
    m_PrestigeFlashTimer = 0.5; // 0.5 second flash
}
