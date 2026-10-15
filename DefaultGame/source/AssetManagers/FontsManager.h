#pragma once

#include <string>
#include <unordered_map>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

#include "Config/Config.h"

enum class FontNames : uint8_t;
struct TTF_Font;

namespace FontColor
{
    static constexpr SDL_Color Black = { 0,0,0,255 };
    static constexpr SDL_Color Red = { 255,0,0,255 };
    static constexpr SDL_Color Orange = { 255,50,0,255 };

    // Add the colours as needed for the map

}

namespace FontSize
{
   static constexpr int Title = 32;
}

namespace FontText
{
    static std::string Title = "Default Game";
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

static const FontData TitleFontInfo = {.baseText = FontText::Title, .origin = {20.0f, 10.0f }, .color = FontColor::Black, .fontName = FontNames::GameBoyFont, .fontSize = FontSize::Title};

const FontData& getLevelFontInfo();
const FontData& getLinesFontInfo();
const FontData& getGameOverFontInfo();
const FontData& getPauseFontInfo();

static const FontData HelpFontInfo = { .baseText = FontText::HelptText, .origin = {20.0f, 10.0f }, .color = FontColor::Black, .fontName = FontNames::DefaultFont, .fontSize = FontSize::Help };

class FontsManager {

public:

    static FontsManager& getInstance()
    {
        static FontsManager instance;
        return instance;
    }

    TTF_Font* getFont(FontNames font) const;
    void shutdown();

private:
    FontsManager();
    ~FontsManager();

    std::unordered_map<std::string, TTF_Font*> _fontsTTFMap;
};

