# ⚛️ Quantum Idle

**A quantum mechanics inspired incremental idle game built with C++ and WebAssembly**

🎮 **[Play Now!](https://frowe8.github.io/C-Game/)** | 📖 [How to Deploy](DEPLOY.md) | 🎯 [Marketing Strategy](MARKETING.md) | 🗺️ [Roadmap](ROADMAP.md) | 🕑 [Release Cadence](RELEASE_CADENCE.md)

---

## 🌟 Features

### Core Gameplay
- 🔬 **Quantum Superposition** - Resources accumulate in uncertain states
- 👁️ **Wave Observation** - Collapse quantum states to earn resources
- 🔗 **Entanglement** - Link stations for powerful bonuses
- ⚡ **Prestige System** - Reset for permanent photon bonuses
- 💾 **Auto-Save** - Never lose your progress
- 🌙 **Offline Progress** - Earn resources for up to 4 hours while away

### Progression Systems

#### 🔬 Research Tree (23 Discoveries)
Unlock powerful quantum discoveries across 4 tiers:
- **Tier 1**: Quantum Mechanics 101, Wave Function Theory, Coherence Stabilization, Entanglement Basics
- **Tier 2**: Quantum Computing, Superposition Mastery, Particle Physics, Quantum Tunneling
- **Tier 3**: Quantum Field Theory, String Theory, Quantum Gravity, Time Dilation
- **Tier 4**: Multiverse Theory, Quantum Immortality, Zero Point Energy, **Quantum Singularity**

**Bonuses**: Up to **+10,000% production**!

#### 🎯 Milestone System (24 Goals)
Achieve major milestones for permanent rewards:
- 💰 Wealth: First Thousand → Quantum Trillionaire
- 🏭 Infrastructure: 5 Stations → All Stations Maxed
- ⚡ Prestige: First Prestige → 100 Photons
- ⏱️ Dedication: 1 Hour → 1 Week Playtime
- 🏆 Completionist: All Achievements, All Research
- 🌟 Ultimate: Quantum Master, **The True Ending**

**Rewards**: Up to 1B qubits, 1000 photons, **+500% permanent production**!

#### 🏆 Achievement System (14 Achievements)
Track your progress and unlock rewards:
- First Steps, Observer, Millionaire, Quantum Leap
- Industrialist, Perfect Stability, Entangled, Passive Income
- Speed Runner, Hoarder, Quantum Master, Collector
- Event Hunter, Week Streak

### Game Systems

#### 🏭 Research Stations (5 Types)
- Qubit Generator - Primary resource production
- Coherence Stabilizer - Maintains quantum states
- Entanglement Chamber - Creates quantum links
- Advanced Quantum Lab - High-tier production
- Quantum Singularity Engine - Ultimate power

**Exponential Scaling**: Each level doubles production!

#### 🎲 Quantum Events (7 Types)
Random events that shake up gameplay:
- Wave Collapse - Bonus observation probability
- Coherence Boost - Temporary coherence increase
- Quantum Fluctuation - Random resource bonus
- Entanglement Surge - Free entanglement creation
- Time Dilation - 2x production for 30 seconds
- Lucky Observation - Guaranteed success
- Resource Rain - All resources from the sky!

#### 📊 Statistics Tracking
- Total qubits, coherence, entanglement earned
- Total observations and upgrades
- Total prestiges performed
- Session stats and records
- Daily login streak
- Highest qubits and fastest prestige

### Visual Polish
- ✨ **Particle Effects** - Beautiful visual feedback on all actions
- 🎨 **Quantum-Themed UI** - Glowing blue and purple color scheme
- 📱 **Responsive Design** - Works on desktop, tablet, and mobile
- 🌈 **Animated Elements** - Smooth transitions and hover effects
- 🎯 **Progress Bars** - Visual progress tracking everywhere

---

## 🎮 How to Play

### Basic Controls
- **Keyboard Shortcuts**:
  - `A` - Toggle Achievements panel
  - `S` - Toggle Statistics panel
  - `R` - Toggle Research Tree
  - `M` - Toggle Milestones
  - `ESC` - Close all panels

### Gameplay Loop
1. **Click stations** to observe and collapse quantum states
2. **Earn qubits** from successful observations
3. **Upgrade stations** to increase production
4. **Unlock research** to boost production permanently
5. **Complete milestones** for massive permanent bonuses
6. **Prestige** when ready for photon currency
7. **Repeat** with exponentially growing power!

### Strategy Tips
- 💡 Balance upgrades between station levels and new stations
- 🔬 Prioritize research that gives production bonuses
- 🎯 Work toward milestones for permanent bonuses
- ⚡ Prestige when you can earn 5+ photons for meaningful gains
- 🌙 Close the game - offline progress keeps working!

---

## 🛠️ Technical Stack

### Built With
- **Language**: C++17
- **Build System**: CMake 3.15+
- **Graphics**: OpenGL 1.1 (immediate mode)
- **Windowing**: SDL2
- **Web Compilation**: Emscripten (WebAssembly)
- **Platform Support**: Windows, Linux, macOS, Web

### Architecture
```
Quantum Idle
├── Engine Layer
│   ├── Platform Abstraction (SDL2)
│   ├── Application Framework
│   ├── Renderer (OpenGL)
│   ├── Input System
│   └── Logger
└── Game Layer
    ├── Game State Management
    ├── Resource System
    ├── Research Tree
    ├── Milestone System
    ├── Achievement System
    ├── Quantum Events
    ├── Statistics Tracking
    └── Save/Load System
```

### Code Quality
- ✅ **Modern C++17** features
- ✅ **Cross-platform** codebase
- ✅ **Header-only** utility libraries
- ✅ **Modular design** for easy extension
- ✅ **No external dependencies** (except SDL2)
- ✅ **Optimized builds** with -O3
- ✅ **WebAssembly** compilation ready

---

## 🚀 Getting Started

### Play Online (Easiest!)
Visit **[https://frowe8.github.io/C-Game/](https://frowe8.github.io/C-Game/)** to play instantly in your browser!

### Build from Source

#### Prerequisites
- CMake 3.15+
- C++17 compatible compiler
- SDL2 library
- (Optional) Emscripten for web builds

#### Windows (MinGW)
```bash
# 1. Install SDL2 to C:/SDL2/x86_64-w64-mingw32/
# 2. Build
mkdir build && cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
mingw32-make
./QuantumIdle.exe
```

#### Linux
```bash
# Install SDL2
sudo apt-get install libsdl2-dev

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./QuantumIdle
```

#### macOS
```bash
# Install SDL2
brew install sdl2

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
./QuantumIdle
```

#### Web (Emscripten)
```bash
# Install Emscripten SDK first
./build_web.sh

# Validate asset preload bundle (checks for missing files)
python3 scripts/smoke_test_web_preload.py build-web/QuantumIdle.data \
  assets/fonts/Roboto-Regular.ttf

# Serve locally
cd build
python3 -m http.server 8000
# Open: http://localhost:8000/QuantumIdle.html
```

#### Mobile CI Kickoff (Android/iOS)
- Trigger the **Mobile Platform Readiness** workflow from GitHub Actions (manual run).
- The workflow performs CMake configure-only passes for Android (NDK r26) and iOS (Xcode generator) so the toolchains stay green without requiring signing keys.
- Outputs `build-android/` and `build-ios/` configuration folders you can download from the workflow artifacts for local iteration.

---

## 📦 Project Structure

```
C-Game/
├── .github/
│   └── workflows/
│       ├── deploy.yml          # GitHub Pages deployment
│       └── mobile.yml          # Android/iOS configure-only readiness
├── include/                    # Header files
│   ├── Types.h                 # Core type definitions
│   ├── Application.h           # App framework
│   ├── Renderer.h              # Graphics
│   ├── Input.h                 # Input handling
│   ├── GameState.h             # Main game state
│   ├── Research.h              # Research tree
│   ├── Milestones.h            # Milestone system
│   ├── ParticleCollection.h    # (Future) Particle collection
│   ├── GameUtils.h             # Utility functions
│   └── SteamIntegration.h      # Steam achievements/cloud hook layer
├── src/
│   ├── Main.cpp                # Entry point
│   ├── engine/                 # Engine implementation
│   │   ├── Application.cpp
│   │   ├── Renderer.cpp
│   │   ├── Input.cpp
│   │   ├── SteamIntegration.cpp
│   │   └── Platform.cpp
│   └── game/                   # Game implementation
│       ├── GameState.cpp       # Main game logic (1900+ lines!)
│       ├── Research.cpp        # Research system
│       └── Milestones.cpp      # Milestone system
├── web/
│   └── index.html              # Web interface template
├── CMakeLists.txt              # Build configuration
├── build_web.sh                # Web build script
├── scripts/smoke_test_web_preload.py # Asset preload smoke test
├── DEPLOY.md                   # Deployment guide
├── MARKETING.md                # Marketing strategy
├── ROADMAP.md                  # Development roadmap
└── README.md                   # This file!
```

---

## 📈 Statistics

- **Total Lines of Code**: ~4,000+
- **Source Files**: 11 .cpp files
- **Header Files**: 10 .h files
- **Research Nodes**: 23

## 🕹️ Platform Integrations

- **Steam (stub-ready)**: `SteamIntegration` initializes on startup, queues achievement unlocks, and mirrors local saves for Steam Cloud slots once the Steamworks SDK is linked.
- **Emscripten asset preloading**: Web builds are packaged with `--preload-file assets`, and a smoke test (`scripts/smoke_test_web_preload.py`) runs locally and in CI to ensure required assets are embedded.
- **Mobile configure pipelines**: Manual GitHub Action (`mobile.yml`) exercises Android NDK and iOS CMake generators to keep platform toolchains warmed up without requiring signing certificates.
- **Milestones**: 24
- **Achievements**: 14
- **Quantum Events**: 7
- **Production Bonuses**: Up to 10,000%+ combined!

---

## 🎯 Roadmap

### ✅ Completed (v1.0)
- Core idle game mechanics
- Research Tree system
- Milestone system
- Achievement system
- Quantum Events
- Prestige system
- Offline progress
- Statistics tracking
- Save/load system
- Particle effects
- Web build with responsive UI

### 🚧 In Progress (v1.1)
- Particle Collection system (28 particles)
- Quantum Experiments (8 challenges)
- Enhanced UI polish
- Sound effects and music
- More visual effects

### 📅 Planned (v1.2+)
- Multiplayer leaderboards
- Challenge modes
- Seasonal events
- More research tiers
- Mobile app versions
- Mod support

See [ROADMAP.md](ROADMAP.md) for detailed plans.

---

## 🤝 Contributing

This is a personal project, but suggestions and bug reports are welcome!

1. Open an issue to discuss your idea
2. Fork the repository
3. Create a feature branch
4. Submit a pull request

---

## 📄 License

This project is released under the MIT License - see LICENSE file for details.

---

## 🙏 Acknowledgments

- Quantum mechanics inspiration from real physics concepts
- Idle game mechanics inspired by classics like Cookie Clicker, Anti-Idle
- Built with love using C++, SDL2, and WebAssembly

---

## 📞 Contact & Support

- 🐛 **Bug Reports**: [Open an issue](https://github.com/FRowe8/C-Game/issues)
- 💡 **Feature Requests**: [Open an issue](https://github.com/FRowe8/C-Game/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/FRowe8/C-Game/discussions)

---

## 🎮 Play Now!

**Ready to explore the quantum realm?**

### **[🚀 PLAY QUANTUM IDLE 🚀](https://frowe8.github.io/C-Game/)**

---

<div align="center">

**Made with ⚛️ and C++**

*Collapse the wave function. Earn the qubits. Rule the multiverse.*

</div>
