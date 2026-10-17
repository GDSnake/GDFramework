#include "FontsManager.h"

#include <SDL3/SDL_log.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Config/Config.h"

FontsManager::FontsManager() = default;

FontsManager::~FontsManager()
{
    shutdown();
}

void FontsManager::load()
{
    clear();

    for (const auto& [name, path] : Config::getInstance().getConfigData().fonts)
    {
        TTF_Font* font = TTF_OpenFont(path.c_str(), 32.0f);
        if (!font)
        {
            SDL_Log("Unable to load font '%s' (%s): %s", name.c_str(), path.c_str(), SDL_GetError());
            continue;
        }
        _fontsTTFMap.emplace(name, font);
    }
}

TTF_Font* FontsManager::getFont(const std::string& name) const noexcept
{
    const auto font = _fontsTTFMap.find(name);
    return font == _fontsTTFMap.end() ? nullptr : font->second;
}

void FontsManager::clear()
{
    for (auto& [name, font] : _fontsTTFMap)
    {
        if (font)
        {
            TTF_CloseFont(font);
        }
    }
    _fontsTTFMap.clear();
}

void FontsManager::shutdown()
{
    clear();
}