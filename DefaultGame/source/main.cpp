#include <iostream>

#include "Config/Config.h"
#include "Game/Game.h"

int main() {

    Config::getInstance().loadConfig();

    if (Config::getInstance().getConfigData().hasError)
    {
        std::cerr << "Configuration file has an error!" << '\n';
        std::cerr << "Press Enter to exit...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return 1;
    }

    Game game;
    if (game.init())
    {
        game.run();
    }
    return 0;
}