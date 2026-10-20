#include "SpriteManager.h"

#include <SDL3/SDL_log.h>
#include <SDL3_image/SDL_image.h>

#include <exception>
#include <filesystem>
#include <utility>
#include <vector>

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
            _sprites.emplace(name, std::make_shared<Sprite>(
                image.path,
                image.width,
                image.height));
        }
        catch (const std::exception& error)
        {
            SDL_Log("Unable to load sprite '%s' from '%s': %s", name.c_str(), image.path.c_str(), error.what());
        }
    }

    for (const auto& [name, sheet] : Config::getInstance().getConfigData().imageSheets)
    {
        if (std::filesystem::is_directory(sheet.path))
        {
            continue;
        }

        try
        {
            std::vector<SDL_FRect> frames;
            frames.reserve(sheet.frames.size());
            for (const ImageSheetConfigData::Frame& frame : sheet.frames)
            {
                frames.push_back({ frame.x, frame.y, frame.width, frame.height });
            }

            _sprites.emplace(name, std::make_shared<Sprite>(
                sheet.path,
                0.0f,
                0.0f,
                sheet.frameWidth,
                sheet.frameHeight,
                sheet.columns,
                sheet.rows,
                std::move(frames)));
        }
        catch (const std::exception& error)
        {
            SDL_Log("Unable to load image sheet '%s' from '%s': %s",
                name.c_str(), sheet.path.c_str(), error.what());
        }
    }
}