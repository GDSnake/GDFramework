#include "Renderer.h"

#include <SDL3/SDL_log.h>
#include <SDL3/SDL_video.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <memory>

#include "Config/Config.h"

Renderer::~Renderer()
{
    shutdown();
}

bool Renderer::init()
{
    if (_window && _renderer)
    {
        return true;
    }

    const ConfigData& config = Config::getInstance().getConfigData();
    const SDL_WindowFlags flags = config.fullscreen ? SDL_WINDOW_FULLSCREEN : 0;
    if (!SDL_CreateWindowAndRenderer(config.title.c_str(), config.width, config.height, flags, &_window, &_renderer))
    {
        SDL_Log("Window/renderer creation failed: %s", SDL_GetError());
        shutdown();
        return false;
    }
    return true;
}

void Renderer::clear() const
{
    if (_renderer)
    {
        SDL_RenderClear(_renderer);
    }
}

void Renderer::present() const
{
    if (_renderer)
    {
        SDL_RenderPresent(_renderer);
    }
}

void Renderer::setDrawColor(const SDL_Color& color) const
{
    if (_renderer)
    {
        SDL_SetRenderDrawColor(_renderer, color.r, color.g, color.b, color.a);
    }
}

void Renderer::drawTexture(SDL_Texture* texture, const SDL_FRect& destination, const SDL_FRect* source) const
{
    if (_renderer && texture)
    {
        SDL_RenderTexture(_renderer, texture, source, &destination);
    }
}

void Renderer::drawTextureRotated(SDL_Texture* texture, const SDL_FRect& destination,
    const SDL_FRect* source, const double angle) const
{
    if (_renderer && texture)
    {
        SDL_RenderTextureRotated(_renderer, texture, source, &destination, angle, nullptr, SDL_FLIP_NONE);
    }
}

void Renderer::drawText(TTF_Font* font, const std::string& text, const SDL_FPoint& origin, const SDL_Color& color) const
{
    if (!_renderer || !font || text.empty())
    {
        return;
    }

    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
        TTF_RenderText_Blended(font, text.c_str(), text.size(), color), SDL_DestroySurface);
    if (!surface)
    {
        return;
    }

    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture(
        SDL_CreateTextureFromSurface(_renderer, surface.get()), SDL_DestroyTexture);
    if (!texture)
    {
        return;
    }

    float width = 0.0f;
    float height = 0.0f;
    if (SDL_GetTextureSize(texture.get(), &width, &height))
    {
        const SDL_FRect destination{origin.x, origin.y, width, height};
        SDL_RenderTexture(_renderer, texture.get(), nullptr, &destination);
    }
}

SDL_Window* Renderer::getWindow() const noexcept
{
    return _window;
}

SDL_Renderer* Renderer::getRenderer() const noexcept
{
    return _renderer;
}

bool Renderer::applyWindowSettings() const
{
    if (!_window)
    {
        return false;
    }

    const ConfigData& config = Config::getInstance().getConfigData();
    if (!SDL_SetWindowFullscreen(_window, config.fullscreen))
    {
        return false;
    }

    return config.fullscreen || SDL_SetWindowSize(_window, config.width, config.height);
}

void Renderer::shutdown()
{
    if (_renderer)
    {
        SDL_DestroyRenderer(_renderer);
        _renderer = nullptr;
    }
    if (_window)
    {
        SDL_DestroyWindow(_window);
        _window = nullptr;
    }
}

#if DEBUG_BUILD
void Renderer::writeFPSOnWindowTitle(Uint64 fps)
{
    std::string windowTitle = std::format("{} - FPS: {}", Config::getInstance().getConfigData().title, fps);
    SDL_SetWindowTitle(_window, windowTitle.c_str());
}
#endif
