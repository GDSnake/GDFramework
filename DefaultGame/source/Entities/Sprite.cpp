#include "Sprite.h"

#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

#include <cmath>
#include <format>
#include <memory>
#include <stdexcept>

#include "Core/Renderer.h"

Sprite::Sprite(const std::string& filePath, float width, float height)
    : _texture(nullptr, SDL_DestroyTexture)
{
    if (!std::isfinite(width) || !std::isfinite(height) || width < 0.0f || height < 0.0f)
    {
        throw std::invalid_argument("Sprite dimensions must be finite and non-negative");
    }

    std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface(
        IMG_Load(filePath.c_str()), SDL_DestroySurface);
    if (!surface)
    {
        throw std::runtime_error(std::format("Failed to load sprite '{}': {}", filePath, SDL_GetError()));
    }

    SDL_Renderer* renderer = Renderer::getInstance().getRenderer();
    if (!renderer)
    {
        throw std::runtime_error("Cannot create sprite texture before the renderer is initialized");
    }

    std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)> texture(
        SDL_CreateTextureFromSurface(renderer, surface.get()), SDL_DestroyTexture);
    if (!texture)
    {
        throw std::runtime_error(std::format("Failed to create sprite texture '{}': {}", filePath, SDL_GetError()));
    }


    if (!SDL_GetTextureSize(texture.get(), &width, &height))
    {
        throw std::runtime_error(std::format("Failed to get sprite dimensions '{}': {}", filePath, SDL_GetError()));
    }

    _sourceRect = {0.0f, 0.0f, width, height};
    _texture = std::shared_ptr<SDL_Texture>(texture.release(), SDL_DestroyTexture);
}

std::shared_ptr<SDL_Texture> Sprite::getTexture() const noexcept
{
    return _texture;
}

const SDL_FRect& Sprite::getSourceRect() const noexcept
{
    return _sourceRect;
}