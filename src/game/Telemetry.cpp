#include "Telemetry.h"
#include "Platform.h"
#include <algorithm>

TelemetryManager::TelemetryManager()
    : m_SessionStartTimestamp(0.0)
    , m_SessionLengthSeconds(0.0)
    , m_LastPrestigeIntervalSeconds(0.0)
    , m_SessionPrestigeCount(0)
    , m_LifetimePrestigeBaseline(0) {}

void TelemetryManager::StartSession(f64 startTimestampSeconds) {
    m_SessionStartTimestamp = startTimestampSeconds > 0.0 ? startTimestampSeconds : Platform::GetTime();
    m_SessionLengthSeconds = 0.0;
    m_LastPrestigeIntervalSeconds = 0.0;
    m_SessionPrestigeCount = 0;
    m_LifetimePrestigeBaseline = 0;
}

void TelemetryManager::AddSessionTime(f64 deltaSeconds) {
    m_SessionLengthSeconds += std::max(0.0, deltaSeconds);
}

void TelemetryManager::RecordPrestige(f64 timeSinceLastPrestigeSeconds, i32 lifetimePrestigeCount) {
    m_SessionPrestigeCount++;
    m_LastPrestigeIntervalSeconds = std::max(0.0, timeSinceLastPrestigeSeconds);

    // The baseline helps us infer how many prestiges happened before the session,
    // which is useful for dashboards that show lifetime vs. session rates.
    if (m_LifetimePrestigeBaseline == 0) {
        m_LifetimePrestigeBaseline = lifetimePrestigeCount - m_SessionPrestigeCount;
    }
}

f64 TelemetryManager::GetPrestigesPerHour() const {
    if (m_SessionLengthSeconds <= 0.0 || m_SessionPrestigeCount <= 0) {
        return 0.0;
    }

    f64 hours = m_SessionLengthSeconds / 3600.0;
    return m_SessionPrestigeCount / hours;
}

f64 TelemetryManager::GetAveragePrestigeIntervalMinutes() const {
    if (m_SessionPrestigeCount <= 0) {
        return 0.0;
    }

    f64 averageSeconds = m_SessionLengthSeconds / static_cast<f64>(m_SessionPrestigeCount);
    return averageSeconds / 60.0;
}
