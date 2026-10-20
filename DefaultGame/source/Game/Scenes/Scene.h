#pragma once

#include <memory>
#include <SDL3/SDL_events.h>

#include <string>

class Scene
{
public:
    virtual ~Scene() = default;

    virtual void init() = 0;
    virtual void handleEvents(const SDL_Event& event) = 0;
    virtual void handleInput() = 0;
    virtual void update(float deltaTime) = 0;
    virtual void render() = 0;
    virtual void cleanup() = 0;
    virtual std::string getName() const = 0;
    virtual bool shouldQuit() const = 0;
    virtual std::unique_ptr<Scene> getNextScene() = 0;
};