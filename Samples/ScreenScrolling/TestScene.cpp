#include "TestScene.h"

#include <algorithm>
#include <array>
#include <memory>

#include "AssetManagers/FontsManager.h"
#include "AssetManagers/SpriteManager.h"
#include "Core/Renderer.h"
#include "Entities/Sprite.h"

namespace
{
    constexpr float CarWidth = 64.0f;
    constexpr float CarHeight = 128.0f;
    constexpr float RoadWidth = 720.0f;
    constexpr float RoadHeight = 720.0f;
    constexpr float RoadX = 120.0f;
    constexpr float PlayerY = 460.0f;
    constexpr float SteeringSpeed = 360.0f;
    constexpr SDL_FRect PlayerSource{ 0.0f, 0.0f, 32.0f, 64.0f };
}

void TestScene::init()
{
    _input = std::make_unique<InputManager>();
    _title = "Screen Scrolling Racer";
    _font = FontsManager::getInstance().getFont(FontNames::DefaultFont);

    _road = SpriteManager::getInstance().getSprite("road");
    _cars = SpriteManager::getInstance().getSprite("cars");

    _trafficCars = {{
        { 360.0f, 80.0f, 1.0f, 1 },
        { 500.0f, 300.0f, 0.75f, 2 },
        { 625.0f, 560.0f, 1.15f, 3 }
    }};
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
    if (!_input)
    {
        return;
    }

    if (_input->isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        _input->quit();
    }

    if (_input->isKeyHeld(SDL_SCANCODE_LEFT) || _input->isKeyHeld(SDL_SCANCODE_A))
    {
        _playerX -= SteeringSpeed / 60.0f;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_RIGHT) || _input->isKeyHeld(SDL_SCANCODE_D))
    {
        _playerX += SteeringSpeed / 60.0f;
    }
    _playerX = std::clamp(_playerX, RoadX + 90.0f, RoadX + RoadWidth - 90.0f);

    if (_input->isKeyHeld(SDL_SCANCODE_UP) || _input->isKeyHeld(SDL_SCANCODE_W))
    {
        _speed = std::min(_speed + 4.0f, 520.0f);
    }
    if (_input->isKeyHeld(SDL_SCANCODE_DOWN) || _input->isKeyHeld(SDL_SCANCODE_S))
    {
        _speed = std::max(_speed - 4.0f, 120.0f);
    }
}

void TestScene::update(const float deltaTime)
{
    _input->update();
    _scrollOffset += _speed * deltaTime;
    while (_scrollOffset >= RoadHeight)
    {
        _scrollOffset -= RoadHeight;
    }

    for (TrafficCar& car : _trafficCars)
    {
        car.y += _speed * car.speed * deltaTime;
        if (car.y > static_cast<float>(_configData.height) + CarHeight)
        {
            car.y = -CarHeight;
        }
    }

    render();
}

void TestScene::render()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.setDrawColor({ 32, 86, 44, 255 });
    renderer.clear();

    if (_road)
    {
        const SDL_FRect& roadSource = _road->getSourceRect();
        const float firstRoadY = -RoadHeight + _scrollOffset;
        const SDL_FRect firstRoad{ RoadX, firstRoadY, RoadWidth, RoadHeight };
        const SDL_FRect secondRoad{ RoadX, firstRoadY + RoadHeight, RoadWidth, RoadHeight };
        renderer.drawTexture(_road->getTexture().get(), firstRoad, &roadSource);
        renderer.drawTexture(_road->getTexture().get(), secondRoad, &roadSource);
    }

    if (_cars)
    {
        for (const TrafficCar& car : _trafficCars)
        {
            const SDL_FRect destination{ car.x, car.y, CarWidth, CarHeight };
            renderer.drawTexture(
                _cars->getTexture().get(), destination, &_cars->getSourceRect(car.frameIndex));
        }

        const SDL_FRect player{ _playerX - CarWidth * 0.5f, PlayerY, CarWidth, CarHeight };
        renderer.drawTexture(_cars->getTexture().get(), player, &PlayerSource);
    }

    renderHud();
    renderer.present();
}

void TestScene::renderHud()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.drawText(_font, _title, { 20.0f, 16.0f }, { 255, 255, 255, 255 });
    renderer.drawText(
        _font,
        "A/D or arrows: steer   W/S: speed   ESC: exit",
        { 20.0f, static_cast<float>(_configData.height) - 44.0f },
        { 255, 226, 140, 255 });
}

void TestScene::cleanup()
{
    _font = nullptr;
    _cars.reset();
    _road.reset();
    _input.reset();
}

std::string TestScene::getName() const
{
    return "ScreenScrollingRacer";
}

std::unique_ptr<Scene> TestScene::getNextScene()
{
    return {};
}
