#include "Input/InputManager.h"
#include "TestScene.h"

#include "AssetManagers/FontsManager.h"
#include "AssetManagers/SpriteManager.h"
#include "Core/Renderer.h"
#include "Entities/Sprite.h"
#include "TestConfig/TestConfig.h"

void TestScene::init()
{
    _input = std::make_unique<InputManager>();
    _titleText = TitleFontInfo.baseText + "Image Viewer";
    _titleFont = FontsManager::getInstance().getFont(TitleFontInfo.fontName);
    _sprite = SpriteManager::getInstance().getSprite("sample");

    for (const ImageConfigData& image : TestConfig::getInstance().getConfigData().blockImages)
    {
        _blockSprites.push_back(std::make_shared<Sprite>(image.path, image.width, image.height));
    }
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
}

void TestScene::update(const float deltaTime)
{
    (void)deltaTime;
    if (_input)
    {
        _input->update();
    }
    render();
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

    constexpr float blockSize = 64.0f;
    constexpr float blockGap = 12.0f;
    const float rowWidth = (_blockSprites.size() * (blockSize + blockGap)) - blockGap;
    float x = (static_cast<float>(_configData.width) - rowWidth) * 0.5f;
    const float y = static_cast<float>(_configData.height) - blockSize - 150.0f;
    for (const std::shared_ptr<Sprite>& block : _blockSprites)
    {
        const SDL_FRect destination{x, y, blockSize, blockSize};
        renderer.drawTexture(block->getTexture().get(), destination, &block->getSourceRect());
        x += blockSize + blockGap;
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
    _titleFont = nullptr;
    _blockSprites.clear();
    _sprite.reset();
    _input.reset();
}

std::string TestScene::getName() const
{
    return "ImageViewerScene";
}
