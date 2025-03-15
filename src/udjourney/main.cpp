// Copyright 2024 Quentin Cartier

#include <kos.h>

#include "Game.hpp"

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char **argv) {
    Game game = Game(640, 480);
    game.run();
}
