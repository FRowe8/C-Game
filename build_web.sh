#!/bin/bash
# Build script for Web (Emscripten) version

set -e  # Exit on error

echo "========================================="
echo "Quantum Idle - Web Build (Emscripten)"
echo "========================================="
echo ""

# Check if Emscripten is installed
if ! command -v emcc &> /dev/null; then
    echo "❌ ERROR: Emscripten not found!"
    echo ""
    echo "Please install Emscripten SDK:"
    echo "  git clone https://github.com/emscripten-core/emsdk.git"
    echo "  cd emsdk"
    echo "  ./emsdk install latest"
    echo "  ./emsdk activate latest"
    echo "  source ./emsdk_env.sh"
    echo ""
    exit 1
fi

echo "✅ Emscripten found: $(emcc --version | head -n 1)"
echo ""

# Create build directory
echo "[1/4] Creating build directory..."
mkdir -p build-web
cd build-web

# Configure with Emscripten
echo "[2/4] Configuring CMake with Emscripten..."
emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DPLATFORM_WEB=ON \
    -G "Unix Makefiles"

# Build
echo "[3/4] Building..."
emmake make -j$(nproc 2>/dev/null || echo 4)

# Copy to web directory
echo "[4/4] Copying files to web directory..."
cd ..
mkdir -p web/dist
cp build-web/QuantumIdle.js web/dist/
cp build-web/QuantumIdle.wasm web/dist/
cp build-web/QuantumIdle.data web/dist/ 2>/dev/null || true  # Data file may not exist
cp web/index.html web/dist/

echo ""
echo "========================================="
echo "✨ Build Complete!"
echo "========================================="
echo ""
echo "Output directory: web/dist/"
echo ""
echo "To test locally:"
echo "  cd web/dist"
echo "  python3 -m http.server 8080"
echo "  Then open: http://localhost:8080"
echo ""
echo "To deploy:"
echo "  Upload the contents of web/dist/ to your web server"
echo ""
