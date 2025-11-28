# Quantum Idle - Current Features & Improvement Plan

## 🎮 Current Game Features

### Core Mechanics
1. **Superposition System** (The confusing part!)
   - Stations generate resources passively into "superposition"
   - Resources accumulate over time in this quantum state
   - Click **OBSERVE** button to "collapse" and collect resources
   - **Problem**: Not explained anywhere in the game!

2. **Research Stations** (5 total)
   - Qubit Generator (starts at level 1 - generates passively)
   - Coherence Stabilizer (locked)
   - Entanglement Chamber (locked)
   - Qubit Synthesizer (locked)
   - Quantum Supercomputer (locked)

3. **Resources** (3 types)
   - **Qubits**: Primary currency
   - **Coherence**: Stability resource (slowly decays)
   - **Entanglement**: Strategic resource
   - **Photons**: Prestige currency

4. **Progression Systems**
   - Unlock new stations with Qubits
   - Upgrade stations to increase production
   - Prestige system for permanent bonuses

### Advanced Features (That most players won't see)
5. **Research Tree** (Press R)
   - Unlock production multipliers
   - Observation bonuses
   - Coherence improvements

6. **Achievements** (Press A)
   - 14 achievements with rewards
   - Progress tracking
   - Qubit and Photon rewards

7. **Milestones** (Press M)
   - Long-term progression goals
   - Production bonuses
   - Track completion percentage

8. **Statistics** (Press S)
   - Track total resources earned
   - Session tracking
   - Personal records

9. **Quantum Events**
   - Random beneficial events (every 2 minutes)
   - Wave Collapse, Time Dilation, Lucky Observation, etc.
   - Temporary bonuses

10. **Offline Progress**
    - Earn resources while away (50% efficiency)
    - Max 4 hours stored
    - Auto-calculated on return

11. **Persistent Saves**
    - Auto-save every 30 seconds
    - IndexedDB browser storage
    - Survives page refresh

---

## 🚨 Current Problems

### Critical Issues
1. **No Tutorial/Help**
   - Players don't understand superposition
   - Hidden keyboard shortcuts (A/S/R/M)
   - No explanation of game mechanics

2. **Scrolling Not Working Properly**
   - Mouse wheel may not be captured
   - No visual scroll indicator
   - Users don't know content is off-screen

3. **Boring Visual Design**
   - Generic colored boxes
   - No animations (besides particles)
   - No visual feedback for progression
   - Text is functional but not appealing

4. **Unclear Game Loop**
   - What should I do next?
   - Why observe instead of auto-collect?
   - When should I prestige?

5. **Hidden Features**
   - Research tree (R key) not discoverable
   - Achievements hidden
   - No indication these exist

---

## 💡 Improvement Suggestions

### Priority 1: Make it Playable
- [ ] Add visible UI buttons for Achievements, Research, Stats, Milestones
- [ ] Add Help/Tutorial overlay (H key or ? button)
- [ ] Add visual scroll indicator showing more content below
- [ ] Add tooltips explaining buttons and mechanics
- [ ] Consider auto-collecting instead of OBSERVE clicks (or make it optional)

### Priority 2: Make it Look Good
- [ ] Add modern gradient backgrounds
- [ ] Animated resource counters (numbers roll up)
- [ ] Progress bars for station production
- [ ] Visual "quantum glow" effects
- [ ] Station unlock animations
- [ ] Better color scheme (cyberpunk quantum theme?)

### Priority 3: Improve Game Feel
- [ ] Add sound effects (click, collect, upgrade, prestige)
- [ ] Floating damage numbers when observing
- [ ] Screen shake on major events
- [ ] Particle burst improvements
- [ ] Smooth transitions between screens

### Priority 4: Game Balance
- [ ] Faster early game (more starting qubits?)
- [ ] Better progression curve
- [ ] More interesting station abilities
- [ ] Clearer prestige benefits

---

## 🎯 Recommended Next Steps

**Option A: Quick Fixes** (30 minutes)
1. Add visible UI buttons at top (Replace keyboard shortcuts)
2. Add Help overlay with controls
3. Add scroll indicator arrow
4. Auto-observe option (checkbox to auto-collect)

**Option B: Visual Overhaul** (2-3 hours)
1. Redesign UI with modern theme
2. Add animations and transitions
3. Improve color palette
4. Add visual effects

**Option C: Tutorial First** (1 hour)
1. Create step-by-step tutorial
2. Highlight features as you unlock them
3. Explain mechanics inline
4. Add tooltips everywhere

---

## ❓ Questions for You

1. **What's your vision for the game's visual style?**
   - Minimalist and clean?
   - Sci-fi cyberpunk quantum theme?
   - Colorful and playful?
   - Dark mode with glowing effects?

2. **Should superposition be automatic or manual?**
   - Keep OBSERVE button (more engaging but clicky)?
   - Auto-collect (more idle, less active)?
   - Option to toggle between modes?

3. **What features should be prioritized?**
   - Making it look better?
   - Making it easier to understand?
   - Adding more content/features?
   - Balancing existing systems?

4. **Target platform priority?**
   - Desktop (mouse + keyboard)?
   - Mobile (touch)?
   - Both equally?

Let me know which direction you'd like to go, and I'll implement it!
