// Copyright 2024 Quentin Cartier

#ifdef PLATFORM_DREAMCAST
#include <kos.h>
#endif

#include "udjourney/Game.hpp"

#ifdef PLATFORM_DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);
#endif

int main(int argc, char **argv) {
    Game game = Game(640, 480);
    game.run();
}
