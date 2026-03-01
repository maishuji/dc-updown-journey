#!/bin/bash
# UDJourney Dreamcast Debug Build Script
# Builds with debug symbols for use with sh-elf-gdb

set -e  # Exit on any error

echo "=== UDJourney Dreamcast Debug Build Script ==="

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

# Check if KallistiOS environment is set up
if [ -z "$KOS_BASE" ]; then
    print_error "KallistiOS environment not configured!"
    print_error "Please run: source /opt/toolchains/dc/kos/environ.sh"
    exit 1
fi

print_status "KOS_BASE: $KOS_BASE"

# Create build directory
BUILD_DIR="build-dreamcast-debug"
print_status "Creating debug build directory: $BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Change to build directory
cd "$BUILD_DIR"

# Configure with CMake with debug flags
print_status "Configuring with CMake (Debug mode with symbols)..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../toolchains/dreamcast.cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_FLAGS="-g -ggdb3 -O0" \
    -DCMAKE_CXX_FLAGS="-g -ggdb3 -O0"

if [ $? -ne 0 ]; then
    print_error "CMake configuration failed!"
    exit 1
fi

print_success "CMake configuration complete"

# Build
print_status "Building..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    print_error "Build failed!"
    exit 1
fi

print_success "Build completed successfully!"
print_status "Output files:"
print_status "  - ELF: $BUILD_DIR/src/udjourney/updown-journey.elf"
print_status "  - BIN: $BUILD_DIR/src/udjourney/updown-journey.bin"

print_success "Debug build complete! You can now debug with sh-elf-gdb"
echo ""
print_status "To debug over network, use:"
echo "  sh-elf-gdb $BUILD_DIR/src/udjourney/updown-journey.elf"
echo "  (gdb) target remote <dreamcast-ip>:2159"
echo ""
print_status "Or use 'make debug-dc' to start an interactive debug session"
