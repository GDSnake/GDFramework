#pragma once
#include <memory>

#include <SDL3/SDL_stdinc.h>
#include "Scenes/Scene.h"

class Game
{
public:
	explicit Game(std::unique_ptr<Scene> entryScene = nullptr);
	~Game();

	bool init();
	void run();
	void shutdown();

private:
	void handleEvents();
	void update(float dt);

	bool _isRunning = false;


	Uint64 _lastTime = 0;
	float _deltaTime = 0.0f;
    Uint32 _delayTimer = 0;
	std::unique_ptr<Scene> _currentScene;
	bool _initialized = false;
};
