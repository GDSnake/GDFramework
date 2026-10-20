#pragma once

#include <memory>
#include <string>

#include "Input/InputManager.h"
#include "Scene.h"

struct TTF_Font;

class MenuScene : public Scene
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
    void render() override;
    SDL_FRect optionRect(int index, int windowWidth, int windowHeight) const;
    void activateSelection();

    std::unique_ptr<InputManager> _input;
    std::unique_ptr<Scene> _nextScene;
    TTF_Font* _titleFont = nullptr;
    int _selectedOption = 0;
};
