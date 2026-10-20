#include "TestScene.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <utility>

#include <SDL3/SDL_render.h>

#include "AssetManagers/FontsManager.h"
#include "AssetManagers/SpriteManager.h"
#include "Core/Renderer.h"
#include "Entities/Sprite.h"

namespace
{
    constexpr float CarWidth = 64.0f;
    constexpr float CarHeight = 128.0f;
    constexpr float PlayerScreenX = 480.0f;
    constexpr float PlayerY = 460.0f;
    constexpr float SteeringSpeed = 360.0f;
    constexpr float Acceleration = 420.0f;
    constexpr float Braking = 720.0f;
    constexpr float RollingResistance = 180.0f;
    constexpr float MaximumSpeed = 520.0f;
    constexpr float MaximumReverseSpeed = 220.0f;
    constexpr float TurningRate = 120.0f;
    constexpr b2Vec2 PlayerCollisionHalfSize{ 0.2f, 0.35f };
    constexpr float VisibleMapColumns = 6.0f;
    constexpr float VisibleMapRows = 4.0f;
    constexpr float MapRoadStartColumn = 14.0f;
    constexpr float MapRoadStartRow = 57.0f;
    constexpr float RightCollisionInset = 0.08f;
}

void TestScene::init()
{
    _input = std::make_unique<InputManager>();
    _title = "Screen Scrolling Racer";
    _font = FontsManager::getInstance().getFont(FontNames::DefaultFont);

    _road = SpriteManager::getInstance().getSprite("road");
    _cars = SpriteManager::getInstance().getSprite("cars");
    loadMap();
    _mapTileSize = std::min(
        static_cast<float>(_configData.width) / VisibleMapColumns,
        static_cast<float>(_configData.height) / VisibleMapRows);
    _playerWorldX = (MapRoadStartColumn + 0.5f) * _mapTileSize;
    _cameraX = _playerWorldX - PlayerScreenX;
    _scrollOffset = MapRoadStartRow * _mapTileSize;
    createCollisionWorld();

}

void TestScene::createCollisionWorld()
    {
        b2WorldDef worldDefinition = b2DefaultWorldDef();
        worldDefinition.gravity = { 0.0f, 0.0f };
        _worldId = b2CreateWorld(&worldDefinition);

        for (std::size_t row = 0; row < _map.size(); ++row)
        {
            for (std::size_t column = 0; column < _map[row].size(); ++column)
            {
                if (_map[row][column] != 'C')
                {
                    continue;
                }

                const auto isRoad = [this](const std::size_t neighborRow, const std::size_t neighborColumn)
                {
                    if (neighborRow >= _map.size() || neighborColumn >= _map[neighborRow].size())
                    {
                        return false;
                    }
                    const char tile = _map[neighborRow][neighborColumn];
                    return (tile >= '1' && tile <= '9') || tile == 'B';
                };

                const bool roadAbove = row > 0 && isRoad(row - 1, column);
                const bool roadBelow = isRoad(row + 1, column);
                const bool roadLeft = column > 0 && isRoad(row, column - 1);
                const bool roadRight = isRoad(row, column + 1);
                const auto addBarrier = [this](const b2Vec2 position, const b2Vec2 halfSize)
                {
                    b2BodyDef bodyDefinition = b2DefaultBodyDef();
                    bodyDefinition.position = position;
                    const b2BodyId body = b2CreateBody(_worldId, &bodyDefinition);
                    b2ShapeDef shapeDefinition = b2DefaultShapeDef();
                    shapeDefinition.material.friction = 0.0f;
                    const b2Polygon polygon = b2MakeBox(halfSize.x, halfSize.y);
                    b2CreatePolygonShape(body, &shapeDefinition, &polygon);
                    _collisionBodies.push_back({ body, halfSize });
                };

                if (roadAbove || roadBelow)
                {
                    addBarrier(
                        {
                            static_cast<float>(column) + 0.5f,
                            roadAbove ? static_cast<float>(row) : static_cast<float>(row + 1)
                        },
                        { 0.5f, 0.04f });
                }
                if (roadLeft || roadRight)
                {
                    addBarrier(
                        {
                            roadLeft
                                ? static_cast<float>(column)
                                : static_cast<float>(column + 1) - RightCollisionInset,
                            static_cast<float>(row) + 0.5f
                        },
                        { 0.04f, 0.5f });
                }
            }
        }

        b2BodyDef playerDefinition = b2DefaultBodyDef();
        playerDefinition.type = b2_dynamicBody;
        playerDefinition.position = {
            _playerWorldX / _mapTileSize,
            (_scrollOffset + PlayerY + CarHeight * 0.5f) / _mapTileSize
        };
        playerDefinition.linearDamping = 0.0f;
        playerDefinition.fixedRotation = true;
        _playerBody = b2CreateBody(_worldId, &playerDefinition);

        b2ShapeDef playerShapeDefinition = b2DefaultShapeDef();
        playerShapeDefinition.density = 1.0f;
        playerShapeDefinition.material.friction = 0.0f;
        const b2Polygon playerPolygon = b2MakeBox(
            PlayerCollisionHalfSize.x, PlayerCollisionHalfSize.y);
        b2CreatePolygonShape(_playerBody, &playerShapeDefinition, &playerPolygon);
}

void TestScene::destroyCollisionWorld()
    {
        _collisionBodies.clear();
        _playerBody = b2_nullBodyId;
        if (_worldId.index1 != 0)
        {
            b2DestroyWorld(_worldId);
            _worldId = b2_nullWorldId;
        }
}

void TestScene::loadMap()
{
    std::ifstream mapFile("maps/map01.txt");
    if (!mapFile)
    {
        return;
    }

    std::string row;
    while (std::getline(mapFile, row))
    {
        if (!row.empty() && row.back() == '\r')
        {
            row.pop_back();
        }
        if (!row.empty())
        {
            _map.push_back(std::move(row));
        }
    }
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
    if (_input->isKeyPressed(SDL_SCANCODE_M))
    {
        _showCollisionBoxes = !_showCollisionBoxes;
    }
}

void TestScene::update(const float deltaTime)
{
    _input->update();

    float steering = 0.0f;
    if (_input->isKeyHeld(SDL_SCANCODE_A))
    {
        steering -= 1.0f;
    }
    if (_input->isKeyHeld(SDL_SCANCODE_D))
    {
        steering += 1.0f;
    }

    if (_input->isKeyHeld(SDL_SCANCODE_W))
    {
        _speed = std::min(_speed + Acceleration * deltaTime, MaximumSpeed);
    }
    else if (_input->isKeyHeld(SDL_SCANCODE_S))
    {
        if (_speed > 0.0f)
        {
            _speed = std::max(_speed - Braking * deltaTime, 0.0f);
        }
        else
        {
            _speed = std::max(_speed - Acceleration * deltaTime, -MaximumReverseSpeed);
        }
    }
    else
    {
        if (_speed > 0.0f)
        {
            _speed = std::max(_speed - RollingResistance * deltaTime, 0.0f);
        }
        else
        {
            _speed = std::min(_speed + RollingResistance * deltaTime, 0.0f);
        }
    }

    const float speedRatio = std::clamp(std::abs(_speed) / MaximumSpeed, 0.0f, 1.0f);
    _playerAngle += steering * TurningRate * (0.25f + speedRatio) * deltaTime;
    const float headingRadians = _playerAngle * 0.0174532925f;
    const float lateralSpeed = std::sin(headingRadians) * _speed;
    const float forwardSpeed = std::cos(headingRadians) * _speed;
    b2Body_SetTransform(_playerBody, b2Body_GetPosition(_playerBody), b2MakeRot(headingRadians));
    b2Body_SetLinearVelocity(_playerBody, {
        lateralSpeed / _mapTileSize,
        -forwardSpeed / _mapTileSize
    });
    b2World_Step(_worldId, deltaTime, 4);

    const b2Vec2 playerPosition = b2Body_GetPosition(_playerBody);
    _playerWorldX = playerPosition.x * _mapTileSize;
    const float mapWidth = _map.empty()
        ? static_cast<float>(_configData.width)
        : static_cast<float>(_map.front().size()) * _mapTileSize;
    _playerWorldX = std::clamp(_playerWorldX, CarWidth * 0.5f, mapWidth - CarWidth * 0.5f);
    _cameraX = std::clamp(
        _playerWorldX - PlayerScreenX,
        0.0f,
        std::max(0.0f, mapWidth - static_cast<float>(_configData.width)));

    _scrollOffset = playerPosition.y * _mapTileSize - PlayerY - CarHeight * 0.5f;

    render();
}

void TestScene::render()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.setDrawColor({ 32, 86, 44, 255 });
    renderer.clear();

    renderMap();
    if (_showCollisionBoxes)
    {
        renderCollisionBoxes();
    }

    if (_cars)
    {
        const b2Vec2 playerPosition = b2Body_GetPosition(_playerBody);
        const SDL_FRect player{
            _playerWorldX - _cameraX - CarWidth * 0.5f,
            playerPosition.y * _mapTileSize - _scrollOffset - CarHeight * 0.5f,
            CarWidth,
            CarHeight
        };
        renderer.drawTextureRotated(
            _cars->getTexture().get(), player, &_cars->getSourceRect(0), _playerAngle);
    }

    renderHud();
    renderer.present();
}

void TestScene::renderCollisionBoxes()
{
    SDL_Renderer* rendererHandle = Renderer::getInstance().getRenderer();
    if (!rendererHandle)
    {
        return;
    }

    SDL_SetRenderDrawColor(rendererHandle, 255, 64, 64, 255);
    for (const CollisionBox& collision : _collisionBodies)
    {
        const b2Vec2 position = b2Body_GetPosition(collision.body);
        const SDL_FRect rectangle{
            position.x * _mapTileSize - _cameraX - collision.halfSize.x * _mapTileSize,
            position.y * _mapTileSize - _scrollOffset - collision.halfSize.y * _mapTileSize,
            collision.halfSize.x * 2.0f * _mapTileSize,
            collision.halfSize.y * 2.0f * _mapTileSize
        };
        SDL_RenderRect(rendererHandle, &rectangle);
    }

    if (_playerBody.index1 != 0)
    {
        const b2Vec2 position = b2Body_GetPosition(_playerBody);
        const b2Rot rotation = b2Body_GetRotation(_playerBody);
        const b2Vec2 corners[] = {
            { -PlayerCollisionHalfSize.x, -PlayerCollisionHalfSize.y },
            { PlayerCollisionHalfSize.x, -PlayerCollisionHalfSize.y },
            { PlayerCollisionHalfSize.x, PlayerCollisionHalfSize.y },
            { -PlayerCollisionHalfSize.x, PlayerCollisionHalfSize.y },
            { -PlayerCollisionHalfSize.x, -PlayerCollisionHalfSize.y }
        };
        SDL_FPoint screenCorners[5]{};
        for (int index = 0; index < 5; ++index)
        {
            const float worldX = position.x
                + rotation.c * corners[index].x - rotation.s * corners[index].y;
            const float worldY = position.y
                + rotation.s * corners[index].x + rotation.c * corners[index].y;
            screenCorners[index] = {
                worldX * _mapTileSize - _cameraX,
                worldY * _mapTileSize - _scrollOffset
            };
        }
        SDL_SetRenderDrawColor(rendererHandle, 64, 160, 255, 255);
        SDL_RenderLines(rendererHandle, screenCorners, 5);
    }
}

void TestScene::renderMap()
{
    if (!_road || _map.empty())
    {
        return;
    }

    Renderer& renderer = Renderer::getInstance();
    for (std::size_t row = 0; row < _map.size(); ++row)
    {
        for (std::size_t column = 0; column < _map[row].size(); ++column)
        {
            const char tile = _map[row][column];
            std::size_t frameIndex = 0;
            if (tile >= '1' && tile <= '9')
            {
                frameIndex = static_cast<std::size_t>(tile - '1');
            }
            else if (tile == 'B')
            {
                frameIndex = 9;
            }
            else
            {
                continue;
            }

            const SDL_FRect destination{
                static_cast<float>(column) * _mapTileSize - _cameraX,
                static_cast<float>(row) * _mapTileSize - _scrollOffset,
                _mapTileSize,
                _mapTileSize
            };
            renderer.drawTexture(
                _road->getTexture().get(), destination, &_road->getSourceRect(frameIndex));
        }
    }
}

void TestScene::renderHud()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.drawText(_font, _title, { 20.0f, 16.0f }, { 255, 255, 255, 255 });
    renderer.drawText(
        _font,
        "W: accelerate   S: brake/reverse   A/D: steer   M: collisions   ESC: exit",
        { 20.0f, static_cast<float>(_configData.height) - 44.0f },
        { 255, 226, 140, 255 });
}

void TestScene::cleanup()
{
    destroyCollisionWorld();
    _collisionBodies.clear();
    _font = nullptr;
    _cars.reset();
    _road.reset();
    _map.clear();
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
