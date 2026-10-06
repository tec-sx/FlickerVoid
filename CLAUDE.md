# FlickerVoid

Third-person narrative RPG/adventure (references: Witcher 3, Tomb Raider, Cyberpunk 2077's scanner) and the reusable game framework behind it. Little combat; survival, sneaking, hiding and going undercover; factions, fame, notoriety, relationships; outfits instead of armor; linear and non-linear storylines. The game is player-paced: no decay timers or forced timings.

The project builds against the owner's custom Unreal Engine fork with AngelScript. Builds and tests happen locally; don't assume a stock engine.

Full design: `Docs/Framework/FrameworkDesign.md`.

## Layers

1. **Plugins** (`Plugins/FV*System`, FVFramework): feature-based, generic building blocks only. Nothing game-specific.
2. **Game modules** (`Source/FV*`): domain-based (World, Narrative, Gameplay, ...), core setup and RPG blocks for this game. The scanner lives here (`Source/FVGameplay/.../Scanner`).
3. **AngelScript**: game-specific reusable objects.
4. **Blueprints**: data assets and cosmetics only. No settings in Blueprints beyond occasional overrides.

Much older code in `Source/` is experimental or obsolete; don't assume it should be kept.

## Plugins

| Plugin | Purpose |
|---|---|
| FVFramework | Core: `UFVDefinition`, fragments, facts DB, conditions/effects, identity, Flow nodes, debug HUD base (FVCoreDebug), editor helpers (FVCoreEditor) |
| FVAttributeSystem | Data-defined attributes, modifiers, checks |
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
- Definitions (items, characters, attributes, factions, titles...) derive from `UFVDefinition` (FVCoreRuntime) and extend through `TInstancedStruct` fragments. `UFVItemDataAsset` is outdated.
- Data-driven, composition over inheritance, SOLID/KISS/YAGNI, open/closed. Reuse FVFramework before adding base types.
- Rules extend through `FFVConditionSet` / `FFVEffectList`; AngelScript adds new ones via `UFVScriptCondition` / `UFVScriptEffect`.
- Plugins stay independent of each other; they meet through facts, gameplay tags, conditions/effects and components on actors.
- Prefer FlowGraph (custom nodes) for story, quests and dialogue; StateTree and current UE5 features elsewhere.
- Minimal comments: a short class description; comment a method only when its name or logic is unclear.
- Log categories are `Log<PluginName>`; never reuse names from FVCore's `FVLogCategories.h`.

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
