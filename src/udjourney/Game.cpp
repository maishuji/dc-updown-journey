#include "Game.hpp"

#include <vector>
#include <algorithm>

#include <kos.h>
#include "raylib/raymath.h"
#include "raylib/rlgl.h"

#include "IActor.hpp"
#include "Platform.hpp"

const double kUpdateInterval = 0.06;

Rectangle r;

std::vector<std::unique_ptr<IActor>> init_platforms(Game &game)
{
	std::vector<std::unique_ptr<IActor>> res;
	int lastx = 0;
	int lastx2 = 100;

	for (int i = 0; i < 800; i += 100)
	{
		auto r = Rectangle(lastx, i, lastx2, 5);
		res.emplace_back(std::make_unique<Platform>(game, r));
		int ra = std::rand();
		lastx = (ra % 10) * 50;
		lastx2 = ra % 100 + 50;
	}
	return res;
}

Game::Game(int w, int h) : IGame(), m_state(GameState::TITLE)
{
	r = Rectangle(0, 0, w, h);
	m_actors.reserve(10);
}

void Game::run()
{
	m_actors = init_platforms(*this);

	InitWindow(r.width, r.height, "Up-Down Journey");
	SetTargetFPS(60);
	last_update_time = GetTime();

	while (true)
	{
		update();
	}
}

void Game::add_actor(std::unique_ptr<IActor> actor)
{
	if (m_updating_actors)
	{
		m_pending_actors.push_back(std::move(actor));
	}
	else
	{
		m_actors.push_back(std::move(actor));
	}
}

void Game::remove_actor(IActor &actor)
{
	auto iter = std::find_if(
		m_actors.begin(),
		m_actors.end(),
		[&actor](const std::unique_ptr<IActor> &p)
		{
			return p.get() == &actor;
		});
	if (iter != m_actors.end())
	{
		m_actors.erase(iter);
	}
}

void Game::process_input(cont_state_t *cont)
{
	for (auto &a : m_actors)
	{
		a->process_input(cont);
	}
}

void Game::update()
{
	// Update actors
	if (m_state == GameState::PLAY)
	{
		m_updating_actors = true;
		for (auto &p : m_actors)
		{
			p->update(0.0f);
		}
		m_updating_actors = false;

		// Move pending actors to actors
		for (auto &pa : m_pending_actors)
		{
			m_actors.push_back(std::move(pa));
		}
		m_pending_actors.clear();

		// TODO: Remove dead actors

	} // GameState::PLAY

	double cur_update_time = GetTime();
	if (cur_update_time - last_update_time > kUpdateInterval)
	{
		maple_device_t *controller = maple_enum_type(0, MAPLE_FUNC_CONTROLLER);
		if (controller)
		{
			cont_state_t *cont = (cont_state_t *)maple_dev_status(controller);
			if (cont)
			{

				// Preprocessing of input
				if (cont->buttons & CONT_START)
				{
					if (m_state == GameState::PLAY)
						m_state = GameState::PAUSE;
					else
						m_state = GameState::PLAY;
				}
				process_input(cont);
			}
		}
		for (auto &p : m_actors)
		{
			p->update(0.0f);
		}
		last_update_time = cur_update_time;
	}

	// TODO
	BeginDrawing();
	ClearBackground(RAYWHITE); // Clear the background with a color

	// Draw the rectangle
	// DrawRectangleRec(r, BLUE);

	DrawText("Hello, World. Press START to break.\n", 10, 10, 20, RED);

	if (m_state == GameState::PLAY)
	{
		for (const auto &p : m_actors)
		{
			p->draw();
		}
	} // GameState::PLAY

	DrawFPS(10, 50); // Draw FPS counter

	EndDrawing();
}