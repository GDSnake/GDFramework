#pragma once

#include <memory>

#include <SDL3/SDL_events.h>
#include <SDL3/SDL_stdinc.h>

#include "Input/InputManager.h"

class GameScene;

class Game
{
public:
    Game();
    virtual ~Game();

    bool init();
    void run();
    void shutdown();

protected:
    virtual void onEvent(const SDL_Event& event);
    virtual void onUpdate(float deltaTime);
    virtual void onRender();

    InputManager& input() noexcept;

private:
    void handleEvents();
    void update(float deltaTime);
    void render();

    bool _isRunning = false;
    bool _sdlInitialized = false;
    bool _ttfInitialized = false;
    Uint64 _lastTime = 0;
    InputManager _input;
    std::unique_ptr<GameScene> _scene;
};