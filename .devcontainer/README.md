# UDJourney Linux Development Environment

This directory contains the development container configuration for building the UDJourney editor on Linux.

## Quick Start

### Option 1: Using the Linux Devcontainer

1. **Open in VS Code**: Make sure you have the Dev Containers extension installed
2. **Copy the configuration**: 
   ```bash
   cp .devcontainer/devcontainer-linux.json .devcontainer/devcontainer.json
   ```
3. **Reopen in container**: Use `Ctrl+Shift+P` → "Dev Containers: Reopen in Container"
4. **Build the project**:
   ```bash
   ./build-linux.sh
   ```

### Option 2: Manual Docker Setup

1. **Build the container**:
   ```bash
   docker build -f .devcontainer/Dockerfile -t udjourney-linux .devcontainer/
   ```

2. **Run the container**:
   ```bash
   docker run -it -v $(pwd):/workspace udjourney-linux
   ```

3. **Build inside container**:
   ```bash
   cd /workspace
   ./build-linux.sh
   ```

## What's Included

### Development Tools
- **GCC 12** with C++20 support
- **CMake 3.22+** for build configuration  
- **Ninja** build system for fast compilation
- **Git** for version control
- **GDB** for debugging

### Graphics Libraries
- **raylib 5.6-dev** (built from source)
- **OpenGL** development libraries
- **GLFW3** for windowing
- **Mesa OpenGL** implementation

### Dependencies
All required dependencies are automatically fetched by CMake:
- **ImGui** (docking branch) - UI framework
- **ImGuiFileDialog** - File operations
- **nlohmann/json** - JSON parsing
- **Catch2** - Testing framework

## Building

### Using the Build Script (Recommended)
```bash
./build-linux.sh
```

This script will:
- Create a `build/editor-linux` directory
- Configure CMake with optimal settings
- Build using Ninja (parallel compilation)
- Run tests automatically
- Show colored output for easy debugging

### Manual Build
```bash
mkdir -p build/editor-linux
cd build/editor-linux
cmake ../../src/udjourney-editor -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja
```

## Running the Editor

After a successful build:
```bash
cd build/editor-linux
./udjourney_editor
```

## Features Available

### Editor Features
- **60×30 grid** by default (no need to create manually)
- **Platform editing** with udjourney integration
- **Multi-feature platforms** (spikes, checkpoints, etc.)
- **Resizable selection panel**
- **F1 key focus** for panel management

### Platform Types
- Static platforms
- Horizontal moving platforms  
- Eight-turn horizontal platforms
- Oscillating size platforms

### Export/Import
- **Ctrl+E**: Export tilemap as JSON
- **Ctrl+I**: Import tilemap from JSON
- **Ctrl+N**: Create new level

## Troubleshooting

### Common Issues

1. **"Cannot find raylib"**
   - The devcontainer builds raylib from source automatically
   - If building outside container, install: `sudo apt install libraylib-dev`

2. **OpenGL errors**
   - Make sure you have proper graphics drivers
   - The container includes Mesa software rendering as fallback

3. **Permission issues**
   - The devcontainer uses a `developer` user with sudo access
   - SSH keys are mounted read-only for security

4. **Build failures**
   - Check that you're using GCC 12 and C++20
   - Ensure all submodules are properly fetched by CMake

### Environment Check

Run inside the container:
```bash
/home/developer/check-env.sh
```

This will verify all required tools and libraries are available.

## VS Code Integration

The devcontainer includes optimized VS Code settings:

- **C++ IntelliSense** configured for GCC 12
- **CMake Tools** integration
- **Debugging support** with GDB/LLDB  
- **Git integration** with history and lens
- **Code formatting** on save
- **Syntax highlighting** for CMake files

## Performance Tips

- **Ninja builds** are much faster than Make
- **Release builds** are optimized for performance
- **Parallel compilation** uses all CPU cores
- **Incremental builds** only recompile changed files

## Next Steps

After setting up the environment:

1. **Familiarize yourself** with the editor interface
2. **Test platform creation** and feature assignment
3. **Export a level** to see the JSON format
4. **Add new features** to the platform system
5. **Integrate with the main game** for testing

---

*This development environment is optimized for the UDJourney editor and provides all necessary tools for Linux development.*