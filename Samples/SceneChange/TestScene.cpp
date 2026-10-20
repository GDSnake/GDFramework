#include "Input/InputManager.h"
#include "TestScene.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>

#include "AssetManagers/FontsManager.h"
#include "Core/Renderer.h"
#include "MenuScene.h"

namespace
{
    constexpr std::array<SDL_Color, 4> ShapePalette = {{
        { 79, 199, 170, 255 },
        { 242, 122, 92, 255 },
        { 245, 190, 83, 255 },
        { 126, 171, 255, 255 }
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

    SDL_Color animatedColor(const double time, const double phase)
    {
        const double cyclePosition = std::fmod(time * 0.22 + phase, 1.0);
        const double palettePosition = cyclePosition * static_cast<double>(ShapePalette.size());
        const std::size_t currentColor = static_cast<std::size_t>(palettePosition);
        const float blend = static_cast<float>(palettePosition - static_cast<double>(currentColor));
        return interpolateColor(
            ShapePalette[currentColor],
            ShapePalette[(currentColor + 1) % ShapePalette.size()],
            blend);
    }

    SDL_FPoint rotatePoint(
        const float offsetX,
        const float offsetY,
        const float centerX,
        const float centerY,
        const float angle)
    {
        const float cosine = std::cos(angle);
        const float sine = std::sin(angle);
        return {
            centerX + offsetX * cosine - offsetY * sine,
            centerY + offsetX * sine + offsetY * cosine
        };
    }

    SDL_Vertex makeVertex(const SDL_FPoint& position, const SDL_Color& color)
    {
        constexpr float ColorScale = 1.0f / 255.0f;
        return {
            position,
            {
                static_cast<float>(color.r) * ColorScale,
                static_cast<float>(color.g) * ColorScale,
                static_cast<float>(color.b) * ColorScale,
                static_cast<float>(color.a) * ColorScale
            },
            { 0.0f, 0.0f }
        };
    }

    void drawGradientCircle(
        SDL_Renderer* renderer,
        const float centerX,
        const float centerY,
        const float radius,
        const SDL_Color& centerColor,
        const SDL_Color& edgeColor)
    {
        constexpr std::size_t SegmentCount = 64;
        constexpr float TwoPi = 6.28318530718f;
        std::array<SDL_Vertex, SegmentCount + 2> vertices{};
        std::array<int, SegmentCount * 3> indices{};
        vertices[0] = makeVertex({ centerX, centerY }, centerColor);

        for (std::size_t segment = 0; segment <= SegmentCount; ++segment)
        {
            const float angle = static_cast<float>(segment) / static_cast<float>(SegmentCount) * TwoPi;
            vertices[segment + 1] = makeVertex(
                { centerX + std::cos(angle) * radius, centerY + std::sin(angle) * radius },
                edgeColor);
        }

        for (std::size_t segment = 0; segment < SegmentCount; ++segment)
        {
            const std::size_t index = segment * 3;
            indices[index] = 0;
            indices[index + 1] = static_cast<int>(segment + 1);
            indices[index + 2] = static_cast<int>(segment + 2);
        }

        SDL_RenderGeometry(
            renderer,
            nullptr,
            vertices.data(),
            static_cast<int>(vertices.size()),
            indices.data(),
            static_cast<int>(indices.size()));
    }
}

void TestScene::init()
{
    _input = std::make_unique<InputManager>();
    _titleText = "Scene Change: Shapes";
    _titleFont = FontsManager::getInstance().getFont(TitleFontInfo.fontName);
    _animationTime = 0.0;
}

void TestScene::handleEvents(const SDL_Event& event)
{
    if (_input)
    {
        _input->handleEvent(event);
    }
}

bool TestScene::shouldQuit() const
{
    return _input && _input->shouldQuit();
}

void TestScene::handleInput()
{
    if (_input && _input->isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        _nextScene = std::make_unique<MenuScene>();
    }
}

void TestScene::update(const float deltaTime)
{
    _animationTime += static_cast<double>(std::max(0.0f, deltaTime));
    if (_input)
    {
        _input->update();
    }
    render();
}

void TestScene::render()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.setDrawColor({ 20, 36, 52, 255 });
    renderer.clear();

    SDL_Renderer* drawRenderer = renderer.getRenderer();
    if (drawRenderer)
    {
        int windowWidth = 960;
        int windowHeight = 640;
        if (SDL_Window* window = renderer.getWindow())
        {
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        }

        const float time = static_cast<float>(_animationTime);
        const float shapeCenterY = static_cast<float>(windowHeight) * 0.54f;
        const float rectangleSize = std::min(144.0f, static_cast<float>(windowHeight) * 0.24f);
        const float rectangleCenterX = static_cast<float>(windowWidth) * 0.2f;
        const float rectangleWidth = rectangleSize * (1.0f + 0.1f * std::sin(time * 1.7f));
        const float rectangleHeight = rectangleSize * (1.0f + 0.08f * std::cos(time * 1.7f));
        const float rectangleCenterY = shapeCenterY + 14.0f * std::sin(time * 1.1f);
        const float rectangleAngle = 0.24f * std::sin(time * 0.8f);
        const std::array<SDL_Color, 4> rectangleColors = {{
            animatedColor(_animationTime, 0.0),
            animatedColor(_animationTime, 0.25),
            animatedColor(_animationTime, 0.5),
            animatedColor(_animationTime, 0.75)
        }};
        const std::array<SDL_FPoint, 4> rectangleCorners = {{
            { -rectangleWidth * 0.5f, -rectangleHeight * 0.5f },
            { rectangleWidth * 0.5f, -rectangleHeight * 0.5f },
            { rectangleWidth * 0.5f, rectangleHeight * 0.5f },
            { -rectangleWidth * 0.5f, rectangleHeight * 0.5f }
        }};
        std::array<SDL_Vertex, 4> rectangleVertices{};
        for (std::size_t index = 0; index < rectangleCorners.size(); ++index)
        {
            rectangleVertices[index] = makeVertex(
                rotatePoint(
                    rectangleCorners[index].x,
                    rectangleCorners[index].y,
                    rectangleCenterX,
                    rectangleCenterY,
                    rectangleAngle),
                rectangleColors[index]);
        }
        constexpr std::array<int, 6> quadIndices = { 0, 1, 2, 0, 2, 3 };
        SDL_RenderGeometry(
            drawRenderer,
            nullptr,
            rectangleVertices.data(),
            static_cast<int>(rectangleVertices.size()),
            quadIndices.data(),
            static_cast<int>(quadIndices.size()));

        const int circleCenterX = static_cast<int>(static_cast<float>(windowWidth) * 0.5f + 16.0f * std::sin(time * 0.75f));
        const int circleCenterY = static_cast<int>(shapeCenterY + 10.0f * std::sin(time * 1.3f + 0.8f));
        const int baseCircleRadius = std::min(72, static_cast<int>(static_cast<float>(windowHeight) * 0.12f));
        const int circleRadius = std::max(1, static_cast<int>(static_cast<float>(baseCircleRadius) *
            (1.0f + 0.14f * std::sin(time * 2.2f))));
        const SDL_Color circleEdgeColor = animatedColor(_animationTime, 0.33);
        const SDL_Color circleCenterColor = interpolateColor(
            circleEdgeColor,
            { 255, 255, 255, 255 },
            0.82f);
        drawGradientCircle(
            drawRenderer,
            static_cast<float>(circleCenterX),
            static_cast<float>(circleCenterY),
            static_cast<float>(circleRadius),
            circleCenterColor,
            circleEdgeColor);

        const float triangleCenterX = static_cast<float>(windowWidth) * 0.8f + 12.0f * std::sin(time * 0.9f + 1.4f);
        const float triangleCenterY = shapeCenterY + 10.0f * std::cos(time * 1.1f + 1.4f);
        const float triangleSize = std::min(160.0f, static_cast<float>(windowHeight) * 0.28f) *
            (1.0f + 0.07f * std::sin(time * 1.6f + 0.5f));
        const float triangleAngle = time * 0.55f;
        const std::array<SDL_Color, 3> triangleColors = {{
            animatedColor(_animationTime, 0.08),
            animatedColor(_animationTime, 0.41),
            animatedColor(_animationTime, 0.74)
        }};
        const std::array<SDL_FPoint, 3> triangleCorners = {{
            { 0.0f, -triangleSize * 0.58f },
            { -triangleSize * 0.5f, triangleSize * 0.42f },
            { triangleSize * 0.5f, triangleSize * 0.42f }
        }};
        std::array<SDL_Vertex, 3> triangle{};
        for (std::size_t index = 0; index < triangleCorners.size(); ++index)
        {
            triangle[index] = makeVertex(
                rotatePoint(
                    triangleCorners[index].x,
                    triangleCorners[index].y,
                    triangleCenterX,
                    triangleCenterY,
                    triangleAngle),
                triangleColors[index]);
        }
        SDL_RenderGeometry(drawRenderer, nullptr, triangle.data(), static_cast<int>(triangle.size()), nullptr, 0);
    }

    renderTexts();
    renderer.present();
}

void TestScene::renderTexts()
{
    Renderer& renderer = Renderer::getInstance();
    int windowWidth = 960;
    int windowHeight = 640;
    if (SDL_Window* window = renderer.getWindow())
    {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    }

    renderer.drawText(_titleFont, _titleText, { 32.0f, 28.0f }, { 238, 241, 232, 255 });
    const float labelY = static_cast<float>(windowHeight) * 0.54f + 100.0f;
    renderer.drawText(_titleFont, "Rectangle", { static_cast<float>(windowWidth) * 0.2f - 72.0f, labelY }, animatedColor(_animationTime, 0.0));
    renderer.drawText(_titleFont, "Circle", { static_cast<float>(windowWidth) * 0.5f - 42.0f, labelY }, animatedColor(_animationTime, 0.33));
    renderer.drawText(_titleFont, "Triangle", { static_cast<float>(windowWidth) * 0.8f - 57.0f, labelY }, animatedColor(_animationTime, 0.66));
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

std::unique_ptr<Scene> TestScene::getNextScene()
{
    return std::move(_nextScene);
}
