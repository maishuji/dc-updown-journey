// Copyright 2025 Quentin Cartier
#ifndef UDJOURNEY_HUD_SCENE_IHUD_HPP
#define UDJOURNEY_HUD_SCENE_IHUD_HPP

namespace udjourney {
namespace hud {
namespace scene {

// Simple interface for scene-based HUDs
class IHUD {
 public:
    virtual ~IHUD() = default;
    virtual void draw() const = 0;
    virtual bool is_visible() const = 0;
    virtual void set_visible(bool visible) = 0;
};

}  // namespace scene
}  // namespace hud
}  // namespace udjourney

#endif  // UDJOURNEY_HUD_SCENE_IHUD_HPP
