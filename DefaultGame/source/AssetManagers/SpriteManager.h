#pragma once
#include <memory>
#include <string>

#include <unordered_map>

class Sprite;
struct SDL_Surface;

namespace SpriteManagerHelpers
{
    SDL_Surface* loadImage(const std::string& filePath);
}


class SpriteManager
{
public:

    static SpriteManager& getInstance()
    {
        static SpriteManager instance;
        return instance;
    }

    void load();
    std::shared_ptr<Sprite> getSprite(const std::string& name) const;
    void shutdown();
private:
    SpriteManager();
    ~SpriteManager();

    void loadSprites();

    std::unordered_map<std::string, std::shared_ptr<Sprite>> _sprites;
};
