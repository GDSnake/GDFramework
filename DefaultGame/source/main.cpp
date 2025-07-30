#include <SDL3/SDL.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>

using json = nlohmann::json;

struct Config {
    std::string title = "";
    int width = 800;
    int height = 600;
    bool fullscreen = false;
    bool hasError = false;
};

std::string getConfigPath(const char* argv0) {
    std::filesystem::path exePath = std::filesystem::absolute(argv0);
    return (exePath.parent_path() / "config.json").string();
}

Config loadConfig(const std::string& path) {
    std::ifstream file(path);
    if (!file)
    {
        std::cerr << "Cannot load Config File." << std::endl;
        Config errorConfig;
        errorConfig.hasError = true;
        return errorConfig;
    }
    json data = json::parse(file);
    Config cfg;
    cfg.title = data["window"]["title"];
    cfg.width = data["window"]["width"];
    cfg.height = data["window"]["height"];
    cfg.fullscreen = data["window"]["fullscreen"];
    return cfg;
}

int main(int argc, char* argv[]) {

    Config cfg = loadConfig(getConfigPath(argv[0]));

    if (cfg.hasError)
    {
        std::cerr << "Configuration file has an error!" << std::endl;
        return 1;
    }

    // Initialize SDL (video + events)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cerr << "SDL Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    Uint32 flags = cfg.fullscreen ? SDL_WINDOW_FULLSCREEN : 0;

    SDL_Window* window = SDL_CreateWindow(cfg.title.c_str(),
										cfg.width, cfg.height,
										flags);

    if (!window) {
        std::cerr << "Window Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Wait 2 seconds
    SDL_Delay(2000);

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
