#pragma once

#include <raylib/raylib.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace udjourney {
namespace scene {

enum class PlatformBehaviorType {
    Static,
    Horizontal,
    EightTurnHorizontal,
    OscillatingSize
};

enum class PlatformFeatureType { None, Spikes };

struct PlatformData {
    // Tile-based position (will be converted to world coordinates)
    int tile_x;
    int tile_y;

    // Size in tiles
    int width_tiles = 1;
    int height_tiles = 1;

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
    const std::string& get_name() const { return m_name; }

    // Setters
    void set_name(const std::string& name) { m_name = name; }
    void add_platform(const PlatformData& platform) {
        m_platforms.push_back(platform);
    }
    void set_player_spawn(int tile_x, int tile_y) {
        m_player_spawn = {tile_x, tile_y};
    }

    // Tile conversion
    static Rectangle tile_to_world_rect(int tile_x, int tile_y, int width_tiles,
                                        int height_tiles);
    static Vector2 tile_to_world_pos(int tile_x, int tile_y);

    // Constants
    static constexpr float kTileSize = 32.0f;

 private:
    std::vector<PlatformData> m_platforms;
    PlayerSpawnData m_player_spawn{0, 0};
    std::string m_name = "Unnamed Level";
};

}  // namespace scene
}  // namespace udjourney