#pragma once

#include <cstddef>
#include <memory>
#include <random>
#include <string>

#include "Config/Config.h"
#include "Entities/Entities.h"
#include "Input/InputManager.h"
#include "Scene.h"

struct TTF_Font;

class TestScene : public Scene
{
public:
    void init() override;
    void handleEvents(const SDL_Event& event) override;
    void handleInput() override;
    void update(float deltaTime) override;
    void render() override;
    void cleanup() override;
    std::string getName() const override;
    bool shouldQuit() const override;
    std::unique_ptr<Scene> getNextScene() override;

    bool captureFrame(const std::string& path);

private:
    ParticleData makeParticle(std::size_t particleIndex, bool prewarm = false);
    float random(float minimum, float maximum);
    void renderParticles();

    static constexpr std::size_t SparkCount = 96;
    static constexpr std::size_t SquareCount = 32;

    std::unique_ptr<InputManager> _input;
    std::shared_ptr<Sprite> _sheet;
    EntityData _entity{};
    SDL_FPoint _mousePosition{};
    std::mt19937 _random{0xF1A4E5u};
    TTF_Font* _font = nullptr;
    float _elapsed = 0.0f;
    std::string _capturePath;
    bool _captureSucceeded = false;
};