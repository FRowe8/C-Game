#pragma once

#include "Types.h"
#include <string>
#include <unordered_map>

/**
 * SoundManager - Audio playback system
 *
 * Provides sound effects for game events:
 * - UI interactions (clicks, hovers)
 * - Combat (attacks, hits, victories)
 * - Loot (drops, upgrades, rare items)
 * - Progression (level ups, unlocks, achievements)
 *
 * NOTE: Requires SDL_mixer or Soloud library for actual audio playback.
 * If not available, operates in silent mode without errors.
 */

enum class SoundEffect {
    // UI Sounds
    Click,
    Hover,
    ButtonPress,
    TabSwitch,

    // Combat Sounds
    AttackLight,
    AttackHeavy,
    Hit,
    CriticalHit,
    Victory,
    Defeat,

    // Loot Sounds
    LootDrop,
    LootRare,
    LootEpic,
    LootLegendary,

    // Progression Sounds
    LevelUp,
    SkillUnlock,
    Achievement,
    Prestige,

    // Resource Sounds
    ObserveSuccess,
    ObserveCritical,
    UpgradeComplete,
    ResearchComplete,

    COUNT
};

enum class MusicTrack {
    MainTheme,
    CombatTheme,
    BossTheme,
    VictoryTheme,
    COUNT
};

class SoundManager {
public:
    SoundManager();
    ~SoundManager();

    // Initialization
    bool Initialize();
    void Shutdown();

    // Sound effects
    void PlaySound(SoundEffect effect, f32 volume = 1.0f);
    void StopSound(SoundEffect effect);
    void StopAllSounds();

    // Music
    void PlayMusic(MusicTrack track, bool loop = true);
    void StopMusic();
    void PauseMusic();
    void ResumeMusic();

    // Volume control
    void SetMasterVolume(f32 volume);      // 0.0 to 1.0
    void SetSoundVolume(f32 volume);       // 0.0 to 1.0
    void SetMusicVolume(f32 volume);       // 0.0 to 1.0

    f32 GetMasterVolume() const { return m_MasterVolume; }
    f32 GetSoundVolume() const { return m_SoundVolume; }
    f32 GetMusicVolume() const { return m_MusicVolume; }

    // Mute/unmute
    void SetMuted(bool muted) { m_Muted = muted; }
    bool IsMuted() const { return m_Muted; }

    // System status
    bool IsInitialized() const { return m_Initialized; }
    bool IsAudioAvailable() const { return m_AudioAvailable; }

private:
    // Internal state
    bool m_Initialized;
    bool m_AudioAvailable;      // True if audio library is present
    bool m_Muted;

    // Volume settings
    f32 m_MasterVolume;
    f32 m_SoundVolume;
    f32 m_MusicVolume;

    // Currently playing music
    MusicTrack m_CurrentMusicTrack;
    bool m_MusicPlaying;

    // Helper methods
    void LoadSounds();
    void LoadMusic();
    const char* GetSoundPath(SoundEffect effect) const;
    const char* GetMusicPath(MusicTrack track) const;

    // Platform-specific audio handles (opaque pointers)
    // These would be Mix_Chunk* or soloud::Wav* depending on library
    void* m_SoundHandles[static_cast<i32>(SoundEffect::COUNT)];
    void* m_MusicHandles[static_cast<i32>(MusicTrack::COUNT)];
};
