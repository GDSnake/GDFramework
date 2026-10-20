#pragma once

#include <SDL3/SDL_render.h>

#include <memory>
#include <string>
#include <vector>

class Sprite
{
public:
    Sprite() = delete;
    explicit Sprite(const std::string& filePath, float width, float height);
    Sprite(const std::string& filePath, float width, float height,
        float frameWidth, float frameHeight, int columns = 0, int rows = 0,
        std::vector<SDL_FRect> frames = {});

    std::shared_ptr<SDL_Texture> getTexture() const noexcept;
    const SDL_FRect& getSourceRect() const noexcept;
    const SDL_FRect& getSourceRect(std::size_t frameIndex) const;
    std::size_t getFrameCount() const noexcept;

private:
    std::shared_ptr<SDL_Texture> _texture;
    SDL_FRect _sourceRect{};
    std::vector<SDL_FRect> _frames;
};