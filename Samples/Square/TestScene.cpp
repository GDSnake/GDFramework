#include "Input/InputManager.h"
#include "TestScene.h"

#include <array>

#include "AssetManagers/FontsManager.h"
#include "Core/Renderer.h"

namespace
{
    constexpr std::array<SDL_Color, 4> SquareColors = {{
        { 255, 0, 0, 255 },
        { 0, 255, 0, 255 },
        { 0, 0, 255, 255 },
        { 255, 255, 0, 255 }
    }};

    SDL_Color interpolateColor(const SDL_Color& from, const SDL_Color& to, const float amount)
    {
        return {
            static_cast<Uint8>(from.r + (to.r - from.r) * amount),
            static_cast<Uint8>(from.g + (to.g - from.g) * amount),
            static_cast<Uint8>(from.b + (to.b - from.b) * amount),
            255
        };
    }

    void drawGradientSquare(SDL_Renderer* renderer, const SDL_FRect& square,
        const std::array<SDL_Color, 4>& corners)
    {
        const int width = static_cast<int>(square.w);
        const int height = static_cast<int>(square.h);

        for (int y = 0; y < height; ++y)
        {
            const float verticalAmount = static_cast<float>(y) / static_cast<float>(height - 1);
            const SDL_Color leftColor = interpolateColor(corners[0], corners[3], verticalAmount);
            const SDL_Color rightColor = interpolateColor(corners[1], corners[2], verticalAmount);

            for (int x = 0; x < width; ++x)
            {
                const float horizontalAmount = static_cast<float>(x) / static_cast<float>(width - 1);
                const SDL_Color color = interpolateColor(leftColor, rightColor, horizontalAmount);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

                const SDL_FRect pixel{
                    square.x + static_cast<float>(x),
                    square.y + static_cast<float>(y),
                    1.0f,
                    1.0f
                };
                SDL_RenderFillRect(renderer, &pixel);
            }
        }
    }
}

void TestScene::init()
{
    _input = std::make_unique<InputManager>();
    _titleText = TitleFontInfo.baseText + "Test Square Scene";
    _titleFont = FontsManager::getInstance().getFont(TitleFontInfo.fontName);
}

void TestScene::handleEvents(const SDL_Event& event)
{
    _input->handleEvent(event);
}

bool TestScene::shouldQuit() const
{
    return _input && _input->shouldQuit();
}

void TestScene::handleInput()
{
    if (_input && _input->isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        _input->quit();
    }
}

void TestScene::update(const float deltaTime)
{
    if (_input)
    {
        _input->update();
    }

    _colorTimer += deltaTime;
    while (_colorTimer >= 5.0f)
    {
        _colorTimer -= 5.0f;
        _colorIndex = (_colorIndex + 1) % SquareColors.size();
    }

    updateStrings();
    render();
}

void TestScene::render()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.setDrawColor({ 0, 0, 0, 255 });
    renderer.clear();

    const float transitionAmount = _colorTimer / 5.0f;
    const std::array<SDL_Color, 4> corners = {
        interpolateColor(SquareColors[_colorIndex],
            SquareColors[(_colorIndex + 1) % SquareColors.size()], transitionAmount),
        interpolateColor(SquareColors[(_colorIndex + 1) % SquareColors.size()],
            SquareColors[(_colorIndex + 2) % SquareColors.size()], transitionAmount),
        interpolateColor(SquareColors[(_colorIndex + 2) % SquareColors.size()],
            SquareColors[(_colorIndex + 3) % SquareColors.size()], transitionAmount),
        interpolateColor(SquareColors[(_colorIndex + 3) % SquareColors.size()],
            SquareColors[_colorIndex], transitionAmount)
    };

    constexpr float squareSize = 160.0f;
    const SDL_FRect square = {
        (static_cast<float>(_configData.width) - squareSize) * 0.5f,
        (static_cast<float>(_configData.height) - squareSize) * 0.5f,
        squareSize,
        squareSize
    };

    drawGradientSquare(renderer.getRenderer(), square, corners);

    renderTexts();
    renderer.present();
}

void TestScene::renderTexts()
{
    Renderer::getInstance().drawText(
        _titleFont,
        _titleText,
        TitleFontInfo.origin,
        TitleFontInfo.color);
    Renderer::getInstance().drawText(
        _titleFont,
        ExitFontInfo.baseText,
        ExitFontInfo.origin,
        ExitFontInfo.color);
}

void TestScene::updateStrings()
{
}

void TestScene::cleanup()
{
    _titleFont = nullptr;
    _input.reset();
}

std::string TestScene::getName() const
{
    return "TestScene";
}
