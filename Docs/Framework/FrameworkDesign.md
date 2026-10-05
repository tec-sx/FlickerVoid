# FlickerVoid Framework Design

Status: implemented in `feature/framework-dev`, revised after the owner's answers of 2026-10-05. The answers are recorded at the end; nothing has been compiled yet.

## Goals

- Third-person narrative RPG/adventure (Witcher 3, Tomb Raider, Cyberpunk 2077 scanner). Little combat.
- Survival with sneaking, hiding and going undercover. Factions, reputation, notoriety, relationships.
- Outfits instead of armor; quests can require outfits; story-driven customization.
- Linear and non-linear storylines. Data-driven, AngelScript-friendly (no exposed interfaces), composition over inheritance.

## What AAA references teach (and how we apply it)

| Reference | Practice | Here |
| --- | --- | --- |
| CDPR (Witcher 3, Cyberpunk) | One **facts DB** is the single source of truth for story state. Quest graphs are signal graphs with phases, conditions and *pause until condition* nodes. | `UFVFactDatabase` (exists). Flow graphs for quests/dialogue, `FV Wait For Conditions` node = Witcher's Pause node. Quests, knowledge, reputation, relationships, notoriety all live in facts, so saving and conditions come for free. |
| CDPR scanner / Witcher senses / Tomb Raider instinct | A mode that highlights categories of objects through post-process stencils, shows data on focus and turns findings into clues. Time slows while scanning. | Scanner in `FVGameplay`: candidates from the interaction registry, category stencils, focus by view alignment, timed scan, effects on scan (learn knowledge, set facts). Optional time dilation. |
| Hitman | Disguises are tags; some NPCs (enforcers) see through them; suspicion builds rather than flipping. | Outfits grant gameplay tags (`Disguise.Faction.*`). `FVSocialSystem` resolves attitude from standing tier, notoriety and disguise, and a disguise lowers the fame onlookers read. Suspicion meter is phase 2 (stealth). |
| Lyra / modern UE5 | Item fragments, instanced structs, data assets, GAS, StateTree, modular components. | Fragments on definitions (`TInstancedStruct`), components found on actors instead of interfaces. |
| Tarkov / Resident Evil | Carrying is a decision: weight and container space, no free stacking. | Items carry weight and a grid footprint; bags and backpacks grant both. |

## Layers

```
Plugins (FV*System, feature based)  ->  Game modules (domain based)  ->  AngelScript (game objects)  ->  Blueprints (data + cosmetics)
```

### Plugins

| Plugin | Status | Owns |
| --- | --- | --- |
| **FVFramework** | kept, extended | Facts, conditions/effects, definitions, **fragments**, **character definition + identity component**, world clock, cinematics, spawner, input, UI layout, StateTree nodes. New module **FVCoreFlow**: generic Flow nodes (Branch, Apply Effects, Wait For Conditions, Conditions predicate). |
| **FVInteractionSystem** | kept as is | Interactor/interactable, offers, focus. |
| **FVAttributeSystem** | new | Attribute and attribute set definitions, attribute component with modifiers, skill checks, attribute conditions/effects. |
| **FVInventorySystem** | new (replaces the Mountea fork) | Item definition + fragments (equippable, usable, container), inventory with weight and grid space, layered equipment slots, pickup and item receiver, item conditions/effects. |
| **FVDialogueSystem** | new (replaces the Yap fork) | Flow-based conversations: Line, Choice, End Conversation, Bark nodes; dialogue subsystem for UI; participant component for animation/voice. |
| **FVStorySystem** | reworked | Quests (+ optional quest Flow graph), knowledge, save. Yap bridge removed; reputation moved to Social. |
| **FVSocialSystem** | new | Factions and standing tiers, one global fame and one global notoriety, titles, relationships, disguise-aware recognition. |
| FVStealthSystem | phase 2 (proposed) | Visibility (light, stance, disguise), hiding spots, suspicion meters for NPCs (StateTree + AI perception). |

Every plugin depends only on FVFramework (and Flow where it has graph nodes). Plugins never depend on each other; they meet through **facts, gameplay tags, conditions/effects and components on actors**.

### Game modules

| Module | Role | Change |
| --- | --- | --- |
| FVCore | logging, tags, flow triggers | kept |
| FVCharacter | locomotion, animation, traversal | kept |
| FVGameplay | GAS: ASC, abilities, interaction ability base, scanner | attribute sets, checks and the personality system removed |
| FVAI | NPC config, StateTree tasks | faction now from FVSocialSystem |
| FVNarrative | narrative tags | memory and psychology systems removed |
| FVWorld | world objects (doors, hiding spots), world tags | `UFVWorldStateSubsystem` removed, facts and the clock are the single source |
| FVUI | HUD, journal, dialogue, scanner widgets | kept |
| FVGameplay | also owns the **scanner** now (`Scanner/`), built on the interaction registry | scanner moved in from a plugin |
| FVItems | — | **removed**: pickup/receiver/item definition moved to FVInventorySystem |
| FlickerVoidGame | game mode, player, asset manager | kept |

## Data model

```
UFVDefinition (Id tag, Display, Tags)                        FVFramework
 ├─ UFVCharacterDefinition  + Fragments<FFVCharacterFragment> FVFramework
 │     FFVCharacterFragment_Social { Faction, RelationshipFact }      (Social)
 ├─ UFVAttributeDefinition  { Category, Default, Min, Max }     FVAttributeSystem
 ├─ UFVAttributeSetDefinition { Parents, Attributes }           FVAttributeSystem
 ├─ UFVCheckDefinition      { Attribute, Roll, Modifiers<Conditions> }  FVAttributeSystem
 ├─ UFVItemDefinition       { Weight, GridSize, MaxQuantity } + Fragments<FFVItemFragment>  FVInventorySystem
 │     FFVItemFragment_Equippable { Slot, GrantedTags }
 │     FFVItemFragment_Usable     { Effects, bConsume }
 │     FFVItemFragment_Container  { Cells, WeightBonus }
 ├─ UFVFactionDefinition    { standing range, tiers->attitude, relations, NotorietyFact, DisguiseTag }  FVSocialSystem
 ├─ UFVTitleDefinition      { Conditions, OnEarned, Replaces }   FVSocialSystem
 ├─ UFVScanDefinition       { Category, Duration, Entries, OnScanned effects }  FVGameplay
 ├─ UFVQuestDefinition      { objectives, auto start/fail, effects, Flow }      FVStorySystem
 └─ UFVKnowledgeDefinition  { kind, prerequisites, OnLearned }                  FVStorySystem
```

Actor components (composition, found with `FindComponentByClass` / `::Get(Actor)` in AngelScript):

| Component | Plugin | On |
| --- | --- | --- |
| `UFVIdentityComponent` | FVFramework | any character: links actor <-> `UFVCharacterDefinition` |
| `UFVInventoryComponent`, `UFVEquipmentComponent`, `UFVItemReceiverComponent` | Inventory | player, containers, NPCs |
| `UFVPickupComponent` | Inventory | world items |
| `UFVDialogueParticipantComponent` | Dialogue | speakers: events for lines (anim, lip sync, barks) |
| `UFVAttributeComponent` | Attributes | any character with attributes |
| `UFVScannerComponent` | FVGameplay | player |
| `UFVScannableComponent` | FVGameplay | anything scannable (needs an interactable component too) |

## Key flows

**Talk** (owner's example)
1. Interactable offer `Interaction.Action.Talk` with effect *Activate Ability* (event `Ability.Event.Talk`).
2. `UFVTalkAbility` (AngelScript) reads the target, finds its `UFVDialogueParticipantComponent` and its `Dialogue` flow.
3. `UFVDialogueSubsystem::StartConversation(Dialogue, Player, Npc)` starts the graph as a root flow owned by the NPC.
4. `Line` / `Choice` nodes push lines and choices to the subsystem; UI binds to its events and calls `Advance` / `Choose`.
5. `End Conversation` (or aborting) broadcasts `OnConversationEnded`; the ability ends.

**Scan**
1. Scan input toggles `UFVScannerComponent::SetScanMode`. Scannables in range light up (custom depth stencil per category), optional time dilation.
2. The best scannable in view becomes the focus (`OnFocusChanged`). Holding scan fills progress (`Duration`).
3. On completion the definition's `OnScanned` effects run (e.g. *Learn Knowledge*, *Set Fact*), entries whose `VisibleWhen` pass are shown.

**Undercover**
1. Equipping an outfit item adds its `GrantedTags` (e.g. `Disguise.Faction.Police`) to the owner's ASC.
2. `UFVSocialStatics::GetAttitude(ObserverFaction, Actor)` treats a disguised actor as a member unless the notoriety it reads is above `DisguiseNotorietyLimit`.
3. While disguised, the fame and notoriety onlookers read drop to `DisguiseRecognition` of the real value, so a known face can pass unnoticed.
4. Quests and interaction offers require outfits with `Is Wearing` / `Has Gameplay Tag` conditions.

**Fame, notoriety and titles**
1. Standing with a faction changes through `Modify Standing`.
2. The change feeds the one global fame (through the faction's `FameContribution`) and the one global notoriety (through its `NotorietyContribution`), so helping criminals makes the player both talked about and wanted.
3. `UFVSocialSubsystem` re-checks every `UFVTitleDefinition` whenever a fact changes and writes the title's fact, which dialogue, quests and barks read like any other fact.
4. Nothing decays on a timer: the player sets the pace.

**Carrying things**
1. Each item has a weight and a grid footprint; one stack claims its footprint once.
2. Pockets (`BasePocketCells`) plus the container fragments of equipped bags and backpacks give the space; the base limit plus their `WeightBonus` gives the weight allowance.
3. The game layer adds allowance from attributes with `SetBonuses`, which keeps the inventory plugin independent of the attribute plugin.
4. A pickup that does not fit is refused, or takes only part of the pile and leaves the rest in the world.

**Linear and non-linear story**
- Linear: the main story Flow graph (world settings root flow) runs chapters with Sub Graph nodes and `Set Quest State`.
- Non-linear: quests auto-start from conditions on facts; each quest can own a Flow graph that runs while it is active. `Wait For Conditions` holds a branch until the world state allows it.
- Everything is in facts, so any graph can test any outcome.

## Conventions

- No `UINTERFACE` exposed to Blueprint/AngelScript. Plugins talk through components with multicast delegates, facts, tags and instanced condition/effect structs.
- New data types build on `UFVDefinition`; extensions are fragments, not subclasses.
- Every plugin has three modules: runtime, `*Debug` (console variables, console commands, on-screen readouts) and `*Editor` (settings entry in the FlickerVoid menu, validation).
- Extension points are open without editing existing code: new attributes, items, factions, titles, scans and checks are new data assets, and `UFVScriptCondition` / `UFVScriptEffect` let AngelScript add rules that plug into the same condition sets the offers use.
- Comments minimal.

## Where a setting belongs

| Mechanism | Use it for | Examples |
| --- | --- | --- |
| **Gameplay tags** | identity and classification that data assets and conditions match on | `Equipment.Slot.Coat`, `Item.Outfit`, `Disguise.Faction.Police`, `Scan.Category.*`, fact tags |
| **Facts** | world and player state that must save and be testable | faction standing, fame, notoriety, titles, quest state, knowledge |
| **Config (`UDeveloperSettings`)** | per-project defaults a designer tunes once, under the FlickerVoid settings category | weight and space limits, scan categories and dilation, dialogue timing, default attribute set |
| **Console variables (`FVCvar.*`)** | switches for debugging and playtesting, never for shipping balance | `FVCvar.Inventory.EnforceWeight`, `FVCvar.*.Debug.HUD`, `FVCvar.Interaction.Debug.Draw` |
| **Console commands (`FV.*`)** | one-off actions while testing | `FV.Inventory.Give`, `FV.Quest.Start`, `FV.Social.Notoriety`, `FV.Facts.Set` |

A CVar that changes a rule (such as the two inventory limits) reads `-1` by default and then follows the config value, so the project setting stays the source of truth.

## Answers (owner, 2026-10-05)

1. **Equipment is granular**: head, eyewear, underwear top and bottom, socks, upper body, lower body, coat, gloves, shoes, accessories (head, neck, arm left and right, leg left and right, body), bag, backpack.
2. **Weight and grid**, both switchable: items have weight and a footprint, bags and backpacks grant space, no blind stacking.
3. **One global notoriety**, fed by faction standing. **No decay**: the player sets the pace.
4. Factions keep standing; the global value is **fame** rather than reputation. Standing with a disliked faction raises notoriety, and fame earns **titles** as facts. Changing appearance lowers what onlookers read.
5. **One affinity value** per character, open to extension.
6. Attributes are **data assets** with modifiers and condition-gated checks, so new ones can come from AngelScript.
7. Obsolete code **removed**: world state subsystem, memory subsystem, personality attributes, SUDS leftovers.
8. **Stealth is the next PR** (visibility, hiding spots, suspicion).
9. Flow stays an **external submodule**, pinned on `5.x`.
10. Settings split as in the table above.
11. Every plugin has a **debug module** and an **editor module**.
12. Settings follow the `FVInteractionSystem` pattern: a `UDeveloperSettings` in the FlickerVoid category with an entry in the FlickerVoid editor menu.

## Still open

- 2D grid placement: space is counted in cells rather than packed into a layout. Real placement (rotation, per-slot grids) can come later if the game needs it.
- Outfit meshes are not applied visually yet; the equipment component broadcasts and the game layer dresses the character.
- Quest flow graphs restart from their beginning after a load.
