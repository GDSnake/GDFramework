#pragma once

#include <SDL3/SDL_render.h>

#include <memory>
#include <string>

class Sprite
{
public:
    Sprite() = delete;
    explicit Sprite(const std::string& filePath, float width, float height);

    std::shared_ptr<SDL_Texture> getTexture() const noexcept;
    const SDL_FRect& getSourceRect() const noexcept;

private:
    std::shared_ptr<SDL_Texture> _texture;
    SDL_FRect _sourceRect{};
};