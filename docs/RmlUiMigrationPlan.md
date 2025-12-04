# RmlUi Migration Plan

This document outlines a staged approach for introducing [RmlUi](https://github.com/mikke89/RmlUi) as the primary UI layer for Quantum Idle while keeping the game playable during the transition.

## Current status
- **Build toggle**: `ENABLE_RMLUI` CMake option added with stub runtime wiring so builds stay stable until the library is linked. When RmlUi is available, defining `RMLUI_ENABLED` will activate the pipeline.
- **Runtime scaffold**: `RmlUiSystem` placeholder is created and hooked into `Application` to receive events, resize notifications, and a render hook before ImGui. It is ready to host the real context once the backend is wired.
- **Assets bootstrap**: `assets/ui/rml/hud.rml` and `hud.rcss` stand in as the first document/style pair to validate loading and layout once RmlUi rendering is enabled.

## Goals
- Deliver a responsive, web-like UI suitable for idle/incremental gameplay (grid-heavy layouts, rich styling, and readable typography).
- Maintain desktop and web builds (Emscripten) with consistent behavior.
- Avoid breaking existing ImGui-driven screens until their RmlUi equivalents are ready.

## Phase 0: Preparation
- **Vendoring strategy**: Add RmlUi as a submodule or fetch via CMake's `FetchContent`. Enable the `rmlui_sdl_renderer` and `rmlui_sdl2_system` backends for SDL2 + OpenGL, and confirm Emscripten support with the GLES/WebGL renderer.
- **Build setup**: Extend `CMakeLists.txt` to build RmlUi for desktop and web targets. Ensure font assets include variable/static faces for body text and headings (e.g., Inter/Roboto + a mono face for numbers).
- **Asset layout**: Create `assets/ui/rml/` for `.rml` (markup) and `.rcss` (styles) plus shared image/svg assets. Add a hot-reload toggle in debug builds to speed iteration.

## Phase 1: Runtime scaffolding
- **Context management**: Initialize a global `Rml::Context` sized to the main framebuffer. Listen to window/canvas resize events to call `context->SetDimensions`.
- **Input routing**: Forward SDL2 events (mouse, keyboard, wheel, text input) to RmlUi before ImGui so documents handle focus and text entry correctly.
- **Rendering**: Insert a render pass that clears depth/stencil if needed and calls `context->Render()` after the game scene but before ImGui (during coexistence).
- **Data binding utilities**: Implement helpers for populating elements (`SetInnerRML`, data attributes) and registering event callbacks that relay to C++ lambdas or state actions.

## Phase 2: Feature parity with ImGui
- **Shell HUD**: Build a base HUD document (`hud.rml`) with panels for resources, navigation, and notifications. Style with responsive flex/grid to match current ImGui layout.
- **Views migration**: Recreate high-traffic screens first (Stations, Buyables, Research, Challenges) using reusable components (cards, tables, tabs). Keep ImGui versions accessible behind a debug toggle until QA completes.
- **Tooltip + dialogs**: Implement reusable tooltip and modal dialog components with keyboard/escape handling. Mirror existing tutorial overlays and prestige confirmations.

## Phase 3: Polish and UX upgrades
- **Accessibility**: Add ARIA-like annotations via `role`/`aria-` attributes and ensure focus order works with keyboard navigation. Provide scalable font sizes and a high-contrast theme variant via CSS variables.
- **Animations & feedback**: Use CSS transitions for hover/press states, resource gain flashes, and navigation focus. Introduce toast notifications for achievements and research completions.
- **Localization readiness**: Keep strings in external tables and avoid hard-coded text in `.rml` where possible to enable future localization.

## Phase 4: Decommission ImGui UI
- **Feature toggle removal**: Once each screen is validated in RmlUi, retire its ImGui counterpart and delete redundant layout code.
- **Testing matrix**: Validate desktop (Win/Linux/macOS) and web builds for input, font rendering, DPI scaling, and performance. Add CI steps for building the web target with RmlUi enabled.

## Risks and Mitigations
- **Bundle size**: Audit generated CSS/markup and shared textures; minify release assets and reuse sprites to keep the web build small.
- **Input conflicts**: During coexistence, gate ImGui input when an RmlUi document has focus to avoid double-handling.
- **Performance**: Profile redraw frequency; keep background documents static and favor data patching over full document reloads.

## Definition of Done
- All player-facing HUD and management screens are rendered through RmlUi with consistent styling across desktop and web.
- ImGui remains only for developer/debug tooling.
- Build pipelines produce RmlUi-enabled artifacts without manual steps.
