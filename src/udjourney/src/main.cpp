// Copyright 2024 Quentin Cartier

#ifdef PLATFORM_DREAMCAST
#include <kos.h>
#endif

#include "udjourney/Game.hpp"

#ifdef PLATFORM_DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);
#endif

int main(int argc, char **argv) {
    constexpr int kWidth = 640;
    constexpr int kHeigth = 480;

    Game game = Game(kWidth, kHeigth);
    game.run();
}
