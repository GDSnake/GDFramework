#pragma once

#include <SDL3/SDL_render.h>

#include <string>

struct SDL_Window;
struct TTF_Font;

class Renderer
{
public:
    static Renderer& getInstance()
    {
        static Renderer instance;
        return instance;
    }

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool init();
    void clear() const;
    void present() const;
    void setDrawColor(const SDL_Color& color) const;
    void drawTexture(SDL_Texture* texture, const SDL_FRect& destination, const SDL_FRect* source = nullptr) const;
    void drawText(TTF_Font* font, const std::string& text, const SDL_FPoint& origin, const SDL_Color& color) const;
    SDL_Window* getWindow() const noexcept;
    SDL_Renderer* getRenderer() const noexcept;
    void shutdown();

#if DEBUG_BUILD
	void writeFPSOnWindowTitle(Uint64 fps);
#endif

private:
    Renderer() = default;
    ~Renderer();

    SDL_Window* _window = nullptr;
    SDL_Renderer* _renderer = nullptr;
};