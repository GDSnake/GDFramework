#include "SpriteManager.h"

#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>

#include "Config/Config.h"
#include "Core/Renderer.h"

SpriteManager::~SpriteManager()
{
    clear();
}

void SpriteManager::load()
{
    clear();

    SDL_Renderer* renderer = Renderer::getInstance().getRenderer();
    if (!renderer)
    {
        return;
    }

    for (const auto& [name, path] : Config::getInstance().getConfigData().images)
    {
        SDL_Texture* texture = IMG_LoadTexture(renderer, path.c_str());
        if (!texture)
        {
            SDL_Log("Unable to load image '%s' (%s): %s", name.c_str(), path.c_str(), SDL_GetError());
            continue;
        }
        _textures.emplace(name, TexturePtr(texture, SDL_DestroyTexture));
    }
}

SDL_Texture* SpriteManager::getTexture(const std::string& name) const noexcept
{
    const auto texture = _textures.find(name);
    return texture == _textures.end() ? nullptr : texture->second.get();
}

void SpriteManager::clear()
{
    _textures.clear();
}