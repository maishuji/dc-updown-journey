// Copyright 2025 Quentin Cartier
#ifndef UDJOURNEY_HUD_SCENE_HEART_HEALTH_HUD_HPP
#define UDJOURNEY_HUD_SCENE_HEART_HEALTH_HUD_HPP

#include <raylib/raylib.h>
#include <string>
#include <unordered_map>
#include "udjourney/scene/Scene.hpp"
#include "udjourney/Player.hpp"
#include "udjourney/hud/scene/IHUD.hpp"

namespace udjourney {
namespace hud {
namespace scene {

class HeartHealthHUD : public IHUD {
 public:
    HeartHealthHUD(const udjourney::scene::HUDData& hud_data, Player* player);
    ~HeartHealthHUD() override = default;

    void draw() const override;
    bool is_visible() const override { return m_visible; }
    void set_visible(bool visible) override { m_visible = visible; }

 private:
    Vector2 calculate_position() const;
    Texture2D get_texture(const std::string& path) const;
    void draw_heart(Vector2 pos, int heart_index,
                    int current_half_hearts) const;

    const udjourney::scene::HUDData& m_hud_data;
    Player* m_player;
    bool m_visible;

    // Shared texture cache
    static std::unordered_map<std::string, Texture2D> s_texture_cache;
};

}  // namespace scene
}  // namespace hud
}  // namespace udjourney

#endif  // UDJOURNEY_HUD_SCENE_HEART_HEALTH_HUD_HPP
