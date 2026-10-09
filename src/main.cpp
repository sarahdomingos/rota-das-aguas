// Ponto de entrada: o loop principal fica na classe Game (src/core/Game.cpp)

#include "core/Game.h"

int main()
{
    Game game;

    if (!game.init()) {
        return -1;
    }

    game.run();

    return 0;
}
