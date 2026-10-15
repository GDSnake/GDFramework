#include "AudioManager.h"

#include <SDL3/SDL_log.h>

#include "Config/Config.h"

AudioManager::AudioManager()
    : _mixer(nullptr, MIX_DestroyMixer),
      _music(nullptr, MIX_DestroyAudio),
      _track(nullptr, MIX_DestroyTrack)
{
    const auto& audio = Config::getInstance().getConfigData().audio;
    auto music = audio.find("background_music");
    if (music == audio.end())
    {
        music = audio.find("bgm");
    }
    if (music == audio.end() || music->second.empty())
    {
        return;
    }

    if (!MIX_Init())
    {
        SDL_Log("Audio mixer initialization failed: %s", SDL_GetError());
        return;
    }
    _mixerInitialized = true;

    const SDL_AudioSpec audioSpec{SDL_AUDIO_S16, 2, 44100};
    _mixer.reset(MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec));
    if (!_mixer)
    {
        SDL_Log("Audio mixer creation failed: %s", SDL_GetError());
        return;
    }

    _music.reset(MIX_LoadAudio(_mixer.get(), music->second.c_str(), true));
    if (!_music)
    {
        SDL_Log("Background music load failed: %s", SDL_GetError());
        return;
    }

    _track.reset(MIX_CreateTrack(_mixer.get()));
    if (!_track || !MIX_SetTrackAudio(_track.get(), _music.get()))
    {
        SDL_Log("Audio track setup failed: %s", SDL_GetError());
    }
}

AudioManager::~AudioManager()
{
    shutdown();
}

void AudioManager::toggleMusic() const
{
    if (!_track)
    {
        return;
    }
    if (MIX_TrackPlaying(_track.get()))
    {
        stopMusic();
    }
    else
    {
        playMusic();
    }
}

void AudioManager::playMusic() const
{
    if (!_track || MIX_TrackPlaying(_track.get()))
    {
        return;
    }

    SDL_PropertiesID properties = SDL_CreateProperties();
    if (!properties)
    {
        SDL_Log("Audio properties creation failed: %s", SDL_GetError());
        return;
    }
    SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
    MIX_PlayTrack(_track.get(), properties);
    SDL_DestroyProperties(properties);
}

void AudioManager::stopMusic() const
{
    if (_track)
    {
        MIX_StopTrack(_track.get(), 0);
    }
}

void AudioManager::resumeMusic() const
{
    if (_track && MIX_TrackPaused(_track.get()))
    {
        MIX_ResumeTrack(_track.get());
    }
    else
    {
        playMusic();
    }
}

void AudioManager::pauseMusic() const
{
    if (_track)
    {
        MIX_PauseTrack(_track.get());
    }
}

void AudioManager::shutdown()
{
    stopMusic();
    _track.reset();
    _music.reset();
    _mixer.reset();
    if (_mixerInitialized)
    {
        MIX_Quit();
        _mixerInitialized = false;
    }
}