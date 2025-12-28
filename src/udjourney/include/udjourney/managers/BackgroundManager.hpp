// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace udjourney::scene {
class Scene;
struct BackgroundLayerData;
}  // namespace udjourney::scene

namespace udjourney {

class BackgroundManager {
 public:
    BackgroundManager() = default;
    ~BackgroundManager();

    BackgroundManager(const BackgroundManager&) = delete;
    BackgroundManager& operator=(const BackgroundManager&) = delete;

    // Scene-bound: rebuild sorted layer list + preload textures referenced by
    // the scene.
    void set_scene(const udjourney::scene::Scene& scene);

    // UI-only scroll state (used by TITLE/GAMEOVER/WIN screens).
    void reset_ui_scroll() noexcept { m_ui_scroll_y = 0.0f; }
    float ui_scroll_y() const noexcept { return m_ui_scroll_y; }

    // Call every frame (only needed for UiScreen scenes).
    void update_ui_scroll(float dt, float viewport_height);

    // Draw backgrounds. If use_ui_scroll == true, camera Y comes from
    // m_ui_scroll_y, otherwise it comes from gameplay_camera_y.
    void draw(float gameplay_camera_y, bool use_ui_scroll, float viewport_width,
              float viewport_height) const;

    void clear();  // unload textures + detach scene

 private:
    void rebuild_sorted_layers_();
    void ensure_textures_loaded_() const;
    void unload_textures_();

    const udjourney::scene::Scene* m_scene = nullptr;
    std::vector<const udjourney::scene::BackgroundLayerData*> m_sorted_layers;

    float m_ui_scroll_y = 0.0f;

    // Textures are loaded lazily once the raylib window/context exists.
    mutable bool m_textures_loaded = false;

    // sprite_sheet (relative) -> Texture2D
    mutable std::unordered_map<std::string, Texture2D> m_textures;
};

}  // namespace udjourney
