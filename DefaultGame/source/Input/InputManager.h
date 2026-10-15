#pragma once

#include <SDL3/SDL.h>
#include <unordered_map>

enum class KeyState {
    None,
    Pressed,
    Held,
    Released
};

class InputManager {
public:
    void handleEvent(const SDL_Event& event);
    void update();

    bool isKeyPressed(SDL_Scancode key) const;
    bool isKeyHeld(SDL_Scancode key) const;
    bool isKeyReleased(SDL_Scancode key) const;

    bool isMouseButtonPressed(Uint8 button) const;
    bool isMouseButtonHeld(Uint8 button) const;
    bool isMouseButtonReleased(Uint8 button) const;

    inline void quit() { _quit = true; }
    inline bool shouldQuit() const { return _quit; }

    inline int getMouseX() const { return _mouseX; }
    inline int getMouseY() const { return _mouseY; }

private:
    void updateKeyState(SDL_Scancode key, bool down);
    void updateMouseState(Uint8 button, bool down);

    std::unordered_map<SDL_Scancode, KeyState> _keyStates;
    std::unordered_map<Uint8, KeyState> _mouseStates;

    int _mouseX = 0;
    int _mouseY = 0;
    bool _quit = false;
};