#pragma once

#include <array>
#include <memory>
#include <string>
#include <vector>

#include <box2d/box2d.h>

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
    void createPhysicsWorld();
    void destroyPhysicsWorld();
    void moveSelectedBody();

    std::unique_ptr<InputManager> _input;
    const ConfigData& _configData = Config::getInstance().getConfigData();
    std::vector<std::shared_ptr<Sprite>> _blockSprites;
    std::array<b2BodyId, 9> _blockBodies{};
    b2WorldId _worldId = b2_nullWorldId;
    std::size_t _selectedBlock = 0;
    std::shared_ptr<Sprite> _sprite;

    TTF_Font* _titleFont = nullptr;
    std::string _titleText;
};
