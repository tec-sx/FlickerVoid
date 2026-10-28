# FlickerVoid

Third-person narrative RPG/adventure (references: Witcher 3, Tomb Raider, Cyberpunk 2077's scanner) and the reusable game framework behind it. Little combat; survival, sneaking, hiding and going undercover; factions, fame, notoriety, relationships; outfits instead of armor; linear and non-linear storylines. The game is player-paced: no decay timers or forced timings.

The project builds against the owner's custom Unreal Engine fork with AngelScript. Builds and tests happen locally; don't assume a stock engine.

Full design: `Docs/Framework/FrameworkDesign.md`. Per-plugin setup guides: `Docs/Setup/` (keep them in sync when a plugin's assets, settings or components change).

## Layers

Functional layers stack vertically and dependencies only point down: Content -> AngelScript -> game module -> plugins -> FVFramework. The ontology domains (see Folders and naming) cut across all layers; domains never call each other and meet only through definitions, tags, facts and conditions/effects.

1. **Plugins** (`Plugins/FV*System`, FVFramework): feature-based, generic building blocks only. Nothing game-specific. FVFramework is the ontological "Game": it provides the blocks for Gameplay, Interface and Presentation.
2. **Game module** (`Source/FlickerVoid`, with `FlickerVoidDebug` and `FlickerVoidEditor`): C++ only where AngelScript can't do the job (see below).
3. **AngelScript** (`Script/`): most of the game: actors, characters, game modes, abilities, rules, widget logic.
4. **Content**: data assets, meshes, textures, materials, FX, Blueprint world actors, Widget Blueprints. Blueprints add only looks and layout on top of C++ or AngelScript classes; no settings beyond occasional overrides. Character looks (skeletal mesh, animation, physics asset) are set on the character Blueprint so they can be tweaked in its viewport; data assets hold gameplay data. Looks that must come from data (outfits, variants) are applied in the construction script or a component's `OnRegister`, which also run in the Blueprint editor viewport, and the data asset is set on the Blueprint's defaults.

Refactor in progress: the old `Source/FV*` modules still exist and fold into `FlickerVoid` as described in the design doc's Module structure; until then the scanner lives in `Source/FVGameplay/.../Scanner`. Much older code in `Source/` is experimental or obsolete; don't assume it should be kept.

## AngelScript and C++

- C++ in the game module is only **Setup** (game mode base, game state, player state, player controller with Enhanced Input forwarding, game data, settings), **Shapes** (components, definitions, data asset types, the fragments, conditions and effects C++ systems need, runtime item data) and **Hosts** (what AngelScript can't create: Flow nodes, struct StateTree tasks, rig units, anim instances, custom Mover input and layered move data). Everything else is AngelScript.
- **Components are always C++** (plugin or game module). AngelScript composes actors from them and never declares components.
- AngelScript has no struct inheritance and can't implement interfaces. So fragments, conditions and effects are instanced UObjects (`UFVFragment`, `UFVCondition`, `UFVEffect`: read-only and shared by everything that uses the asset), which AngelScript subclasses directly; runtime data (item stacks, instance data) stays in C++ structs; and C++ never casts an actor to an interface (engine ones such as `IGameplayTagAssetInterface` included), it finds a component.
- C++ never names an AngelScript class. It reaches script-built things by **Find** (a C++ component on the actor), **Spawn** (a class picked in data or config, typed as a C++ base: Default Pawn Class, ability sets, attribute sets) or **Host** (an instanced Blueprintable C++ base that AngelScript subclasses: `UFVFragment`, `UFVCondition`, `UFVEffect`, Mover movement modes, Blueprint-base StateTree tasks).
- AngelScript calls plugin APIs directly; the game module doesn't wrap them.

## Folders and naming

- `Script/` follows the ontology tree (design doc, Ontology): Game -> Interface, Gameplay (Protagonist, World, Narrative), Presentation, three children per node. Files at the root of `Script/` belong to Game itself (game modes, player controller).
- `Source/FlickerVoid/Public` and `Private` mirror the same paths, so a feature's C++ half sits at the same path as its script half. The C++ tree stays sparse.
- Each node is a **folder** (inner node with code below it), a **class** (leaf with behaviour) or **data** (a tag, a definition or a fact). Don't create empty folders or classes to complete the tree.
- Code used by several nodes lives at their lowest common ancestor (character movement, used by protagonist and residents, goes in `Gameplay/`).
- C++ classes keep the `FV` prefix (`UFVTraversalComponent`). AngelScript classes drop it (`ADoor`, `AStoryGameMode`, `UTradeEffect`), and each `.as` file is named after its main class without the type prefix (`Door.as`, `StoryGameMode.as`).

## Plugins

| Plugin | Purpose |
|---|---|
| FVFramework | Core: `UFVDefinition`, fragments, facts DB, conditions/effects, identity, Flow nodes, debug HUD base (FVCoreDebug), editor helpers (FVCoreEditor) |
| FVAttributeSystem | Definitions for GAS attributes (attribute sets in AngelScript, Gameplay Effects change them), checks |
| FVInventorySystem | Items, weight + grid limits, granular outfit slots, pickup |
| FVDialogueSystem | Dialogue on FlowGraph |
| FVStorySystem | Quests, knowledge |
| FVSocialSystem | Factions, standing, fame, notoriety, titles, disguise, affinity |
| FVInteractionSystem | Interaction offers, registry subsystem, focus |
| FVNavigationSystem | Maps and layers, map markers, discovery, waypoint and tracking, minimap/world map/compass view logic, map capture tool |
| Flow | FlowGraph (MothCocoon), external git submodule; don't edit |

Every plugin has three modules: runtime, `<Plugin>Debug` and `<Plugin>Editor`. FVInteractionSystemEditor is the reference pattern.

## Conventions

- **Never expose interfaces to Blueprint/AngelScript.** Put a component with multicast delegates on the actor; callers find the component and broadcast. C++-only internal interfaces are OK when truly needed.
- **Never modify, move, delete or create anything in Content folders** (project or plugin, any `.uasset`/`.umap`). Scan read-only and report each asset's path and problem; the owner handles it.
- Definitions (items, characters, attributes, factions, titles...) derive from `UFVDefinition` (FVCoreRuntime) and extend through instanced `UFVFragment` objects (being migrated from `TInstancedStruct`; see the design doc). `UFVItemDataAsset` is outdated.
- **Rule of three**: the game is split in threes, at most 3 of anything (HUD sections, hub pages, page sections, menu entries, dialogue choices + cancel). Data-driven lists that must stay small get `meta = (MaxEntries = 3)`; the FVCoreEditor validator warns above it.
- Data-driven, composition over inheritance, SOLID/KISS/YAGNI, open/closed. Reuse FVFramework before adding base types.
- Rules extend through `FFVConditionSet` / `FFVEffectList`, which hold instanced `UFVCondition` / `UFVEffect` objects; C++ and AngelScript subclass them the same way (being migrated from `TInstancedStruct` and the `UFVScriptCondition` / `UFVScriptEffect` wrapper).
- Plugins stay independent of each other; they meet through facts, gameplay tags, conditions/effects and components on actors.
- Prefer FlowGraph (custom nodes) for story, quests and dialogue; StateTree and current UE5 features elsewhere.
- Minimal comments: a short class description; comment a method only when its name or logic is unclear.
- Headers list functions before properties: first what the class does, then what it needs to work. Short descriptions of UFUNCTIONs and UPROPERTYs go in `meta = (ToolTip = "...")`, not in comments.
- Enum values are described with `UMETA(...)`, not comments, and the UENUM gets `meta = (ScriptName = "...")` without the `EFV` prefix (`EFVMapViewMode` is `MapViewMode`).
- Log categories are `Log<PluginName>`, declared in the module header and defined in the module `.cpp`; reuse a plugin's category before adding one, and never declare the same name in two modules.
- Follow engine naming: Blueprint function libraries are `UFV<Feature>Library` (like `UKismetMathLibrary`), never shorthand such as BFL.

### Where a setting belongs

- **Gameplay tags**: identity and classification (slots, categories, disguises, scan categories, fact tags).
- **Facts** (`UFVFactDatabase`): saved state (standing, fame, notoriety, titles, quest state, knowledge).
- **Config**: `UDeveloperSettings` in the `FlickerVoid` category, registered in the plugin's Editor module with `FFVCoreEditorModule::AddSettingsMenuAction`.
- **CVars** `FVCvar.<System>.Debug.*` and commands `FV.<System>.<Verb>`: debugging only, in the Debug module. A CVar overriding a rule defaults to -1, meaning "use config".

## Repo gotchas

- `.gitignore` ignores `Plugins/*`; each new plugin needs a `!Plugins/<Name>` line or its files are silently skipped.
- Renames need CoreRedirects in `Config/DefaultEngine.ini` and tag redirects in `Config/DefaultGameplayTags.ini`.

## Git

- Branches: `feature/<title>`, `improvement/<title>` or `fix/<title>`.
- PRs open ready for review; the owner reviews and may push to the branch.
