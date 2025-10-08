# Unit Test Suite for UpDown Journey

This directory contains comprehensive unit tests for the UpDown Journey game, built using Google Test framework.

## Test Structure

```
tests/
├── CMakeLists.txt              # Test build configuration
├── test_main.cpp               # Test entry point
├── test_data/                  # Test data files
│   ├── valid_scene.json        # Valid scene for testing
│   └── invalid_scene.json      # Invalid scene for error testing
└── scene/                      # Scene system tests
    ├── test_scene.cpp                      # Core Scene class tests
    ├── test_scene_serialization.cpp       # Save/load roundtrip tests
    └── test_coordinate_conversion.cpp      # Coordinate system tests
```

## Test Categories

### 1. Scene Core Tests (`test_scene.cpp`)
- **Purpose**: Test basic Scene class functionality
- **Coverage**: 
  - Default constructor behavior
  - Scene loading from JSON files
  - Complex scene parsing with all behavior types
  - File constructor usage
  - Error handling for non-existent files
  - Setter/getter methods
  - All platform behavior type parsing

### 2. Scene Serialization Tests (`test_scene_serialization.cpp`)
- **Purpose**: Test save/load data integrity
- **Coverage**:
  - Complete save/load roundtrip testing
  - Error handling for invalid file paths
  - Empty scene serialization
  - Player spawn only scenes
  - Multiple save/load cycles for data stability
  - Complex scene data preservation

### 3. Coordinate Conversion Tests (`test_coordinate_conversion.cpp`)
- **Purpose**: Test tile-to-world coordinate system
- **Coverage**:
  - Tile size constant verification
  - Basic tile to world position conversion
  - Tile to world rectangle conversion
  - Edge cases and boundary conditions
  - Coordinate consistency validation
  - Mathematical relationship verification
  - Common game scenario testing
  - Player spawn scenario validation

## Running Tests

### Quick Test Run
```bash
cd build
./tests/updown_journey_tests
```

### Full CTest Integration
```bash
cd build
ctest --verbose
```

### Run Specific Test Suite
```bash
cd build
./tests/updown_journey_tests --gtest_filter="SceneTest.*"
./tests/updown_journey_tests --gtest_filter="CoordinateConversionTest.*"
./tests/updown_journey_tests --gtest_filter="SceneSerializationTest.*"
```

### Build and Run Tests
```bash
cd build
make updown_journey_tests
./tests/updown_journey_tests
```

### Use Custom Target
```bash
cd build
make run_tests
```

## Test Results Summary

Current test status: **20/20 tests passing**

- ✅ **SceneTest**: 7 tests - Basic functionality
- ✅ **SceneSerializationTest**: 5 tests - Data integrity
- ✅ **CoordinateConversionTest**: 8 tests - Math validation

## Test Data Files

### `test_data/valid_scene.json`
- Complete valid scene with all features
- Multiple platform behaviors
- Features like spikes
- Behavior and feature parameters
- Used for positive testing scenarios

### `test_data/invalid_scene.json`
- Intentionally malformed JSON
- Invalid data types
- Missing required fields
- Used for error handling testing

## Writing New Tests

### Adding Scene Tests
1. Create new test file in `tests/scene/`
2. Include necessary headers:
   ```cpp
   #include <gtest/gtest.h>
   #include "udjourney/scene/Scene.hpp"
   ```
3. Add test file to `tests/CMakeLists.txt`
4. Follow existing patterns for test fixtures

### Test Naming Convention
- Test files: `test_[component].cpp`
- Test classes: `[Component]Test`
- Test methods: `[TestClass].[FeatureBeing​Tested]`

### Example Test Template
```cpp
#include <gtest/gtest.h>
#include "udjourney/scene/Scene.hpp"

class MyComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Setup code
    }
    
    void TearDown() override {
        // Cleanup code
    }
};

TEST_F(MyComponentTest, FeatureBeingTested) {
    // Arrange
    // Act
    // Assert
    EXPECT_EQ(expected, actual);
}
```

## Dependencies

- **Google Test**: v1.14.0 (fetched automatically)
- **nlohmann/json**: For JSON parsing in Scene tests
- **raylib**: For Vector2/Rectangle types
- **C++20**: Required for modern C++ features

## Build Configuration

Tests are automatically built with the main project but only for non-Dreamcast builds. The CMake configuration:

- Fetches Google Test automatically
- Links required libraries
- Copies test data files
- Configures CTest integration
- Provides convenient build targets

## Continuous Integration

These tests are designed to run in CI environments and provide detailed output for debugging failures. All tests complete quickly (< 100ms total) and are deterministic.