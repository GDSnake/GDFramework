#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Config/Config.h"
#include "Input/InputManager.h"
#include "Scene.h"

struct TTF_Font;

class SettingsScene : public Scene
{
public:
    void init() override;
    void handleEvents(const SDL_Event& event) override;
    void handleInput() override;
    void update(float deltaTime) override;
    void cleanup() override;
    std::string getName() const override;
    bool shouldQuit() const override;
    std::unique_ptr<Scene> getNextScene() override;

private:
    struct Resolution
    {
        int width;
        int height;
    };

    void render() override;
    SDL_FRect optionRect(int index, int windowWidth, int windowHeight) const;
    std::string optionLabel(int index) const;
    void activateSelection();
    void adjustSetting(int direction);
    void applySettings(int width, int height, bool fullscreen);

    std::unique_ptr<InputManager> _input;
    std::unique_ptr<Scene> _nextScene;
    const ConfigData& _configData = Config::getInstance().getConfigData();
    std::vector<Resolution> _resolutions;
    TTF_Font* _titleFont = nullptr;
    std::string _statusText;
    double _statusTimer = 0.0;
    int _selectedOption = 0;
    int _resolutionIndex = 0;
};