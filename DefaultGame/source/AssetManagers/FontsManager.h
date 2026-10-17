#pragma once

#include <string>
#include <unordered_map>

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

#include "Config/Config.h"

struct TTF_Font;

namespace FontColor
{
    static constexpr SDL_Color Black = { 0,0,0,255 };
    static constexpr SDL_Color Red = { 255,0,0,255 };
    static constexpr SDL_Color Orange = { 255,165,0,255 };

    // Add the colours as needed for the map

}

namespace FontSize
{
   static constexpr int Title = 32;
}

namespace FontText
{
    static std::string Title = "Default Game";
    static std::string Exit = "Press ESC to exit";
    // Define the texts you need
    // TODO create a map from a config file
}

struct FontData
{
    std::string baseText;
    SDL_FPoint origin;
    SDL_Color color;
    FontNames fontName;
    int fontSize;
};

inline const FontData TitleFontInfo = {
    .baseText = FontText::Title,
    .origin = {20.0f, 10.0f},
    .color = FontColor::Red,
    .fontName = FontNames::DefaultFont,
    .fontSize = FontSize::Title
};

inline const FontData ExitFontInfo = {
    .baseText = FontText::Exit,
    .origin = {20.0f, 500.0f},
    .color = FontColor::Orange,
    .fontName = FontNames::DefaultFont,
    .fontSize = FontSize::Title
};

class FontsManager {

public:

    static FontsManager& getInstance()
    {
        static FontsManager instance;
        return instance;
    }

    void load();
    TTF_Font* getFont(const std::string& name) const noexcept;
    TTF_Font* getFont(FontNames font) const noexcept
    {
        return getFont(FontUtils::to_string(font));
    }
    void clear();
    void shutdown();

private:
    FontsManager();
    ~FontsManager();

    std::unordered_map<std::string, TTF_Font*> _fontsTTFMap;
};

