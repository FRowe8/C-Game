# UI/UX Overhaul Plan

## Design Philosophy

### Core Principles
1. **Mobile-First Design**: All screens must work beautifully on small devices
2. **Professional Polish**: Eye-catching, modern, and appealing for advertising
3. **Fun & Rewarding**: Engaging graphics and animations that feel satisfying
4. **Genre-Leading**: Stand head and shoulders above competition
5. **Asset-Driven**: Thoughtful use of colors, effects, and visual feedback
6. **Guided Progression**: Features unlock gradually, never overwhelming the player
7. **Narrative Flow**: Each system connects to a cohesive space exploration story

### Visual Standards
- **Touch-Friendly**: All buttons minimum 44px tap targets
- **Responsive Layout**: Adapts to screen sizes from 320px to 1920px+
- **High Contrast**: Clear text and UI elements on all backgrounds
- **Color Theming**: Zone-specific colors that create distinct atmospheres
- **Particle Effects**: Summons, upgrades, and victories feel spectacular
- **Smooth Animations**: Transitions, reveals, and feedback animations

### Progression Integration
See **PROGRESSION_FLOW.md** for complete feature unlock system:
- Features unlock based on player level (Combat@10, Summon@15, Skills@20, etc.)
- Locked features show "🔒 Unlocks at Level X"
- New features pulse/glow when first unlocked
- Tutorial popups guide players through each new system
- Never show more than 6 main features at once (prevents overwhelm)

## Current Issues

### Critical Problems
1. **No Exit Mechanism**: Users cannot exit from new overlay screens (Combat, Summon, Skills, Enhancement)
2. **ESC Key Not Working**: ESC key doesn't close overlay screens
3. **Trapped in Screens**: Once opened, users are stuck in overlay screens
4. **Inconsistent Close Buttons**: Some screens have close buttons, some don't
5. **No Visual Feedback**: No indication of how to close screens

### Usability Issues
1. Missing "Press ESC to close" hints on most screens
2. No X button in top-right corner for many overlays
3. Clicking outside overlay doesn't close it
4. Multiple overlays can be open at once (confusing)
5. No clear navigation hierarchy

---

## Phase 1: Universal Close Mechanism (HIGH PRIORITY)

### Goal
Every overlay screen should be closable via ESC key, X button, or clicking outside.

### Implementation Steps

#### 1.1 ESC Key Handler in GameState
```cpp
// In GameState::UpdateUI() - add global ESC handler
if (input->IsKeyPressed(Key::Escape)) {
    // Close overlays in priority order (most recently opened first)
    if (m_ShowEnhancement) { m_ShowEnhancement = false; }
    else if (m_ShowSkills) { m_ShowSkills = false; }
    else if (m_ShowGatcha) { m_ShowGatcha = false; }
    else if (m_ShowCombat) { m_ShowCombat = false; }
    else if (m_ShowSpaceship) { m_ShowSpaceship = false; }
    else if (m_ShowChallenges) { m_ShowChallenges = false; }
    else if (m_ShowBuyables) { m_ShowBuyables = false; }
    else if (m_ShowMilestones) { m_ShowMilestones = false; }
    else if (m_ShowResearch) { m_ShowResearch = false; }
    else if (m_ShowEssenceShop) { m_ShowEssenceShop = false; }
    else if (m_ShowSingularityShop) { m_ShowSingularityShop = false; }
    else if (m_ShowAchievements) { m_ShowAchievements = false; }
    else if (m_ShowStats) { m_ShowStats = false; }
}
```

#### 1.2 Close Button Standard
All overlay screens should have:
- **X button** in top-right corner (30x30px)
- **"Press ESC to close"** text in bottom-right
- **Click-outside-to-close** functionality

#### 1.3 Files to Update
- [x] CombatSystem - Add close button rendering
- [x] GatchaSystem - Add close button rendering
- [x] SkillTreeSystem - Add close button rendering
- [x] EnhancementSystem - Add close button rendering
- [ ] All existing overlay screens (verify they have close buttons)

---

## Phase 2: Consistent Overlay Pattern (MEDIUM PRIORITY)

### Goal
Standardize how all overlays look and behave.

### Standard Overlay Structure
```cpp
void RenderOverlay(Renderer* renderer) {
    // 1. Dark background overlay (80% opacity)
    Rect bg(0, 0, screenWidth, screenHeight);
    renderer->DrawRect(bg, Color(0, 0, 0, 0.8f), true);

    // 2. Main panel (centered)
    f32 panelWidth = 900.0f;
    f32 panelHeight = 700.0f;
    f32 panelX = (screenWidth - panelWidth) / 2;
    f32 panelY = (screenHeight - panelHeight) / 2;

    // 3. Panel background with border
    Rect panel(panelX, panelY, panelWidth, panelHeight);
    renderer->DrawRect(panel, Color::DarkPanel(), true);
    renderer->DrawRect(panel, themeColor, false); // Colored border

    // 4. Title (top-left)
    renderer->DrawText("TITLE", Vec2(panelX + 20, panelY + 15), themeColor, 24.0f);

    // 5. Close button (top-right)
    RenderCloseButton(renderer, panelX, panelY, panelWidth);

    // 6. Content area
    // ... render specific content ...

    // 7. Help text (bottom-right)
    renderer->DrawText("Press ESC to close",
                      Vec2(panelX + panelWidth - 150, panelY + panelHeight - 25),
                      Color(0.6f, 0.6f, 0.6f, 1.0f), 12.0f);
}
```

### Standard Close Button Helper
```cpp
void RenderCloseButton(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;

    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);
    renderer->DrawRect(closeBtn, Color(0.3f, 0.1f, 0.1f, 0.8f), true);
    renderer->DrawRect(closeBtn, Color(1.0f, 0.3f, 0.3f, 1.0f), false);
    renderer->DrawText("X", Vec2(closeBtnX + 10, closeBtnY + 8), Color::White(), 16.0f);
}

bool IsCloseButtonClicked(f32 mouseX, f32 mouseY, f32 panelX, f32 panelY, f32 panelWidth) {
    f32 closeBtnSize = 30.0f;
    f32 closeBtnX = panelX + panelWidth - closeBtnSize - 10.0f;
    f32 closeBtnY = panelY + 10.0f;

    Rect closeBtn(closeBtnX, closeBtnY, closeBtnSize, closeBtnSize);
    return closeBtn.Contains(Vec2(mouseX, mouseY));
}
```

---

## Phase 3: Click-Outside-to-Close (MEDIUM PRIORITY)

### Goal
Allow users to close overlays by clicking on the dark background.

### Implementation
```cpp
void HandleOverlayClick(f32 mouseX, f32 mouseY, bool mousePressed) {
    if (!mousePressed) return;

    // Check if click is outside panel bounds
    Rect panel(panelX, panelY, panelWidth, panelHeight);

    if (!panel.Contains(Vec2(mouseX, mouseY))) {
        // Clicked outside - close overlay
        m_ShowThisOverlay = false;
    }
}
```

### Consideration
- Add visual feedback when hovering over background (cursor change?)
- Maybe add "Click outside to close" hint text

---

## Phase 4: Single-Overlay Rule (LOW PRIORITY)

### Goal
Only one overlay can be open at a time.

### Implementation
```cpp
void OpenOverlay(OverlayType type) {
    // Close all other overlays first
    CloseAllOverlays();

    // Open the requested overlay
    switch (type) {
        case OverlayType::Combat: m_ShowCombat = true; break;
        case OverlayType::Skills: m_ShowSkills = true; break;
        // ... etc
    }
}

void CloseAllOverlays() {
    m_ShowCombat = false;
    m_ShowGatcha = false;
    m_ShowSkills = false;
    m_ShowEnhancement = false;
    m_ShowSpaceship = false;
    m_ShowChallenges = false;
    m_ShowBuyables = false;
    m_ShowMilestones = false;
    m_ShowResearch = false;
    m_ShowEssenceShop = false;
    m_ShowSingularityShop = false;
    m_ShowAchievements = false;
    m_ShowStats = false;
}
```

---

## Phase 5: Navigation Improvements (NICE-TO-HAVE)

### 5.1 Breadcrumb Navigation
Show where the user is in the navigation hierarchy:
```
Main → MORE → ENHANCE
```

### 5.2 Back Button
Add a "Back" button to return to previous screen:
```cpp
std::vector<OverlayType> m_OverlayHistory;

void OpenOverlay(OverlayType type) {
    m_OverlayHistory.push_back(currentOverlay);
    // ... open new overlay
}

void GoBack() {
    if (!m_OverlayHistory.empty()) {
        OverlayType prev = m_OverlayHistory.back();
        m_OverlayHistory.pop_back();
        OpenOverlay(prev);
    }
}
```

### 5.3 Keyboard Shortcuts
- **ESC**: Close current overlay
- **H**: Toggle Help/Controls
- **I**: Open Inventory/Spaceship
- **C**: Open Combat/Zones
- **S**: Open Skills
- **E**: Open Enhancement
- **Tab**: Cycle through tabs within an overlay

---

## Phase 6: Visual Polish (NICE-TO-HAVE)

### 6.1 Overlay Transitions
```cpp
// Fade in/out animations
f32 m_OverlayAlpha = 0.0f;

void UpdateOverlay(f64 deltaTime) {
    if (m_ShowOverlay && m_OverlayAlpha < 1.0f) {
        m_OverlayAlpha += deltaTime * 4.0f; // Fade in over 0.25s
        if (m_OverlayAlpha > 1.0f) m_OverlayAlpha = 1.0f;
    } else if (!m_ShowOverlay && m_OverlayAlpha > 0.0f) {
        m_OverlayAlpha -= deltaTime * 4.0f; // Fade out
        if (m_OverlayAlpha < 0.0f) m_OverlayAlpha = 0.0f;
    }
}
```

### 6.2 Hover Effects
- Buttons glow on hover
- Close button highlights on hover
- Visual feedback for clickable areas

### 6.3 Sound Effects (Future)
- Open overlay: Whoosh sound
- Close overlay: Click sound
- Button hover: Subtle beep
- Error/invalid action: Error beep

---

## Implementation Priority

### Immediate (Fix Now)
1. ✅ Add ESC key handler to close all overlays
2. ✅ Add close button rendering to all new overlays
3. ✅ Add close button click handling to all new overlays
4. ✅ Add "Press ESC to close" text to all overlays

### Short Term (Next Session)
5. ⬜ Implement click-outside-to-close
6. ⬜ Standardize overlay panel sizes and positions
7. ⬜ Add single-overlay rule (close others when opening new)

### Medium Term
8. ⬜ Add keyboard shortcuts for all major screens
9. ⬜ Implement breadcrumb navigation
10. ⬜ Add hover effects and visual feedback

### Long Term
11. ⬜ Overlay transitions and animations
12. ⬜ Sound effects
13. ⬜ Advanced navigation (back button, history)

---

## Testing Checklist

### For Each Overlay Screen:
- [ ] Can open from menu/button
- [ ] ESC key closes it
- [ ] X button closes it
- [ ] Clicking outside closes it (when implemented)
- [ ] "Press ESC to close" text visible
- [ ] Doesn't block other UI elements when closed
- [ ] Only one overlay open at a time

### Screens to Test:
- [ ] Combat/Battle
- [ ] Summon/Gatcha
- [ ] Skills
- [ ] Enhancement
- [ ] Spaceship
- [ ] Achievements
- [ ] Statistics
- [ ] Research Tree
- [ ] Milestones
- [ ] Buyables
- [ ] Challenges
- [ ] Essence Shop
- [ ] Singularity Shop

---

## Code Organization

### Suggested Helper Class
```cpp
class OverlayManager {
public:
    void OpenOverlay(OverlayType type);
    void CloseCurrentOverlay();
    void CloseAllOverlays();
    bool IsOverlayOpen() const;
    OverlayType GetCurrentOverlay() const;

    void Update(f64 deltaTime, Input* input);
    void RenderCloseButton(Renderer* renderer, f32 panelX, f32 panelY, f32 panelWidth);
    bool HandleCloseButtonClick(f32 mouseX, f32 mouseY, f32 panelX, f32 panelY, f32 panelWidth);
    bool HandleClickOutside(f32 mouseX, f32 mouseY, f32 panelX, f32 panelY, f32 panelWidth, f32 panelHeight);

private:
    OverlayType m_CurrentOverlay;
    std::vector<OverlayType> m_History;
    f32 m_TransitionAlpha;
};
```

---

## Next Steps

1. **Immediate Fix**: Implement Phase 1 (ESC key + close buttons)
2. **Test All Screens**: Verify ESC works on every overlay
3. **Standardize**: Apply Phase 2 pattern to all overlays
4. **Polish**: Gradually add Phase 3-6 features

---

**Last Updated:** 2025-11-28
**Status:** Planning Complete - Ready for Implementation
**Priority:** HIGH - Blocks usability of all new features
