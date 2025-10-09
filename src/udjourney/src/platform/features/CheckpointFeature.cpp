// Copyright 2025 Quentin Cartier
#include "udjourney/platform/features/CheckpointFeature.hpp"

#include <raylib/raylib.h>

#include "udjourney/platform/Platform.hpp"
#include "udjourney/interfaces/IActor.hpp"

void CheckpointFeature::draw(const Platform& platform) const {
    // Draw checkpoint visual indicator (flag-like effect)
    auto rect = platform.get_drawing_rect();

    // Draw a flag pole in the center
    float pole_x = rect.x + rect.width / 2.0f;
    float pole_top = rect.y - 20.0f;
    float pole_bottom = rect.y;

    // Draw pole
    DrawLineEx(Vector2{pole_x, pole_top}, Vector2{pole_x, pole_bottom},
               2.0f, DARKGRAY);

    // Draw flag
    float flag_width = 16.0f;
    float flag_height = 10.0f;
    Rectangle flag_rect = {
        pole_x,
        pole_top,
        flag_width,
        flag_height
    };

    DrawRectangleRec(flag_rect, GREEN);
    DrawRectangleLinesEx(flag_rect, 1.0f, DARKGREEN);

    // Draw checkpoint text if platform is wide enough
    if (rect.width > 60) {
        const char* text = "CHECKPOINT";
        int text_width = MeasureText(text, 8);
        DrawText(text,
                static_cast<int>(rect.x + (rect.width - text_width) / 2),
                static_cast<int>(rect.y + rect.height + 2),
                8,
                GREEN);
    }
}

void CheckpointFeature::handle_collision(Platform& platform, IActor& actor) {
    // Check if this is a player collision (group_id == 0)
    if (actor.get_group_id() == 0) {
        // Get platform position for checkpoint
        Rectangle platform_rect = platform.get_rectangle();

        // Notify the game about the checkpoint reached
        auto& game = platform.get_game();
        game.on_checkpoint_reached(platform_rect.x, platform_rect.y);
    }
}
