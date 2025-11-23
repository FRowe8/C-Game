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

### Prerequisites
- CMake 3.15+
- C++17 compatible compiler
- SDL2
- OpenGL 3.0+

### Desktop Build

```bash
# Clone repository
git clone <repository-url>
cd C-Game

# Create build directory
mkdir build && cd build

# Configure
cmake ..

# Build
cmake --build .

# Run
./QuantumIdle
```

### Platform-Specific Notes

#### Windows
- Install SDL2 via vcpkg: `vcpkg install sdl2`
- Or download from https://www.libsdl.org/

#### Linux
```bash
sudo apt-get install libsdl2-dev  # Debian/Ubuntu
sudo dnf install SDL2-devel        # Fedora
```

#### macOS
```bash
brew install sdl2
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
