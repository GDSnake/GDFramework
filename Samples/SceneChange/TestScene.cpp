#include "Input/InputManager.h"
#include "TestScene.h"

#include "AssetManagers/FontsManager.h"
#include "Core/Renderer.h"

void TestScene::init()
{
    _input = std::make_unique<InputManager>();
    _titleText = TitleFontInfo.baseText + "Scene Change Sample";
    _titleFont = FontsManager::getInstance().getFont(TitleFontInfo.fontName);
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
    renderer.setDrawColor({ 20, 36, 52, 255 });
    renderer.clear();
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
    _input.reset();
}

std::string TestScene::getName() const
{
    return "SceneChangeScene";
}
