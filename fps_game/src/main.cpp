#include "Game.h"
#include <iostream>

int main() {
    Game g;
    if (!g.init(1280, 720, "Battle Royale - Step 6: Effects & Polish")) {
        std::cerr << "Game::init failed\n";
        return 1;
    }
    g.run();
    g.shutdown();
    return 0;
}
