#include <SDL3/SDL.h>

#include "Config/Config.h"
#include "Core/Renderer.h"

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

    const float squareSize = 220.0f;
    const SDL_FRect square{
        (static_cast<float>(config.getConfigData().width) - squareSize) * 0.5f,
        (static_cast<float>(config.getConfigData().height) - squareSize) * 0.5f,
        squareSize,
        squareSize
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
        SDL_SetRenderDrawColor(renderer.getRenderer(), 56, 190, 170, 255);
        SDL_RenderFillRect(renderer.getRenderer(), &square);
        renderer.present();
    }

    renderer.shutdown();
    SDL_Quit();
    return 0;
}