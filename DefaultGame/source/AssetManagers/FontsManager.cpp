#include "FontsManager.h"

#include <SDL3/SDL_log.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "Config/Config.h"

FontsManager::~FontsManager()
{
    clear();
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
        _fonts.emplace(name, font);
    }
}

TTF_Font* FontsManager::getFont(const std::string& name) const noexcept
{
    const auto font = _fonts.find(name);
    return font == _fonts.end() ? nullptr : font->second;
}

void FontsManager::clear()
{
    for (auto& [name, font] : _fonts)
    {
        if (font)
        {
            TTF_CloseFont(font);
        }
    }
    _fonts.clear();
}