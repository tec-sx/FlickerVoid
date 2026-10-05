# FlickerVoid Framework Design

Status: proposal, implemented in `feature/framework-dev` (phase 1). Open questions are at the end.

## Goals

- Third-person narrative RPG/adventure (Witcher 3, Tomb Raider, Cyberpunk 2077 scanner). Little combat.
- Survival with sneaking, hiding and going undercover. Factions, reputation, notoriety, relationships.
- Outfits instead of armor; quests can require outfits; story-driven customization.
- Linear and non-linear storylines. Data-driven, AngelScript-friendly (no exposed interfaces), composition over inheritance.

## What AAA references teach (and how we apply it)

| Reference | Practice | Here |
| --- | --- | --- |
| CDPR (Witcher 3, Cyberpunk) | One **facts DB** is the single source of truth for story state. Quest graphs are signal graphs with phases, conditions and *pause until condition* nodes. | `UFVFactDatabase` (exists). Flow graphs for quests/dialogue, `FV Wait For Conditions` node = Witcher's Pause node. Quests, knowledge, reputation, relationships, notoriety all live in facts, so saving and conditions come for free. |
| CDPR scanner / Witcher senses / Tomb Raider instinct | A mode that highlights categories of objects through post-process stencils, shows data on focus and turns findings into clues. Time slows while scanning. | `FVScannerSystem`: registry of scannables, category stencils, focus by view alignment, timed scan, effects on scan (learn knowledge, set facts). Optional time dilation. |
| Hitman | Disguises are tags; some NPCs (enforcers) see through them; suspicion builds rather than flipping. | Outfits grant gameplay tags (`Disguise.Faction.*`). `FVSocialSystem` resolves attitude from standing tier, notoriety and disguise. Suspicion meter is phase 2 (stealth). |
| Lyra / modern UE5 | Item fragments, instanced structs, data assets, GAS, StateTree, modular components. | Fragments on definitions (`TInstancedStruct`), components found on actors instead of interfaces. |

## Layers

```
Plugins (FV*System, feature based)  ->  Game modules (domain based)  ->  AngelScript (game objects)  ->  Blueprints (data + cosmetics)
```

### Plugins

| Plugin | Status | Owns |
| --- | --- | --- |
| **FVFramework** | kept, extended | Facts, conditions/effects, definitions, **fragments**, **character definition + identity component**, world clock, cinematics, spawner, input, UI layout, StateTree nodes. New module **FVCoreFlow**: generic Flow nodes (Branch, Apply Effects, Wait For Conditions, Conditions predicate). |
| **FVInteractionSystem** | kept as is | Interactor/interactable, offers, focus. |
| **FVInventorySystem** | new (replaces the Mountea fork) | Item definition + fragments, inventory, equipment (outfits), pickup and item receiver, item conditions/effects. |
| **FVDialogueSystem** | new (replaces the Yap fork) | Flow-based conversations: Line, Choice, End Conversation, Bark nodes; dialogue subsystem for UI; participant component for animation/voice. |
| **FVStorySystem** | reworked | Quests (+ optional quest Flow graph), knowledge, save. Yap bridge removed; reputation moved to Social. |
| **FVSocialSystem** | new | Factions, standing tiers with attitude, personal reputation, notoriety, relationships, disguise-aware attitude. |
| **FVScannerSystem** | new | Scanner component, scannable component, scan definitions, category highlighting. |
| FVStealthSystem | phase 2 (proposed) | Visibility (light, stance, disguise), hiding spots, suspicion meters for NPCs (StateTree + AI perception). |

Every plugin depends only on FVFramework (and Flow where it has graph nodes). Plugins never depend on each other; they meet through **facts, gameplay tags, conditions/effects and components on actors**.

### Game modules

| Module | Role | Change |
| --- | --- | --- |
| FVCore | logging, tags, flow triggers | kept |
| FVCharacter | locomotion, animation, traversal | kept |
| FVGameplay | GAS: ASC, abilities, attribute sets, checks, interaction ability base | attributes to be reworked (question 6) |
| FVAI | NPC config, StateTree tasks | faction now from FVSocialSystem |
| FVNarrative | narrative glue | memory fragments pending question 7 |
| FVWorld | world objects (doors, hiding spots) | `UFVWorldStateSubsystem` duplicates facts + clock (question 7) |
| FVUI | HUD, journal, dialogue, scanner widgets | kept |
| FVItems | — | **removed**: pickup/receiver/item definition moved to FVInventorySystem |
| FlickerVoidGame | game mode, player, asset manager | kept |

## Data model

```
UFVDefinition (Id tag, Display, Tags)                        FVFramework
 ├─ UFVCharacterDefinition  + Fragments<FFVCharacterFragment> FVFramework
 │     FFVCharacterFragment_Social { Faction, RelationshipFact }      (Social)
 ├─ UFVItemDefinition       + Fragments<FFVItemFragment>      FVInventorySystem
 │     FFVItemFragment_Equippable { Slot, GrantedTags }
 │     FFVItemFragment_Usable     { Effects, bConsume }
 ├─ UFVFactionDefinition    { standing range, tiers->attitude, relations, NotorietyFact, DisguiseTag }  FVSocialSystem
 ├─ UFVScanDefinition       { Category, Duration, Entries, OnScanned effects }  FVScannerSystem
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
| `UFVScannerComponent` | Scanner | player |
| `UFVScannableComponent` | Scanner | anything scannable |

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
2. `UFVSocialStatics::GetAttitude(ObserverFaction, Actor)` treats a disguised actor as a member unless the faction notoriety is above its `DisguiseNotorietyLimit`.
3. Quests and interaction offers require outfits with `Is Wearing` / `Has Gameplay Tag` conditions.

**Linear and non-linear story**
- Linear: the main story Flow graph (world settings root flow) runs chapters with Sub Graph nodes and `Set Quest State`.
- Non-linear: quests auto-start from conditions on facts; each quest can own a Flow graph that runs while it is active. `Wait For Conditions` holds a branch until the world state allows it.
- Everything is in facts, so any graph can test any outcome.

## Conventions

- No `UINTERFACE` exposed to Blueprint/AngelScript. Plugins talk through components with multicast delegates, facts, tags and instanced condition/effect structs.
- New data types build on `UFVDefinition`; extensions are fragments, not subclasses.
- Comments minimal.

## Open questions (recommendation first)

1. Outfit granularity: **one full-body outfit slot plus accessory slots** (head, eyes, bag), or separate top/bottom/shoes?
2. Inventory limits: **none (categories only)**, weight, or grid?
3. Notoriety: **per faction**, or one global wanted level? Should it decay with game time (**yes**)?
4. "Solo" reputation: **one personal renown value independent of factions**?
5. Relationships: **one affinity value per character**, or affinity plus trust?
6. Attributes: replace the personality set with **Perception, Stealth, Deception, Persuasion, Intimidation, Streetwise, Tinkering, Athletics, Composure** plus Health, Stamina, Stress?
7. Remove obsolete code: FVWorldStateSubsystem, FVMemorySubsystem (amnesia/sanity/addiction), personality attributes, SUDS leftovers?
8. Stealth system (visibility, hiding, suspicion) as the **next PR**?
9. Flow plugin: add it back as a pinned submodule (**yes**) or keep it local only?
