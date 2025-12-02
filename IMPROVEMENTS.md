# Quantum Idle: AI Implementation & Remediation Roadmap

**Objective:** Transform the "Quantum Idle" prototype into a polished, production-ready incremental game by resolving technical debt, implementing missing features, and refining the user experience.

**Context:** The project currently suffers from monolithic code architecture (`GameState.cpp`), a lack of onboarding, and disjointed gameplay systems. This roadmap prioritizes architectural health and user retention.

---

## 📅 Phase 1: Architecture Refactoring (Critical Technical Debt)
**Goal:** Decouple rendering from game logic to improve maintainability and enable easier UI iterations.

### 1.1. Decouple UI from GameState
* **Analysis**: `src/game/GameState.cpp` currently mixes logic updates with direct `ImGui::` rendering calls (e.g., `RenderStationsContent`, `RenderUI`).
* **Action Plan**:
    1.  Create a new module: `src/game/ui/GuiLayer.cpp` (and `.h`).
    2.  Define a `View` interface or specific view classes: `StationView`, `ResearchView`, `CombatView`.
    3.  **Refactor**: Move the body of `GameState::RenderStationsContent` to `StationView::Render(const std::vector<ResearchStation>& stations)`.
    4.  **Refactor**: Move the body of `GameState::RenderUI` (navigation bar) to `GuiLayer::RenderNavigation()`.
    5.  **Update GameState**: `GameState::Render()` should only call `GuiLayer::Render(this)`. `GameState` should pass *data*, not draw pixels.

### 1.2. Implement Game State Machine
* **Analysis**: `GameState` uses multiple booleans (`m_ShowCombat`, `m_ShowResearch`) to manage screens, leading to potential state conflicts.
* **Action Plan**:
    1.  Define a strict enum in `GameState.h`:
        ```cpp
        enum class GameMode {
            Idle,
            Combat,
            ResearchTree,
            Spaceship,
            Menu
        };
        ```
    2.  Replace boolean flags with a single `m_CurrentState` variable.
    3.  Update input processing in `UpdateUI` to respect `m_CurrentState` (e.g., ignore station clicks when in `Combat` mode).

### 1.3. Centralize Input Handling
* **Analysis**: Input is currently checked in `UpdateUI`, `CombatSystem::Update`, and potentially other places, causing click-through issues.
* **Action Plan**:
    1.  Enhance `src/engine/Input.h` to support "consuming" inputs.
    2.  In the main loop, UI (ImGui) should check input first. If `ImGui::GetIO().WantCaptureMouse` is true, do *not* pass click events to the game world (e.g., Quantum Anomalies).

---

## 🛒 Phase 2: User Experience (UX) & Onboarding
**Goal:** Drastically reduce player confusion within the first 15 minutes of gameplay.

### 2.1. Implement Tutorial Overlay System
* **Analysis**: Players do not understand the "Superposition/Observation" mechanic.
* **Action Plan**:
    1.  Create `src/game/ui/TutorialOverlay.cpp`.
    2.  Implement a "Step" system:
        * **Step 1**: Mask everything except the "Observe" button. Show tooltip: "Click to collapse wave function."
        * **Step 2**: Trigger when Resources > 10. Point to "Upgrade".
        * **Step 3**: Trigger when Coherence drops < 50%. Point to "Coherence Stabilizer".
    3.  Save tutorial progress in `SaveManager` so it doesn't repeat on reload.

### 2.2. Visual Feedback (The "Juice")
* **Analysis**: Resources change silently; lacking satisfaction.
* **Action Plan**:
    1.  **Floating Text**: Create a `FloatingTextManager`. When `AddResource()` is called in `GameState.cpp`, spawn a text object at the mouse position moving upwards (e.g., "+10 Qubits" in Blue).
    2.  **Button Feedback**: Modify `ImGuiUtils::Button` wrappers to include a "pulse" animation on hover and a "shrink" effect on click.
    3.  **Particle Integration**: Ensure `SpawnParticleBurst` is called on *every* successful manual observation and upgrade.

### 2.3. Tooltip & Formatting Standardization
* **Analysis**: Inconsistent number formats (`100000` vs `100k`) and unexplained icons.
* **Action Plan**:
    1.  **Refactor**: Search for all raw `std::to_string` calls on resource values. Replace them with `GameUtils::FormatNumber()`.
    2.  **Tooltips**: Add `ImGui::SetTooltip` to:
        * Resource Icons (Top Bar): Explain what the resource is used for.
        * Stats (Damage/Defense): Explain how they are calculated.
        * "Observe" Button: Explain the RNG chance (e.g., "70% chance to collapse").

---

## ⚖️ Phase 3: Game Design & Pacing
**Goal:** Fix the early-game wall and integrate the disjointed Combat system.

### 3.1. Early Game Smoothing
* **Analysis**: Zero passive income at start leads to frustration.
* **Action Plan**:
    1.  **Modify `ResearchStation`**: Give the "Qubit Generator" (Level 1) a base `passiveCollapseRate` of 0.05 (5% per second).
    2.  **UI Update**: Show this passive rate in the station UI bar so players see the bar moving even without clicking.

### 3.2. Combat & Economy Integration
* **Analysis**: Combat rewards (Credits) have limited use.
* **Action Plan**:
    1.  **New Building**: Add "Matter Converter" station (unlocks at Tier 2).
    2.  **Mechanic**: Allow converting Combat `Credits` -> `Global Production Multiplier` (temporary or permanent).
    3.  **Loot**: Add "Research Data" as a drop from enemies, which provides a boost to Research Point accumulation.

### 3.3. Mid-Game Gatekeeping (Spaceship)
* **Analysis**: Spaceship is currently optional.
* **Action Plan**:
    1.  **Lock Tier 3 Research**: Require specific "Exotic Materials" to unlock Tier 3 nodes.
    2.  **Source**: These materials should *only* be obtainable via Spaceship Expeditions (idle timers) or Combat Drops in specific high-level zones.

---

## 🚧 Phase 4: Missing Feature Implementation
**Goal:** Implement features referenced in code/docs but missing from the game loop.

### 4.1. Particle Collection System
* **Analysis**: Referenced in `ROADMAP.md` but logic is missing.
* **Action Plan**:
    1.  Create `src/game/ParticleSystem.cpp`.
    2.  **Data Structure**: Define 28 particles with `Rarity` (Common to Mythical) and `StatBonus` (e.g., +5% Qubit Gain).
    3.  **Trigger**: On `Observe()`, add a small RNG check (`0.5%`) to "Discover" a particle.
    4.  **UI**: Create a `Collection` view (grid of 28 slots) to view found particles and their lore.

### 4.2. Audio System Expansion
* **Analysis**: `SoundManager` exists but is underutilized.
* **Action Plan**:
    1.  **SFX**: Add sounds for: `UI_Hover`, `UI_Click`, `Error` (not enough resources), `Unlock_Achievement`.
    2.  **Music**: Implement simple cross-fading background ambient loops that change based on the current `GameMode` (Combat music vs. Idle music).

---

## 🔒 Phase 5: Production Polish & Security
**Goal:** Prepare for release (Web/Desktop).

### 5.1. Save System Security
* **Analysis**: `quantum_save.json` is plain text and easily cheatable.
* **Action Plan**:
    1.  **Hashing**: In `SaveManager::Save`, calculate a SHA-256 (or simple hash) of the JSON string + a salt (secret key). Append this hash to the file.
    2.  **Validation**: In `SaveManager::Load`, recalculate the hash of the content. If it doesn't match the stored hash, flag as corrupted/cheated.

### 5.2. Narrative "Flavor"
* **Analysis**: The game is dry.
* **Action Plan**:
    1.  **Upgrade Descriptions**: Rewrite generic text.
        * *Old*: "Increases production by 2x."
        * *New*: "Calibrate the tachyon emitters. It hums aggressively. (Production x2)"
    2.  **Enemy Names**: Rename generic enemies to thematic ones: "Entropy Ghost", "Decoherence Field", "Maxwell's Demon".

### 5.3. Performance Optimization
* **Analysis**: High particle counts may lag on web builds.
* **Action Plan**:
    1.  **Pooling**: Implement an object pool for Particles instead of `new`/`delete` or `vector::push_back` every frame.
    2.  **Culling**: Do not render particles or anomalies that are outside the current viewport bounds.

---

## 📝 Execution Order Summary

1.  **Sprint 1 (Foundations)**: Refactor UI architecture (1.1) and Input (1.3).
2.  **Sprint 2 (UX Fixes)**: Tutorial (2.1), Tooltips (2.3), and Floating Text (2.2).
3.  **Sprint 3 (Game Loop)**: Passive Income (3.1), Particle Collection (4.1), and Economy Integration (3.2).
4.  **Sprint 4 (Polish)**: Audio (4.2), Narrative (5.2), and Save Security (5.1).