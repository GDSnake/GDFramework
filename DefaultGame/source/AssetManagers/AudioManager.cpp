#include "AudioManager.h"

#include <iostream>

#include "Config/Config.h"

AudioManager::AudioManager(): _mixer(nullptr, MIX_DestroyMixer),
                              _bgm(nullptr, MIX_DestroyAudio),
                              _track(nullptr, MIX_DestroyTrack)
{   
    _mixInitialized = MIX_Init();
    if (!_mixInitialized) {
        std::cerr << "MIX_Init failed: " << SDL_GetError() << '\n';
        return;
    }
    SDL_AudioSpec audioSpec = {SDL_AUDIO_S16, 2, 44100};
    _mixer.reset(MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audioSpec));
    if (!_mixer)
    {
        std::cerr << "Audio Manager Init Error: " << SDL_GetError() << '\n';
    }

    _bgm.reset(MIX_LoadAudio(_mixer.get(), Config::getInstance().getConfigData().bgmPath.c_str(), true));
    if (!_bgm)
    {
        std::cerr << "Audio Manager BGM Error: " << SDL_GetError() << '\n';
        return;
    }

    _track.reset(MIX_CreateTrack(_mixer.get()));
    if (!_track || !MIX_SetTrackAudio(_track.get(), _bgm.get()))
    {
        std::cerr << "Audio Manager Track Error: " << SDL_GetError() << '\n';
    }
}

AudioManager::~AudioManager()
{
    shutdown();
}

void AudioManager::shutdown()
{
    stopMusic();
    _track.reset();
    _bgm.reset();
    _mixer.reset();
    if (_mixInitialized)
    {
        MIX_Quit();
        _mixInitialized = false;
    }
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
    if (_track && !MIX_TrackPlaying(_track.get()))
    {
        SDL_PropertiesID properties = SDL_CreateProperties();
        if (!properties)
        {
            std::cerr << "Audio Manager Properties Error: " << SDL_GetError() << '\n';
            return;
        }
        SDL_SetNumberProperty(properties, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
        MIX_PlayTrack(_track.get(), properties);
        SDL_DestroyProperties(properties);
    }
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
