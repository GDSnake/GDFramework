#include "SpriteManager.h"

#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>

#include <exception>
#include <filesystem>

#include "Entities/Sprite.h"
#include "Config/Config.h"


SDL_Surface* SpriteManagerHelpers::loadImage(const std::string& filePath)
{
    return IMG_Load(filePath.c_str());
}

std::shared_ptr<Sprite> SpriteManager::getSprite(const std::string& name) const
{
    const auto sprite = _sprites.find(name);
    return sprite == _sprites.end() ? nullptr : sprite->second;
}

SpriteManager::SpriteManager()
{
}

SpriteManager::~SpriteManager()
{
    shutdown();
}

void SpriteManager::shutdown()
{
    _sprites.clear();
}

void SpriteManager::load()
{
    loadSprites();
}

void SpriteManager::loadSprites()
{
    shutdown();
    for (const auto& [name, image] : Config::getInstance().getConfigData().images)
    {
        if (std::filesystem::is_directory(image.path))
        {
            continue;
        }

        try
        {
            _sprites.emplace(name, std::make_shared<Sprite>(image.path, image.width, image.height));
        }
        catch (const std::exception& error)
        {
            SDL_Log("Unable to load sprite '%s' from '%s': %s", name.c_str(), image.path.c_str(), error.what());
        }
    }
}