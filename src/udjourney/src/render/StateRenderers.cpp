// Copyright 2025 Quentin Cartier
#include "udjourney/render/StateRenderers.hpp"

#include <raylib/raylib.h>

#include "udjourney/Game.hpp"

namespace udjourney {

// Helper: draw widgets (actors with group_id == 4)
static void draw_widgets_(const Game& game) {
    for (const auto& actor : game.get_actors()) {
        if (actor && actor->get_group_id() == 4) {
            actor->draw();
        }
    }
}

// Helper: draw finish line
static void draw_finish_line_(const Game& game) {
    if (!game.get_current_scene() || game.get_level_height() <= 0) {
        return;
    }

    float win_threshold = game.get_level_height() * 0.98f;
    Rectangle game_rect = game.get_rectangle();
    float line_y = win_threshold - game_rect.y;

    if (line_y >= -10 && line_y <= game_rect.height + 10) {
        float line_thickness = 4.0f;
        Rectangle finish_line = {
            0, line_y - line_thickness / 2, game_rect.width, line_thickness};

        DrawRectangleRec(finish_line, MAGENTA);
        DrawRectangleLinesEx(finish_line, 1.0f, PINK);

        if (line_y >= 50 && line_y <= game_rect.height - 50) {
            const char* finish_text = "FINISH LINE";
            int text_width = MeasureText(finish_text, 16);
            DrawText(finish_text,
                     static_cast<int>(game_rect.width - text_width - 10),
                     static_cast<int>(line_y - 25),
                     16,
                     MAGENTA);
        }
    }
}

// UiScreenRenderer: for TITLE, WIN, GAMEOVER
void UiScreenRenderer::render(const Game& game) const {
    game.draw_backgrounds();
    game.draw_huds();
    draw_widgets_(game);
}

// PlayStateRenderer: for PLAY
void PlayStateRenderer::render(const Game& game) const {
    game.draw_backgrounds();

    // Draw all actors
    for (const auto& actor : game.get_actors()) {
        actor->draw();
    }

    // Draw player
    if (auto* player = game.get_player()) {
        player->draw();
    }

    // Draw finish line
    draw_finish_line_(game);

    // Draw particles
    game.draw_particles();

    // Draw dash HUD (TODO: move to HUDManager)
    const auto& dash_hud = game.get_dash_hud();
    Rectangle rect = game.get_rectangle();
    DrawCircle(static_cast<int>(rect.width) - 50,
               45,
               17,
               dash_hud.dashable == 1 ? GREEN : RED);
}

// PauseStateRenderer: for PAUSE
void PauseStateRenderer::render(const Game& game) const {
    // Draw gameplay scene first
    PlayStateRenderer play_renderer;
    play_renderer.render(game);

    // Overlay pause text
    DrawText(" -- PAUSE -- \n", 10, 10, 20, RED);
    DrawText("Press START to resume\n", 300, 40, 20, WHITE);
    DrawText("Press L to load level\n", 300, 70, 20, WHITE);
    DrawText("Press B button to quit\n", 300, 100, 20, RED);
}

}  // namespace udjourney
