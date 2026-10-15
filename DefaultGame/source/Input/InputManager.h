#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>

enum class KeyState {
    None,
    Pressed,
    Held,
    Released
};

struct KeyCooldownData {
    float dasTimer = 0.0f;
    float arrTimer = 0.0f;
    bool dasTriggered = false;
    float softDrop = 0.0f;
};

class InputManager {
public:
    void handleEvent(const SDL_Event& event);
    void update(float deltaTime);

    bool isKeyPressed(SDL_Scancode key) const;
    bool isKeyHeld(SDL_Scancode key) const;
    bool isKeyReleased(SDL_Scancode key) const;

    bool isMouseButtonPressed(Uint8 button) const;
    bool isMouseButtonHeld(Uint8 button) const;
    bool isMouseButtonReleased(Uint8 button) const;

    bool horizontalMovementIfNotOnCooldown(SDL_Scancode key, float das, float arr);
    bool isSoftDropping(SDL_Scancode key);

    inline void quit() { _quit = true; }
    inline bool shouldQuit() const { return _quit; }

    inline int getMouseX() const { return _mouseX; }
    inline int getMouseY() const { return _mouseY; }

private:
    void updateKeyState(SDL_Scancode key, bool down);
    void updateMouseState(Uint8 button, bool down);

    std::unordered_map<SDL_Scancode, KeyState> _keyStates;
    std::unordered_map<Uint8, KeyState> _mouseStates;
    std::unordered_map<SDL_Scancode, KeyCooldownData> _keyCooldowns;

    int _mouseX = 0;
    int _mouseY = 0;
    bool _quit = false;
};