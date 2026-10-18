#include "Input/InputManager.h"
#include "TestScene.h"

#include "AssetManagers/AudioManager.h"
#include "AssetManagers/FontsManager.h"
#include "AssetManagers/SpriteManager.h"
#include "Core/Renderer.h"
#include "Entities/Sprite.h"
#include "TestConfig/TestConfig.h"

namespace
{
    // Converts Box2D world units to SDL screen pixels.
    constexpr float PixelsPerMeter = 64.0f;
    constexpr float BlockSize = 1.0f;
    constexpr float MoveSpeed = 6.0f;
}

void TestScene::init()
{
    _input = std::make_unique<InputManager>();
    _titleText = TitleFontInfo.baseText + "Image Viewer";
    _titleFont = FontsManager::getInstance().getFont(TitleFontInfo.fontName);
    _sprite = SpriteManager::getInstance().getSprite("sample");
    AudioManager::getInstance().playMusic();

    for (const ImageConfigData& image : TestConfig::getInstance().getConfigData().blockImages)
    {
        _blockSprites.push_back(std::make_shared<Sprite>(image.path, image.width, image.height));
    }

    createPhysicsWorld();
}

void TestScene::handleEvents(const SDL_Event& event)
{
    if (_input)
    {
        _input->handleEvent(event);
    }
}

bool TestScene::shouldQuit() const
{
    return _input && _input->shouldQuit();
}

void TestScene::handleInput()
{
    if (_input && _input->isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        _input->quit();
    }

    static constexpr std::array<SDL_Scancode, 9> selectionKeys = {
        SDL_SCANCODE_KP_1, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_3,
        SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_5, SDL_SCANCODE_KP_6,
        SDL_SCANCODE_KP_7, SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_9
    };
    static constexpr std::array<SDL_Scancode, 9> rowSelectionKeys = {
        SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6,
        SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9
    };
    for (std::size_t index = 0; index < selectionKeys.size(); ++index)
    {
        if (_input && (_input->isKeyPressed(selectionKeys[index])
            || _input->isKeyPressed(rowSelectionKeys[index])))
        {
            _selectedBlock = index;
            break;
        }
    }

    moveSelectedBody();
}

void TestScene::update(const float deltaTime)
{
    if (_input)
    {
        _input->update();
    }
    if (_worldId.index1 != 0)
    {
        b2World_Step(_worldId, deltaTime, 4);
    }
    render();
}

void TestScene::createPhysicsWorld()
{
    b2WorldDef worldDefinition = b2DefaultWorldDef();
    worldDefinition.gravity = { 0.0f, 0.0f };
    _worldId = b2CreateWorld(&worldDefinition);

    auto createBody = [this](b2BodyType type, const b2Vec2 position, const b2Vec2 halfSize)
    {
        b2BodyDef bodyDefinition = b2DefaultBodyDef();
        bodyDefinition.type = type;
        bodyDefinition.position = position;
        const b2BodyId body = b2CreateBody(_worldId, &bodyDefinition);

        b2ShapeDef shapeDefinition = b2DefaultShapeDef();
        shapeDefinition.density = type == b2_dynamicBody ? 1.0f : 0.0f;
        shapeDefinition.material.friction = 0.4f;
        const b2Polygon polygon = b2MakeBox(halfSize.x, halfSize.y);
        b2CreatePolygonShape(body, &shapeDefinition, &polygon);
        return body;
    };

    const float width = static_cast<float>(_configData.width) / PixelsPerMeter;
    const float height = static_cast<float>(_configData.height) / PixelsPerMeter;
    // Create static boundaries around the edges of the screen
    createBody(b2_staticBody, { width * 0.5f, -0.25f }, { width * 0.5f, 0.25f });
    createBody(b2_staticBody, { width * 0.5f, height + 0.25f }, { width * 0.5f, 0.25f });
    createBody(b2_staticBody, { -0.25f, height * 0.5f }, { 0.25f, height * 0.5f });
    createBody(b2_staticBody, { width + 0.25f, height * 0.5f }, { 0.25f, height * 0.5f });

    // Create dynamic blocks in a 3x3 grid
    for (std::size_t index = 0; index < _blockBodies.size(); ++index)
    {
        const float column = static_cast<float>(index % 3);
        const float row = static_cast<float>(index / 3);
        _blockBodies[index] = createBody(
            b2_dynamicBody,
            { width * 0.5f + (column - 1.0f) * 1.25f, height * 0.5f + (row - 1.0f) * 1.25f },
            { BlockSize * 0.5f, BlockSize * 0.5f });
    }
}

void TestScene::destroyPhysicsWorld()
{
    if (_worldId.index1 != 0)
    {
        b2DestroyWorld(_worldId);
        _worldId = b2_nullWorldId;
    }
}

void TestScene::moveSelectedBody()
{
    if (_worldId.index1 == 0 || _selectedBlock >= _blockBodies.size())
    {
        return;
    }

    b2Vec2 velocity{ 0.0f, 0.0f };
    if (_input->isKeyHeld(SDL_SCANCODE_LEFT) || _input->isKeyHeld(SDL_SCANCODE_A)) velocity.x -= MoveSpeed;
    if (_input->isKeyHeld(SDL_SCANCODE_RIGHT) || _input->isKeyHeld(SDL_SCANCODE_D)) velocity.x += MoveSpeed;
    if (_input->isKeyHeld(SDL_SCANCODE_UP) || _input->isKeyHeld(SDL_SCANCODE_W)) velocity.y -= MoveSpeed;
    if (_input->isKeyHeld(SDL_SCANCODE_DOWN) || _input->isKeyHeld(SDL_SCANCODE_S)) velocity.y += MoveSpeed;
    b2Body_SetLinearVelocity(_blockBodies[_selectedBlock], velocity);
}

void TestScene::render()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.setDrawColor({ 24, 24, 24, 255 });
    renderer.clear();

    if (_sprite)
    {
        const SDL_FRect& source = _sprite->getSourceRect();
        const SDL_FRect destination{
            (static_cast<float>(_configData.width) - source.w) * 0.5f,
            (static_cast<float>(_configData.height) - source.h) * 0.5f,
            source.w,
            source.h
        };
        renderer.drawTexture(_sprite->getTexture().get(), destination, &source);
    }

    for (std::size_t index = 0; index < _blockSprites.size() && index < _blockBodies.size(); ++index)
    {
        const b2Vec2 position = b2Body_GetPosition(_blockBodies[index]);
        const float blockSize = PixelsPerMeter * BlockSize;
        const SDL_FRect destination{
            position.x * PixelsPerMeter - blockSize * 0.5f,
            position.y * PixelsPerMeter - blockSize * 0.5f,
            blockSize,
            blockSize
        };
        const std::shared_ptr<Sprite>& block = _blockSprites[index];
        renderer.drawTexture(block->getTexture().get(), destination, &block->getSourceRect());
    }

    renderTexts();
    renderer.present();
}

void TestScene::renderTexts()
{
    Renderer::getInstance().drawText(
        _titleFont,
        _titleText,
        TitleFontInfo.origin,
        TitleFontInfo.color);
    Renderer::getInstance().drawText(
        _titleFont,
        ExitFontInfo.baseText,
        ExitFontInfo.origin,
        ExitFontInfo.color);
}

void TestScene::cleanup()
{
    destroyPhysicsWorld();
    _titleFont = nullptr;
    _blockSprites.clear();
    _sprite.reset();
    _input.reset();
}

std::string TestScene::getName() const
{
    return "ImageViewerScene";
}
