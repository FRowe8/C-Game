# Quantum Idle - Cross-Platform Game Engine

A custom C++ game engine designed for creating engaging idle games across multiple platforms.

## Game Concept

**Quantum Idle** - Manage a quantum research facility where resources exist in superposition states. The unique "observation mechanic" lets players collapse quantum states into usable resources, creating an engaging blend of active and idle gameplay.

### Key Features
- 🔬 Quantum resource management with superposition states
- 🔗 Entanglement system for strategic bonus multipliers
- 📊 Coherence mechanics that reward regular check-ins
- ♻️ Prestige system through parallel quantum realities
- 💾 Cross-platform save system
- 🎨 Particle effects and visual feedback

## Supported Platforms

- ✅ Windows (Desktop)
- ✅ Linux (Desktop)
- ✅ macOS (Desktop)
- 🚧 iOS (Planned)
- 🚧 Android (Planned)
- 🚧 Web (via Emscripten - Planned)

## Building

### 🚀 Quick Start

Choose your platform:

#### **Windows** (Easiest!)
```cmd
build.bat
```
The script will automatically offer to download SDL2 for you!

**👉 See [QUICKSTART_WINDOWS.md](QUICKSTART_WINDOWS.md) for detailed Windows instructions**

#### **Linux**
```bash
sudo apt-get install libsdl2-dev cmake build-essential  # Ubuntu/Debian
# OR
sudo dnf install SDL2-devel cmake gcc-c++               # Fedora

./build.sh
./build/QuantumIdle
```

#### **macOS**
```bash
brew install sdl2 cmake

./build.sh
./build/QuantumIdle
```

---

### Prerequisites

- **CMake** 3.15+ ([Download](https://cmake.org/download/))
- **C++17 compiler**
  - Windows: Visual Studio Build Tools 2019+ ([Download](https://visualstudio.microsoft.com/downloads/))
  - Linux: GCC 7+ or Clang 5+
  - macOS: Xcode Command Line Tools
- **SDL2** ([Download](https://github.com/libsdl-org/SDL/releases) or use package manager)
- **OpenGL** 2.1+ (included with graphics drivers)

### Detailed Build Instructions

#### Windows (Manual Method)
See [WINDOWS_BUILD.md](WINDOWS_BUILD.md) for detailed instructions including:
- Manual SDL2 installation
- MSYS2 alternative
- Troubleshooting tips

#### All Platforms
```bash
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

## Architecture

### Engine Components
- **Core**: Application lifecycle, time management
- **Platform**: Abstraction layer for OS-specific features
- **Renderer**: OpenGL ES rendering system
- **Input**: Cross-platform input handling
- **Audio**: Sound and music system

### Game Components
- **Quantum**: Idle game logic and mechanics
- **UI**: User interface system
- **Persistence**: Save/load system

## Development Roadmap

### MVP (v1.0)
- [x] Core engine architecture
- [ ] Basic rendering system
- [ ] Input handling
- [ ] Game state management
- [ ] Core quantum mechanics
- [ ] UI system
- [ ] Save/load functionality

### Future Updates
- Multiple quantum particles
- Research tree expansion
- Achievement system
- Quantum paradoxes (special events)
- Multiplayer/leaderboards
- Mobile platform support

## License

MIT License - See LICENSE file for details

## Contributing

This is a personal project, but suggestions and feedback are welcome!
