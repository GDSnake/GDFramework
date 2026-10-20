#include "SettingsScene.h"

#include <algorithm>
#include <format>
#include <iterator>
#include <memory>
#include <utility>

#include <SDL3/SDL.h>

#include "AssetManagers/FontsManager.h"
#include "Core/Renderer.h"
#include "MenuScene.h"

namespace
{
    constexpr int SettingsOptionCount = 3;
    constexpr double StatusDisplayDuration = 3.0;
    constexpr double StatusFadeDuration = 1.0;
}

void SettingsScene::init()
{
    _input = std::make_unique<InputManager>();
    _titleFont = FontsManager::getInstance().getFont(TitleFontInfo.fontName);
    _resolutions = { { 800, 600 }, { 960, 640 }, { 1280, 720 }, { 1600, 900 }, { 1920, 1080 } };
    _selectedOption = 0;
    _statusText.clear();
    _statusTimer = 0.0;
    _nextScene.reset();

    const auto currentResolution = std::find_if(
        _resolutions.begin(),
        _resolutions.end(),
        [this](const Resolution& resolution)
        {
            return resolution.width == _configData.width && resolution.height == _configData.height;
        });
    if (currentResolution == _resolutions.end())
    {
        _resolutions.push_back({ _configData.width, _configData.height });
        _resolutionIndex = static_cast<int>(_resolutions.size()) - 1;
    }
    else
    {
        _resolutionIndex = static_cast<int>(std::distance(_resolutions.begin(), currentResolution));
    }
}

void SettingsScene::handleEvents(const SDL_Event& event)
{
    if (_input)
    {
        _input->handleEvent(event);
    }
}

bool SettingsScene::shouldQuit() const
{
    return _input && _input->shouldQuit();
}

void SettingsScene::handleInput()
{
    if (!_input)
    {
        return;
    }

    if (_input->isKeyPressed(SDL_SCANCODE_ESCAPE))
    {
        _nextScene = std::make_unique<MenuScene>();
        return;
    }

    if (_input->isMouseButtonPressed(SDL_BUTTON_LEFT))
    {
        int windowWidth = _configData.width;
        int windowHeight = _configData.height;
        if (SDL_Window* window = Renderer::getInstance().getWindow())
        {
            SDL_GetWindowSize(window, &windowWidth, &windowHeight);
        }

        const float mouseX = static_cast<float>(_input->getMouseX());
        const float mouseY = static_cast<float>(_input->getMouseY());
        for (int index = 0; index < SettingsOptionCount; ++index)
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
        _selectedOption = (_selectedOption + SettingsOptionCount - 1) % SettingsOptionCount;
    }
    else if (_input->isKeyPressed(SDL_SCANCODE_DOWN))
    {
        _selectedOption = (_selectedOption + 1) % SettingsOptionCount;
    }

    if (_input->isKeyPressed(SDL_SCANCODE_LEFT))
    {
        adjustSetting(-1);
    }
    else if (_input->isKeyPressed(SDL_SCANCODE_RIGHT))
    {
        adjustSetting(1);
    }

    if (_input->isKeyPressed(SDL_SCANCODE_RETURN) || _input->isKeyPressed(SDL_SCANCODE_SPACE))
    {
        activateSelection();
    }
}

void SettingsScene::update(const float deltaTime)
{
    if (!_statusText.empty())
    {
        _statusTimer += static_cast<double>(std::max(0.0f, deltaTime));
        if (_statusTimer >= StatusDisplayDuration)
        {
            _statusText.clear();
            _statusTimer = 0.0;
        }
    }

    if (_input)
    {
        _input->update();
    }
    render();
}

void SettingsScene::render()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.setDrawColor({ 17, 30, 42, 255 });
    renderer.clear();

    int windowWidth = _configData.width;
    int windowHeight = _configData.height;
    if (SDL_Window* window = renderer.getWindow())
    {
        SDL_GetWindowSize(window, &windowWidth, &windowHeight);
    }

    SDL_Renderer* drawRenderer = renderer.getRenderer();
    if (drawRenderer)
    {
        renderer.setDrawColor({ 241, 174, 91, 255 });
        const SDL_FRect accent = { 48.0f, 108.0f, 72.0f, 4.0f };
        SDL_RenderFillRect(drawRenderer, &accent);

        for (int index = 0; index < SettingsOptionCount; ++index)
        {
            const SDL_FRect option = optionRect(index, windowWidth, windowHeight);
            const bool selected = index == _selectedOption;
            renderer.setDrawColor(selected ? SDL_Color{ 65, 57, 48, 255 } : SDL_Color{ 34, 43, 49, 255 });
            SDL_RenderFillRect(drawRenderer, &option);
            renderer.setDrawColor(selected ? SDL_Color{ 241, 174, 91, 255 } : SDL_Color{ 84, 92, 96, 255 });
            SDL_RenderRect(drawRenderer, &option);
            renderer.drawText(
                _titleFont,
                optionLabel(index),
                { option.x + 20.0f, option.y + 13.0f },
                { 239, 238, 231, 255 });
        }
    }

    renderer.drawText(_titleFont, "SETTINGS", { 48.0f, 48.0f }, { 239, 238, 231, 255 });
    if (!_statusText.empty())
    {
        const double fadeStart = StatusDisplayDuration - StatusFadeDuration;
        const float alpha = _statusTimer <= fadeStart
            ? 1.0f
            : static_cast<float>((StatusDisplayDuration - _statusTimer) / StatusFadeDuration);
        const SDL_Color statusColor = {
            241,
            174,
            91,
            static_cast<Uint8>(255.0f * std::clamp(alpha, 0.0f, 1.0f))
        };
        renderer.drawText(
            _titleFont,
            _statusText,
            { 48.0f, static_cast<float>(windowHeight) - 48.0f },
            statusColor);
    }
    renderer.present();
}

SDL_FRect SettingsScene::optionRect(const int index, const int windowWidth, const int windowHeight) const
{
    const float width = std::min(480.0f, static_cast<float>(windowWidth) - 48.0f);
    return {
        (static_cast<float>(windowWidth) - width) * 0.5f,
        static_cast<float>(windowHeight) * 0.36f + static_cast<float>(index) * 80.0f,
        width,
        64.0f
    };
}

std::string SettingsScene::optionLabel(const int index) const
{
    switch (index)
    {
    case 0:
        return std::format("Windowed size: {} x {}", _configData.width, _configData.height);
    case 1:
        return std::format("Fullscreen: {}", _configData.fullscreen ? "On" : "Off");
    default:
        return "Back";
    }
}

void SettingsScene::activateSelection()
{
    if (_selectedOption == 2)
    {
        _nextScene = std::make_unique<MenuScene>();
        return;
    }

    adjustSetting(1);
}

void SettingsScene::adjustSetting(const int direction)
{
    if (_selectedOption == 0)
    {
        const int resolutionCount = static_cast<int>(_resolutions.size());
        _resolutionIndex = (_resolutionIndex + direction + resolutionCount) % resolutionCount;
        const Resolution& resolution = _resolutions[_resolutionIndex];
        applySettings(resolution.width, resolution.height, _configData.fullscreen);
    }
    else if (_selectedOption == 1)
    {
        applySettings(_configData.width, _configData.height, !_configData.fullscreen);
    }
}

void SettingsScene::applySettings(const int width, const int height, const bool fullscreen)
{
    _statusTimer = 0.0;
    Config& config = Config::getInstance();
    config.setWindowSettings(width, height, fullscreen);
    const bool saved = config.saveConfig();
    const bool applied = Renderer::getInstance().applyWindowSettings();

    if (!saved)
    {
        _statusText = "Could not save config.json";
    }
    else if (!applied)
    {
        _statusText = "Could not apply window settings";
    }
    else
    {
        _statusText = "Settings Saved";
    }
}

void SettingsScene::cleanup()
{
    _titleFont = nullptr;
    _input.reset();
}

std::string SettingsScene::getName() const
{
    return "SettingsScene";
}

std::unique_ptr<Scene> SettingsScene::getNextScene()
{
    return std::move(_nextScene);
}