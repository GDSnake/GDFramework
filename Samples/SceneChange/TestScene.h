#pragma once

#include <memory>
#include <string>

#include "Config/Config.h"
#include "Input/InputManager.h"
#include "Scene.h"

struct TTF_Font;

class TestScene : public Scene
{
public:
    void init() override;
    void handleInput() override;
    void update(float deltaTime) override;
    void cleanup() override;
    std::string getName() const override;

    bool shouldQuit() const override;
    void handleEvents(const SDL_Event& event) override;

private:
    void render() override;
    void renderTexts();

    std::unique_ptr<InputManager> _input;
    const ConfigData& _configData = Config::getInstance().getConfigData();
    TTF_Font* _titleFont = nullptr;
    std::string _titleText;
};
