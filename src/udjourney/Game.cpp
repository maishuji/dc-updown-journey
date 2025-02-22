#include "Game.hpp"

#include <vector>

#include <kos.h>
#include "raylib/raymath.h"
#include "raylib/rlgl.h"

#include "Actor.hpp"
#include "Platform.hpp"

std::vector<std::unique_ptr<IActor>> platforms;
Rectangle r;

std::vector<std::unique_ptr<IActor>> init_platforms()
{
	std::vector<std::unique_ptr<IActor>> res;
	int lastx = 0;
	int lastx2 = 100;

	for (int i = 0; i < 800; i += 100)
	{
		auto r = Rectangle(lastx, i, lastx2, 5);
		res.emplace_back(std::make_unique<Platform>(r));
		int ra = std::rand();
		lastx = (ra % 10) * 50;
		lastx2 = ra % 100 + 50;
	}
	return res;
}

Game::Game(int w, int h)
{
	r = Rectangle(0, 0, w, h);
	platforms.reserve(10);
}

void Game::run()
{

	maple_device_t *controller;
	cont_state_t *cont;
	platforms = init_platforms();

	InitWindow(r.width, r.height, "Block stacking puzzle game in KOS!");
	SetTargetFPS(60);
	while (true)
	{
		controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
		if (controller)
		{
			cont = (cont_state_t *)maple_dev_status(controller);
			if (cont->buttons & CONT_DPAD_UP)
			{
			}
			else if (cont->buttons & CONT_DPAD_DOWN)
			{
			}
			else if (cont->buttons & CONT_A)
			{
			}
			else if (cont->buttons & CONT_START)
			{
				break;
			}
		}
		for (auto &p : platforms)
		{
			p->update(0.0f);
		}
		// TODO
		BeginDrawing();
		ClearBackground(RAYWHITE); // Clear the background with a color

		// Draw the rectangle
		// DrawRectangleRec(r, BLUE);

		DrawText("Hello, World. Press START to break.\n", 10, 10, 20, RED);
		for (const auto &p : platforms)
		{
			p->draw();
		}

		DrawFPS(10, 50); // Draw FPS counter

		EndDrawing();
	}
}