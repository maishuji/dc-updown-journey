// Copyright 2025 Quentin Cartier
#include "udjourney/hud/scene/ScoreDisplayHUD.hpp"
#include "udjourney/Game.hpp"

namespace udjourney {
namespace hud {
namespace scene {

ScoreDisplayHUD::ScoreDisplayHUD(const udjourney::scene::HUDData& hud_data,
                                 udjourney::Game* game) :
    m_hud_data(hud_data), m_game(game), m_visible(hud_data.visible) {}

Vector2 ScoreDisplayHUD::calculate_position() const {
    float anchor_x = 0.0f;
    float anchor_y = 0.0f;

    float screen_width = 640.0f;
    float screen_height = 480.0f;

    switch (m_hud_data.anchor) {
        case udjourney::scene::HUDAnchor::TopLeft:
            anchor_x = 0;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::TopCenter:
            anchor_x = screen_width / 2;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::TopRight:
            anchor_x = screen_width;
            anchor_y = 0;
            break;
        case udjourney::scene::HUDAnchor::MiddleLeft:
            anchor_x = 0;
            anchor_y = screen_height / 2;
            break;
        case udjourney::scene::HUDAnchor::MiddleCenter:
            anchor_x = screen_width / 2;
            anchor_y = screen_height / 2;
            break;
        case udjourney::scene::HUDAnchor::MiddleRight:
            anchor_x = screen_width;
            anchor_y = screen_height / 2;
            break;
        case udjourney::scene::HUDAnchor::BottomLeft:
            anchor_x = 0;
            anchor_y = screen_height;
            break;
        case udjourney::scene::HUDAnchor::BottomCenter:
            anchor_x = screen_width / 2;
            anchor_y = screen_height;
            break;
        case udjourney::scene::HUDAnchor::BottomRight:
            anchor_x = screen_width;
            anchor_y = screen_height;
            break;
    }

    return Vector2{anchor_x + m_hud_data.offset_x,
                   anchor_y + m_hud_data.offset_y};
}

void ScoreDisplayHUD::draw() const {
    if (!m_visible || !m_game) {
        return;
    }

    Vector2 pos = calculate_position();

    // Draw background
    DrawRectangle(static_cast<int>(pos.x),
                  static_cast<int>(pos.y),
                  static_cast<int>(m_hud_data.size_x),
                  static_cast<int>(m_hud_data.size_y),
                  ColorAlpha(BLACK, 0.5f));

    // Draw score text
    char score_text[64];
    snprintf(score_text, sizeof(score_text), "Score: %d", m_game->get_score());
    DrawText(score_text,
             static_cast<int>(pos.x + 10),
             static_cast<int>(pos.y + 10),
             20,
             WHITE);
}

}  // namespace scene
}  // namespace hud
}  // namespace udjourney
