#pragma once

#include "Scene.h"

class GameScene : public Scene
{
public:
    void init() override;
    void handleEvents(const SDL_Event& event) override;
    void handleInput() override;
    void update(float deltaTime) override;
    void render() override;
    void cleanup() override;
    std::string getName() const override;
};