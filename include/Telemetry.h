#pragma once

#include "Types.h"

// Lightweight telemetry collector for runtime-only analytics.
// Tracks session length and prestige pacing so we can surface
// high-signal health metrics without impacting save data.
class TelemetryManager {
public:
    TelemetryManager();

    // Start or reset the current session. If startTimestampSeconds is 0
    // we will query Platform::GetTime() internally.
    void StartSession(f64 startTimestampSeconds = 0.0);

    // Advance the tracked session time.
    void AddSessionTime(f64 deltaSeconds);

    // Record a prestige event and capture how long the previous run lasted.
    void RecordPrestige(f64 timeSinceLastPrestigeSeconds, i32 lifetimePrestigeCount);

    [[nodiscard]] f64 GetSessionLengthSeconds() const { return m_SessionLengthSeconds; }
    [[nodiscard]] i32 GetSessionPrestiges() const { return m_SessionPrestigeCount; }
    [[nodiscard]] f64 GetLastPrestigeIntervalSeconds() const { return m_LastPrestigeIntervalSeconds; }

    // Prestiges per hour for the active session. Returns 0 when we don't
    // have enough data to compute a meaningful rate.
    [[nodiscard]] f64 GetPrestigesPerHour() const;

    // Average minutes between prestiges in the current session.
    [[nodiscard]] f64 GetAveragePrestigeIntervalMinutes() const;

private:
    f64 m_SessionStartTimestamp;
    f64 m_SessionLengthSeconds;
    f64 m_LastPrestigeIntervalSeconds;
    i32 m_SessionPrestigeCount;
    i32 m_LifetimePrestigeBaseline;
};
