#pragma once

#include <memory>
#include <string>

#include "Config/Config.h"
#include "Scene.h"
#include "Input/InputManager.h"

struct TTF_Font;

class GameScene : public Scene
{
public:
    void init() override;
    void handleInput() override;
    void update(float deltaTime) override;
    void cleanup() override;
    std::string getName() const override;
    
    bool shouldQuit() const;
    void handleEvents(const SDL_Event& event) override;

private:
    void render() override;
    void renderTexts();
    
    void updateStrings();

    std::unique_ptr<InputManager> _input;
    const ConfigData& _configData = Config::getInstance().getConfigData();

    // Text
    TTF_Font* _titleFont = nullptr;
    std::string _titleText;
};