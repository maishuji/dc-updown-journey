/*
 * @author maishuji
 * This code is just a template to get you started.
 * You can delete it and start from scratch.
 */

#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <string>

#include "raylib/raylib.h"
#include "raylib/raymath.h"
#include "raylib/rlgl.h"
#include <kos.h>

KOS_INIT_FLAGS(INIT_DEFAULT);

int main(int argc, char **argv)
{
	maple_device_t *controller;
	cont_state_t *cont;

	InitWindow(640, 480, "Block stacking puzzle game in KOS!");
	Rectangle r = Rectangle(40, 30, 200, 200);
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
		BeginDrawing();
		ClearBackground(RAYWHITE); // Clear the background with a color

        // Draw the rectangle
        DrawRectangleRec(r, BLUE);
		
		DrawText("Hello, World. Press START to break.\n", 10, 10, 20, RED);


		DrawFPS(10, 50); // Draw FPS counter

		EndDrawing();
	}
}