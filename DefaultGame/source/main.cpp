#include <iostream>

#include "Config/Config.h"
#include "Game/Game.h"

int main() {
    if (!Config::getInstance().loadConfig()) {
        std::cerr << "Configuration file has an error.\n";
        return 1;
    }

    Game game;
    if (!game.init()) {
        return 1;
    }

    game.run();
    return 0;
}
