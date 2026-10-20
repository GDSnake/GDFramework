#pragma once

#include <array>
#include <memory>
#include <string>

#include "Config/Config.h"
#include "Input/InputManager.h"
#include "Scene.h"

struct SDL_Texture;
struct TTF_Font;
class Sprite;

class TestScene : public Scene
{
public:
    void init() override;
    void handleInput() override;
    void update(float deltaTime) override;
    void cleanup() override;
    std::string getName() const override;

    bool shouldQuit() const override;
    std::unique_ptr<Scene> getNextScene() override;
    void handleEvents(const SDL_Event& event) override;

private:
    void render() override;
    void renderHud();

    struct TrafficCar
    {
        float x;
        float y;
        float speed;
        std::size_t frameIndex;
    };

    std::unique_ptr<InputManager> _input;
    const ConfigData& _configData = Config::getInstance().getConfigData();
    std::shared_ptr<Sprite> _road;
    std::shared_ptr<Sprite> _cars;
    std::array<TrafficCar, 3> _trafficCars{};
    TTF_Font* _font = nullptr;
    std::string _title;
    float _playerX = 480.0f;
    float _scrollOffset = 0.0f;
    float _speed = 280.0f;
};
