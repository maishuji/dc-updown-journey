# Platform Reuse Strategy Architecture

## Problem Description

Previously, the UpDown Journey game applied a global reuse strategy to ALL platforms when they went out of scope. This caused issues where:

- **Level-based platforms** (from scenes/JSON files) would respawn and clutter the screen
- **Random platforms** and **borders** correctly reused for continuous gameplay
- No way to distinguish between designed level platforms and procedurally generated platforms

## Solution: Strategy Pattern Architecture

### Architecture Overview

The reuse system now uses the **Strategy Pattern** where each platform can have its own reuse strategy:

1. **`PlatformReuseStrategy`** - Base abstract class for all reuse strategies
2. **`RandomizePositionStrategy`** - For procedural platforms (respawns at random positions)
3. **`NoReuseStrategy`** - For level-based platforms (marks for removal)
4. **Platform Class** - Now stores its own reuse strategy

### Class Hierarchy

```
PlatformReuseStrategy (abstract)
├── RandomizePositionStrategy    // For random/procedural platforms
└── NoReuseStrategy             // For level-based platforms
```

### Platform Reuse Behavior

| Platform Type | Reuse Strategy | Behavior When Out of Scope |
|---------------|----------------|----------------------------|
| **Scene-based** (level platforms) | `nullptr` (default) | Marked as CONSUMED → Removed |
| **Random platforms** | `RandomizePositionStrategy` | Repositioned randomly |
| **Border walls** | `RandomizePositionStrategy` | Y-repeated (circular buffer) |

## Implementation Details

### 1. Platform Class Changes

```cpp
class Platform {
public:
    // Constructor now accepts optional reuse strategy
    Platform(const IGame &iGame, Rectangle iRect, Color iColor = BLUE,
             bool iIsRepeatedY = false, 
             std::unique_ptr<PlatformReuseStrategy> reuseStrategy = nullptr);
    
    // Platform handles its own reuse
    void reuse() noexcept {
        if (m_reuse_strategy) {
            m_reuse_strategy->reuse(*this);
        } else {
            // Default behavior: no reuse (level-based platform)
            set_state(ActorState::CONSUMED);
        }
    }
    
    // Strategy management
    void set_reuse_strategy(std::unique_ptr<PlatformReuseStrategy> strategy);
    bool has_reuse_strategy() const noexcept;

private:
    std::unique_ptr<PlatformReuseStrategy> m_reuse_strategy;
};
```

### 2. Reuse Strategies

#### NoReuseStrategy
```cpp
void NoReuseStrategy::reuse(Platform& platform) {
    // Level-based platforms should not be reused - mark for removal
    platform.set_state(ActorState::CONSUMED);
}
```

#### RandomizePositionStrategy  
```cpp
void RandomizePositionStrategy::reuse(Platform& platform) {
    // Randomize X position, move to bottom of screen
    // Handles both regular platforms and Y-repeated borders
}
```

### 3. Platform Creation Patterns

#### Scene-Based Platforms (No Reuse)
```cpp
// Level platforms get no reuse strategy (nullptr by default)
auto platform = std::make_unique<Platform>(*this, world_rect, platform_color, 
                                          false); // not Y-repeated
// Default nullptr reuse strategy means no reuse
```

#### Random Platforms (With Reuse)
```cpp
// Random platforms get RandomizePositionStrategy
auto platform = std::make_unique<Platform>(iGame, rect, BLUE, false,
                                          std::make_unique<RandomizePositionStrategy>());
```

#### Border Walls (With Y-Repeat Reuse)
```cpp
// Borders use RandomizePositionStrategy with Y-repeat behavior
auto platform = std::make_unique<Platform>(iGame, border_rect, color, 
                                          true, // Y-repeated
                                          std::make_unique<RandomizePositionStrategy>());
```

## Game Integration

### Updated Reuse Logic
```cpp
case static_cast<uint8_t>(ActorType::PLATFORM): {
    // Let the platform handle its own reuse strategy
    Platform& platform_ref = static_cast<Platform &>(*actor);
    platform_ref.reuse();
    
    // Platforms with no strategy will be marked CONSUMED
    // Platforms with strategies will be repositioned and reset to ONGOING
} break;
```

### Default Behavior
- **No reuse strategy specified** → Platform marked as CONSUMED → Removed from game
- **This is perfect for level-based platforms** → No screen clutter from respawning level elements

## Benefits

### 1. Clean Level Design
- ✅ Level platforms don't respawn when going out of scope
- ✅ No unwanted clutter on screen from level elements
- ✅ Maintains intended level design integrity

### 2. Proper Procedural Gameplay  
- ✅ Random platforms continue to reuse for infinite gameplay
- ✅ Border walls maintain Y-repeat behavior
- ✅ Smooth continuous gameplay experience

### 3. Flexible Architecture
- ✅ Easy to add new reuse strategies in the future
- ✅ Per-platform control over reuse behavior
- ✅ Strategy can be changed at runtime if needed

### 4. Backward Compatibility
- ✅ Existing random platform generation works unchanged
- ✅ All existing gameplay mechanics preserved
- ✅ Only scene-based platforms get the new behavior

## Testing

### Unit Tests Coverage
- **4 new tests** validate the reuse strategy system:
  1. `ScenePlatformNoReuse` - Platforms without strategy mark as CONSUMED
  2. `RandomPlatformReuse` - Platforms with RandomizePositionStrategy get repositioned  
  3. `NoReuseStrategyBehavior` - Explicit NoReuseStrategy marks as CONSUMED
  4. `SetReuseStrategy` - Strategy can be set/changed after platform creation

### Test Results
- **26/26 tests passing** - All existing functionality preserved
- **New tests validate** the strategy pattern implementation
- **Mock Game class** used for isolated platform testing

## Usage Examples

### Creating Level-Based Platform
```cpp
// Scene-based platform (will not reuse)
auto level_platform = std::make_unique<Platform>(*this, world_rect, BLUE);
// Default behavior: no reuse when going out of scope
```

### Creating Random Platform
```cpp
// Random platform (will reuse)
auto random_platform = std::make_unique<Platform>(*this, rect, GREEN, false,
                                                 std::make_unique<RandomizePositionStrategy>());
```

### Changing Strategy at Runtime
```cpp
// Start with no reuse, then enable reuse
platform->set_reuse_strategy(std::make_unique<RandomizePositionStrategy>());
```

## Future Extensions

The strategy pattern makes it easy to add new reuse behaviors:

### Potential New Strategies
- **`FadeOutStrategy`** - Gradually fade platform before removal
- **`ExplodeStrategy`** - Visual explosion effect on removal  
- **`ReturnToOriginalStrategy`** - Move platform back to original scene position
- **`ConditionalReuseStrategy`** - Reuse based on game conditions/score/time

### JSON Configuration (Future)
```json
{
  "platforms": [{
    "x": 5, "y": 10, "width": 3, "height": 1,
    "reuse_strategy": {
      "type": "randomize_position",
      "params": { "min_x": 0, "max_x": 640 }
    }
  }]
}
```

---

## Summary

The new reuse strategy architecture provides:
- **Problem Solved**: Level platforms no longer clutter screen by respawning
- **Flexibility**: Each platform controls its own reuse behavior  
- **Clean Code**: Strategy pattern separates concerns properly
- **Extensibility**: Easy to add new reuse behaviors in the future
- **Testing**: Comprehensive test coverage ensures reliability

**Result**: Clean level gameplay where designed platforms stay designed, while procedural platforms continue to provide infinite gameplay experience.