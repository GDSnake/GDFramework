#include "Sprite.h"

#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

#include <cmath>
#include <format>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Core/Renderer.h"

Sprite::Sprite(const std::string& filePath, float width, float height)
    : Sprite(filePath, width, height, 0.0f, 0.0f)
{
}

Sprite::Sprite(const std::string& filePath, float width, float height,
    const float frameWidth, const float frameHeight, const int columns, const int rows,
    std::vector<SDL_FRect> frames)
    : _texture(nullptr, SDL_DestroyTexture)
{
    if (!std::isfinite(width) || !std::isfinite(height) || width < 0.0f || height < 0.0f
        || !std::isfinite(frameWidth) || !std::isfinite(frameHeight)
        || frameWidth < 0.0f || frameHeight < 0.0f || columns < 0 || rows < 0)
    {
        throw std::invalid_argument("Sprite dimensions and sheet metadata must be valid");
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
    if (!frames.empty())
    {
        for (const SDL_FRect& frame : frames)
        {
            if (!std::isfinite(frame.x) || !std::isfinite(frame.y)
                || !std::isfinite(frame.w) || !std::isfinite(frame.h)
                || frame.x < 0.0f || frame.y < 0.0f
                || frame.w <= 0.0f || frame.h <= 0.0f
                || frame.x + frame.w > width || frame.y + frame.h > height)
            {
                throw std::invalid_argument("Sprite sheet frame must fit within the texture");
            }
        }
        _frames = std::move(frames);
    }
    else if (frameWidth > 0.0f || frameHeight > 0.0f)
    {
        if (frameWidth <= 0.0f || frameHeight <= 0.0f
            || frameWidth > width || frameHeight > height)
        {
            throw std::invalid_argument("Sprite sheet frame dimensions must fit within the texture");
        }

        const int frameColumns = columns > 0
            ? columns
            : static_cast<int>(width / frameWidth);
        const int frameRows = rows > 0
            ? rows
            : static_cast<int>(height / frameHeight);
        if (frameColumns <= 0 || frameRows <= 0
            || frameColumns * frameWidth > width || frameRows * frameHeight > height)
        {
            throw std::invalid_argument("Sprite sheet grid does not fit within the texture");
        }

        _frames.reserve(static_cast<std::size_t>(frameColumns * frameRows));
        for (int row = 0; row < frameRows; ++row)
        {
            for (int column = 0; column < frameColumns; ++column)
            {
                _frames.push_back({
                    static_cast<float>(column) * frameWidth,
                    static_cast<float>(row) * frameHeight,
                    frameWidth,
                    frameHeight
                });
            }
        }
    }
    else
    {
        _frames.push_back(_sourceRect);
    }
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

const SDL_FRect& Sprite::getSourceRect(const std::size_t frameIndex) const
{
    if (frameIndex >= _frames.size())
    {
        throw std::out_of_range("Sprite frame index is out of range");
    }
    return _frames[frameIndex];
}

std::size_t Sprite::getFrameCount() const noexcept
{
    return _frames.size();
}