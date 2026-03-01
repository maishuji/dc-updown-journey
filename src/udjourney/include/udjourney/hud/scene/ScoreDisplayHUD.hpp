// Copyright 2025 Quentin Cartier
#ifndef UDJOURNEY_HUD_SCENE_SCORE_DISPLAY_HUD_HPP
#define UDJOURNEY_HUD_SCENE_SCORE_DISPLAY_HUD_HPP

#include <raylib/raylib.h>
#include "udjourney/scene/Scene.hpp"
#include "udjourney/hud/scene/IHUD.hpp"

namespace udjourney {
class Game;
}

namespace udjourney {
namespace hud {
namespace scene {

class ScoreDisplayHUD : public IHUD {
 public:
    ScoreDisplayHUD(const udjourney::scene::HUDData& hud_data,
                    udjourney::Game* game);
    ~ScoreDisplayHUD() override = default;

    void draw() const override;
    bool is_visible() const override { return m_visible; }
    void set_visible(bool visible) override { m_visible = visible; }

 private:
    Vector2 calculate_position() const;

    const udjourney::scene::HUDData& m_hud_data;
    udjourney::Game* m_game;
    bool m_visible;
};

}  // namespace scene
}  // namespace hud
}  // namespace udjourney

#endif  // UDJOURNEY_HUD_SCENE_SCORE_DISPLAY_HUD_HPP
