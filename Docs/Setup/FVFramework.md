# FVFramework setup

The core every other plugin builds on: definitions and fragments, the facts database, conditions and effects, character identity, UI layout and HUD, world clock, cinematics, spawners, Flow and StateTree nodes, and the debug HUD.

Modules: `FVCoreRuntime`, `FVCoreFlow`, `FVCoreUI`, `FVCoreDebug`, `FVCoreEditor`. Requires the Flow, StateTree, CommonUI, EnhancedInput and DataValidation plugins (enabled by the plugin).

## 1. Quick start

1. Make the UI layout widget (section 3) and set it in *FlickerVoid > UI > Layout Class*.
2. Use a game mode whose **HUD Class** is `AFVHUD` (or a Blueprint child of it).
3. Add the fact tags you need (section 2).
4. Make a character definition for the player and put an Identity component on the player pawn (section 4).
5. Press Play. The HUD creates the layout on BeginPlay.

## 2. Facts

Facts are integers keyed by gameplay tags, held by `UFVFactDatabase` (a game instance subsystem). Quest state, knowledge, standing, fame, titles, discovered places and the world clock all live here, so they save together and every condition can read them.

- Create fact tags under `Fact.` (Project Settings > GameplayTags, or a tag `.ini`), e.g. `Fact.Quest.FindTheKey`, `Fact.Faction.Police.Standing`.
- Declaring a fact is optional; an undeclared fact reads 0. Declare one in *FlickerVoid > Facts* (`Config/DefaultFacts.ini`) to give it a **Default Value**, a **Min/Max** clamp or **Value Names** (labels for enum-like facts).
- Read and write facts from Blueprint/AngelScript through the subsystem (`Get Fact`, `Set Fact`, `Add Fact`), or in data with the **Fact** condition and **Fact** effect.

## 3. UI layout and HUD (FVCoreUI)

### How layers work

The layout is a full-screen widget with a few **layers** placed on top of each other. Each layer is a `CommonActivatableWidgetStack`:
- **All layers are visible at once**, bottom to top. A menu on a higher layer draws over the HUD on a lower one.
- **Inside one layer only the top widget is visible.** Pushing a widget onto a layer hides the one under it until the new one is popped.

So a layer is a *slot for one thing at a time*, not a folder per feature. Pick a layer by asking "what should this replace when it opens?":
- Things that are on screen together (minimap, compass, quest tracker, health, notifications) go **inside one HUD widget** pushed once. Don't give each its own layer.
- Things that replace each other (world map, inventory, journal) are screens on a **menu layer**: opening the inventory from the map hides the map, closing it shows the map again.

### Recommended layers

```
UI.Layer.HUD        // WBP_GameHUD only: the always-on HUD, pushed once
UI.Layer.Game       // transient gameplay overlays: interaction prompt, aim reticle, scanner overlay
UI.Layer.GameMenu   // in-game screens: world map, inventory, journal, character
UI.Layer.Menu       // pause menu, settings, main menu
UI.Layer.Modal      // confirmations, popups
```
Create the tags (Project Settings > GameplayTags). A layer you don't register simply can't be pushed to.

### `WBP_FVUILayout` (parent class `FVUILayout`)
The plugin ships one in `FVFramework/Content/WBP_FVUILayout`; make your own to change the layers.
1. Root: an `Overlay` that fills the screen.
2. Add one `CommonActivatableWidgetStack` per layer, **bottom to top in the hierarchy**: `Stack_HUD`, `Stack_Game`, `Stack_GameMenu`, `Stack_Menu`, `Stack_Modal` (each *Is Variable*, horizontal and vertical alignment *Fill*).
3. In **Event Construct**, call `Register Layer` once per stack: `Register Layer(UI.Layer.HUD, Stack_HUD)`, `Register Layer(UI.Layer.Game, Stack_Game)`, and so on.
4. Set it in *FlickerVoid > UI > Layout Class*.

### The main HUD: `WBP_GameHUD`
1. Create `WBP_GameHUD` with parent class **Common Activatable Widget**. Leave *Is Back Handler* off and set its input config to *Game* so it doesn't take the mouse.
2. Lay out the always-on widgets in it: `WBP_Minimap`, `WBP_Compass`, quest tracker, notification area. Each child is a plain User Widget that finds its own data (the navigator, the quest subsystem...) in its Construct; the HUD widget itself holds no logic.
3. Push it once when the layout is ready: make `BP_FVHUD` (parent `FVHUD`), and in **On Layout Ready** call `Get UFVUIManagerSubsystem > Push Screen(UI.Layer.HUD, WBP_GameHUD)`. Set `BP_FVHUD` as the game mode's HUD class (or in *FlickerVoid > UI > Default HUD Class*).
4. Optional: hide the HUD while a menu is open. Bind the UI manager's **On Stack Changed** in `WBP_GameHUD` and set its visibility to *Collapsed* when `Layout > Get Layer(UI.Layer.GameMenu) > Get Active Widget` is valid.

### Screens and menus
- Screens (world map, inventory) are **Common Activatable Widgets**. Open one with `Push Screen(UI.Layer.GameMenu, WBP_WorldMap)` from an input action; close it with `Pop Screen(Widget)` or *Deactivate Widget* (e.g. from a Back action). Set their input config to *Menu* so the mouse shows and game input stops.
- `Push User Widget` / `Pop User Widget` push a plain User Widget wrapped in a host; plugin UIs use this (the interaction prompt goes to the layer in `DA_InteractionUISettings`, `UI.Layer.Game` by default).
- Never add widgets or the layout to the viewport yourself; always go through `UFVUIManagerSubsystem`.

### HUD class
- Set the game mode's **HUD Class** to `AFVHUD` or `BP_FVHUD`. It creates the layout for the local player on BeginPlay and removes it on EndPlay.

| Setting (*FlickerVoid > UI*) | Value |
|---|---|
| Layout Class | `WBP_FVUILayout` |
| Default HUD Class | `BP_FVHUD` |
| Warn If HUD Missing | on |

## 4. Characters and identity

| Asset | Class | Notes |
|---|---|---|
| `DA_Character_Player`, `DA_Character_<Npc>` | `FVCharacterDefinition` | One per named character or NPC archetype. Other plugins add fragments here: **Social** (faction, relationship fact), **Attributes** (starting attributes) |

- Add an **FV Identity Component** to every character Blueprint (player and NPCs) and set its **Definition**. Systems find actors by definition through `UFVIdentitySubsystem` (`Find Actor`, `Find Actors`).
- Dialogue speakers, faction membership and relationships all resolve through this component.

## 5. Conditions and effects

Built-in types you can pick anywhere a rule appears:

| Conditions | Effects |
|---|---|
| Fact, Time of Day, Group (nested set), Script | Fact (set, add or remove), Advance Time, Play Cinematic, Script |

- A condition set has **Mode** (All or Any) and **Failure Presentation** (Hidden, Show Locked, Show Locked With Reason), which UIs use for offers and dialogue choices.
- **Custom rules in AngelScript**: subclass `UFVScriptCondition` (override `Evaluate` and `GetDescription`) or `UFVScriptEffect` (override `Apply`), then pick the **Script** condition or effect in data and choose your class.
- From Blueprint/AngelScript: `UFVConditionStatics` > `Evaluate Condition Set`, `Apply Effects`, `Describe Condition Set`.

## 6. World clock

*FlickerVoid > World Clock*:

| Setting | Meaning |
|---|---|
| Seconds Per Hour | Real seconds per in-game hour; 0 means time moves only through **Advance Time** (player-paced) |
| Start Hour | Hour when a new game starts |
| Phases | `Time.Phase.*` tags with a start hour, sorted ascending (Dawn 5, Day 8, Dusk 19, Night 21) |
| Minutes Fact, Phase Fact, Day Fact | Fact tags the clock writes, so time saves with the facts |

Use the **Time of Day** condition for schedules and the **Advance Time** effect for sleeping or waiting.

## 7. Cinematics

| Asset | Class | Fields |
|---|---|---|
| `DA_Cinematic_<Name>` | `FVCinematicDefinition` | Sequence, Skippable, Disable Player Input, Hide Player, On Finished effects. Optional Id = fact set to 1 once watched |

Play one with the **Play Cinematic** effect (from quests, dialogue, Flow) or `UFVCinematicSubsystem::Play`.

## 8. Spawners

| Asset | Class | Fields |
|---|---|---|
| `DA_Spawn_<Name>` | `FVSpawnDefinition` | Actor Class, optional Payload data asset, Spawn When, Despawn When, Respawn |

Place an `AFVSpawner` (or a Blueprint of it) in the level and set its **Definition**. It spawns and despawns as the conditions change, e.g. an NPC who only appears after a quest step.

## 9. Flow and StateTree nodes

- **Flow** (FVCoreFlow): *FV Branch* (condition set to True/False), *FV Apply Effects*, *FV Wait For Conditions* (holds the signal until the conditions pass; re-checked on every fact change) and the *FV Conditions Predicate* add-on. Inside a Flow graph, Instigator is the first local player pawn and Target is the actor that owns the root flow.
- **StateTree**: the *FV Conditions* condition and the *FV Apply Effects* task (effects on state enter and exit).

## 10. Input

| Asset | Class | Notes |
|---|---|---|
| `DA_InputConfig` | `FVInputConfig` | Maps Input Actions to input tags (native and ability actions) plus a key-icon data table |

`UFVGestureComponent` measures press, hold, mash and hover gestures for interaction offers.

## 11. Debug

| CVar / command | Effect |
|---|---|
| `FVCvar.Facts.Debug.HUD 1` | On-screen fact readout |
| `FVCvar.Facts.Debug.Filter Quest` | Limit the readout to tags containing this text |
| `FV.Facts.Set <Tag> <Value>` | Write a fact |
| `FV.Facts.Dump [Filter]` | Log all facts |
| `FV.Facts.Reset` | Reset facts to their declared defaults |
| `FV.ValidateAll` (editor) | Run data validation on every definition |

Every plugin's debug readout stacks on the same on-screen panel (`UFVDebugHUDSubsystem`).

## Troubleshooting

| Symptom | Fix |
|---|---|
| Nothing on screen, log says `LayoutClass is not set` | Set *FlickerVoid > UI > Layout Class* |
| Log says `HUD is not an AFVHUD` | Set the game mode's HUD Class to `AFVHUD` |
| Widgets pushed but invisible | The layer tag isn't registered in the layout's Event Construct |
| A definition fails validation with "has no Id tag" | Quests, knowledge, factions and titles store their state in the fact their Id names, so they need one |
