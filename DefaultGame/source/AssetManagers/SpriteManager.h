#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include <SDL3/SDL_render.h>

class SpriteManager
{
public:
    static SpriteManager& getInstance()
    {
        static SpriteManager instance;
        return instance;
    }

    SpriteManager(const SpriteManager&) = delete;
    SpriteManager& operator=(const SpriteManager&) = delete;

    void load();
    SDL_Texture* getTexture(const std::string& name) const noexcept;
    void clear();

private:
    using TexturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;

    SpriteManager() = default;
    ~SpriteManager();

    std::unordered_map<std::string, TexturePtr> _textures;
};