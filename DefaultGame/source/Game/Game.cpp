#include "Game.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "AssetManagers/AudioManager.h"
#include "AssetManagers/FontsManager.h"
#include "AssetManagers/SpriteManager.h"
#include "Config/Config.h"
#include "Core/Renderer.h"


Game::Game(std::unique_ptr<Scene> scene)
    : _currentScene(std::move(scene))
{
}

Game::~Game()
{
    shutdown();
}

bool Game::init()
{
    if (Config::getInstance().getConfigData().hasError)
    {
        return false;
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }
    _initialized = true;

    if (!Renderer::getInstance().init() || !TTF_Init())
    {
        shutdown();
        return false;
    }

    FontsManager::getInstance().load();
    SpriteManager::getInstance().load();
    _currentScene->init();

    _lastTime = SDL_GetTicks();
    _isRunning = true;
    return true;
}

void Game::run()
{
    Uint64 frames = 0;
    Uint64 fpsResetTick = 0;
    const float targetFrameTime = 1.0f / static_cast<float>(Config::getInstance().getConfigData().targetFPS);
    while (_isRunning)
    {
        Uint64 current = SDL_GetTicks();
        _deltaTime = static_cast<float>(current - _lastTime) / 1000.0f;
        _lastTime = current;

        handleEvents();
        update(_deltaTime);
        frames++;

        if (current - fpsResetTick >= 1000) {
            fpsResetTick = current;
#if DEBUG_BUILD
            Renderer::getInstance().writeFPSOnWindowTitle(frames);
#endif
            frames = 0;
        }

        if (_currentScene->shouldQuit())
        {
            _isRunning = false;
        }

        // Cap frame rate
        Uint64 frameEnd = SDL_GetTicks();
        float frameDuration = static_cast<float>(frameEnd - current) / 1000.0f;

        if (frameDuration < targetFrameTime) {
            Uint32 delayMs = static_cast<Uint32>((targetFrameTime - frameDuration) * 1000.0f);
            SDL_Delay(delayMs);
        }
    }
}

void Game::shutdown()
{
    _isRunning = false;
    if (_currentScene)
    {
        _currentScene->cleanup();
        _currentScene.reset();
    }
    if (!_initialized)
    {
        return;
    }

    AudioManager::getInstance().shutdown();
    SpriteManager::getInstance().shutdown();
    FontsManager::getInstance().shutdown();
    Renderer::getInstance().shutdown();
    TTF_Quit();
    SDL_Quit();
    _initialized = false;
}

void Game::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            _isRunning = false;
        }
        if (_currentScene)
        {
            _currentScene->handleEvents(event);
        }
    }
}

void Game::update(float dt)
{
    if (!_currentScene)
    {
        return;
    }

    _currentScene->handleInput();
    _currentScene->update(dt);

    if (std::unique_ptr<Scene> nextScene = _currentScene->getNextScene())
    {
        //Cleans current scene
        _currentScene->cleanup();
        //Releases the current scene and takes ownership of the next scene
        _currentScene = std::move(nextScene);
        _currentScene->init();
    }
}