# Feature Roadmap

## MVP Features ✅ (v1.0)

### Core Mechanics
- [x] Quantum resource system (Qubits, Coherence, Entanglement)
- [x] Superposition accumulation
- [x] Observation/collapse mechanic
- [x] 5 Research stations with unique properties
- [x] Upgrade system
- [x] Prestige system (Photons)
- [x] Auto-save functionality
- [x] Particle effects

### Technical
- [x] Cross-platform build system (CMake)
- [x] OpenGL 2D renderer
- [x] Input handling (mouse, keyboard, touch-ready)
- [x] Platform abstraction layer
- [x] Save/load system

## Post-MVP Features 🚀

### Phase 1: Content Expansion (v1.1)
**Goal: Keep players engaged for 10+ hours**

- [ ] 10 more research stations
  - Quantum Tunneler (bypasses upgrade costs occasionally)
  - Wave Function Manipulator (changes probabilities)
  - Schrödinger's Lab (produces random resources)
  - Quantum Eraser (resets station costs for Coherence)

- [ ] Entanglement system
  - Link 2+ stations for multiplicative bonuses
  - Cost: Entanglement resource
  - Visual connections between entangled stations
  - Break entanglement to reallocate

- [ ] Coherence mechanics expansion
  - Coherence affects observation success rate
  - Decoherence events (random negative events)
  - Coherence restoration upgrades
  - Time-based decay (offline penalty mitigation)

- [ ] 15 Prestige upgrades
  - Persistent production bonuses
  - Auto-observation unlocks
  - Probability improvements
  - Starting resource boosts

### Phase 2: Meta Progression (v1.2)
**Goal: Long-term engagement, replay value**

- [ ] Achievement system
  - 50+ achievements
  - Unlock bonuses for milestones
  - Hidden achievements for exploration

- [ ] Quantum particles discovery
  - Unlock new particle types through research
  - Each particle has unique mechanics
  - Photons, Electrons, Neutrinos, Quarks, Bosons
  - Collect full sets for bonuses

- [ ] Research tree
  - Branching upgrade paths
  - Permanent unlocks
  - Trade-offs and specialization
  - Respec option for Photons

- [ ] Statistics page
  - Total resources generated
  - Time played
  - Observations made
  - Prestiges completed
  - Graphs and charts

### Phase 3: Automation & QoL (v1.3)
**Goal: Reduce tedium, improve idle aspects**

- [ ] Auto-observers
  - Unlock with Photons
  - Automatically observe at thresholds
  - Configurable per station

- [ ] Bulk upgrades
  - Buy max button
  - Buy 10x button
  - Keybinds for quick purchases

- [ ] Offline progress
  - Calculate production while away
  - Diminishing returns after 24 hours
  - Offline Coherence protection upgrades

- [ ] Notification system
  - Alert when prestigeable
  - Unlock notifications
  - Configurable toast messages

- [ ] Settings menu
  - Volume controls (when audio added)
  - Graphics quality
  - Auto-save interval
  - Keybind customization

### Phase 4: Unique Mechanics (v2.0)
**Goal: Stand out from other idle games**

- [ ] Quantum uncertainty events
  - Random events with risk/reward
  - "Observe or wait?" decision points
  - Uncertainty increases with time
  - Higher risk = higher reward

- [ ] Wave function manipulation
  - Mini-game to improve observation outcomes
  - Timing-based or pattern-matching
  - Optional (can auto-resolve for true idle)

- [ ] Parallel universes
  - Advanced prestige layer
  - Each universe has different rules
  - Transfer resources between universes
  - Universe-specific upgrades

- [ ] Quantum entanglement puzzles
  - Arrange stations in specific patterns
  - Unlock massive bonuses
  - Daily challenges
  - Community-created patterns

- [ ] Temporal mechanics
  - Slow down/speed up time for resources
  - "Rewind" to retry observations
  - Time dilation upgrades
  - Causality paradoxes (beneficial bugs)

### Phase 5: Social Features (v2.1)
**Goal: Community engagement**

- [ ] Leaderboards
  - Fastest prestige
  - Highest photon count
  - Total resources generated
  - Weekly/monthly/all-time

- [ ] Daily challenges
  - Special constraints
  - Bonus rewards
  - Competitive leaderboards

- [ ] Sharing
  - Export builds/strategies
  - Share observation results
  - Community station setups

- [ ] Cloud saves
  - Sync across devices
  - Multiple save slots
  - Import/export functionality

### Phase 6: Polish & Expansion (v2.2)
**Goal: Premium experience**

- [ ] Enhanced graphics
  - Texture atlas for UI
  - Shader effects for quantum states
  - Animated backgrounds
  - Screen shake on big observations

- [ ] Audio system
  - Background music (quantum/ambient)
  - SFX for observations, upgrades
  - Configurable volume
  - Audio feedback for all interactions

- [ ] Better text rendering
  - TrueType font support (FreeType)
  - Multiple font sizes
  - Text shadows and outlines
  - Localization support

- [ ] Animations
  - Smooth UI transitions
  - Number counter animations
  - Station pulse effects
  - Entanglement line animations

- [ ] Tutorial system
  - First-time user experience
  - Tooltips and hints
  - Contextual help
  - Can be disabled

## Platform Expansion

### Mobile Ports (v3.0)

#### iOS
- [ ] Touch-optimized UI
- [ ] Portrait and landscape modes
- [ ] iOS-specific controls
- [ ] iCloud save sync
- [ ] App Store integration

#### Android
- [ ] Touch-optimized UI
- [ ] Material Design guidelines
- [ ] Google Play Services
- [ ] Multiple screen size support
- [ ] Adaptive icons

### Web Version (v3.1)
- [ ] Emscripten build
- [ ] WebGL rendering
- [ ] LocalStorage saves
- [ ] Responsive design
- [ ] PWA support (play offline)

## Monetization (Optional)

### Ethical F2P Model
- [ ] Completely free core game
- [ ] Optional cosmetics
  - UI themes (Matrix, Neon, Retro)
  - Particle effects
  - Station skins
- [ ] Ad removal option
- [ ] Tip jar / support developer
- [ ] **NO PAY-TO-WIN**
- [ ] **NO ENERGY SYSTEMS**
- [ ] **NO FORCED ADS**

## Technical Debt & Refactoring

### High Priority
- [ ] Replace immediate mode rendering with batching
- [ ] Proper JSON library (nlohmann/json)
- [ ] Unit tests for game logic
- [ ] Automated CI/CD pipeline
- [ ] Memory leak detection

### Medium Priority
- [ ] Entity-Component-System refactor
- [ ] Shader abstraction
- [ ] Asset management system
- [ ] Input system improvements (keybinds)
- [ ] Better error handling

### Low Priority
- [ ] Custom allocators
- [ ] Multi-threading for particle updates
- [ ] Compression for save files
- [ ] Network multiplayer infrastructure

## Community Requests
*(To be filled based on player feedback)*

- [ ] TBD
- [ ] TBD
- [ ] TBD

## Version History

- **v1.0** (Current): MVP release with core mechanics
- **v1.1** (Planned): Content expansion
- **v1.2** (Planned): Meta progression
- **v2.0** (Planned): Unique mechanics
- **v3.0** (Future): Mobile ports

## Success Metrics

### Engagement
- Average session length: 15+ minutes
- Daily active users returning
- Prestige completion rate: 60%+

### Technical
- 60 FPS on target platforms
- < 100MB memory usage
- < 1 second load time
- < 5MB save file size

### Monetization (if implemented)
- Conversion rate: 2-5%
- Positive reviews: 4.5+ stars
- No pay-to-win complaints
