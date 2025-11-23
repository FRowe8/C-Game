#!/bin/bash
# Build script for Unix-like systems (Linux, macOS)

set -e

echo "Quantum Idle - Build Script"
echo "============================"
echo ""

# Create build directory
mkdir -p build
cd build

# Configure
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo "Building..."
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "Build complete! Run with: ./build/QuantumIdle"
