# Scene System Documentation

## Overview

The Scene system provides tile-based level loading and management for the UpDown Journey game. It allows levels to be defined in JSON files with platform data, behaviors, features, and spawn points.

## Scene File Format

Scene files are JSON documents with the following structure:

```json
{
  "name": "Level Name",
  "player_spawn": {
    "x": 1,
    "y": 10
  },
  "platforms": [
    {
      "x": 0,
      "y": 12,
      "width": 5,
      "height": 1,
      "behavior": "static",
      "features": ["spikes"],
      "behavior_params": {
        "speed": 2.0,
        "range": 4.0
      },
      "feature_params": {
        "damage": 1.0
      }
    }
  ]
}
```

## Platform Properties

### Position and Size
- `x`, `y`: Tile coordinates (integers)
- `width`, `height`: Size in tiles (integers)

### Behavior Types
- `"static"`: No movement (default)
- `"horizontal"`: Side-to-side movement
- `"eight_turn"`: Figure-8 movement pattern
- `"oscillating_size"`: Size scaling animation

### Behavior Parameters
- `speed`: Movement/animation speed (float)
- `range`: Movement range for horizontal behavior (float)  
- `amplitude`: Movement amplitude for eight-turn behavior (float)
- `min_scale`, `max_scale`: Size limits for oscillating behavior (float)

### Features
- `"spikes"`: Damaging spikes on platform

### Feature Parameters
- `damage`: Spike damage amount (float)

## Tile System

- Each tile is 32x32 pixels
- Coordinate (0,0) is top-left
- Y-axis increases downward
- Use `Scene::tile_to_world_pos()` to convert tile coordinates to world coordinates
- Use `Scene::tile_to_world_rect()` to get world rectangles for platforms

## Usage Example

```cpp
#include "udjourney/scene/Scene.hpp"

using namespace udjourney::scene;

// Load a scene from file
Scene scene;
if (scene.load_from_file("levels/level1.json")) {
    // Get player spawn position
    auto spawn = scene.get_player_spawn();
    Vector2 world_pos = scene.tile_to_world_pos(spawn.tile_x, spawn.tile_y);
    
    // Access platforms
    const auto& platforms = scene.get_platforms();
    for (const auto& platform : platforms) {
        Rectangle world_rect = scene.tile_to_world_rect(
            platform.tile_x, platform.tile_y,
            platform.width_tiles, platform.height_tiles
        );
        // Create game platform from data...
    }
}

// Save modified scene
scene.save_to_file("levels/modified_level.json");
```

## Integration with Game Class

The Game class includes methods for loading scenes:

```cpp
Game game(800, 600);
if (game.load_scene("levels/test_level_1.json")) {
    // Scene loaded successfully
    game.run();
}
```

## File Locations

- Scene files: `levels/` directory
- Example scene: `levels/test_level_1.json`
- Headers: `src/udjourney/include/udjourney/scene/Scene.hpp`
- Implementation: `src/udjourney/src/scene/Scene.cpp`