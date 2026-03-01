// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <udj-core/Logger.hpp>
#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/platform/Platform.hpp"
#include "udjourney/platform/features/PlatformFeatureBase.hpp"
namespace udjourney {
struct DownwardSpikeFeature : public PlatformFeatureBase {
    float height = 20.0f;  // Default spike height
    int damage = 1;        // Default damage
    Rectangle collision_rect;
    Color c = ORANGE;

    int_fast8_t get_type() const override {
        return 2;
    }  // Unique type ID for downward spikes

    void draw(const Platform& platform) const override {
        /* draw downward spikes */
        auto rect = platform.get_drawing_rect();
        DrawRectangleLinesEx(rect, 1.0F, DARKPURPLE);

        // Draw spikes on bottom of the platform (pointing downward)
        float spike_width = rect.width / 8.0f;
        for (int i = 0; i < 8; ++i) {
            float x = rect.x + i * spike_width;
            DrawTriangle(
                Vector2{x, rect.y + rect.height},
                Vector2{x + spike_width, rect.y + rect.height},
                Vector2{x + spike_width / 2, rect.y + rect.height + 20},
                DARKPURPLE);
        }

        DrawRectangleLinesEx(collision_rect, 1.0F, c);
    }

    void handle_collision(Platform& platform, IActor& actor) override {
        auto rect = platform.get_drawing_rect();
        // Handle collision logic here, e.g., reduce player health
        this->collision_rect =
            Rectangle{rect.x, rect.y + rect.height, rect.width, height};

        auto r2 = platform.get_rectangle();
        r2.y += r2.height;  // Move to bottom of platform
        r2.height = height;
        if (CheckCollisionRecs(r2, actor.get_rectangle())) {
            static_cast<Player&>(actor).notify("12;1");
            c = GREEN;
        } else {
            c = ORANGE;
        }
    }
};
}  // namespace udjourney
