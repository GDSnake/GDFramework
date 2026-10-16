#include <SDL3/SDL.h>

#include <algorithm>

#include "AssetManagers/SpriteManager.h"
#include "Config/Config.h"
#include "Core/Renderer.h"
#include "Entities/Sprite.h"

int main()
{
    Config& config = Config::getInstance();
    config.loadConfig();
    if (config.getConfigData().hasError)
    {
        return 1;
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return 1;
    }

    Renderer& renderer = Renderer::getInstance();
    if (!renderer.init())
    {
        SDL_Quit();
        return 1;
    }

    SpriteManager& sprites = SpriteManager::getInstance();
    sprites.load();
    const std::shared_ptr<Sprite> sprite = sprites.getSprite("sample");
    if (!sprite)
    {
        SDL_Log("Sample image could not be loaded");
        sprites.shutdown();
        renderer.shutdown();
        SDL_Quit();
        return 1;
    }

    const SDL_FRect& source = sprite->getSourceRect();
    const float maxWidth = static_cast<float>(config.getConfigData().width) * 0.8f;
    const float maxHeight = static_cast<float>(config.getConfigData().height) * 0.8f;
    const float scale = std::min(maxWidth / source.w, maxHeight / source.h);
    const float width = source.w * scale;
    const float height = source.h * scale;
    const SDL_FRect destination{
        (static_cast<float>(config.getConfigData().width) - width) * 0.5f,
        (static_cast<float>(config.getConfigData().height) - height) * 0.5f,
        width,
        height
    };

    bool running = true;
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT ||
                (event.type == SDL_EVENT_KEY_DOWN && event.key.scancode == SDL_SCANCODE_ESCAPE))
            {
                running = false;
            }
        }

        renderer.setDrawColor({18, 24, 32, 255});
        renderer.clear();
        renderer.drawTexture(sprite->getTexture().get(), destination, &source);
        renderer.present();
    }

    sprites.shutdown();
    renderer.shutdown();
    SDL_Quit();
    return 0;
}