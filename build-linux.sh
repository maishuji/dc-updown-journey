#!/bin/bash
# UDJourney Editor Build Script for Linux

set -e  # Exit on any error

echo "=== UDJourney Editor Linux Build Script ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're in the right directory
if [ ! -f "src/udjourney-editor/CMakeLists.txt" ]; then
    print_error "Please run this script from the project root directory"
    exit 1
fi

# Create build directory
BUILD_DIR="build/editor-linux"
print_status "Creating build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Change to build directory
cd "$BUILD_DIR"

# Configure with CMake
print_status "Configuring with CMake..."
cmake ../../src/udjourney-editor \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20 \
    -DCMAKE_C_COMPILER=gcc-12 \
    -DCMAKE_CXX_COMPILER=g++-12 \
    -G Ninja

if [ $? -ne 0 ]; then
    print_error "CMake configuration failed"
    exit 1
fi

print_success "CMake configuration completed"

# Build the project
print_status "Building UDJourney Editor..."
ninja

if [ $? -ne 0 ]; then
    print_error "Build failed"
    exit 1
fi

print_success "Build completed successfully!"

# Check if the executable was created
if [ -f "udjourney_editor" ]; then
    print_success "Executable created: $(pwd)/udjourney_editor"
    
    # Make it executable
    chmod +x udjourney_editor
    
    print_status "You can run the editor with:"
    echo "  cd $(pwd)"
    echo "  ./udjourney_editor"
else
    print_error "Executable not found after build"
    exit 1
fi

# Run tests if they exist
if [ -f "tilepanel_tests" ]; then
    print_status "Running tests..."
    chmod +x tilepanel_tests
    if ./tilepanel_tests; then
        print_success "All tests passed!"
    else
        print_warning "Some tests failed, but build is complete"
    fi
fi

print_success "=== Build process completed ==="