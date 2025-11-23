# Development Guide

## Architecture Overview

### Engine Layer
The engine is designed to be completely game-agnostic and reusable:

- **Platform** (`src/engine/Platform.cpp`): OS abstraction for file I/O, time, display
- **Application** (`src/engine/Application.cpp`): Main loop, window management, SDL integration
- **Renderer** (`src/engine/Renderer.cpp`): 2D rendering with OpenGL, particle system
- **Input** (`src/engine/Input.cpp`): Keyboard, mouse, and touch input handling
- **Logger** (`src/engine/Logger.cpp`): Logging system with levels
- **Types** (`src/engine/Types.cpp`): Math types (Vec2, Vec3, Color, Rect)

### Game Layer
Game-specific logic for Quantum Idle:

- **GameState** (`src/game/GameState.cpp`): Core game logic, resource management, UI
- **ResearchStation**: Generates resources in superposition states
- **QuantumTimeline**: Prestige system and bonuses

## Key Design Patterns

### Entity-Component Pattern
While not a full ECS, the ResearchStation structure acts as a component with:
- Data (level, production, superposition)
- Behavior (Update, Observe, Upgrade)

### Observer Pattern
The "Observe" mechanic is both thematic and functional:
- Resources accumulate in superposition (uncertain state)
- Observation collapses them into definite values
- Probabilistic outcomes create engagement

### Game Loop
Fixed timestep with delta time:
```
while (running) {
    ProcessInput()    // Handle events
    Update(dt)        // Update game state
    Render()          // Draw everything
}
```

## Adding New Features

### Adding a New Resource Type
1. Add to `QuantumResource` enum in `GameState.h`
2. Expand `m_Resources` array size
3. Add UI rendering in `RenderResources()`
4. Create stations that produce it

### Adding a New Research Station
1. Add to `InitializeStations()` in `GameState.cpp`
2. Configure properties:
   - `baseProduction`: How much per second
   - `upgradeCost`: Initial cost
   - `superpositionProbability`: Success rate on observation
   - `unlockCost`: Cost to unlock

### Adding Entanglement System
Currently stubbed out, but the design:
1. Create `EntanglementPair` linking two stations
2. When both stations are active, apply bonus multiplier
3. Cost: Coherence resource
4. UI: Connection visualization between stations

### Adding Upgrades/Perks
1. Create `QuantumUpgrade` struct
2. Store in `m_Timeline.permanentUpgrades`
3. Check unlocked status in update loops
4. Render in prestige UI panel

## Performance Considerations

### Current Performance
- Immediate mode rendering (simple but not optimal)
- ~60 FPS target with VSync
- Minimal draw calls for UI

### Optimization Opportunities
1. **Batch Rendering**: Combine similar draw calls
2. **Texture Atlas**: For UI elements and particles
3. **Object Pooling**: Reuse particle objects
4. **Delta Compression**: For save files
5. **Dirty Flags**: Only update changed UI elements

## Testing Strategy

### Unit Testing
Future: Add Google Test for:
- Resource calculations
- Prestige formulas
- Save/load integrity

### Platform Testing
Test on:
- Windows 10/11 (MSVC, MinGW)
- Ubuntu 20.04+ (GCC, Clang)
- macOS 12+ (Apple Clang)

### Input Testing
- Mouse clicks on all buttons
- Keyboard shortcuts (future)
- Touch on mobile (future)

## Save System

### Format
Simple JSON-like format stored in:
- Windows: `%APPDATA%\QuantumIdle\`
- Linux: `~/.quantumidle/`
- macOS: `~/Library/Application Support/QuantumIdle/`

### Auto-save
- Every 30 seconds
- On application exit (future)

### Cloud Save (Future)
- Sync to cloud service
- Conflict resolution
- Multiple save slots

## Future Platform Support

### Mobile (iOS/Android)
Changes needed:
1. Touch-optimized UI (larger buttons)
2. Portrait orientation support
3. Battery optimization (reduce update rate when backgrounded)
4. Platform-specific builds in CMake

### Web (Emscripten)
Changes needed:
1. OpenGL ES 2.0 compatibility
2. Async file loading
3. LocalStorage for saves
4. Browser-specific input handling

## Contributing

When adding features:
1. Keep engine layer game-agnostic
2. Use consistent code style (see existing files)
3. Add logging for important events
4. Update this documentation
5. Test on at least one platform

## Debug Tips

### Enable Debug Logging
```cpp
Log::SetLevel(Log::Level::Debug);
```

### Common Issues
- **Black screen**: OpenGL context failed, check SDL errors
- **Save not loading**: Check file permissions
- **Input not working**: Verify SDL event processing
- **Particles not showing**: Check particle life > 0

## Performance Profiling

### Built-in Metrics
Access via Application class:
```cpp
app.GetFrameCount()  // Total frames
app.GetDeltaTime()   // Frame time
app.GetTime()        // Total runtime
```

### External Tools
- **Windows**: Visual Studio Profiler
- **Linux**: Valgrind, perf
- **macOS**: Instruments
