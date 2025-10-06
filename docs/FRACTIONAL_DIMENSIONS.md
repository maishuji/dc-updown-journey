# Fractional Tile Dimensions Feature

## Overview

The Scene system now supports **fractional tile dimensions**, allowing you to create platforms with precise sizes like 0.3 tile height (approximately 10 pixels) instead of being restricted to whole tile units.

## Feature Details

### What Changed
- **Platform dimensions** now accept float values instead of integers
- **JSON format** supports decimal numbers for width and height
- **Coordinate conversion** handles fractional multipliers
- **All existing functionality** remains compatible

### Tile Size Reference
- **1 tile = 32 pixels**
- **0.5 tiles = 16 pixels**
- **0.3 tiles = 9.6 pixels**  
- **0.25 tiles = 8 pixels**
- **0.1 tiles = 3.2 pixels**

## JSON Format Examples

### Basic Fractional Dimensions
```json
{
  "x": 5,
  "y": 10,
  "width": 3.5,      // 3.5 tiles = 112 pixels wide
  "height": 0.3,     // 0.3 tiles = 9.6 pixels tall
  "behavior": "static"
}
```

### Thin Platforms (Great for Precision Jumping)
```json
{
  "x": 8,
  "y": 15,
  "width": 4,
  "height": 0.2,     // Very thin: 6.4 pixels tall
  "behavior": "horizontal",
  "behavior_params": {
    "speed": 1.5,
    "range": 3.0
  }
}
```

### Narrow Vertical Platforms
```json
{
  "x": 12,
  "y": 8,
  "width": 0.5,      // Narrow: 16 pixels wide
  "height": 3,       // Tall: 96 pixels
  "behavior": "static"
}
```

### Mixed Fractional Dimensions
```json
{
  "x": 2,
  "y": 20,
  "width": 2.75,     // 2.75 tiles = 88 pixels
  "height": 1.8,     // 1.8 tiles = 57.6 pixels
  "behavior": "oscillating_size",
  "behavior_params": {
    "min_scale": 0.6,
    "max_scale": 1.4,
    "speed": 2.0
  }
}
```

## Use Cases

### 1. **Thin Platforms** (height < 1.0)
- **Purpose**: Create challenging precision jumps
- **Recommended Heights**: 0.2 - 0.5 tiles
- **Example**: `"height": 0.3` creates a thin ledge

### 2. **Narrow Platforms** (width < 1.0)  
- **Purpose**: Require precise landing
- **Recommended Widths**: 0.5 - 0.8 tiles
- **Example**: `"width": 0.6` creates a narrow pillar

### 3. **Large Fractional Platforms** (> 1.0 but not whole)
- **Purpose**: Fine-tune platform sizes for perfect fit
- **Example**: `"width": 2.5, "height": 1.3`

### 4. **Micro Platforms** (both dimensions < 1.0)
- **Purpose**: Extreme precision challenges
- **Example**: `"width": 0.8, "height": 0.4`

## Level Design Benefits

### Enhanced Precision
- **Fine-tuned difficulty**: Create platforms that are "just right" for specific challenges
- **Better spacing**: Platforms can fit perfectly in desired locations
- **Visual variety**: Mix of thick and thin platforms creates interesting visual rhythm

### Gameplay Mechanics
- **Thin platforms**: Require more precise jumping
- **Narrow platforms**: Test horizontal accuracy
- **Mixed dimensions**: Create unique platform shapes

### Performance Considerations
- **No performance impact**: Fractional calculations are handled at load time
- **Memory efficient**: Same memory usage as integer dimensions
- **Render optimized**: Raylib handles fractional rectangles efficiently

## Compatibility

### Backwards Compatibility
- **Existing levels**: All integer-based levels continue to work
- **JSON format**: Both `1` and `1.0` are valid and equivalent
- **Migration**: No changes needed to existing level files

### Forward Compatibility
- **Range**: Supports any positive float value
- **Precision**: Limited by float precision (~7 decimal places)
- **Minimum size**: Practically limited by pixel visibility (0.1+ recommended)

## Technical Implementation

### Code Changes
```cpp
// Before (integers only)
int width_tiles = 1;
int height_tiles = 1;

// After (fractional support)
float width_tiles = 1.0f;
float height_tiles = 1.0f;
```

### Coordinate Conversion
```cpp
// Updated method signature
static Rectangle tile_to_world_rect(int tile_x, int tile_y, 
                                   float width_tiles, float height_tiles);

// Calculation remains simple
width_pixels = width_tiles * kTileSize;   // e.g., 0.3 * 32 = 9.6
height_pixels = height_tiles * kTileSize; // e.g., 2.5 * 32 = 80.0
```

### JSON Loading
- **Automatic type conversion**: JSON numbers load as floats
- **Default values**: `1.0f` for width and height if not specified
- **Validation**: No special validation needed (any positive float works)

## Testing Coverage

### Unit Tests Added
- ✅ **Fractional coordinate conversion**: 0.3, 0.5, 1.5, 2.75 dimensions
- ✅ **Scene serialization**: Save/load roundtrip with fractional values
- ✅ **Edge cases**: Very small (0.1) and mixed (1.8x0.25) dimensions
- ✅ **Backwards compatibility**: Integer values still work correctly

### Test Results
```
[==========] 21 tests from 3 test suites ran. (4 ms total)
[  PASSED  ] 21 tests.
```

## Example Levels

### Demo Level: `fractional_demo.json`
Showcases various fractional dimension use cases:
- Thin platforms (0.2 - 0.5 height)
- Narrow platforms (0.5 - 0.8 width)  
- Mixed fractional dimensions
- Integration with all behavior types

### Modified Level 1
Your `level1.json` now includes:
```json
{
  "x": 5,
  "y": 9,
  "width": 4,
  "height": 0.3,    // Thin horizontal moving platform
  "behavior": "horizontal"
}
```

## Best Practices

### Design Guidelines
1. **Start conservative**: Use 0.3+ for height, 0.5+ for width
2. **Test playability**: Very thin platforms may be too difficult
3. **Visual clarity**: Ensure platforms are visible at game resolution
4. **Progressive difficulty**: Introduce fractional dimensions gradually

### JSON Formatting
```json
// Good: Clear decimal notation
"height": 0.3

// Also good: Explicit decimal
"height": 1.0

// Avoid: Too many decimal places
"height": 0.33333333  // Use 0.33 instead
```

### Performance Tips
- **Round to reasonable precision**: 0.3 instead of 0.333333333
- **Batch similar sizes**: Group platforms with similar dimensions
- **Test on target hardware**: Verify performance on Dreamcast if applicable

## Future Enhancements

### Potential Additions
- **Position fractionals**: Allow fractional x,y positioning (currently integers)
- **Auto-sizing**: Platforms that automatically adjust to content
- **Aspect ratio locks**: Maintain width/height ratios during scaling
- **Templates**: Predefined fractional dimension sets

The fractional tile dimensions feature provides precise control over platform sizing while maintaining full backwards compatibility and excellent performance.