#pragma once

#include <raylib/raylib.h>

#include "udjourney/interfaces/IActor.hpp"
#include "udjourney/interfaces/IGame.hpp"
#include "udjourney/platform/Platform.hpp"
#include "udjourney/platform/features/PlatformFeatureBase.hpp"

struct SpikeFeature : public PlatformFeatureBase {
    float height = 20.0f;  // Default spike height
    int damage = 1;        // Default damage
    Rectangle collision_rect;
    Color c = ORANGE;

    int_fast8_t get_type() const override {
        return 1;
    }  // Unique type ID for spikes

    void draw(const Platform& platform) const override {
        /* draw spikes */
        auto rect = platform.get_drawing_rect();
        DrawRectangleLinesEx(rect, 1.0F, RED);

        // Draw spikes on top of the platform
        float spike_width = rect.width / 8.0f;
        for (int i = 0; i < 8; ++i) {
            float x = rect.x + i * spike_width;
            DrawTriangle(Vector2{x, rect.y},
                         Vector2{x + spike_width, rect.y},
                         Vector2{x + spike_width / 2, rect.y - 20},
                         RED);
        }

        DrawRectangleLinesEx(collision_rect, 1.0F, c);
    }

    void handle_collision(Platform& platform, IActor& actor) override {
        /* damage player */

        auto rect = platform.get_drawing_rect();
        // Handle collision logic here, e.g., reduce player health
        this->collision_rect =
            Rectangle{rect.x, rect.y - height, rect.width, height};

        auto r2 = platform.get_rectangle();
        r2.y -= height;
        if (CheckCollisionRecs(r2, actor.get_rectangle())) {
            static_cast<Player&>(actor).notify("12;1");
            c = GREEN;
        } else {
            c = ORANGE;
        }
    }
};