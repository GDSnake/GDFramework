#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Config/Config.h"
#include "Input/InputManager.h"
#include "Scene.h"

struct TTF_Font;
class Sprite;

class TestScene : public Scene
{
public:
    void init() override;
    void handleInput() override;
    void update(const float deltaTime) override;
    void cleanup() override;
    std::string getName() const override;

    bool shouldQuit() const override;
    void handleEvents(const SDL_Event& event) override;

private:
    void render() override;
    void renderTexts();

    std::unique_ptr<InputManager> _input;
    const ConfigData& _configData = Config::getInstance().getConfigData();
    std::vector<std::shared_ptr<Sprite>> _blockSprites;
    std::shared_ptr<Sprite> _sprite;

    TTF_Font* _titleFont = nullptr;
    std::string _titleText;
};
