# Prestige Tree Features - Implementation Order

## 🎯 Implementation Strategy

**Philosophy:** Build foundation → Add core systems → Layer meta-progression

**Avoid:** Don't add automation before the systems exist to automate!

---

## Phase 1: Foundation (2 hours) ⚡

### 1.1 Number Formatting (30 min)
**Why first:** Everything uses numbers - do this before adding new systems
**What:** Add `FormatNumber()` improvements
- Scientific notation (1.23e8)
- Suffix notation (1.23M, 4.56B, 7.89T)
- Toggle in settings
**Files:** `src/game/GameUtils.cpp`, `include/GameUtils.h`
**Risk:** Low - just changes display

### 1.2 Progress Bars Component (30 min)
**Why now:** Needed for buyables, stations, everything
**What:** Create reusable progress bar renderer
**Files:** New helper in `Renderer.cpp`
**Risk:** Low - new code, doesn't change existing

### 1.3 Station Progress Bars (30 min)
**Why now:** Test the component, immediate visual improvement
**What:** Show "75% to next upgrade" on each station
**Files:** `GameState.cpp` RenderStations()
**Risk:** Low - just adds visual element

### 1.4 Cost Display Improvements (30 min)
**Why now:** Before adding buyables, make costs clearer
**What:** Color-code (green=can afford, yellow=close, red=far)
**Files:** `GameState.cpp` button rendering
**Risk:** Low - just changes colors

---

## Phase 2: Core Buyable System (2.5 hours) 💰

### 2.1 Buyable Upgrade Data Structure (30 min)
**Why first:** Foundation for all buyable content
**What:** Create `BuyableUpgrade` class
```cpp
struct BuyableUpgrade {
    std::string id;
    std::string name;
    std::string description;
    f64 baseCost;
    f64 costMultiplier;  // Cost increases each purchase
    i32 timesPurchased = 0;
    i32 maxPurchases = -1;  // -1 = infinite
    std::function<void()> onPurchase;
};
```
**Files:** New `include/Buyables.h`
**Risk:** Low - new system

### 2.2 Production Multipliers (1 hour)
**Why now:** First actual buyable content
**What:** Add 3 buyable upgrades:
- Quantum Accelerator (2x qubits, costs 5000 qubits)
- Coherence Amplifier (2x coherence, costs 100 coherence)
- Entanglement Booster (2x entanglement, costs 50 entanglement)
**Files:** `GameState.cpp` - add buyables list, update production
**Risk:** Medium - changes production calculations

### 2.3 Buyables UI Panel (1 hour)
**Why now:** Need to display and click buyables
**What:** Add "BUYABLES" button to nav bar, show panel
**Files:** `GameState.cpp` RenderUI(), RenderBuyables()
**Risk:** Low - new panel like Research

---

## Phase 3: QOL Improvements (1.5 hours) ⚙️

### 3.1 Auto-Observe Research (30 min)
**Why now:** Huge QOL win, doesn't break anything
**What:** Add research node "Quantum Automation"
- Cost: 10,000 qubits
- Effect: Auto-observe when superposition > 10
**Files:** `Research.cpp`, `GameState.cpp` UpdateStations()
**Risk:** Low - just automates existing action

### 3.2 Buy Max Upgrade Button (30 min)
**Why now:** Extends existing upgrade system
**What:** Add "BUY MAX" button next to regular upgrade
- Calculates how many you can afford
- Buys them all at once
**Files:** `GameState.cpp` RenderStations(), station upgrade logic
**Risk:** Low - new button, existing logic

### 3.3 Buy Amount Selector (30 min)
**Why now:** Works with Buy Max
**What:** Toggle between Buy 1 / Buy 10 / Buy 100 / Buy Max
**Files:** `GameState.h` add `m_BuyAmount`, `GameState.cpp`
**Risk:** Low - just UI state

---

## Phase 4: Active Gameplay (1 hour) 🚀

### 4.1 Boost Button System (1 hour)
**Why now:** Adds active gameplay without changing core systems
**What:** "Quantum Surge" button
- 2x production for 60 seconds
- 10 minute cooldown
- Visual: pulsing effect on stations
**Files:** `GameState.h` add boost state, `GameState.cpp` update/render
**Risk:** Low - multiplies production temporarily

---

## Phase 5: Challenge System (2 hours) 🎮

### 5.1 Challenge Data Structure (30 min)
**Why now:** Separate system, won't interfere
**What:** Create `Challenge` class
```cpp
struct Challenge {
    std::string id;
    std::string name;
    std::string description;
    bool active = false;
    bool completed = false;
    std::function<void()> apply;    // Modify game state
    std::function<void()> remove;   // Restore normal
    std::function<void()> reward;   // Grant on completion
};
```
**Files:** New `include/Challenges.h`
**Risk:** Low - new system

### 5.2 First Challenge Implementation (1 hour)
**Why now:** Test the challenge system
**What:** "Quantum Instability" challenge
- Effect: Coherence decays 5x faster
- Reward: +50% photons on next prestige
- Can toggle on/off
**Files:** `GameState.cpp`, `Challenges.cpp`
**Risk:** Medium - changes decay rate when active

### 5.3 Challenge UI (30 min)
**Why now:** Need to activate challenges
**What:** Add CHALLENGES button to nav, show panel
**Files:** `GameState.cpp` RenderChallenges()
**Risk:** Low - new panel

---

## Phase 6: Meta-Currency (2.5 hours) 💎

### 6.1 Quantum Essence System (30 min)
**Why now:** All core systems in place
**What:** Add new currency
- Earned from: Challenges (5 essence), achievements (1-3 essence)
- Never lost on prestige
**Files:** `GameState.h` add `m_QuantumEssence`
**Risk:** Low - new currency

### 6.2 Essence Shop Upgrades (1.5 hours)
**Why now:** Something to spend essence on
**What:** Permanent upgrades:
- All Production +10% (repeatable, cost: 10 essence)
- Offline Progress +25% (cost: 50 essence)
- Starting Qubits +1000 (cost: 20 essence)
- Unlock Station 6 (cost: 100 essence)
**Files:** New shop system, permanent bonuses
**Risk:** Medium - affects all production

### 6.3 Essence Shop UI (30 min)
**Why now:** Display and purchase essence upgrades
**What:** Add panel showing permanent upgrades
**Files:** `GameState.cpp` RenderEssenceShop()
**Risk:** Low - new panel

---

## Phase 7: Automation (2 hours) 🤖

### 7.1 Auto-Upgrade Toggle (45 min)
**Why now:** Now that Buy Max exists
**What:** Add checkbox per station "Auto-upgrade"
- Auto-buys when you can afford 10x the cost
- Prevents spending all resources accidentally
**Files:** `ResearchStation` add `autoUpgrade` flag
**Risk:** Low - optional feature

### 7.2 Auto-Research Toggle (45 min)
**Why now:** Research system is stable
**What:** Add checkbox per research node
- Auto-purchases when affordable
**Files:** `Research.cpp` add auto-buy
**Risk:** Low - optional feature

### 7.3 Auto-Prestige (30 min)
**Why now:** Last automation, most powerful
**What:** Research unlock: "Automated Prestige"
- Cost: 1000 photons (expensive!)
- Auto-prestiges at configurable photon amount
**Files:** `GameState.cpp` check prestige threshold
**Risk:** Low - optional endgame feature

---

## Phase 8: Second Prestige Layer (3 hours) 🌌

### 8.1 Singularity Currency (1 hour)
**Why now:** All systems stable, ready for meta-layer
**What:** "Quantum Collapse" - resets photons for singularities
- Formula: singularities = sqrt(photons) / 100
- Requires: 10,000 photons minimum
**Files:** `Timeline.h` add singularities, new prestige function
**Risk:** High - new prestige layer

### 8.2 Singularity Upgrades (1.5 hours)
**Why now:** Need rewards for new layer
**What:** Singularity shop:
- Photon Generation: Gain 1 photon/sec per singularity
- Research Cost -10% (repeatable)
- Challenge Rewards +25%
**Files:** New upgrade system for singularities
**Risk:** Medium - affects economy

### 8.3 Singularity UI (30 min)
**Why now:** Need to display and use new prestige
**What:** Add "COLLAPSE" button (below prestige)
**Files:** `GameState.cpp` RenderStations() add button
**Risk:** Low - new UI element

---

## Phase 9: Content Expansion (flexible timing) 📈

### 9.1 More Buyable Upgrades (per upgrade: 20 min)
**What:** Add variety:
- Observation Bonus +10%
- Starting Coherence +50
- Unlock new stations early
**Files:** `Buyables.cpp`
**Risk:** Low - more of same system

### 9.2 More Challenges (per challenge: 45 min)
**What:** Add variety:
- "No Coherence" - coherence always 0
- "Expensive Upgrades" - all costs 2x
- "Speed Run" - 10 minutes to prestige
**Files:** `Challenges.cpp`
**Risk:** Low - more of same system

### 9.3 More Research Nodes (per node: 15 min)
**What:** Fill out tree with more options
**Files:** `Research.cpp`
**Risk:** Low - more content

---

## 📋 Summary Timeline

| Phase | Feature | Time | Risk | Dependencies |
|-------|---------|------|------|--------------|
| 1 | Foundation | 2h | Low | None |
| 2 | Buyables | 2.5h | Med | Phase 1 |
| 3 | QOL | 1.5h | Low | Phase 2 |
| 4 | Boost | 1h | Low | None |
| 5 | Challenges | 2h | Med | None |
| 6 | Essence | 2.5h | Med | Phase 5 |
| 7 | Automation | 2h | Low | Phases 2-3 |
| 8 | Layer 2 | 3h | High | All above |
| 9 | Expansion | Variable | Low | Respective systems |

**Total Core Implementation: ~16.5 hours**
**Additional Content: Infinite expansion**

---

## 🚀 Suggested Sprint Plan

### Week 1: Foundation + Core Buyables
- Day 1: Phase 1 (Foundation)
- Day 2: Phase 2 (Buyables)
- Day 3: Phase 3 (QOL)

### Week 2: Active Features + Meta
- Day 4: Phase 4 (Boost) + Phase 5 (Challenges)
- Day 5: Phase 6 (Essence)
- Day 6: Phase 7 (Automation)

### Week 3: Meta-Progression
- Day 7: Phase 8 (Second Layer)
- Day 8: Phase 9 (Polish + Content)

---

## ✅ What to Implement RIGHT NOW

**Start with Phase 1.1: Number Formatting**

Why?
- Small, safe change
- Immediate visual improvement
- Won't conflict with anything
- Makes all future numbers look better

Ready to begin? Let's start with better number formatting! 🎯
