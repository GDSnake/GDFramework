#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <SDL3_mixer/SDL_mixer.h>

enum class AudioTrack : uint8_t
{
    BGM = 0,
    // ADD as needed for the map
    Total
};

static std::string toString(const AudioTrack track)
{
    switch (track)
    {
    case AudioTrack::BGM:
        return "bgm";
    default:
        return "";
    }
}

class AudioManager
{
public:
    static AudioManager& getInstance()
    {
        static AudioManager instance;
        return instance;
    }

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
	//std::unordered_map<std::string, std::unique_ptr<MIX_Audio, decltype(&MIX_DestroyAudio)>> _soundsMap;
    std::unique_ptr<MIX_Audio, decltype(&MIX_DestroyAudio)> _bgm;
	std::unique_ptr <MIX_Track, decltype(&MIX_DestroyTrack)> _track;
    bool _mixInitialized = false;
};