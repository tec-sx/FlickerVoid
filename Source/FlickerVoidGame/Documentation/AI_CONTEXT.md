# FlickerVoid Project Context

Purpose: This document provides comprehensive context for AI assistants working on this codebase.

## Project Overview

FlickerVoid is a AAA-style, third-person, story-driven single-player game built with Unreal Engine 5.7.

| Aspect | Detail |
|--------|--------|
| Language | Primarily C++, Blueprints for content assembly only |
| Focus | Narrative progression, psychological systems, player choice |
| Combat | NOT a primary mechanic |
| Progression | State-based (sanity, trust, memories), not power-based |
| Multiplayer | Single-player only, no replication code |

## Core Narrative

The protagonist is a young adult woman suffering from total memory loss. The central mystery is who she is, who she was, and who she may become.

Story progression through:
- Character interactions and dialogue
- Information gathering and infiltration
- Morally ambiguous tasks to earn trust
- Personal struggles with sanity and addiction

## Technical Architecture

### Engine and Framework

| Component | Detail |
|-----------|--------|
| Engine | Unreal Engine 5.7 |
| Base | Modified Lyra Starter Game |
| Attributes | Gameplay Ability System (GAS) |
| Managers | GameInstanceSubsystems |
| Content | DataTables with CSV import |

### Core Subsystems

| Subsystem | Class | Purpose |
|-----------|-------|---------|
| Quests | UFVoidQuestSubsystem | Quest lifecycle management |
| World State | UFVoidWorldStateSubsystem | Tags, relationships, time |
| Dialogue | UFVoidDialogueSubsystem | DRS-like dialogue playback |

### Character Architecture

| Class | Purpose |
|-------|---------|
| AFVoidCharacter | Base character with MoverComponent |
| UFVoidProtagonistComponent | Player-only input handling |
| UCharacterAttributeComponent | Bridge between data assets and GAS |
| UFVoidCharacterDataAsset | Designer-editable character profile |
| AFVoidPlayerState | Owns the AbilitySystemComponent |

### Attribute Sets

| Class | Attributes |
|-------|------------|
| UFVoidCharacterAttributeSet | Social, Personality, Mood, Physical |
| UFVoidProtagonistAttributeSet | Sanity, Addiction, Memory/Identity |

## Gameplay Tags Namespaces

| Namespace | Purpose |
|-----------|---------|
| Ability_* | Ability system |
| InputTag_* | Input mapping |
| InitState_* | Component initialization |
| Status_* | Character status |
| Movement_Mode_* | Movement states |
| Character_State_* | AI states |
| Character_Mood_* | Mood tags |
| Character_Trait_* | Personality traits |
| Quest_* | Quest system |
| Objective_* | Objective types |
| Sanity_* | Psychological state |
| Addiction_* | Addiction system |
| Substance_* | Substance types |
| Memory_* | Memory system |
| WorldState_* | World flags |
| Consequence_* | Effect types |
| Blocker_* | Progression gates |

## File Structure

### AbilitySystem

| File | Purpose |
|------|---------|
| FVoidAbilitySystemComponent | Core ASC |
| FVoidAbilitySet | Ability granting data asset |
| Attributes/FVoidCharacterAttributeSet | Base character attributes |
| Attributes/FVoidProtagonistAttributeSet | Player psychological attributes |

### Character

| File | Purpose |
|------|---------|
| FVoidCharacter | Base character class |
| FVoidProtagonistComponent | Player input component |
| FVoidPawnData | Pawn configuration |
| FVoidCharacterDataAsset | Character profile data asset |
| CharacterAttributeComponent | GAS bridge component |

### Narrative

| File | Purpose |
|------|---------|
| Dialogue/FVoidDialogueTypes | DataTable row structs |
| Dialogue/FVoidDialogueConditionEvaluator | Condition parsing |
| Dialogue/FVoidDialogueQuery | Line matching |
| Quest/FVoidQuestDefinition | Quest template |
| Quest/FVoidQuestInstance | Runtime state |
| Quest/FVoidQuestObjective | Objective classes |
| Memory/FVoidMemoryFragment | Memory data asset |

### Systems

| File | Purpose |
|------|---------|
| FVoidQuestSubsystem | Quest management |
| FVoidWorldStateSubsystem | World state tracking |
| FVoidDialogueSubsystem | Dialogue playback |
| FVoidAssetManager | Asset loading |
| FVoidGameData | Global game data |

### Player

| File | Purpose |
|------|---------|
| FVoidPlayerState | Owns ASC, persistent player data |
| FVoidPlayerController | Input processing |

### GameModes

| File | Purpose |
|------|---------|
| FVoidStoryGameMode | Story-focused gameplay |
| FVoidExploreGameMode | Exploration gameplay |
| FVoidGameState | Game state management |

## Design Patterns

| Pattern | Usage |
|---------|-------|
| GameInstanceSubsystem | Quest, WorldState, Dialogue managers |
| Primary Data Asset | Quest definitions, Memory fragments, Character data |
| DataTable CSV | Dialogue content, conditions, speakers |
| Gameplay Tags | All state tracking, conditions, effects |
| Gameplay Ability System | Attributes, effects, abilities |
| Component Architecture | Protagonist features as components |

## Naming Conventions

| Prefix | Usage |
|--------|-------|
| FVoid | Project-specific UClasses |
| FFVoid | Project-specific UStructs |
| EFVoid | Project-specific Enums |
| FC_ | Flowchart documentation files |
| EG_ | Example data files |

## Important Rules

### Single-Player Focus

No replication code anywhere. Remove any:
- OnRep_* functions
- GetLifetimeReplicatedProps overrides
- ReplicatedUsing specifiers
- DOREPLIFETIME macros

### GAS Attribute Pattern

Always use FGameplayAttributeData with ATTRIBUTE_ACCESSORS macro.

### Dialogue Inline Formats

Condition format: Type:Key:Comparison:Value

Effect format: Type:Key:Value

## Common Tasks

### Adding a New Attribute

1. Add FGameplayAttributeData property to attribute set header
2. Add ATTRIBUTE_ACCESSORS macro below property
3. Handle clamping in PreAttributeChange
4. Handle side effects in PostGameplayEffectExecute

### Adding a New Quest

1. Create UFVoidQuestDefinition data asset
2. Configure prerequisites in FFVoidQuestPrerequisite
3. Add objectives (subclasses of UFVoidQuestObjective)
4. Define consequences in FFVoidQuestConsequence
5. Create dialogue in CSV tables

### Adding New Dialogue

1. Add rows to dialogue CSV files
2. Use inline conditions/effects format
3. Import as DataTable in editor
4. Test with SendSignal in gameplay

## Documentation

| Document | Location |
|----------|----------|
| Dialogue System | Documentation/DialogueSystem/README.md |
| Story System | Documentation/StorySystem/README.md |
| Ability System | Documentation/AbilitySystem/README.md |
| Example CSVs | Documentation/DialogueSystem/EG_*.csv |

Last Updated: February 2026