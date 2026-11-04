# FlickerVoid

A third-person, single-player narrative RPG/adventure, and the reusable game framework behind it. There is little combat: the game is about survival, sneaking, hiding and going undercover, factions, fame, notoriety and relationships, and outfits instead of armor. Storylines can be linear or non-linear, and the game is player-paced, with no decay timers or forced timings.

## Story

The story follows a **young adult woman suffering from total memory loss**.

The central mystery revolves around:
- who she is
- who she was
- who she may become

Story progression is driven by:
- character interactions
- dialogue and communication
- information gathering
- infiltration and social manipulation
- performing morally ambiguous or “shady” errands to earn trust, access, or resources

## How it is built

- **FVFramework** and the FV plugins (interaction, attributes, inventory, dialogue, story, navigation) hold generic building blocks: definitions with fragments, facts, conditions and effects, scales.
- The **game modules** in `Source/` hold the C++ the game needs and AngelScript can't do; **AngelScript** in `Script/` holds most of the game; **Content** adds looks and layout.

Read on:

- [Framework design](Docs/Framework/FrameworkDesign.md): layers, ontology, module structure and the decisions behind them.
- [Setup guides](Docs/Setup/README.md): what to create and configure for each plugin and game system.
- [CLAUDE.md](CLAUDE.md): coding rules and conventions, for people and AI assistants alike.

## Building

The project builds against the owner's custom Unreal Engine fork with AngelScript; a stock engine won't open it. Generate project files from the `.uproject` with that engine, then build the `FlickerVoidEditor` target.
