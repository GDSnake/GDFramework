#include <iostream>
#include <memory>

#include "Config/Config.h"
#include "Game/Game.h"
#include "TestConfig/TestConfig.h"
#include "TestScene.h"

int main() {

    Config::getInstance().loadConfig();
    TestConfig::getInstance().load();

    if (Config::getInstance().getConfigData().hasError)
    {
        std::cerr << "Configuration file has an error!" << '\n';
        std::cerr << "Press Enter to exit...";
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return 1;
    }

    Game game(std::make_unique<TestScene>());
    if (game.init())
    {
        game.run();
    }
    return 0;
}