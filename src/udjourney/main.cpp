/*
 * @author maishuji
 * Entry point for the game
 */

#include <kos.h>
#include "Game.hpp"

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char **argv)
{
	Game game = Game(640, 480);
	game.run();
}