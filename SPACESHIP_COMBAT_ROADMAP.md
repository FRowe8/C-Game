# Spaceship Combat & Gatcha System - Implementation Roadmap

## Overview
Transform the existing spaceship repair system into a full-featured combat RPG with gatcha mechanics, inspired by Idle Heroes and Idle Skilling. This document outlines the complete implementation plan.

## Current State (✅ Completed - Phase 1)
- ✅ Ship part system with 5 rarity tiers (Common → Legendary)
- ✅ 4 equipment slots (Hull, Engine, Weapons, Shields)
- ✅ Basic inventory management
- ✅ Stat bonuses (Power, Combat, Drop Rate)
- ✅ Repair progression (0-100%)
- ✅ Part generation with weighted drop rates

---

## Phase 2: Core Combat System
**Goal:** Implement turn-based or auto-combat battles with enemies

### 2.1 Enemy System
- [ ] Create `Enemy` class with:
  - Name, level, health, damage, defense
  - Enemy types: Scout, Fighter, Cruiser, Battleship, Boss
  - Loot tables for each enemy type
  - Visual representation (sprite/icon)

- [ ] Create `EnemyDatabase` for storing enemy templates
  - Different enemies for different zones/areas
  - Scaling difficulty based on player progression
  - Special abilities for boss enemies

### 2.2 Combat Mechanics
- [ ] Create `CombatSystem` class:
  - Turn-based or auto-battle system
  - Damage calculation formula: `base_damage * (1 + combat_bonus%) - enemy_defense`
  - Critical hit system (5-20% chance based on gear)
  - Status effects: Stun, Burn, Shield, Heal-over-time

- [ ] Combat stats derived from ship:
  - **Attack:** Based on Weapons + Combat Bonus
  - **Defense:** Based on Hull + Shield parts
  - **Speed:** Based on Engine (determines turn order)
  - **HP:** Base HP + Hull bonus

- [ ] Victory/Defeat conditions:
  - Victory: Earn credits, XP, parts, and materials
  - Defeat: Lose some credits but keep XP (no death penalty)

### 2.3 Combat UI
- [ ] Battle screen layout:
  - Player ship on left, enemy on right
  - Health bars with numbers
  - Damage numbers floating up on hit
  - Combat log showing actions
  - Auto-battle toggle button
  - Speed control (1x, 2x, 4x)

---

## Phase 3: Gatcha/Summoning System
**Goal:** Add exciting randomized rewards for part acquisition

### 3.1 Summon Mechanics
- [ ] Create `GatchaSystem` class:
  - **Basic Summon:** Costs credits, mostly Common/Uncommon parts
  - **Advanced Summon:** Costs premium currency, better rates
  - **Elite Summon:** Guaranteed Rare+ parts

- [ ] Summon currencies:
  - **Credits:** Earned from combat and research
  - **Stellar Shards:** Premium currency from achievements/milestones
  - **Summon Tickets:** Special items from events/raids

- [ ] Pity system:
  - Guaranteed Rare part every 10 basic summons
  - Guaranteed Epic part every 30 advanced summons
  - Guaranteed Legendary every 100 advanced summons
  - Counter persists across sessions

### 3.2 Summon UI & Animations
- [ ] Summon screen with:
  - "Pull x1" and "Pull x10" buttons (10x gives bonus)
  - Display current pity counters
  - Show drop rates (transparency)
  - Summon history log

- [ ] Celebration animations:
  - Color flash for rarity reveal
  - Screen shake for Epic+
  - Special effects for Legendary pulls
  - Skip animation option for bulk pulls

### 3.3 New Drop Sources
- [ ] Ship parts can now drop from:
  - Research stations (existing)
  - Combat victories (common)
  - Boss kills (guaranteed rare+)
  - Raid completions (best source)
  - Gatcha summons (most exciting)

---

## Phase 4: Hero/Captain System (Optional Enhancement)
**Goal:** Add collectible characters that pilot ships

### 4.1 Hero Classes
- [ ] Create `Hero` class with:
  - Name, portrait, rarity
  - Class types: Engineer, Warrior, Tactician, Explorer
  - Unique passive abilities
  - Level system (1-100)

- [ ] Hero abilities:
  - **Engineer:** +15% repair speed, +10% production
  - **Warrior:** +25% combat damage, +HP
  - **Tactician:** +15% drop rates, better critical chance
  - **Explorer:** +20% research speed, rare part discovery

### 4.2 Hero Management
- [ ] Hero roster/collection screen
- [ ] Assign hero to ship (active hero)
- [ ] Hero leveling with XP from combat
- [ ] Hero star-ranking system (1-6 stars)
- [ ] Duplicate heroes = upgrade materials

---

## Phase 5: Skill Tree System
**Goal:** Deep progression with unlockable abilities

### 5.1 Skill Categories
- [ ] Create skill tree with branches:

  **Combat Branch:**
  - Multi-target attacks
  - Counter-attack chance
  - Life steal
  - Execution (bonus damage to low HP enemies)

  **Engineering Branch:**
  - Auto-repair during combat
  - Overcharge weapons (+50% damage, costs HP)
  - Emergency shields
  - Part efficiency (+stat bonuses from equipped parts)

  **Exploration Branch:**
  - Advanced scanning (reveal enemy stats)
  - Treasure hunter (+loot quality)
  - Fast travel
  - Safe passage (avoid some battles)

### 5.2 Skill Acquisition
- [ ] Earn skill points from:
  - Leveling up (1 point per level)
  - Milestone achievements (5 points)
  - Raid first-clear rewards

- [ ] Skill requirements:
  - Level requirements
  - Prerequisite skills
  - Credit/material costs to unlock

### 5.3 Skill UI
- [ ] Interactive skill tree visualization
- [ ] Hover tooltips for skill descriptions
- [ ] Show locked/unlocked/available states
- [ ] Reset option (costs premium currency)

---

## Phase 6: Gear Enhancement & Upgrade System
**Goal:** Make invested parts more powerful over time

### 6.1 Enhancement Mechanics
- [ ] Parts can be enhanced to +1 through +15:
  - Each level adds 10% to base stats
  - Enhancement costs credits + upgrade materials
  - Failure chance increases after +10 (item doesn't break, just fails)
  - Success rate boosters available

- [ ] Enhancement materials:
  - **Tech Scraps:** Common material (from dismantling parts)
  - **Nano-Alloy:** Rare material (from elite enemies)
  - **Quantum Core:** Epic material (from raids)

### 6.2 Star Ascension
- [ ] Promote parts to higher star levels (1-6 stars):
  - Requires duplicate parts OR universal shards
  - Each star adds significant stat boost
  - Changes visual appearance (glow effect)
  - Max level increases with stars (+5 levels per star)

### 6.3 Part Awakening (Legendary only)
- [ ] Legendary parts can be "awakened":
  - Unlocks special bonus effect
  - Examples: "Regenerate 1% HP per turn", "20% dodge chance"
  - Requires rare awakening stones
  - One awakening per part

### 6.4 Enhancement UI
- [ ] Part details screen showing:
  - Current enhancement level and stats
  - Enhancement success rate
  - Materials required
  - Preview of next level bonuses
  - Dismantle option (convert to materials)

---

## Phase 7: Zone/Area System
**Goal:** Multiple locations with different themes and challenges

### 7.1 Area Progression
- [ ] Create explorable zones:

  **Zone 1: Asteroid Belt** (Levels 1-20)
  - Enemy types: Mining drones, pirates
  - Drops: Basic parts, common materials

  **Zone 2: Nebula Frontier** (Levels 20-40)
  - Enemy types: Alien scouts, cosmic creatures
  - Drops: Uncommon/Rare parts, better materials

  **Zone 3: Derelict Fleet** (Levels 40-60)
  - Enemy types: Corrupted AI ships, salvage mechs
  - Drops: Rare/Epic parts, tech upgrades

  **Zone 4: Black Hole Horizon** (Levels 60-80)
  - Enemy types: Void entities, temporal anomalies
  - Drops: Epic/Legendary parts, awakening stones

  **Zone 5: Ancient Armada** (Levels 80-100)
  - Enemy types: Legendary warships, final bosses
  - Drops: Best loot in game

### 7.2 Area Features
- [ ] Each zone has:
  - Unique background/theme
  - 10-15 combat stages
  - 3 boss stages
  - First-clear rewards
  - Repeatable farming

### 7.3 Area Unlocking
- [ ] Unlock requirements:
  - Complete previous zone's boss
  - Reach minimum level
  - Pay unlock fee (credits/materials)

---

## Phase 8: Raid/Dungeon System
**Goal:** Challenging endgame content with best rewards

### 8.1 Daily Raids
- [ ] Create raid instances:

  **Tech Raid** (Monday/Thursday)
  - Focus: Engineering challenges
  - Rewards: Enhancement materials, credits

  **Combat Raid** (Tuesday/Friday)
  - Focus: Difficult battles
  - Rewards: Rare parts, combat materials

  **Resource Raid** (Wednesday/Saturday)
  - Focus: Loot collection
  - Rewards: Credits, summon tickets, shards

  **Legendary Raid** (Sunday)
  - Focus: Ultimate challenge
  - Rewards: Legendary parts, premium currency

### 8.2 Raid Mechanics
- [ ] Energy system for raids:
  - 100 max energy, regenerate 1 per 5 minutes
  - Each raid attempt costs 20 energy
  - Can store overflow energy (max 200)

- [ ] Raid difficulty tiers:
  - Normal (Level 20+)
  - Hard (Level 50+)
  - Expert (Level 80+)
  - Mythic (Level 100+, best rewards)

### 8.3 Raid Rewards
- [ ] Completion rewards scale with:
  - Difficulty tier
  - Clear time (speed bonuses)
  - No-damage clear bonuses

- [ ] First clear rewards per difficulty:
  - Premium currency
  - Guaranteed rare+ part
  - Unlock achievement

---

## Phase 9: Leveling & Experience System
**Goal:** Meaningful character progression

### 9.1 Level System
- [ ] Player level system:
  - Max level: 100
  - XP gained from: Combat, research, achievements
  - Each level grants:
    - +1 skill point
    - +5% base stats
    - +10 max energy
    - Credits reward

### 9.2 XP Sources
- [ ] XP multipliers:
  - Research observations: 1x XP
  - Combat victories: 2x XP
  - Boss kills: 5x XP
  - Raid completions: 10x XP
  - First-time clears: 20x XP

- [ ] XP boosters:
  - Time-limited buffs (+50% XP for 1 hour)
  - Purchasable with premium currency
  - Earned from achievements

### 9.3 Level Milestones
- [ ] Special rewards at key levels:
  - Level 10: Unlock skills
  - Level 20: Unlock second zone
  - Level 30: Unlock enhancement to +10
  - Level 50: Unlock hero system
  - Level 75: Unlock legendary raids
  - Level 100: Prestige option

---

## Phase 10: Progression & Prestige Integration
**Goal:** Connect combat system to existing prestige mechanics

### 10.1 Prestige Benefits
- [ ] Combat prestige bonuses:
  - Essence bonuses grant permanent combat stats
  - Singularity upgrades affect combat power
  - Research bonuses apply to combat

- [ ] New prestige upgrade: **Combat Mastery**
  - Costs: Essence/Singularity points
  - Unlocks: Permanent +% combat damage
  - Synergy with ship bonuses

### 10.2 Combat Ascension (3rd Prestige Layer)
- [ ] When player reaches Level 100 + full legendary gear:
  - Option to "Ascend" (new prestige layer)
  - Reset: Level, parts, zone progress
  - Keep: Skills, heroes, achievements, currencies
  - Gain: **Ascension Points** for massive permanent bonuses

- [ ] Ascension benefits:
  - +50% XP gain per ascension
  - +25% drop rates
  - Higher legendary drop chance
  - Unlock "Mythic" rarity parts

---

## Phase 11: UI/UX & Polish
**Goal:** Make the combat system feel amazing

### 11.1 Main Navigation
- [ ] Add new tabs to main menu:
  - **Combat:** Enter battle mode
  - **Summon:** Gatcha screen
  - **Heroes:** Hero collection (if implemented)
  - **Skills:** Skill tree
  - **Zones:** Area selection map
  - **Raids:** Daily raid menu

### 11.2 Visual Polish
- [ ] Combat animations:
  - Ship firing weapons
  - Impact effects
  - Shield absorb effects
  - Death/explosion animations

- [ ] Particle effects:
  - Legendary summon sparkles
  - Level up celebration
  - Critical hit flashes
  - Loot dropping effects

### 11.3 Sound Effects (Future)
- [ ] Add audio for:
  - Weapon fire
  - Explosions
  - Victory fanfare
  - Summon rarity reveals
  - UI clicks and hovers

---

## Phase 12: Meta-Game Features
**Goal:** Keep players engaged long-term

### 12.1 Achievement System Expansion
- [ ] Combat achievements:
  - "First Blood": Win first battle
  - "Legendary Hunter": Defeat 100 bosses
  - "Perfect Captain": Win 50 battles with no damage
  - "Gatcha Addict": Perform 1000 summons
  - "Fully Armed": Collect all legendary parts

### 12.2 Daily/Weekly Quests
- [ ] Quest system:
  - Daily: "Win 10 battles", "Complete 3 raids"
  - Weekly: "Defeat 50 enemies", "Enhance 5 parts"
  - Rewards: Credits, tickets, materials, XP

### 12.3 Leaderboards (Future)
- [ ] Competitive features:
  - Highest level
  - Fastest raid clear times
  - Most powerful ship (stat total)
  - PvP arena (ship vs ship battles)

### 12.4 Events (Future)
- [ ] Limited-time events:
  - Increased legendary drop rates
  - Special event bosses
  - Exclusive event parts
  - Double XP weekends

---

## Implementation Priority Guide

### High Priority (Core Gameplay Loop)
1. **Phase 2:** Combat System - Makes the game playable
2. **Phase 3:** Gatcha System - Core monetization/excitement
3. **Phase 6:** Enhancement System - Progression depth
4. **Phase 7:** Zones - Content variety

### Medium Priority (Depth & Engagement)
5. **Phase 5:** Skills - Strategic depth
6. **Phase 8:** Raids - Endgame content
7. **Phase 9:** Leveling - Clear progression
8. **Phase 11:** UI Polish - Player experience

### Lower Priority (Nice-to-Have)
9. **Phase 4:** Heroes - Additional collection layer
10. **Phase 10:** Prestige Integration - Late-game content
11. **Phase 12:** Meta-features - Long-term retention

---

## Technical Architecture Notes

### New Classes Needed
```cpp
// Combat System
class Enemy;
class CombatSystem;
class CombatState;
class DamageCalculator;

// Gatcha System
class GatchaSystem;
class SummonBanner;
class PityTracker;

// Hero System (optional)
class Hero;
class HeroCollection;

// Skills
class Skill;
class SkillTree;

// Zones
class Zone;
class ZoneManager;
class Stage;

// Raids
class Raid;
class RaidInstance;
class EnergySystem;

// Enhancement
class EnhancementSystem;
class PartUpgrader;
class MaterialInventory;
```

### Database Files
- `enemies.json` - Enemy templates
- `zones.json` - Zone configurations
- `skills.json` - Skill tree data
- `raids.json` - Raid definitions
- `heroes.json` - Hero data (if implemented)

### Save Data Extensions
- Combat stats (wins, losses, kills)
- Gatcha pity counters
- Unlocked zones/skills
- Hero collection
- Enhancement levels
- Material inventory
- Energy timers

---

## Estimated Development Time

- **Phase 2 (Combat):** 3-4 weeks
- **Phase 3 (Gatcha):** 2 weeks
- **Phase 4 (Heroes):** 2 weeks (optional)
- **Phase 5 (Skills):** 2-3 weeks
- **Phase 6 (Enhancement):** 2 weeks
- **Phase 7 (Zones):** 3 weeks
- **Phase 8 (Raids):** 2-3 weeks
- **Phase 9 (Leveling):** 1 week
- **Phase 10 (Prestige):** 1-2 weeks
- **Phase 11 (Polish):** 2-3 weeks
- **Phase 12 (Meta):** Ongoing

**Total Core Features:** ~15-20 weeks
**With All Features:** ~20-25 weeks

---

## Next Steps

1. ✅ Create this roadmap document
2. ⬜ Begin Phase 2: Create Enemy class and basic combat
3. ⬜ Test combat balance with placeholder enemies
4. ⬜ Implement Phase 3: Gatcha system with exciting animations
5. ⬜ Add zones and progression path
6. ⬜ Continue through phases based on priority

---

## Notes & Design Philosophy

- **Respect Player Time:** Auto-battle and speed controls
- **Generous F2P:** No paywalls, premium currency earnable
- **Clear Progression:** Always show next goal/unlock
- **Exciting Moments:** Make summons and drops feel amazing
- **Strategic Depth:** Skills and gear create build variety
- **Fair Monetization:** Cosmetics, convenience, not power (if monetized)

---

**Last Updated:** 2025-11-28
**Current Phase:** Phase 1 Complete ✅
**Next Phase:** Phase 2 - Combat System
