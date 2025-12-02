#include "SoundManager.h"
#include "logger.h"

// Uncomment when SDL_mixer is available:
// #include <SDL_mixer.h>
// #define AUDIO_ENABLED

SoundManager::SoundManager()
    : m_Initialized(false)
    , m_AudioAvailable(false)
    , m_Muted(false)
    , m_MasterVolume(1.0f)
    , m_SoundVolume(0.7f)
    , m_MusicVolume(0.5f)
    , m_CurrentMusicTrack(MusicTrack::MainTheme)
    , m_MusicPlaying(false)
{
    for (i32 i = 0; i < static_cast<i32>(SoundEffect::COUNT); i++) {
        m_SoundHandles[i] = nullptr;
    }
    for (i32 i = 0; i < static_cast<i32>(MusicTrack::COUNT); i++) {
        m_MusicHandles[i] = nullptr;
    }
}

SoundManager::~SoundManager() {
    Shutdown();
}

bool SoundManager::Initialize() {
    Log::Info("Initializing sound system...");

#ifdef AUDIO_ENABLED
    // Initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        Log::Warningf("Failed to initialize SDL_mixer: ", Mix_GetError());
        m_AudioAvailable = false;
        m_Initialized = true;
        return true; // Not a fatal error
    }

    m_AudioAvailable = true;

    // Allocate mixing channels
    Mix_AllocateChannels(16);

    // Load sounds and music
    LoadSounds();
    LoadMusic();

    Log::Info("Sound system initialized with audio support");
#else
    Log::Info("Sound system initialized in silent mode (no audio library linked)");
    m_AudioAvailable = false;
#endif

    m_Initialized = true;
    return true;
}

void SoundManager::Shutdown() {
    if (!m_Initialized) return;

    Log::Info("Shutting down sound system...");

#ifdef AUDIO_ENABLED
    if (m_AudioAvailable) {
        // Stop all sounds and music
        StopAllSounds();
        StopMusic();

        // Free sound effects
        for (i32 i = 0; i < static_cast<i32>(SoundEffect::COUNT); i++) {
            if (m_SoundHandles[i]) {
                Mix_FreeChunk(static_cast<Mix_Chunk*>(m_SoundHandles[i]));
                m_SoundHandles[i] = nullptr;
            }
        }

        // Free music tracks
        for (i32 i = 0; i < static_cast<i32>(MusicTrack::COUNT); i++) {
            if (m_MusicHandles[i]) {
                Mix_FreeMusic(static_cast<Mix_Music*>(m_MusicHandles[i]));
                m_MusicHandles[i] = nullptr;
            }
        }

        Mix_CloseAudio();
    }
#endif

    m_Initialized = false;
    Log::Info("Sound system shut down");
}

void SoundManager::PlaySound(SoundEffect effect, f32 volume) {
    if (!m_Initialized || !m_AudioAvailable || m_Muted) return;

#ifdef AUDIO_ENABLED
    i32 index = static_cast<i32>(effect);
    if (m_SoundHandles[index]) {
        Mix_Chunk* chunk = static_cast<Mix_Chunk*>(m_SoundHandles[index]);

        // Calculate final volume
        i32 finalVolume = static_cast<i32>(m_MasterVolume * m_SoundVolume * volume * MIX_MAX_VOLUME);
        Mix_VolumeChunk(chunk, finalVolume);

        // Play on first available channel
        Mix_PlayChannel(-1, chunk, 0);
    }
#else
    (void)effect;
    (void)volume;
#endif
}

void SoundManager::StopSound(SoundEffect effect) {
    if (!m_Initialized || !m_AudioAvailable) return;

#ifdef AUDIO_ENABLED
    // Note: This stops ALL instances of the sound, not individual channels
    // For more control, would need to track channel assignments
    (void)effect;
#else
    (void)effect;
#endif
}

void SoundManager::StopAllSounds() {
    if (!m_Initialized || !m_AudioAvailable) return;

#ifdef AUDIO_ENABLED
    Mix_HaltChannel(-1); // Stop all channels
#endif
}

void SoundManager::PlayMusic(MusicTrack track, bool loop) {
    if (!m_Initialized || !m_AudioAvailable || m_Muted) return;

#ifdef AUDIO_ENABLED
    i32 index = static_cast<i32>(track);
    if (m_MusicHandles[index]) {
        Mix_Music* music = static_cast<Mix_Music*>(m_MusicHandles[index]);

        // Set music volume
        i32 finalVolume = static_cast<i32>(m_MasterVolume * m_MusicVolume * MIX_MAX_VOLUME);
        Mix_VolumeMusic(finalVolume);

        // Play music
        Mix_PlayMusic(music, loop ? -1 : 0);

        m_CurrentMusicTrack = track;
        m_MusicPlaying = true;
    }
#else
    (void)track;
    (void)loop;
#endif
}

void SoundManager::StopMusic() {
    if (!m_Initialized || !m_AudioAvailable) return;

#ifdef AUDIO_ENABLED
    Mix_HaltMusic();
    m_MusicPlaying = false;
#endif
}

void SoundManager::PauseMusic() {
    if (!m_Initialized || !m_AudioAvailable || !m_MusicPlaying) return;

#ifdef AUDIO_ENABLED
    Mix_PauseMusic();
#endif
}

void SoundManager::ResumeMusic() {
    if (!m_Initialized || !m_AudioAvailable) return;

#ifdef AUDIO_ENABLED
    Mix_ResumeMusic();
#endif
}

void SoundManager::SetMasterVolume(f32 volume) {
    m_MasterVolume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
}

void SoundManager::SetSoundVolume(f32 volume) {
    m_SoundVolume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);
}

void SoundManager::SetMusicVolume(f32 volume) {
    m_MusicVolume = volume < 0.0f ? 0.0f : (volume > 1.0f ? 1.0f : volume);

#ifdef AUDIO_ENABLED
    if (m_Initialized && m_AudioAvailable && m_MusicPlaying) {
        i32 finalVolume = static_cast<i32>(m_MasterVolume * m_MusicVolume * MIX_MAX_VOLUME);
        Mix_VolumeMusic(finalVolume);
    }
#endif
}

void SoundManager::LoadSounds() {
#ifdef AUDIO_ENABLED
    if (!m_AudioAvailable) return;

    // Load sound effect files from assets/sounds/
    // For now, just log which sounds would be loaded
    Log::Info("Loading sound effects...");

    // Example loading code (uncomment when audio files are available):
    // m_SoundHandles[static_cast<i32>(SoundEffect::Click)] = Mix_LoadWAV("assets/sounds/click.wav");
    // m_SoundHandles[static_cast<i32>(SoundEffect::LevelUp)] = Mix_LoadWAV("assets/sounds/levelup.wav");
    // ... etc

    Log::Info("Sound effects loaded (placeholder)");
#endif
}

void SoundManager::LoadMusic() {
#ifdef AUDIO_ENABLED
    if (!m_AudioAvailable) return;

    // Load music files from assets/music/
    Log::Info("Loading music tracks...");

    // Example loading code (uncomment when music files are available):
    // m_MusicHandles[static_cast<i32>(MusicTrack::MainTheme)] = Mix_LoadMUS("assets/music/main_theme.ogg");
    // m_MusicHandles[static_cast<i32>(MusicTrack::CombatTheme)] = Mix_LoadMUS("assets/music/combat.ogg");
    // ... etc

    Log::Info("Music tracks loaded (placeholder)");
#endif
}

const char* SoundManager::GetSoundPath(SoundEffect effect) const {
    switch (effect) {
        // UI Sounds
        case SoundEffect::Click: return "assets/sounds/ui/click.wav";
        case SoundEffect::Hover: return "assets/sounds/ui/hover.wav";
        case SoundEffect::ButtonPress: return "assets/sounds/ui/button_press.wav";
        case SoundEffect::TabSwitch: return "assets/sounds/ui/tab_switch.wav";

        // Combat Sounds
        case SoundEffect::AttackLight: return "assets/sounds/combat/attack_light.wav";
        case SoundEffect::AttackHeavy: return "assets/sounds/combat/attack_heavy.wav";
        case SoundEffect::Hit: return "assets/sounds/combat/hit.wav";
        case SoundEffect::CriticalHit: return "assets/sounds/combat/critical.wav";
        case SoundEffect::Victory: return "assets/sounds/combat/victory.wav";
        case SoundEffect::Defeat: return "assets/sounds/combat/defeat.wav";

        // Loot Sounds
        case SoundEffect::LootDrop: return "assets/sounds/loot/drop_common.wav";
        case SoundEffect::LootRare: return "assets/sounds/loot/drop_rare.wav";
        case SoundEffect::LootEpic: return "assets/sounds/loot/drop_epic.wav";
        case SoundEffect::LootLegendary: return "assets/sounds/loot/drop_legendary.wav";

        // Progression Sounds
        case SoundEffect::LevelUp: return "assets/sounds/progression/level_up.wav";
        case SoundEffect::SkillUnlock: return "assets/sounds/progression/skill_unlock.wav";
        case SoundEffect::Achievement: return "assets/sounds/progression/achievement.wav";
        case SoundEffect::Prestige: return "assets/sounds/progression/prestige.wav";

        // Resource Sounds
        case SoundEffect::ObserveSuccess: return "assets/sounds/resources/observe.wav";
        case SoundEffect::ObserveCritical: return "assets/sounds/resources/observe_critical.wav";
        case SoundEffect::UpgradeComplete: return "assets/sounds/resources/upgrade.wav";
        case SoundEffect::ResearchComplete: return "assets/sounds/resources/research.wav";

        default: return "";
    }
}

const char* SoundManager::GetMusicPath(MusicTrack track) const {
    switch (track) {
        case MusicTrack::MainTheme: return "assets/music/main_theme.ogg";
        case MusicTrack::CombatTheme: return "assets/music/combat_theme.ogg";
        case MusicTrack::BossTheme: return "assets/music/boss_theme.ogg";
        case MusicTrack::VictoryTheme: return "assets/music/victory_theme.ogg";
        default: return "";
    }
}
