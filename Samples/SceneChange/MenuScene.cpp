#include "MenuScene.h"

#include <algorithm>
#include <array>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>

#include "AssetManagers/FontsManager.h"
#include "Core/Renderer.h"
#include "SettingsScene.h"
#include "TestScene.h"

namespace
{
    constexpr std::array<const char*, 3> MenuOptions = { "Play", "Settings", "Exit" };
}

void MenuScene::init()
{
    _input = std::make_unique<InputManager>();
    _titleFont = FontsManager::getInstance().getFont(TitleFontInfo.fontName);
    _selectedOption = 0;
}

void MenuScene::handleEvents(const SDL_Event& event)
{
    if (_input)
    {
        _input->handleEvent(event);
    }
}

bool MenuScene::shouldQuit() const
{
    return _input && _input->shouldQuit();
}

void MenuScene::handleInput()
{
    if (!_input)
    {
        return;
    }

    if (_input->isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        _input->quit();
        return;
    }

    if (_input->isMouseButtonPressed(SDL_BUTTON_LEFT))
    {
        int windowWidth = 960;
        int windowHeight = 640;
        if (SDL_Window* window = Renderer::getInstance().getWindow())
        {
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        }

        const float mouseX = static_cast<float>(_input->getMouseX());
        const float mouseY = static_cast<float>(_input->getMouseY());
        for (int index = 0; index < static_cast<int>(MenuOptions.size()); ++index)
        {
            const SDL_FRect option = optionRect(index, windowWidth, windowHeight);
            if (mouseX >= option.x && mouseX <= option.x + option.w &&
                mouseY >= option.y && mouseY <= option.y + option.h)
            {
                _selectedOption = index;
                activateSelection();
                return;
            }
        }
    }

    if (_input->isKeyPressed(SDL_SCANCODE_UP))
    {
        _selectedOption = (_selectedOption + static_cast<int>(MenuOptions.size()) - 1) % static_cast<int>(MenuOptions.size());
    }
    else if (_input->isKeyPressed(SDL_SCANCODE_DOWN))
    {
        _selectedOption = (_selectedOption + 1) % static_cast<int>(MenuOptions.size());
    }

    if (_input->isKeyPressed(SDL_SCANCODE_RETURN) || _input->isKeyPressed(SDL_SCANCODE_SPACE))
    {
        activateSelection();
    }
}

void MenuScene::update(const float deltaTime)
{
    (void)deltaTime;
    if (_input)
    {
        _input->update();
    }
    render();
}

void MenuScene::render()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.setDrawColor({ 17, 30, 42, 255 });
    renderer.clear();

    int windowWidth = 960;
    int windowHeight = 640;
    if (SDL_Window* window = renderer.getWindow())
    {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    }

    SDL_Renderer* drawRenderer = renderer.getRenderer();
    if (drawRenderer)
    {
        renderer.setDrawColor({ 94, 218, 183, 255 });
        const SDL_FRect accent = { 48.0f, 108.0f, 72.0f, 4.0f };
        SDL_RenderFillRect(drawRenderer, &accent);

        for (int index = 0; index < static_cast<int>(MenuOptions.size()); ++index)
        {
            const SDL_FRect option = optionRect(index, windowWidth, windowHeight);
            const bool selected = index == _selectedOption;
            renderer.setDrawColor(selected ? SDL_Color{ 45, 69, 80, 255 } : SDL_Color{ 27, 46, 59, 255 });
            SDL_RenderFillRect(drawRenderer, &option);
            renderer.setDrawColor(selected ? SDL_Color{ 94, 218, 183, 255 } : SDL_Color{ 64, 88, 99, 255 });
            SDL_RenderRect(drawRenderer, &option);
            renderer.drawText(
                _titleFont,
                MenuOptions[index],
                { option.x + 24.0f, option.y + 13.0f },
                { 235, 241, 238, 255 });
        }
    }

    renderer.drawText(_titleFont, "SCENE CHANGE", { 48.0f, 48.0f }, { 235, 241, 238, 255 });
    renderer.present();
}

SDL_FRect MenuScene::optionRect(const int index, const int windowWidth, const int windowHeight) const
{
    const float width = std::min(480.0f, static_cast<float>(windowWidth) - 48.0f);
    return {
        (static_cast<float>(windowWidth) - width) * 0.5f,
        static_cast<float>(windowHeight) * 0.36f + static_cast<float>(index) * 80.0f,
        width,
        64.0f
    };
}

void MenuScene::activateSelection()
{
    switch (_selectedOption)
    {
    case 0:
        _nextScene = std::make_unique<TestScene>();
        break;
    case 1:
        _nextScene = std::make_unique<SettingsScene>();
        break;
    case 2:
        _input->quit();
        break;
    default:
        break;
    }
}

void MenuScene::cleanup()
{
    _titleFont = nullptr;
    _input.reset();
}

std::string MenuScene::getName() const
{
    return "MenuScene";
}

std::unique_ptr<Scene> MenuScene::getNextScene()
{
    return std::move(_nextScene);
}
