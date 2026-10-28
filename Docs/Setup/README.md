# FlickerVoid plugin setup

How to start using each FlickerVoid plugin: what to enable, which data assets and Blueprints to make, which components go on which actors, and which settings to fill in.

Read [FVFramework](FVFramework.md) first; every other plugin builds on its definitions, facts and conditions.

| Guide | You get |
|---|---|
| [FVFramework](FVFramework.md) | Definitions, facts, conditions and effects, characters, UI layout, world clock, cinematics, spawners, Flow and StateTree nodes, debug HUD |
| [FVInteractionSystem](FVInteractionSystem.md) | Look-at interaction: offers, prompts, focus indicator, highlight, locks and keys, doors |
| [FVAttributeSystem](FVAttributeSystem.md) | Attribute definitions over GAS attributes, Gameplay Effect conditions/effects, skill checks |
| [FVInventorySystem](FVInventorySystem.md) | Items, carrying (weight and space), Keys and Quest list, money, outfits and equipment slots, pickups, loot tables |
| [FVDialogueSystem](FVDialogueSystem.md) | Conversations and barks on FlowGraph |
| [FVStorySystem](FVStorySystem.md) | Quests, knowledge (clues, topics, thoughts, documents), save and load |
| [FVSocialSystem](FVSocialSystem.md) | Factions, standing, fame, notoriety, titles, relationships, gifts, disguise |
| [FVNavigationSystem](FVNavigationSystem.md) | Maps, map markers, discovery, waypoint, minimap/compass/world map logic, map capture |

Game layer guides (the game modules and AngelScript built on the plugins):

| Guide | You get |
|---|---|
| [Items](Items.md) | Item types, world items, notes and investigation, money, loot containers, keys, item abilities and animations, held and thrown items, intoxication, trading, pickpocketing, giving from dialogue |

## Shared conventions

- **Settings** live in *Project Settings > FlickerVoid > (plugin)*. The *FlickerVoid* button in the level editor toolbar and the *FlickerVoid* main menu open them directly.
- **Definitions** are data assets of a `UFVDefinition` subclass (right-click in the Content Browser > *Miscellaneous > Data Asset*, then pick the class). Every definition has an **Id** tag, **Display** info (name, descriptions, icon, tint) and **Tags**. The Id is optional, except for definitions that store their state in the fact it names (quests, knowledge, factions, titles); pick those Ids under `Fact.`.
- **Fragments** add optional data to a definition: add an entry to its *Fragments* array and pick the fragment type.
- **Conditions and effects** (`FFVConditionSet`, `FFVEffectList`) appear wherever a rule is needed (offers, quests, items, markers...). Every plugin adds its own types to the same pickers.
- **Components, not interfaces**: systems find a component on an actor and call it or listen to its events. In AngelScript use `UFVSomethingComponent::Get(Actor)`.
- **AngelScript names**: a static library such as `UFVSocialLibrary` is called as `FVSocial::Function(...)`, and a hidden `WorldContext` argument is filled in automatically.
- **Debugging**: every plugin has `FVCvar.<System>.Debug.HUD` (an on-screen readout) and `FV.<System>.<Verb>` console commands.
- **Naming used in these guides**: `DA_` data assets, `BP_` actor Blueprints, `WBP_` widgets, `DT_` data tables, `FA_` Flow assets. Use whatever suits you; nothing depends on the names.

## Typical order for a new project

1. FVFramework: UI layout, HUD, fact tags, a player character definition.
2. FVInteractionSystem: interactor on the player, a first interactable.
3. FVInventorySystem + FVAttributeSystem: items and the player's attributes.
4. FVDialogueSystem + FVStorySystem: a talkable NPC, a quest, some knowledge.
5. FVSocialSystem: factions and the player's reputation.
6. FVNavigationSystem: map images, markers, minimap and compass.
7. Items: item settings, interactables and the player's item abilities.
