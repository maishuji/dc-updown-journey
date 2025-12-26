// Copyright 2025 Quentin Cartier
#pragma once

#include <raylib/raylib.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace udjourney {
namespace scene {

enum class SceneType {
    LEVEL,     // Regular gameplay level with platforms/monsters
    UI_SCREEN  // UI screen for menus (title/win/gameover)
};

enum class PlatformBehaviorType {
    Static,
    Horizontal,
    EightTurnHorizontal,
    OscillatingSize
};

enum class PlatformFeatureType { None, Spikes, Checkpoint };

struct PlatformData {
    // Tile-based position (will be converted to world coordinates)
    int tile_x;
    int tile_y;

    // Size in tiles (supports fractional values like 0.3)
    float width_tiles = 1.0f;
    float height_tiles = 1.0f;

    // Behavior configuration
    PlatformBehaviorType behavior_type = PlatformBehaviorType::Static;

    // Behavior-specific parameters (stored as key-value pairs)
    std::map<std::string, float> behavior_params;

    // Features
    std::vector<PlatformFeatureType> features;
    std::map<std::string, float> feature_params;

    // Visual
    Color color = BLUE;
};

struct PlayerSpawnData {
    int tile_x;
    int tile_y;
};

struct MonsterSpawnData {
    int tile_x;
    int tile_y;
    std::string preset_name = "goblin";  // Monster preset to use

    // Legacy fields for backward compatibility (deprecated)
    float patrol_range = 100.0f;                   // Patrol range in pixels
    float chase_range = 200.0f;                    // Chase range in pixels
    float attack_range = 50.0f;                    // Attack range in pixels
    std::string sprite_sheet = "char1-Sheet.png";  // Default sprite
    std::string animation_config =
        "animations/player_animations.json";  // Path to animation config
};

struct BackgroundObjectData {
    std::string sprite_name;
    float x = 0.0f;
    float y = 0.0f;
    float scale = 1.0f;
    float rotation = 0.0f;
    std::string sprite_sheet;
    int tile_size = 128;
    int tile_row = 0;
    int tile_col = 0;
};

struct BackgroundLayerData {
    std::string name = "Background Layer";
    std::string texture_file;
    float parallax_factor = 1.0f;
    int depth = 0;
    bool auto_scroll_enabled = true;  // Enable/disable automatic scrolling
    float scroll_speed_x = 0.0f;      // Horizontal scroll speed (pixels/sec)
    float scroll_speed_y = 0.0f;      // Vertical scroll speed (pixels/sec)
    bool repeat = false;  // True = infinite loop, False = stop at texture edge
    std::vector<BackgroundObjectData> objects;
};

enum class HUDAnchor {
    TopLeft,
    TopCenter,
    TopRight,
    MiddleLeft,
    MiddleCenter,
    MiddleRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

struct HUDData {
    std::string name;
    std::string type_id;
    HUDAnchor anchor = HUDAnchor::TopLeft;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float size_x = 100.0f;
    float size_y = 30.0f;
    bool visible = true;
    std::map<std::string, std::string> properties;  // Simplified for runtime

    // Background sprite configuration
    std::string background_sheet;    // Sprite sheet file
    int background_tile_size = 32;   // Size of each tile in sheet
    int background_tile_row = 0;     // Row in sprite sheet
    int background_tile_col = 0;     // Column in sprite sheet
    int background_tile_width = 1;   // Width in tiles (for multi-tile sprites)
    int background_tile_height = 1;  // Height in tiles (for multi-tile sprites)

    // Foreground sprite configuration
    std::string foreground_sheet;    // Sprite sheet file
    int foreground_tile_size = 32;   // Size of each tile in sheet
    int foreground_tile_row = 0;     // Row in sprite sheet
    int foreground_tile_col = 0;     // Column in sprite sheet
    int foreground_tile_width = 1;   // Width in tiles (for multi-tile sprites)
    int foreground_tile_height = 1;  // Height in tiles (for multi-tile sprites)

    float image_scale = 1.0f;  // Scale for images
};

class Scene {
 public:
    Scene() = default;
    explicit Scene(const std::string& filename);
    virtual ~Scene() = default;

    // Load/Save
    bool load_from_file(const std::string& filename);
    bool save_to_file(const std::string& filename) const;

    // Getters
    const std::vector<PlatformData>& get_platforms() const {
        return m_platforms;
    }
    const PlayerSpawnData& get_player_spawn() const { return m_player_spawn; }
    const std::vector<MonsterSpawnData>& get_monster_spawns() const {
        return m_monster_spawns;
    }
    const std::vector<BackgroundLayerData>& get_background_layers() const {
        return m_background_layers;
    }
    const std::vector<HUDData>& get_huds() const { return m_huds; }
    const std::string& get_name() const { return m_name; }
    SceneType get_type() const { return m_scene_type; }
    float get_scroll_speed() const { return m_scroll_speed; }

    // Setters
    void set_name(const std::string& name) { m_name = name; }
    void set_type(SceneType type) { m_scene_type = type; }
    void set_scroll_speed(float speed) { m_scroll_speed = speed; }
    void add_platform(const PlatformData& platform) {
        m_platforms.push_back(platform);
    }
    void set_player_spawn(int tile_x, int tile_y) {
        m_player_spawn = {tile_x, tile_y};
    }
    void add_monster_spawn(const MonsterSpawnData& monster) {
        m_monster_spawns.push_back(monster);
    }

    // Tile conversion
    static Rectangle tile_to_world_rect(int tile_x, int tile_y,
                                        float width_tiles, float height_tiles);
    static Vector2 tile_to_world_pos(int tile_x, int tile_y);

    // Constants
    static constexpr float kTileSize = 32.0f;

 private:
    std::vector<PlatformData> m_platforms;
    PlayerSpawnData m_player_spawn{0, 0};
    std::vector<MonsterSpawnData> m_monster_spawns;
    std::vector<BackgroundLayerData> m_background_layers;
    std::vector<HUDData> m_huds;
    std::string m_name = "Unnamed Level";
    SceneType m_scene_type =
        SceneType::LEVEL;  // Default to level for backward compatibility
    float m_scroll_speed =
        1.0f;  // Default camera scroll speed (pixels per frame)
};

}  // namespace scene
}  // namespace udjourney
