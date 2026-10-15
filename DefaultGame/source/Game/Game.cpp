#include "Game.h"

#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_timer.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "AssetManagers/AudioManager.h"
#include "AssetManagers/FontsManager.h"
#include "AssetManagers/SpriteManager.h"
#include "Config/Config.h"
#include "Core/Renderer.h"
#include "Game/Scenes/GameScene.h"

Game::Game() = default;

Game::~Game()
{
    shutdown();
}

bool Game::init()
{
    const ConfigData& config = Config::getInstance().getConfigData();
    if (config.hasError)
    {
        return false;
    }

    Uint32 initFlags = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    if (config.audio.contains("background_music") || config.audio.contains("bgm"))
    {
        initFlags |= SDL_INIT_AUDIO;
    }

    if (!SDL_Init(initFlags))
    {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }
    _sdlInitialized = true;

    if (!Renderer::getInstance().init())
    {
        shutdown();
        return false;
    }

    if (!TTF_Init())
    {
        SDL_Log("SDL_ttf initialization failed: %s", SDL_GetError());
        shutdown();
        return false;
    }
    _ttfInitialized = true;
    FontsManager::getInstance().load();

    SpriteManager::getInstance().load();
    if (config.audio.contains("background_music") || config.audio.contains("bgm"))
    {
        AudioManager::getInstance().playMusic();
    }

    _scene = std::make_unique<GameScene>();
    _scene->init();

    _lastTime = SDL_GetTicks();
    _isRunning = true;
    return true;
}

void Game::run()
{
    if (!_sdlInitialized || !_isRunning)
    {
        return;
    }

    const int targetFPS = Config::getInstance().getConfigData().targetFPS;
    const Uint64 targetFrameDuration = targetFPS > 0
        ? static_cast<Uint64>(1000.0 / static_cast<double>(targetFPS))
        : 0;

    while (_isRunning)
    {
        const Uint64 frameStart = SDL_GetTicks();
        const float deltaTime = static_cast<float>(frameStart - _lastTime) / 1000.0f;
        _lastTime = frameStart;

        handleEvents();
        if (!_isRunning)
        {
            break;
        }

        update(deltaTime);
        if (!_isRunning)
        {
            break;
        }

        render();

        const Uint64 frameDuration = SDL_GetTicks() - frameStart;
        if (targetFrameDuration > frameDuration)
        {
            SDL_Delay(static_cast<Uint32>(targetFrameDuration - frameDuration));
        }
    }
}

void Game::shutdown()
{
    _isRunning = false;
    if (_scene)
    {
        _scene->cleanup();
        _scene.reset();
    }
    if (!_sdlInitialized)
    {
        return;
    }

    AudioManager::getInstance().shutdown();
    SpriteManager::getInstance().shutdown();
    FontsManager::getInstance().shutdown();
    Renderer::getInstance().shutdown();
    if (_ttfInitialized)
    {
        TTF_Quit();
        _ttfInitialized = false;
    }
    SDL_Quit();
    _sdlInitialized = false;
}

void Game::handleEvents()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        _input.handleEvent(event);
        if (_scene)
        {
            _scene->handleEvents(event);
        }
        onEvent(event);

        if (event.type == SDL_EVENT_QUIT || _input.shouldQuit())
        {
            _isRunning = false;
        }
    }
}

void Game::update(float deltaTime)
{
    if (_scene)
    {
        _scene->handleInput();
    }
    onUpdate(deltaTime);
    if (_scene)
    {
        _scene->update(deltaTime);
    }

    _input.update(deltaTime);
    if (_input.shouldQuit())
    {
        _isRunning = false;
    }
}

void Game::render()
{
    Renderer& renderer = Renderer::getInstance();
    renderer.setDrawColor({20, 20, 24, 255});
    renderer.clear();
    if (_scene)
    {
        _scene->render();
    }
    onRender();
    renderer.present();
}

void Game::onEvent(const SDL_Event&)
{
}

void Game::onUpdate(float)
{
}

void Game::onRender()
{
}

InputManager& Game::input() noexcept
{
    return _input;
}