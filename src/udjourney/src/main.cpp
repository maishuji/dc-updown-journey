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
#if defined(PLATFORM_DREAMCAST) && defined(DEBUG)
    dbglog(DBG_INFO, "Starting Up-Down Journey...");
    gdb_init();
    dbglog(DBG_INFO, "Stepping breakpoint");
    gdb_breakpoint();
#endif

    udjourney::Game game = udjourney::Game(kWidth, kHeigth);
    game.run();
}
