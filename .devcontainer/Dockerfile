# Use Ubuntu 22.04 as base image
FROM ubuntu:22.04

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# Update package list and install dependencies
RUN apt-get update && apt-get install -y \
    # Build essentials
    build-essential \
    cmake \
    ninja-build \
    git \
    pkg-config \
    # C++ development
    gcc-12 \
    g++-12 \
    gdb \
    # raylib dependencies (no package available, will build from source)
    libasound2-dev \
    libx11-dev \
    libxrandr-dev \
    libxi-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libxcursor-dev \
    libxinerama-dev \
    libglfw3-dev \
    libglew-dev \
    # Additional utilities
    curl \
    wget \
    unzip \
    tree \
    vim \
    nano \
    # Clean up to reduce image size
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*

# Set GCC 12 as default
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100

# Build and install raylib from source (stable version)
WORKDIR /tmp
RUN git clone https://github.com/raysan5/raylib.git \
    && cd raylib \
    && git checkout 5.0 \
    && mkdir build \
    && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=ON \
    && make -j$(nproc) \
    && make install \
    && ldconfig \
    && cd / \
    && rm -rf /tmp/raylib

# Create non-root user
RUN useradd -m -s /bin/bash -G sudo developer \
    && echo "developer:developer" | chpasswd \
    && echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

# Set working directory
WORKDIR /workspace

# Switch to non-root user
USER developer

# Set environment variables for C++20
ENV CC=gcc-12
ENV CXX=g++-12

# Create a simple script to verify the environment
RUN echo '#!/bin/bash\n\
echo "=== Development Environment Info ==="\n\
echo "GCC Version: $(gcc --version | head -n1)"\n\
echo "CMake Version: $(cmake --version | head -n1)"\n\
echo "raylib installed: $(pkg-config --exists raylib && echo "Yes ($(pkg-config --modversion raylib))" || echo "Check manual installation")"\n\
echo "raylib library: $(ldconfig -p | grep raylib | head -1 || echo "Not found in ldconfig")"\n\
echo "OpenGL libraries available:"\n\
pkg-config --exists gl && echo "  - OpenGL: Yes" || echo "  - OpenGL: No"\n\
pkg-config --exists glfw3 && echo "  - GLFW3: Yes ($(pkg-config --modversion glfw3))" || echo "  - GLFW3: No"\n\
echo "==================================="\n' > /home/developer/check-env.sh \
    && chmod +x /home/developer/check-env.sh

# Default command
CMD ["/bin/bash"]