# Alchemy Recipe View VR - SKSE Migration Plan

## Goal
Migrate the current Papyrus-based VR patch to a native SKSE VR plugin in C++, with the long-term goal of removing fake inventory injection and making recipe display/craft blocking correct and fast in VR.

## Scope Priorities
1. Replace Papyrus ingredient scanning and caching with native C++ logic.
2. Remove ghost ingredient AddItem/RemoveItem behavior.
3. Make recipe-mode display update immediately without requiring a craft action.
4. Enforce non-crafting natively while recipe mode is active.
5. Keep UI/MCM changes minimal in phase 1.

## Recommended Architecture
Phase 1 should keep the existing button/MCM entrypoints and move the real behavior into the DLL.

That means:
- Keep the existing MCM keybind path for now.
- Keep the current Alchemy Helper UI button bootstrap if it is still useful.
- Move toggle state, ingredient classification, caching, and menu integration into the SKSE plugin.
- Avoid any new SWF work unless the native backend proves it is required.

## Current Papyrus Responsibilities To Replace
The native plugin should replace these responsibilities from the Papyrus implementation:
- Toggle recipe mode on/off.
- Enumerate all ingredient forms.
- Classify ingredients into known, unknown, and missing-known sets.
- Cache ingredient classification efficiently.
- Refresh alchemy recipe display state.
- Prevent crafting when recipe mode is only meant to reveal missing ingredients.
- Clean up state on menu close and load game.

## Phase Plan

### Phase 0: Project Bootstrap
- Create a new VR-native SKSE plugin project under this folder.
- Use the workspace's existing VR plugin conventions and references from:
  - `D:\Dev\Skyrim\Projects\Subclasses of Skyrim 2 VR`
  - `D:\Dev\Skyrim\Projects\Shadow Opportunist VR`
  - `D:\Dev\Skyrim\Git.Clones\SkyrimVRESL`
- Add basic logging, plugin load, and version reporting.

Deliverable:
- A plugin that loads in Skyrim VR and writes a dedicated log.

### Phase 1: Runtime Observability
- Add menu open/close detection for the Crafting Menu.
- Add detection for alchemy subtype and selected recipe changes.
- Add logs for:
  - current selected recipe
  - recipe ingredient list
  - player ingredient counts
  - recipe mode state
- Confirm where the alchemy recipe data is recalculated in the native flow.

Deliverable:
- Verified understanding of the native alchemy refresh path in VR.

### Phase 2: Native Ingredient Engine
- Build a native ingredient registry/cache:
  - all ingredients
  - known ingredients
  - unknown ingredients
  - current player inventory counts
- Use fast native containers keyed by FormID.
- Define cache invalidation rules for:
  - load game
  - menu open/close
  - inventory changes
  - ingredient discovery state changes where detectable

Deliverable:
- Fast native replacement for the expensive Papyrus classification loop.

### Phase 3: Recipe Mode State
- Move the recipe mode state machine into the plugin.
- Preserve expected behavior:
  - toggle on reveals missing-known ingredients
  - toggle off restores normal view
  - menu close always cleans up
- Keep the Papyrus side, if any, as a thin trigger layer only.

Deliverable:
- Plugin-controlled recipe mode with no dependency on Papyrus state for correctness.

### Phase 4: Native Display Injection
- Stop using AddItem/RemoveItem and form renaming as the display mechanism.
- Instead, inject missing-known ingredients into the displayed recipe data only.
- Ensure the displayed count is clearly shown as missing rather than available.
- Force refresh from the actual native recipe-selection/update path rather than from a pure UI redraw.

Deliverable:
- Missing ingredients appear immediately without ghost items in inventory.

### Phase 5: Native Craft Blocking
- Enforce non-craftability in the native path while recipe mode is active unless the player has the real ingredients.
- Do not rely on held keys, `_bCanCraft`, or UI dimming alone.
- Validate against VR controller input specifically.

Deliverable:
- Recipe mode cannot craft using display-only ingredients.

### Phase 6: Papyrus Retirement
- Reduce the Papyrus project to one of these outcomes:
  - keep only MCM keybind/settings bridge
  - keep only button/bootstrap layer
  - remove Papyrus runtime entirely if the plugin handles toggle input and UI integration directly
- Remove obsolete fake-inventory code from the maintained runtime path.

Deliverable:
- Papyrus is no longer responsible for gameplay logic.

## Technical Decisions To Make Early
1. Whether to keep the current MCM keybind flow or replace it with plugin-side input/config entirely.
2. Whether to coexist with `Recipes.dll` or replace the relevant recipe-display behavior.
3. Whether any SWF work is still needed after the native backend is in place.

## Known Risks
- The main risk is selecting the correct native hook point for alchemy recipe refresh in VR.
- `Recipes.dll` may already hook related behavior, so compatibility must be tested early.
- Ingredient discovery state can change over time, so cache invalidation must be designed carefully.
- VR craft input must be blocked in the native path, not just visually in Scaleform.

## Recommended First Implementation Slice
The first implementation slice should be deliberately narrow:
- plugin loads in VR
- logs crafting-menu open/close
- detects alchemy menu subtype
- identifies current selected recipe and its ingredients
- logs player counts for those ingredients
- toggles an internal recipe-mode state

This slice should be completed before any display injection or craft blocking work.

## Success Criteria
The migration is successful when:
- recipe mode updates immediately without requiring a craft action
- no fake ingredients are ever added to inventory
- crafting cannot occur from display-only state
- repeated toggles are fast
- Papyrus is no longer on the critical gameplay path
