#pragma once

#include <string>
#include <memory>
#include <vector>

#include <box2d/box2d.h>

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
    void loadMap();
    void createCollisionWorld();
    void destroyCollisionWorld();
    void render() override;
    void renderMap();
    void renderCollisionBoxes();
    void renderHud();

    struct CollisionBox
    {
        b2BodyId body;
        b2Vec2 halfSize;
    };

    std::unique_ptr<InputManager> _input;
    const ConfigData& _configData = Config::getInstance().getConfigData();
    std::shared_ptr<Sprite> _road;
    std::shared_ptr<Sprite> _cars;
    std::vector<std::string> _map;
    b2WorldId _worldId = b2_nullWorldId;
    b2BodyId _playerBody = b2_nullBodyId;
    std::vector<CollisionBox> _collisionBodies;
    TTF_Font* _font = nullptr;
    std::string _title;
    float _playerWorldX = 0.0f;
    float _cameraX = 0.0f;
    float _playerAngle = 0.0f;
    float _mapTileSize = 160.0f;
    float _scrollOffset = 0.0f;
    float _speed = 0.0f;
    bool _showCollisionBoxes = false;
};
