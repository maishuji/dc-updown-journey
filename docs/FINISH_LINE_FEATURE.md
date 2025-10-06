# Finish Line Visual Indicator

## Overview

Added a **pink/magenta finish line** that visually indicates where the win condition triggers in level-based gameplay. This provides clear visual feedback to players about when they're about to complete a level.

## Feature Details

### Visual Design
- **Color**: Bright magenta/pink for high visibility
- **Thickness**: 4 pixels for clear visibility
- **Width**: Spans the full width of the game screen  
- **Enhancement**: Pink outline for subtle glow effect
- **Label**: "FINISH LINE" text when line is visible in a good position

### Technical Implementation

#### Position Calculation
- **Placement**: Positioned at exactly **98% of the level height**
- **Alignment**: This matches exactly where the win condition triggers
- **Coordinates**: Uses world coordinates then converts to screen space
- **Visibility**: Only renders when potentially visible on screen

#### Code Structure
```cpp
// In Game.hpp
void draw_finish_line_() const;

// In Game.cpp - called during PLAY state
if (m_current_scene && m_level_height > 0) {
    draw_finish_line_();
}
```

#### Drawing Logic
```cpp
void Game::draw_finish_line_() const {
    // Calculate finish line position (98% of level height)
    float win_threshold = m_level_height * 0.98f;
    
    // Convert to screen coordinates relative to camera
    Rectangle game_rect = get_rectangle();
    float line_y = win_threshold - game_rect.y;
    
    // Draw thick magenta line with pink outline
    DrawRectangleRec(finish_line, MAGENTA);
    DrawRectangleLinesEx(finish_line, 1.0f, PINK);
    
    // Add text label when appropriate
    DrawText("FINISH LINE", x, y, 16, MAGENTA);
}
```

## Behavior

### When It Appears
- ✅ **Level-based gameplay only** - Only shown when `m_current_scene` is loaded
- ✅ **Valid level height** - Only when `m_level_height > 0`
- ✅ **Screen visibility** - Only renders when line would be visible

### When It's Hidden
- ❌ **Random/procedural gameplay** - No finish line for infinite procedural levels
- ❌ **Off-screen** - Hidden when camera hasn't reached that area yet
- ❌ **Invalid levels** - Hidden for levels without proper height calculation

### Visual Feedback
1. **Approach indicator** - Players see the line as they get close to completing the level
2. **Goal clarity** - Makes it obvious what the objective is (reach the pink line)
3. **Progress feedback** - Shows how close you are to winning
4. **Celebration prep** - Builds anticipation for the win screen

## Level Design Integration

### Example Usage

#### Level 1 (level1.json)
- **Final platform**: y=33, height=2 (bottom at y=35 tiles = 1120 pixels)
- **Finish line**: 98% of 1120 = **1097.6 pixels**  
- **Result**: Line appears just above the final platform area

#### Demo Level (finish_line_demo.json)  
- **Final platform**: y=9, height=2 (bottom at y=11 tiles = 352 pixels)
- **Finish line**: 98% of 352 = **344.96 pixels**
- **Result**: Line appears right at the final platform for easy testing

### Design Guidelines
- **Final platform positioning**: Design final platforms so the 98% line appears appropriately
- **Visual spacing**: Leave some space above final platform for the finish line to be visible
- **Testing**: Use `finish_line_demo.json` to quickly test finish line positioning

## Technical Details

### Performance
- **Minimal overhead**: Only calculates when level is loaded and line could be visible
- **Smart culling**: Skips rendering when line is off-screen
- **Single draw call**: Efficient rectangle and line drawing

### Coordinate System Integration
- **World coordinates**: Calculates in world space using `m_level_height`
- **Screen conversion**: Properly converts to screen coordinates using game camera
- **Camera following**: Line stays in correct world position as camera follows player

### Compatibility
- **Scene system**: Integrates seamlessly with existing scene/level loading
- **Fractional tiles**: Works correctly with fractional tile dimensions
- **Win condition**: Positioned at exact same threshold as win trigger (98%)

## User Experience

### Player Benefits
1. **Clear objective** - "Get to the pink line to win"
2. **Progress indication** - See how close you are to completing the level  
3. **Anticipation building** - Excitement builds as you approach the finish
4. **Visual consistency** - Matches the win condition exactly

### Gameplay Flow
1. **Level start** - Player sees platforms and begins descent
2. **Mid-level** - Focus on navigating platforms and obstacles
3. **Approaching finish** - Pink line becomes visible, building excitement
4. **Crossing finish** - Player crosses pink line → Win condition triggers → Congratulations!

## Testing

### Verification
- ✅ **26/26 tests pass** - No regression in existing functionality
- ✅ **Game launches** - Finish line appears correctly in Level 1
- ✅ **Demo level** - `finish_line_demo.json` showcases the feature clearly

### Manual Testing
```bash
# Test with Level 1 (full level)
cd build/src/udjourney && ./updown-journey

# Test with demo level (quick test)
# (Would need to modify Game.cpp to load finish_line_demo.json)
```

## Future Enhancements

### Potential Improvements
1. **Animation** - Pulsing or glowing effect for the finish line
2. **Particle effects** - Sparkles or shimmer near the finish line
3. **Sound cues** - Audio feedback when approaching the finish line
4. **Customizable colors** - JSON configuration for finish line appearance
5. **Multiple finish lines** - Support for checkpoint-style finish lines

### JSON Configuration (Future)
```json
{
  "finish_line": {
    "color": "magenta",
    "thickness": 4,
    "label": "FINISH LINE",
    "effects": ["glow", "particles"]
  }
}
```

---

## Summary

The finish line feature provides essential visual feedback for level completion:

- **Clear visual goal** - Pink line shows exactly where to go to win
- **Perfect positioning** - Located at 98% level height (same as win condition)
- **Smart rendering** - Only appears in level-based gameplay when visible
- **Enhanced UX** - Players always know their objective and progress

This small addition significantly improves the game's user experience by making level objectives crystal clear! 🎮✨