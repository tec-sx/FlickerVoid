# FlickerVoid Framework – Content Setup

This guide lists the assets you need to create in content so the FV plugins work.
All settings live in **Project Settings → FlickerVoid** (UI, World Clock, Facts, Quests, Save, Interaction System).
The **FlickerVoid** toolbar button (level editor toolbar) and the **FlickerVoid** main menu open these settings directly.

---

## 1. Quick start

1. Create the UI layout widget (section 2) and assign it in **FlickerVoid → UI → Layout Class**.
2. Use a game mode with an `AFVHUD` HUD:
   - **Simplest:** use `AFVGameModeBase` (or `AFVExploreGameMode` / `AFVStoryGameMode`) from the FlickerVoid project, or
   - in any game mode, set **HUD Class** to `AFVHUD` (or a Blueprint child `BP_FVHUD`).
3. Set up the interaction UI (section 4) if you use FVInteractionSystem.
4. Press Play. The HUD creates the layout on BeginPlay and removes it on EndPlay.

> You don't add the layout to your own HUD widget. `AFVHUD` owns it, and every other widget is pushed *into* its layers.

---

## 2. FVFramework – UI layout

### Gameplay tags
Add layer tags (Project Settings → GameplayTags or a tag `.ini`), for example:
```
UI.Layer.Game        // HUD elements: interaction prompts, mode overlays
UI.Layer.GameMenu    // inventory, journal
UI.Layer.Menu        // pause / main menu
UI.Layer.Modal       // confirmations, popups
```

### `WBP_FVUILayout` (parent: `FVUILayout`)
Suggested location: `Plugins/FVFramework/Content/UI/WBP_FVUILayout`.

1. Root: `Overlay` (fill screen).
2. Add one `CommonActivatableWidgetStack` per layer, stacked bottom → top:
   `Stack_Game`, `Stack_GameMenu`, `Stack_Menu`, `Stack_Modal` (each set to *Is Variable*, fill alignment).
3. In **Event Construct**, call `Register Layer` once for each stack:
   `Register Layer(UI.Layer.Game, Stack_Game)`, etc.
4. Assign it to **Project Settings → FlickerVoid → UI → Layout Class**.

### Optional `BP_FVHUD` (parent: `FVHUD`)
Only needed if you want Blueprint logic on `On Layout Ready` (e.g. push a permanent HUD screen).
Assign it to **FlickerVoid → UI → Default HUD Class**. `AFVGameModeBase` uses it automatically.

### Settings
| Setting | Value |
|---|---|
| Layout Class | `WBP_FVUILayout` |
| Default HUD Class | `FVHUD` or `BP_FVHUD` |
| Warn If HUD Missing | on (logs a warning if the layout gets created lazily without `AFVHUD`) |

---

## 3. FVFramework – Facts and World Clock

- **Facts** (`Config/DefaultFacts.ini`, Project Settings → FlickerVoid → Facts): declare default facts, types and value names.
- **World Clock** (Project Settings → FlickerVoid → World Clock): start hour, time scale and day phases (`Time.Phase.*` tags, sorted ascending by start hour).

No content assets are required beyond the gameplay tags you reference.

---

## 4. FVInteractionSystem

### Assets
| Asset | Parent / Type | Notes |
|---|---|---|
| `DA_InteractionUISettings` | `FVInteractionUISettings` data asset | Widget class, layer tag (`UI.Layer.Game`), default focus indicator brush, focus indicator overrides |
| `WBP_InteractionSet` | `InteractionSetWidget` (AngelScript) or any `FVInteractionWidget` child | Implements `OnOffersChanged`, `OnFocusIndicatorChanged`, `OnOfferProgress`, `OnOfferEnded`. Optional `FocusIndicator` image, centered |
| `WBP_InteractionSlot_First/Mid/Last` | `InteractionSlotWidget` | Assigned on `WBP_InteractionSet` |
| `DT_InteractionSlotStyles` | DataTable, row `InteractionSlotStyle` | Row name = action tag |
| `DT_FocusIndicatorOverrides` | DataTable, row `FVInteractionFocusIndicatorRow` | Row name = interactable type tag (e.g. `Interactable.Door`). Lookup tries the exact tag, then each parent tag, then the default brush |
| `T_FocusIndicator_Default` | Texture2D | Image used by `DefaultFocusIndicatorBrush` (brush rows also set tint, size, etc.) |
| `WBP_AimOverlay` | UserWidget (FVInteractionSystem plugin content) | Overlay with a centered, `HitTestInvisible` Image (Reticle). Assigned as the Interaction Mode UI Overlay on aim modes |

### Settings
**Project Settings → FlickerVoid → Interaction System → Interaction UI Settings** = `DA_InteractionUISettings`.

### Interactables
- Set `Interactable Type` on each interactable definition. It selects the focus indicator row.
- Untick `Show Offers` for simple interactables (e.g. an unlocked door). The widget then gets an empty offer list but still shows the type-specific focus indicator.
- `Focus Indicator Anchor` (Center/Top/Bottom of the detectable bounds) and `Focus Indicator Offset` (actor local space) place the indicator.

### Interactor modes
Tick `Show Overlay` and set `Interaction Mode UI Overlay` (e.g. `WBP_AimOverlay`) on modes that need one. The overlay is pushed to the interaction layer while the mode is active.

### Player
The pawn/controller needs `FVInteractorComponent` and `FVInteractionUIComponent`.
The UI component pushes the interaction widget into the layout once and keeps it alive, so the widget decides what to show for empty offer lists.

---

## 5. Troubleshooting

| Symptom | Cause / fix |
|---|---|
| Nothing on screen, log says `LayoutClass is not set` | Assign `WBP_FVUILayout` in FlickerVoid → UI |
| Log says `HUD is not an AFVHUD` | Set the game mode HUD Class to `AFVHUD`, or derive from `AFVGameModeBase` |
| Widgets pushed but invisible | The layer tag isn't registered in the layout's `Event Construct` |
| Interaction widget gets no events | Its widget class isn't a child of `FVInteractionWidget` |
| Focus indicator never changes | The row name doesn't match the interactable type tag, or the table's row struct is wrong |
must return `FV::Names::SettingsCategory` (`FVCoreNames.h`)
