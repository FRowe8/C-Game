# 🚀 C-Game (Quantum Idle): Comprehensive Improvement & Refactoring Plan

**Current Status:** v1.0 (Functional MVP)
**Target Status:** v2.0 (Market Leader in Idle/Incremental Genre)

This document outlines the necessary steps to transition the project from a basic C++ clicker into a scalable, high-depth idle RPG comparable to industry leaders like *Idle Skilling* or *Wizard and Minion Idle*.

---

## 🛠 Section 1: Critical Code Refactoring (Technical Debt)

The current codebase suffers from the "God Class" anti-pattern, where `GameState` controls logic, rendering, input, and UI state. This will make adding the planned RPG features extremely difficult.

### 1.1 Decouple UI from Game Logic (MVC Pattern)
**Problem:** `GameState.cpp` contains mixed calls to `ImGui::Begin`, `renderer->Draw`, and game logic updates.
**Solution:**
- **Create a `UIManager` Class:** Move all `Render*` functions from `GameState` to `UIManager`. `GameState` should only hold data; `UIManager` reads that data to draw.
- **Remove `UIButton` struct:** The game is moving toward ImGui. The custom `UIButton` struct in `GameState.h` is redundant and legacy code.
- **Strict Separation:** `GameState` should not include `imgui.h`.

### 1.2 The "GameState" Monolith
**Problem:** `GameState.cpp` is becoming unreadable (1900+ lines). It handles particle physics, save parsing, combat loops, and shop transactions.
**Solution:** Break `GameState` into sub-managers:
- `ResourceManager`: Handles Qubits, Coherence, Photons (Add/Spend logic).
- `TimeManager`: Handles `deltaTime`, offline calculation, and speed multipliers.
- `SaveManager`:Isolate the JSON parsing logic (currently manual string parsing in `GameState::Load`) into a robust serializer.

### 1.3 Fix Incomplete/Placeholder Functions
The following functions in `GameState.cpp` are currently **broken or placeholders**:
- **`RenderBuyables`**: The purchase button logic is commented out: `// m_BuyableManager.Buy(i);`
- **`RenderResearchTree`**: The research logic is commented out: `// m_ResearchTree->TryResearch(node->id, this);`
- **`RenderChallenges`**: Enter/Exit logic is commented out: `// m_ChallengeManager.EnterChallenge(i);`
- **`RenderUI` (Boost)**: The boost logic is commented out: `// TryActivateBoost();`
- **`RenderResearchTree` (Auto)**: Auto-toggle logic is commented out: `// if (mutableNode) mutableNode->autoResearch...`

### 1.4 Rendering Pipeline Conflict
**Problem:** The code mixes `ImGui` (Immediate mode) with `Renderer` (custom SDL wrapper) and raw `ImDrawList`.
**Solution:** Commit fully to ImGui for the interface.
- **Refactor `RenderQuantumAnomalies`:** Currently uses `ImGui::GetBackgroundDrawList()`. This is fine, but should be encapsulated in a `VisualFX` system, not `GameState`.

---

## 🎮 Section 2: Missing Gameplay Features (To Be "The Best")

To compete with *Idle Skilling* or *Hero Idle RPG*, the game needs depth beyond "Number goes up".

### 2.1 Thematic Bridge: "Quantum" vs. "Spaceship"
**Current Issue:** The game feels like two separate games glued together (Quantum Clicker + Spaceship RPG).
**Improvement:** Tie them together via **"Quantum Fuel"**.
- The *Research Stations* (Quantum) should not just generate Qubits; they should power the *Spaceship's* shields and weapons.
- **New Mechanic:** "Energy Grid". You must route Coherence (Quantum) to specific Ship Systems (Shields, Engines, Weapons).
- *Why:* This forces the player to balance the idle layer (Quantum) to succeed in the active layer (Combat).

### 2.2 Deep "Crew" System (Gatcha)
**Current:** Basic Gatcha system mentioned.
**Improvement (Market Standard):**
- **Crew Synergy:** Characters should have tags (e.g., "Scientist", "Pilot", "Marine"). Matching tags grants bonuses (e.g., 3 Scientists = +50% Research Speed).
- **Passive vs. Active:** Crew members should have a "Station Skill" (Passive resource boost) and a "Combat Skill" (Active ability in battle).
- **The "Idle" Factor:** Allow assigning Crew to Research Stations to automate them fully.

### 2.3 True Offline Progression (The "Welcome Back" Screen)
**Current:** Basic calculation in `CalculateOfflineProgress`.
**Improvement:**
- **Simulation, not just Calculation:** The game should simulate "loot drops" for offline combat, not just Qubits.
- **Offline Cap Upgrade:** Players hate short offline caps (currently 4 hours). Add a "Cryo-Stasis" upgrade in the Spaceship that extends offline time to 24h+.

### 2.4 Skill Tree Overhaul (The "Path of Exile" Lite)
**Current:** Basic tree mentioned in `SkillTree.h`.
**Improvement:**
- **Non-Linear Paths:** Don't just have straight lines. Allow "Builds".
    - *Build A:* "Quantum Stability" (Focus on Coherence, passive play).
    - *Build B:* "Matter Conversion" (Focus on turning Qubits into Ship Ammo, active play).
- **Respec Feature:** Essential for RPGs. Allow players to reset skills for a small cost.

---

## 💅 Section 3: UX/UI Polish (Quality of Life)

### 3.1 Navigation Hell
**Current:** A mix of hotkeys (A, S, R) and a "More" hamburger menu.
**Improvement:** **Docked Tab Bar.**
- Create a permanent footer or header with icons: `[Lab] [Ship] [Combat] [Crew] [Upgrades]`.
- Remove floating windows. Windows should be "Scenes" or distinct panels to prevent UI clutter overlapping the game.

### 3.2 Feedback Loops
**Current:** Some particles, simple text.
**Improvement:**
- **Floating Text (Combat):** Critical hits need to shake the screen and pop huge numbers.
- **Loot Toast:** When a Legendary Ship Part drops, play a dedicated sound and show a glowing modal overlay.
- **Resource Ticks:** Instead of just changing the number, show `+140/sec` floating up from the resource bar.

---

## 📝 Implementation Checklist

### Phase 1: Cleanup (Week 1)
- [ ] Uncomment and fix `RenderBuyables` purchase logic.
- [ ] Uncomment and fix `RenderResearchTree` research logic.
- [ ] Move `Render*` functions out of `GameState.cpp` into a temporary `Rendering.cpp` or `UIManager.cpp`.
- [ ] Replace `std::vector<UIButton>` with pure ImGui logic.

### Phase 2: The Bridge (Week 2)
- [ ] Implement "Energy Grid" logic: Connect Qubit generation to Ship stats.
- [ ] Refactor `CombatSystem` to use `ResearchStation` output as a damage modifier.

### Phase 3: The RPG Layer (Week 3-4)
- [ ] Implement `Crew` classes (Stats, Equipping to Stations).
- [ ] Create `Inventory` UI (Drag and drop ship parts).
- [ ] Implement `SaveManager` using a library like `nlohmann/json` instead of manual parsing.

### Phase 4: Polish (Week 5)
- [ ] Add sound engine (SDL_Mixer or Soloud).
- [ ] Add "Tutorial Overlay" (highlighting UI elements for new players).
```***



# 🚀 C-Game (Quantum Idle): Comprehensive Improvement & Refactoring Plan

**Current Status:** v1.0 (Functional MVP)
**Target Status:** v2.0 (The "Deck-Building Idle RPG")

This document outlines the roadmap to refactor the technical debt and transform the gameplay loop into a market-leading experience.

---

## 📉 Section 1: Pacing Analysis & The "Boredom Gap"

Based on `PROGRESSION_FLOW.md` and `GameState.cpp`, the current pacing risks losing players in the first 20 minutes.

### 1.1 The "Passive Start" Problem
**Current Flow:** The game starts with "Idle Resource Generation" and "Research Tree" (Tier 0). Active Combat doesn't unlock until **Level 10**, and Summons (the "fun" part) wait until **Level 15**.
**Critique:**
- **Odd Pacing:** Players who like *TFT* or *Hearthstone* want to collect units or build strategies immediately. Waiting 15 levels just to watch numbers go up is a "bounce" risk.
- **Lack of Agency:** The "Superposition" mechanic (`GameState.cpp`) is currently just a "Wait and Click" button. There is no strategy to it yet.

### 1.2 The "Linear Grind" Trap
**Current Flow:** Unlock Station A -> Wait -> Unlock Station B.
**Critique:** This lacks the "dopamine hit" of *RuneScape*. In *RS*, you don't just wait; you choose to grind *Mining* vs. *Smithing*. Currently, `GameState.cpp` treats all progress as a single "Player Level", missing the opportunity for specialized skill grinding.

---

## 🃏 Section 2: New Features (The "Hearthstone x TFT x RuneScape" Blend)

To create a USP (Unique Selling Point), we will pivot from a generic clicker to a **Strategic Deck-Builder Idle Game**.

### 2.1 Feature: The "Quantum Deck" (Hearthstone Influence)
**Concept:** Replace the generic "Observe" button with a hand of **Quantum Cards**.
- **Mechanic:** Every few seconds, you draw a card (e.g., *Wave Collapse*, *Entanglement*, *Tunneling*).
- **Strategy:**
    - *Wave Collapse:* Collects pending resources instantly.
    - *Superposition:* Doubles pending resources but risks destroying them (Push your luck!).
    - *Mana:* Use "Coherence" as Mana to play cards.
- **Why it hooks:** It turns the "clicker" phase into a mini *Hearthstone* turn. You aren't just clicking; you are looking for combos.

### 2.2 Feature: "Crew Constellations" (TFT Influence)
**Concept:** The `Spaceship` and `GatchaSystem` should use a **Synergy Grid**.
- **Mechanic:** The ship has a 3x3 grid for Crew.
- **Synergies:**
    - *3 Scientists:* +20% Card Draw Speed.
    - *2 Engineers:* +15% Coherence (Mana) Regen.
    - *4 Void-Walkers:* Unlock "Dark Matter" resource.
- **Why it hooks:** Players will obsess over optimizing their grid (like *TFT* positioning) to maximize their idle efficiency.

### 2.3 Feature: "Mastery Skills" (RuneScape Influence)
**Concept:** Split the generic "Player Level" into distinct skills that level up *only* when used.
- **New Skills:**
    - **Observation (Active):** Level up by playing Cards. Unlocks better cards.
    - **Engineering (Passive):** Level up by upgrading Stations. Unlocks grid slots.
    - **Command (Combat):** Level up by winning battles. Unlocks active combat skills.
- **Why it hooks:** "Number go up" feels better when it's a specific skill you worked on.

---

## 🔄 Section 3: The "Addictive Loop" (Blending It All)

This is how we merge the features into a cohesive game loop that solves the pacing issues.

### The "Synergy Cycle"
1.  **Active Phase (Hearthstone):**
    - You play **Quantum Cards** to actively generate massive resources (Qubits).
    - *Reward:* XP for your **Observation Skill**.
2.  **Management Phase (TFT):**
    - Use Qubits to summon Crew.
    - Arrange Crew on your **Ship Grid** to activate Synergies (e.g., "Draw extra cards active play").
    - *Reward:* Higher passive income and better card generation.
3.  **Idle Phase (RuneScape):**
    - While offline, your Crew grinds **Engineering XP** and **Command XP** automatically based on their roles.
    - *Reward:* When you return, you have new Skill Levels that unlock new parts of the ship.

### Revised Progression (Fixing Pacing)
- **Level 1:** Unlock **Quantum Deck** immediately. (Active fun right away).
- **Level 3:** Unlock first **Crew Member**. (Early collection hook).
- **Level 5:** Unlock **Synergy Grid**. (Strategic depth).
- **Level 10:** Unlock **Space Combat** (Test your grid against enemies).

---

## 🛠 Section 4: Technical Refactoring (Making it Work)

### 4.1 Decouple UI from Game Logic (God Class Fix)
**Problem:** `GameState` is doing too much.
**Fix:**
- Create `CardManager`: Handles drawing, discarding, and playing cards.
- Create `GridSystem`: Handles the 3x3 array of Crew and calculating Synergy bonuses.
- **Refactor:** Move `Render*` functions out of `GameState.cpp` into a dedicated `UIManager`.

### 4.2 Fix Incomplete Code
- **`CombatSystem`**: Currently basic. Needs to read stats from the `GridSystem` to determine damage output.
- **`GatchaSystem`**: Needs to generate `Crew` objects with specific "Tags" (Class/Origin) for the synergy system.
- **`SkillTree`**: Refactor from a static tree to a dynamic "XP Gain" system per skill.

### 4.3 Implementation Plan
1.  **Week 1:** Extract `UIManager` and clean `GameState`.
2.  **Week 2:** Implement `CardSystem` (The Hearthstone Layer) to replace simple clicking.
3.  **Week 3:** Implement `CrewSystem` & `SynergyGrid` (The TFT Layer).
4.  **Week 4:** Refactor `Progression` to use the new `Skill` system (The RuneScape Layer).

Here is the content for the new file:

Markdown

# 🎨 UI & Layout Guide: The "Quantum Deck" Update

**Visual Theme:** "Cyber-Arcane"
**Palette:** Deep Void (Background), Neon Cyan (Quantum/Cards), Gold (Legendary/Crew), Danger Red (Combat).
**Core Philosophy:** "Never leave the Game Loop." (Avoid full-screen popups that hide the game).

---

## 🧭 1. Global HUD (Persistent Layout)

To solve the navigation issues identified in `GameState.cpp` (hidden hotkeys, floating menus), we move to a strict **Docked Layout**. This HUD remains visible on 90% of screens.

### 1.1 Top Bar (Resource Monitor)
*Fixed at the top (Height: 60px).*

```text
+---------------------------------------------------------------+
| [Lv 12 | XP: =====-----]  [Credits: 1.2M]  [Photons: 50]      |
|                                                               |
| ⚡ Energy: 100/100 (Mana)   ⚛️ Qubits: 4.5B (+12M/s)          |
+---------------------------------------------------------------+
XP Bar: Always visible. Clicking it opens the Skills modal.

Energy (Mana): Critical for the new Card System. Needs to be prominent (Neon Blue).

Qubits: The primary currency. Floating text (+12M) spawns here when cards are played.

1.2 Bottom Dock (Main Navigation)
Fixed at the bottom (Height: 80px). Replaces the current text buttons.

Plaintext

+---------------------------------------------------------------+
|  [🃏]      [🏭]      [🚀]      [⚔️]      [📦]      [⚙️]     |
|  DECK      LAB      SHIP     COMBAT     INV      MENU     |
+---------------------------------------------------------------+
DECK (Screen 1): The new Active Clicker/Card mechanics.

LAB (Screen 2): Traditional Station upgrades (Idle).

SHIP (Screen 3): The Synergy Grid (TFT mechanic).

COMBAT (Screen 4): Space battles.

INV: Crew/Parts collection.

Notifications: Small red badges on icons (e.g., [SHIP] (1)) when upgrades are available.

🃏 2. Screen 1: The Quantum Lab (Active Play)
Replaces the generic "Observe" button with the Hearthstone-style interaction.

Layout Concept: "The Operator's Console"

Plaintext

+---------------------------------------------------------------+
|  [ VISUALIZER: Waveform collapsing in real-time ]             |
|  (Particles flow from left to right into the resource bar)    |
|                                                               |
|  [ PENDING RESOURCES: 1.5 Billion Qubits ]                    |
|  (This pot grows over time. Play cards to bank it!)           |
|                                                               |
+---------------------------------------------------------------+
|                                                               |
|   +------+   +------+   +------+   +------+   +------+        |
|   | WAVE |   | ENT- |   | SUPER|   | TIME |   |      |        |
|   | COL- |   | ANGLE|   | POS. |   | WARP |   | DRAW |        |
|   | LAPSE|   |      |   |      |   |      |   |      |        |
|   | [2⚡]|   | [5⚡]|   | [3⚡]|   | [8⚡]|   | [1⚡]|        |
|   +------+   +------+   +------+   +------+   +------+        |
|                                                               |
|   HAND (Max 5)                          DECK: 15  DISC: 4     |
+---------------------------------------------------------------+
The "Pot": In the center, a glowing orb grows larger as "Pending Resources" accumulate. It visually pulses.

The Hand: 3-5 Cards displayed at the bottom.

Hover State: Card scales up 1.2x and shows detailed tooltip.

Play State: Drag card to the "Pot" to activate.

Feedback: Playing "Wave Collapse" triggers a screen shake and sucks the Pot into the Top Bar.

🚀 3. Screen 3: The Command Bridge (Synergy Grid)
The TFT-style management layer.

Layout Concept: "The Blueprint"

Plaintext

+-----------------------+---------------------------------------+
|  CREW LIST (Sidebar)  |          SYNERGY GRID (3x3)           |
|                       |                                       |
| [Dr. Quantum] (Sci)   |  +-------+  +-------+  +-------+      |
| Lv 10 - Rare          |  |  [S]  |  |  [E]  |  |  [V]  |      |
|                       |  +-------+  +-------+  +-------+      |
| [Eng. Spark] (Eng)    |                                       |
| Lv 5 - Common         |  +-------+  +-------+  +-------+      |
|                       |  |       |  |  [S]  |  |       |      |
| [Void Walker] (Voi)   |  +-------+  +-------+  +-------+      |
| Lv 2 - Epic           |                                       |
|                       |  +-------+  +-------+  +-------+      |
| [Filter: All]         |  |  [E]  |  |       |  |       |      |
|                       |  +-------+  +-------+  +-------+      |
|                       |                                       |
+-----------------------+---------------------------------------+
| ACTIVE SYNERGIES:                                             |
| (Sci) Science Dept (2): +15% Card Draw Speed                  |
| (Eng) Engineering (2): +10% Energy Regen                      |
+---------------------------------------------------------------+
Drag & Drop: Player drags Crew from Sidebar -> Grid Slots.

Connection Lines: Glowing lines connect adjacent Crew members if they share a Tag (e.g., Two Scientists next to each other glow Blue).

Synergy Panel: Bottom panel updates dynamically as you place units.

⚔️ 4. Screen 4: Space Combat
The "Test of Strength".

Layout Concept: "Side-View Battle"

Plaintext

+---------------------------------------------------------------+
|                  [ TURN 4 | ENEMY TURN ]                      |
+---------------------------------------------------------------+
|                                                               |
|    [YOUR SHIP]                         [ENEMY CRUISER]        |
|    HP: [||||||||--]                    HP: [|||-------]       |
|    Shield: [||----]                    Shield: [----------]   |
|                                                               |
|    [Crew Portraits]                                           |
|    (Show active abilities cooling down)                       |
|                                                               |
|    Effect: "Plasma Burn"               Effect: "Stunned"      |
|                                                               |
+---------------------------------------------------------------+
|  COMBAT LOG:                                                  |
|  > Your "Quantum Beam" hit for 450 Dmg!                       |
|  > Enemy "Void Torpedo" hit your Shield (Absorbed).           |
+---------------------------------------------------------------+
Visuals: Use the existing particle system (SpawnParticleBurst in GameState.cpp) to show laser impacts.

Automation: Combat is auto-battler style, but players can click "Ultimate Abilities" charged by the Crew Grid.

📜 5. Screen 5: Skills (Meta Progression)
Accessed by clicking XP Bar or "Menu".

Layout Concept: "The Constellation"

Instead of a list, use a Node Graph (similar to Path of Exile or Skyrim).

Central Node: "The Core" (Player Level).

Branches:

Left Branch (Observation): Upgrades for the Card Deck (Draw speed, Hand size).

Right Branch (Engineering): Upgrades for the Grid (Unlock slots, Buff synergies).

Top Branch (Command): Upgrades for the Ship (HP, Weapon Dmg).

Interaction: Pan and Zoom. Purchased nodes light up permanently.

💡 Implementation Notes for UIManager.cpp
Z-Ordering:

Layer 0: Background (Parallax stars).

Layer 1: Main Screen Content (Deck/Grid/Combat).

Layer 2: Global HUD (Top/Bottom bars).

Layer 3: Tooltips & Modals.

Layer 4: Particle Effects (Must draw over the HUD for "Juice").

Responsive Design:

The Grid (3x3) and Card Hand need to scale.

Use ImGui::GetContentRegionAvail() to calculate card widths dynamically.

Assets Needed:

Icons for Card Types (Wave, Particle, Time).

Portraits for Crew (Generic "Space" avatars).

The "Card Frame" texture (to make ImGui buttons look like cards).



📦 Production Content Expansion Plan: "Quantum Deck" Update

Status: Design Ready
Target: v2.0 Launch Content
Goal: Provide enough depth for 40+ hours of gameplay.

This document contains the specific data tables, card lists, and unit stats required to transition the game from a prototype to a content-rich RPG.

🃏 Section 1: The Quantum Deck (Card Data)

Mechanic: Players draw from a deck of 15 cards. Energy (Coherence) regenerates at 5/sec.
Design Goal: Turn "clicking" into "resource management combos."

1.1 Starter Deck (The "Observer" Class)

Unlocked at Level 1. Focus: Raw Qubit Generation.

Card Name

Cost

Type

Effect

Flavor Text

Wave Collapse

2 E

Active

Harvests 100% of Pending Qubits immediately.

"Don't blink. You'll miss the payout."

Rapid Pulse

1 E

Active

Harvests 20% of Pending Qubits. Draw 1 Card.

"Quick and dirty."

Superposition

4 E

Buff

Doubles current Pending Qubits. 5% chance to lose all pending.

"Greed is a fundamental force."

Entangle

3 E

Synergy

Next 'Wave Collapse' triggers twice.

"Spooky action at a distance."

Deep Focus

0 E

Utility

Gain +3 Energy. Discard your hand.

"Clear your mind."

1.2 Unlockable Cards (Tier 2 - The "Engineer")

Unlocked via Engineering Skill Tree. Focus: Automation & Buffs.

Card Name

Cost

Type

Effect

Flavor Text

Auto-Tuner

5 E

Construct

Passive: Auto-harvests 10% of Pending every second for 10s.

"Set it and forget it."

Overclock

3 E

Buff

+50% Resource Gen for 15s. -2 HP to Ship Shields/sec.

"Warning: Temperature Critical."

Recycle

1 E

Utility

Shuffle your Discard Pile into your Deck. Gain +1 Energy per card.

"Nothing is wasted."

Shield Capacitor

4 E

Defense

Convert 20% of Pending Qubits into Ship Shield HP.

"Power to forward deflectors!"

🚀 Section 2: Crew Manifest (Synergy Grid)

Mechanic: 3x3 Grid. Units have an Origin (Faction) and a Class (Role).
Design Goal: Creating "TFT-style" vertical and horizontal synergies.

2.1 Origins (Factions)

Origin

2 Unit Bonus

4 Unit Bonus

6 Unit Bonus

Void-Walker

+10% Dark Matter Gen

Void attacks ignore 30% Shield

Unlocks "The Void" Zone

Cybernetic

+150 Shield HP

+500 Shield HP, 1% Regen

Crew immune to Stun

Bio-Luminescent

+1 Energy Regen

+3 Energy Regen

Cards cost -1 Energy

2.2 Classes (Roles)

Class

2 Unit Bonus

4 Unit Bonus

Scientist

+15% Card Draw Speed

Double Cast 10% chance

Engineer

+20% Station Output

Auto-Repair Hull 5/sec

Marine

+20% Weapon Damage

Marines Taunt enemies

2.3 Unit Roster (Launch Set)

Common (Grey)

Lab Assistant (Bio / Scientist): Passive: +2% Qubits.

Hull Welder (Cyber / Engineer): Active: Restores 50 Shield.

Security Bot (Cyber / Marine): Active: Laser Shot (Single Target).

Rare (Blue)

Dr. Quantum (Bio / Scientist): Active: Draws 3 Cards instantly.

Void Stalker (Void / Marine): Passive: Attacks steal life.

Chief Engineer (Cyber / Engineer): Passive: Adjacent units get +20% Efficiency.

Legendary (Gold)

The Oracle (Void / Scientist): Ultimate: "Time Stop" - Freezes enemy for 5s while resources generate 10x faster.

Titan Mech (Cyber / Marine): Ultimate: "Nuke" - Deals 5000 Dmg to all enemies.

📜 Section 3: Skill Constellations (Meta Progression)

Mechanic: Earn XP by performing specific actions. Points are spent in a Node Graph.

3.1 Observation Tree (The "Player" Stats)

XP Source: Playing Cards.

Node 1: Neural Link: +1 Max Energy.

Node 2: Sleight of Hand: Draw Speed +10%.

Node 3 (Keystone): Mulligan: At combat start, you may redraw your hand.

Node 4: Mental Fortress: Max Hand Size increased from 5 to 7.

Node 5 (Ultimate): The Flow: Playing 3 cards in <2 seconds restores 2 Energy.

3.2 Engineering Tree (The "Base" Stats)

XP Source: Station Upgrades & Idle Time.

Node 1: Efficiency Protocols: All Stations +10% Output.

Node 2: Expansion: Unlock 4th Crew Slot (Grid expansion).

Node 3 (Keystone): Automation: "Wave Collapse" card is played automatically every 10s if drawn.

Node 4: Deep Mining: Unlock Tier 2 Resources (Dark Matter).

Node 5 (Ultimate): Dysons Sphere: Pending Resources Cap removed.

3.3 Command Tree (The "Combat" Stats)

XP Source: Winning Battles.

Node 1: Reinforced Hull: +200 Max HP.

Node 2: Ballistics: Weapon Damage +15%.

Node 3 (Keystone): Salvage: Enemies drop 2x Credits.

Node 4: Leadership: All Crew active abilities cool down 10% faster.

Node 5 (Ultimate): Orbital Strike: Unlock "Orbital Cannon" card (Deals massive damage for Energy).

⚔️ Section 4: Campaign Zones (Progression)

Design Goal: Distinct visual themes and mechanical challenges requiring deck adjustments.

Zone 1: The Asteroid Belt

Theme: Dusty rocks, mining drones.

Enemy Gimmick: High Armor (Requires Marine Crew).

Boss: "The Excavator" (Huge HP pool, low damage).

Drop: Common Crew, Basic Minerals.

Zone 2: The Neon Nebula

Theme: Gas clouds, energy beings.

Enemy Gimmick: Energy Drain (Steals your Energy/Mana). Requires "Bio-Luminescent" crew.

Boss: "Flux Entity" (Immune to physical damage, must use Energy weapons).

Drop: Rare Scientists.

Zone 3: The Cyber-Graveyard

Theme: Derelict ships, rogue AI.

Enemy Gimmick: Hacking (Disables your Stations periodically).

Boss: "Mainframe Zero" (Spawns endless minions).

Drop: Cybernetic Crew, Ship Parts.

Zone 4: The Void Horizon

Theme: Black hole edge, reality distortion.

Enemy Gimmick: Time Warp (Your cards cost double energy).

Boss: "The Event" (If you don't kill it in 60s, instant game over).

Drop: Legendary Void Crew.

💾 Section 5: Technical Data Structure

To make this production-ready, avoid hardcoding. Use JSON for all content.

Example: data/cards.json

[
  {
    "id": "card_wave_collapse",
    "name": "Wave Collapse",
    "type": "ACTIVE",
    "cost": 2,
    "rarity": "COMMON",
    "effect": {
      "type": "HARVEST_PENDING",
      "value": 1.0
    },
    "description": "Harvest 100% of Pending Qubits."
  },
  {
    "id": "card_superposition",
    "name": "Superposition",
    "type": "BUFF",
    "cost": 4,
    "rarity": "RARE",
    "effect": {
      "type": "MULTIPLY_PENDING",
      "value": 2.0,
      "risk": 0.05
    },
    "description": "Double Pending Qubits. 5% risk of collapse."
  }
]


Example: data/crew.json

[
  {
    "id": "crew_dr_quantum",
    "name": "Dr. Quantum",
    "rarity": "RARE",
    "tags": ["SCIENTIST", "BIO_LUMINESCENT"],
    "stats": {
      "power": 10,
      "efficiency": 1.5
    },
    "skill": {
      "id": "skill_flash_insight",
      "description": "Draw 3 Cards Instantly",
      "cooldown": 20
    }
  }
]
