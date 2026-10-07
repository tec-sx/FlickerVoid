# FVInteractionSystem setup

Look-at interaction: the player's interactor finds interactables around it, picks the best one in view, shows its offers (Talk, Open, Pick up...) and runs an offer's effects when its gesture completes (press, hold, mash, hover).

Modules: `FVInteractionSystem`, `FVInteractionSystemDebug`, `FVInteractionSystemEditor` (Blueprint compile checks for interactable setups).

## 1. Quick start

1. Make an interactor mode, an interactor definition and the UI assets (sections 2 and 4).
2. Add **FV Interactor Component** and **FV Interaction UI Component** to the player pawn; set the interactor definition.
3. Make an interactable definition with one offer, add **FV Interactable Component** to an actor and set the definition.
4. Tag the mesh the player should aim at with `Detectable` (component tag).
5. Press Play, look at the actor and press the offer's input.

## 2. Data assets

| Asset | Class | Key fields |
|---|---|---|
| `DA_InteractorMode_Default` | `FVInteractorModeDefinition` | **Mode Tag** `Interactor.Mode.Default`; **Detection**: trace origin (Character or Camera), offset, radius, range, tick interval, collision and occlusion channels, scoring weights (alignment and distance); optional UI overlay; **Blend Time** |
| `DA_InteractorMode_Aim` | `FVInteractorModeDefinition` | Same, tuned for aiming (camera origin, longer range); set **Optional Interaction Mode UI Overlay** to a crosshair widget |
| `DA_Interactor_Player` | `FVInteractorDefinition` | **Interactor Tag** `Interactor.Tag.Player`, **Granted Tags**, **Blocked Action Tags**, **Default Mode**, other **Modes** pushed at runtime by tag |
| `DA_Interactable_<Name>` | `FVInteractableDefinition` | **Interactable Type** (`Interactable.Door`, `.Item.Pickup`, `.Character`...), **Show Offers**, focus indicator anchor and offset, **Offers**, cooldown, detection channel, compatible interactor tags, detection weight |

### Offers
Each offer on an interactable definition has:
- **Input Tag** (`InputTag.Interaction.Primary`, `.Secondary`, `.Alternate`) and **Action Tag** (`Interaction.Action.Talk`, `.Open`, `.Pickup`, `.Examine`, `.LockPick`...).
- **Gesture**: Press, Hold (duration), Mash (press count), Hover or Automatic.
- **Display**: name, description and icon shown in the prompt.
- **Conditions**: when the offer is available. Its *Failure Presentation* hides the offer or shows it locked (with a reason).
- **Effects**: what happens on commit. In this project the usual effect is **Activate Ability** (from the FVGameplay module), which sends a gameplay event (a tag under `Event.`, e.g. `Event.Interaction.Talk`) to the player. Grant the player an ability derived from `UFVInteractAbility` with an *Ability Trigger* of type *Gameplay Event* on that tag; it reads the interactable and does the work (see `Script/Abilities`: Talk, Pickup, Examine, Lockpick).
- **Weight** (ordering) and **Remaining Uses** (-1 unlimited).

Untick **Show Offers** for simple interactables such as an unlocked door: the widget gets an empty offer list but still shows the type-specific focus indicator.

## 3. Components

| Component | Goes on | Notes |
|---|---|---|
| FV Interactor Component | Player pawn | Set **Definition**. Modes are pushed and popped at runtime by tag (e.g. while aiming). **Ignored Actors** for self-hits |
| FV Interaction UI Component | Player pawn or controller (needs the interactor) | Pushes the interaction widget into the UI layout once and keeps it updated. Optional **UI Settings Override** |
| FV Interactable Component | Anything interactable | Set **Definition**. Primitives tagged `Detectable` (the **Detectable Primitive Tag**) are traced and highlighted |
| FV Interactable Highlight Component | Interactables that should glow | Uses the project's highlight setup unless **Override Setup** is ticked |
| FV Lock Component | Lockable doors and containers | **Locked** (saved), **Lock Pickable**, **Difficulty** 0-1 |
| `UFVInteractableResponseComponent` subclass | Interactables reacting to an action | Abstract; set its **Action Tag** and implement **Execute Action**. `UFVInteractableResponseComponent_Toggle` handles open/close with a lock; see `Script/World/Doors/InteractionResponse_DoorToggle.as` |

## 4. Interaction UI

| Asset | Class | Notes |
|---|---|---|
| `DA_InteractionUISettings` | `FVInteractionUISettings` | **Widget Class**, **Layer Tag** (`UI.Layer.Game`), **Default Focus Indicator Brush**, **Focus Indicator Overrides** table. The plugin ships one in its Content folder |
| `WBP_InteractionSet` | child of `FVInteractionWidget` (or the AngelScript `InteractionSetWidget`) | Implements `OnOffersChanged`, `OnFocusIndicatorChanged`, `OnOfferProgress`, `OnOfferEnded`. Optional centred `FocusIndicator` image |
| `WBP_InteractionSlot_First/Mid/Last` | `InteractionSlotWidget` | Prompt rows, assigned on `WBP_InteractionSet` |
| `DT_InteractionSlotStyles` | Data table, row `InteractionSlotStyle` | Row name = action tag |
| `DT_FocusIndicatorOverrides` | Data table, row `FVInteractionFocusIndicatorRow` | Row name = interactable type tag. Lookup tries the exact tag, then each parent, then the default brush |
| `WBP_AimOverlay` | User widget | Centred, hit-test-invisible reticle; set as a mode's UI overlay. The overlay is pushed while that mode is active |

Requires the FVFramework UI layout with a `UI.Layer.Game` layer.

## 5. Settings

*FlickerVoid > Interaction System* (also *FlickerVoid menu > Interaction System > Global Settings* and *UI Settings*):

| Setting | Meaning |
|---|---|
| Interactor Default Settings | Default state, collision channel and interactor tag |
| Interactable Base Settings | Default interaction period, state, channel, highlight on/off, highlight setup (overlay material or post-process stencil), weight |
| Registry Settings | Activation radius around the player and refresh interval |
| Widget Update Frequency | Seconds between prompt updates |
| Interaction UI Settings | `DA_InteractionUISettings` |

For post-process highlighting, enable *Custom Depth-Stencil Pass: Enabled with Stencil* in Project Settings > Rendering and use a post-process material reading the stencil ID.

## 6. Gameplay tags

Defined natively, nothing to create: `Interactable.*`, `Interaction.Action.*`, `InputTag.Interaction.*`, `Interactor.Tag.*`, `Interactor.Mode.*`, `Interaction.Cancel.*`, `Interaction.Suppression.*`. Add children for new types and actions (e.g. `Interaction.Action.Sit`).

## 7. Conditions

**Interactor Tags**: the interactor carries (or lacks) given tags. Combine it with any other plugin's conditions on offers, e.g. *Has Item* (key), *Faction Standing*, *Is Wearing*, *Knows*.

## 8. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Interaction.Debug.HUD 1` | Candidates and resolved prompts on screen |
| `FVCvar.Interaction.Debug.Draw 1` or `2` | Draw detection volumes, probes, cones and highlights |
| `FV.Interaction.List` | Log registered interactables |
| `FV.Interaction.Focus` | Log what the player focuses |

## Troubleshooting

| Symptom | Fix |
|---|---|
| Interaction widget gets no events | Its class isn't a child of `FVInteractionWidget` |
| Nothing gets focused | No primitive tagged `Detectable`, or the detection channel doesn't hit it |
| Focus indicator never changes | The data table row name doesn't match the interactable type tag |
| Offer commits but nothing happens | The Activate Ability event tag has no matching ability trigger on the player |
