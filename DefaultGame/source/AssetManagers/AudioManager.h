#pragma once

#include <memory>

#include <SDL3_mixer/SDL_mixer.h>

class AudioManager
{
public:
    static AudioManager& getInstance()
    {
        static AudioManager instance;
        return instance;
    }

    AudioManager(const AudioManager&) = delete;
    AudioManager& operator=(const AudioManager&) = delete;

    void toggleMusic() const;
    void playMusic() const;
    void stopMusic() const;
    void resumeMusic() const;
    void pauseMusic() const;
    void shutdown();

private:
    AudioManager();
    ~AudioManager();

    std::unique_ptr<MIX_Mixer, decltype(&MIX_DestroyMixer)> _mixer;
    std::unique_ptr<MIX_Audio, decltype(&MIX_DestroyAudio)> _music;
    std::unique_ptr<MIX_Track, decltype(&MIX_DestroyTrack)> _track;
    bool _mixerInitialized = false;
};