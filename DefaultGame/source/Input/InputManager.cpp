#include "InputManager.h"

#include <ranges>

void InputManager::handleEvent(const SDL_Event& event)
{
    switch (event.type) {
    case SDL_EVENT_KEY_DOWN:
    case SDL_EVENT_KEY_UP:
        updateKeyState(event.key.scancode, event.key.down);
        break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN:
    case SDL_EVENT_MOUSE_BUTTON_UP:
        updateMouseState(event.button.button, event.button.down);
        break;

    case SDL_EVENT_MOUSE_MOTION:
        _mouseX = static_cast<int>(event.motion.x);
        _mouseY = static_cast<int>(event.motion.y);
        break;

    case SDL_EVENT_QUIT:
        _quit = true;
        break;
    }
}

void InputManager::update()
{
    for (auto& state : _keyStates | std::views::values) {
        if (state == KeyState::Pressed) state = KeyState::Held;
        else if (state == KeyState::Released) state = KeyState::None;
    }
    for (auto& state : _mouseStates | std::views::values) {
        if (state == KeyState::Pressed) state = KeyState::Held;
        else if (state == KeyState::Released) state = KeyState::None;
    }
}

bool InputManager::isKeyPressed(SDL_Scancode key) const
{
    auto it = _keyStates.find(key);
    return it != _keyStates.end() && it->second == KeyState::Pressed;
}

bool InputManager::isKeyHeld(SDL_Scancode key) const
{
    auto it = _keyStates.find(key);
    return it != _keyStates.end() && it->second == KeyState::Held;
}

bool InputManager::isKeyReleased(SDL_Scancode key) const
{
    auto it = _keyStates.find(key);
    return it != _keyStates.end() && it->second == KeyState::Released;
}

bool InputManager::isMouseButtonPressed(Uint8 button) const
{
    auto it = _mouseStates.find(button);
    return it != _mouseStates.end() && it->second == KeyState::Pressed;
}

bool InputManager::isMouseButtonHeld(Uint8 button) const
{
    auto it = _mouseStates.find(button);
    return it != _mouseStates.end() && it->second == KeyState::Held;
}

bool InputManager::isMouseButtonReleased(Uint8 button) const
{
    auto it = _mouseStates.find(button);
    return it != _mouseStates.end() && it->second == KeyState::Released;
}

void InputManager::updateKeyState(SDL_Scancode key, bool down)
{
    auto& state = _keyStates[key];
    if (down)
        state = (state == KeyState::Held) ? KeyState::Held : KeyState::Pressed;
    else
        state = KeyState::Released;
}

void InputManager::updateMouseState(Uint8 button, bool down)
{
    auto& state = _mouseStates[button];
    if (down)
        state = (state == KeyState::Held) ? KeyState::Held : KeyState::Pressed;
    else
        state = KeyState::Released;
}