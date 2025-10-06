# Level 1 - Journey Downward Integration

## Overview

Level 1 has been successfully integrated into UpDown Journey using the Scene system. The game now loads structured levels from JSON files instead of random platform generation.

## Level 1 Design

### Objective
Journey from the top platform down to the bottom platform to win the game.

### Level Layout
- **Start Position**: Tile (10, 1) - Near the top of the screen
- **Total Vertical Distance**: 35 tiles (1,120 pixels)
- **Platform Count**: 12 unique platforms with different behaviors

### Platform Details

1. **Starting Platform** (8,3) - 4x1 tiles, Static
   - Safe landing area after spawn

2. **Jump Platform** (15,6) - 3x1 tiles, Static  
   - First jumping challenge

3. **Moving Platform** (5,9) - 4x1 tiles, Horizontal
   - Speed: 1.5, Range: 6 tiles
   - First moving platform encounter

4. **Rest Platform** (12,12) - 5x1 tiles, Static
   - Larger platform for safety

5. **Spike Trap** (2,15) - 3x1 tiles, Static + Spikes
   - Dangerous platform to avoid

6. **Figure-8 Platform** (8,15) - 2x1 tiles, Eight-Turn  
   - Speed: 1.2, Amplitude: 3 tiles
   - Advanced movement pattern

7. **Size-Changing Platform** (14,18) - 4x1 tiles, Oscillating Size
   - Min Scale: 0.6, Max Scale: 1.4, Speed: 2.0
   - Platform that grows and shrinks

8. **Safety Platform** (6,21) - 6x1 tiles, Static
   - Large safe platform

9. **Moving Spikes** (16,24) - 3x1 tiles, Horizontal + Spikes  
   - Speed: 2.0, Range: 4 tiles
   - Dangerous moving platform

10. **Near-End Platform** (3,27) - 5x1 tiles, Static
    - Preparation for final jump

11. **Pre-Final Platform** (11,30) - 4x1 tiles, Static
    - Last challenging jump

12. **Victory Platform** (7,33) - 8x2 tiles, Static
    - **WINNING CONDITION**: Reaching this platform wins the game!

## Game Features

### Win Condition  
- Player reaches 90% of the level height (near the bottom)
- WIN game state is triggered
- Celebratory screen with stars animation
- Option to restart level or quit

### Visual Indicators
- **Blue Platforms**: Static behavior
- **Green Platforms**: Horizontal movement
- **Orange Platforms**: Eight-turn movement  
- **Purple Platforms**: Oscillating size
- **Red Platforms**: Platforms with spikes

### Controls
- **During Play**: Standard movement controls
- **Win Screen**: 
  - START button: Restart level
  - B button: Quit game

## Technical Implementation

### Scene System Integration
- JSON-based level definition
- Automatic tile-to-world coordinate conversion
- Platform behavior and feature assignment
- Player spawn positioning from scene data

### File Structure
```
levels/
└── level1.json          # Level 1 definition
```

### Build System
- Levels automatically copied to build directory
- CMake handles level file deployment
- No manual file copying required

### Code Integration Points
- `Game::load_scene()` - Loads JSON level files
- `Game::create_platforms_from_scene()` - Creates game objects from scene data  
- `Game::restart_level()` - Resets level for replay
- Win condition checking in game update loop

## Playing Level 1

### Strategy Tips
1. **Start Carefully**: Use the starting platform to get familiar with controls
2. **Time Moving Platforms**: Wait for horizontal platforms to be in good position
3. **Avoid Red Platforms**: Spikes cause damage and reset progress
4. **Use Large Platforms**: Blue static platforms are safe resting spots
5. **Plan Ahead**: Look down to see the next platform before jumping
6. **Final Stretch**: The last few platforms require precise timing

### Difficulty Progression
- **Early (Tiles 1-15)**: Basic jumping and simple moving platforms
- **Middle (Tiles 15-25)**: Introduction of spikes and complex behaviors
- **Late (Tiles 25-35)**: Combination challenges requiring mastery

## Development Notes

### Performance
- All platforms loaded at scene start
- No dynamic loading during gameplay
- Optimized for 60 FPS gameplay

### Extensibility
- Easy to create new levels by copying `level1.json`
- All platform behaviors supported
- Feature system ready for new additions

## Next Steps

Future enhancements could include:
- Multiple levels (level2.json, level3.json, etc.)
- Level selection screen
- Progress saving
- Time-based scoring
- Additional platform features (teleporters, boost pads, etc.)

The Scene system provides a solid foundation for creating engaging, structured gameplay experiences in UpDown Journey.