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

### Gameplay tags
Create layer tags, for example:
```
UI.Layer.Game        // HUD elements, interaction prompts, mode overlays
UI.Layer.GameMenu    // inventory, journal, map
UI.Layer.Menu        // pause and main menu
UI.Layer.Modal       // confirmations, popups
```

### `WBP_FVUILayout` (parent class `FVUILayout`)
The plugin ships one in `FVFramework/Content/WBP_FVUILayout`; make your own when you need different layers.
1. Root: an `Overlay` that fills the screen.
2. Add one `CommonActivatableWidgetStack` per layer, bottom to top: `Stack_Game`, `Stack_GameMenu`, `Stack_Menu`, `Stack_Modal` (each *Is Variable*, fill alignment).
3. In **Event Construct**, call `Register Layer` once per stack: `Register Layer(UI.Layer.Game, Stack_Game)`, and so on.
4. Set it in *FlickerVoid > UI > Layout Class*.

### HUD
- Set the game mode's **HUD Class** to `AFVHUD`. It creates the layout for the local player on BeginPlay and removes it on EndPlay.
- Optional `BP_FVHUD` (parent `FVHUD`) when you want Blueprint logic on **On Layout Ready**, e.g. pushing a permanent HUD screen. Set it in *FlickerVoid > UI > Default HUD Class*.
- Push widgets into a layer through `UFVUIManagerSubsystem` (a local player subsystem). Never add the layout to the viewport yourself.

| Setting (*FlickerVoid > UI*) | Value |
|---|---|
| Layout Class | `WBP_FVUILayout` |
| Default HUD Class | `FVHUD` or `BP_FVHUD` |
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
| `DA_Cinematic_<Name>` | `FVCinematicDefinition` | Sequence, Skippable, Disable Player Input, Hide Player, On Finished effects. Id = fact set to 1 once watched |

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
| A definition fails validation with "has no Id tag" | Every definition needs an Id |
