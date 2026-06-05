# FVoid Story System

A comprehensive narrative framework for tracking quests, psychological state, memories, and world progression.

## Overview

The story system manages:

- Quests: Main story, side quests, rehabilitation quests
- Psychological State: Sanity, addiction, withdrawal
- Memory System: Identity recovery through discovered fragments
- World State: Persistent flags, NPC relationships, consequences

## Architecture

See flowcharts:

- [Quest System Architecture](FC_StorySystem_QuestSystemArchitecture.md)
- [Quest Lifecycle](FC_StorySystem_QuestLifecycle.md)
- [Quest State Machine](FC_StorySystem_QuestStateMachine.md)
- [Objective Types](FC_StorySystem_ObjectiveTypes.md)
- [Psychological System](FC_StorySystem_PsychologicalSystem.md)
- [Sanity Flow](FC_StorySystem_SanityFlow.md)
- [Memory System](FC_StorySystem_MemorySystem.md)
- [Memory Clarity](FC_StorySystem_MemoryClarity.md)
- [World State System](FC_StorySystem_WorldStateSystem.md)
- [Progression Blockers](FC_StorySystem_ProgressionBlockers.md)

## Quest System

### Quest Definition (UFVoidQuestDefinition)

Data asset defining quest structure. Created in Content Browser as a Primary Data Asset.

### Quest Types

| Tag | Description |
|-----|-------------|
| Quest.Type.MainStory | Critical path quests |
| Quest.Type.Side | Optional side content |
| Quest.Type.Memory | Memory recovery quests |
| Quest.Type.Rehabilitation | Addiction recovery quests |
| Quest.Type.Errand | Simple fetch/delivery tasks |
| Quest.Type.Social | Relationship building |

### Quest States

| Tag | Description |
|-----|-------------|
| Quest.State.Unavailable | Prerequisites not met |
| Quest.State.Available | Can be started |
| Quest.State.Active | Currently in progress |
| Quest.State.Completed | Successfully finished |
| Quest.State.Failed | Failed permanently |
| Quest.State.Abandoned | Abandoned by player |

### Quest Prerequisites (FFVoidQuestPrerequisite)

| Field | Type | Description |
|-------|------|-------------|
| RequiredCompletedQuests | Array | Must finish these first |
| RequiredWorldStateTags | TagContainer | World state requirements |
| BlockingWorldStateTags | TagContainer | Cannot start if present |
| MinimumSanity | Float 0-1 | Sanity threshold |
| bRequiresCleanState | Bool | No active addiction |
| RequiredNPCTrust | Map | NPC trust thresholds |
| RequiredMemories | Array | Memory prerequisites |

### Quest Objectives

| Objective Type | Class | Description |
|----------------|-------|-------------|
| Talk To | UFVoidObjective_TalkTo | Speak with specific NPC |
| Obtain Item | UFVoidObjective_ObtainItem | Acquire items |
| Reach Location | UFVoidObjective_ReachLocation | Visit a location |
| Reach Threshold | UFVoidObjective_ReachThreshold | Achieve attribute or relationship level |
| Wait Time | UFVoidObjective_WaitTime | Pass game time |
| Maintain State | UFVoidObjective_MaintainState | Stay in a state for duration |

### Quest Consequences (FFVoidQuestConsequence)

Applied on quest completion or failure.

| Field | Type | Description |
|-------|------|-------------|
| WorldStateTags | TagContainer | Tags to add to world state |
| GameplayEffects | Array | Effects to apply to protagonist |
| RelationshipChanges | Map | NPC relationship deltas |
| MemoriesToUnlock | Array | Memories to discover |
| QuestsToUnlock | Array | Quests to make available |
| QuestsToBlock | Array | Quests to permanently fail |
| SanityImpact | Float -1 to 1 | Sanity change |

## Memory System

### Memory Fragment (UFVoidMemoryFragment)

Data asset representing recovered memories.

| Field | Type | Description |
|-------|------|-------------|
| MemoryId | FName | Unique identifier |
| Title | FText | Display title |
| Content | FText | Memory description |
| MemoryType | Tag | Category of memory |
| InitialClarity | Tag | Starting clarity level |
| IdentityContribution | Float 0-1 | Progress toward identity recovery |
| SanityImpact | Float -1 to 1 | Can be negative for trauma |
| GrantedWorldStateTags | TagContainer | Tags granted on discovery |
| UnlockedQuests | Array | Quests made available |
| MinimumSanityForClarity | Float 0-1 | Sanity needed for full recall |
| PrerequisiteMemories | Array | Memories needed first |

### Memory Types

| Tag | Description |
|-----|-------------|
| Memory.Type.Identity | Core identity memories |
| Memory.Type.Relationship | People from the past |
| Memory.Type.Skill | Learned abilities |
| Memory.Type.Trauma | Painful memories (negative sanity) |
| Memory.Type.Location | Familiar places |

### Memory Clarity

| Tag | Description |
|-----|-------------|
| Memory.Clarity.Fragment | Unclear, partial images |
| Memory.Clarity.Partial | Some details visible |
| Memory.Clarity.Clear | Full memory recalled |

## World State System

### UFVoidWorldStateSubsystem

Tracks persistent world state across the game.

### World Tags API

| Method | Description |
|--------|-------------|
| AddWorldStateTag | Add a persistent flag |
| RemoveWorldStateTag | Remove a flag |
| HasWorldStateTag | Check if flag exists |
| HasAnyWorldStateTags | Check if any tags match |
| HasAllWorldStateTags | Check if all tags match |

### NPC Relationships API

Scale: -1.0 (hostile) to 1.0 (trusted ally)

| Method | Description |
|--------|-------------|
| SetNPCRelationship | Set absolute value |
| ModifyNPCRelationship | Add delta to current |
| GetNPCRelationship | Get current value |

### Location Discovery API

| Method | Description |
|--------|-------------|
| DiscoverLocation | Mark location as visited |
| HasDiscoveredLocation | Check if visited |
| GetDiscoveredLocations | Get all visited |

### Time Tracking API

| Method | Description |
|--------|-------------|
| AdvanceGameTime | Add hours to game time |
| GetCurrentGameTime | Total hours elapsed |
| GetCurrentDay | Current day number |
| GetTimeOfDay | Hour within current day |

## Progression Blockers

| Blocker Tag | Condition |
|-------------|-----------|
| Blocker.Sanity.TooLow | Sanity below quest threshold |
| Blocker.Addiction.Active | In active addiction state |
| Blocker.Trust.Insufficient | NPC trust too low |
| Blocker.Memory.Required | Missing prerequisite memory |
| Blocker.Rehabilitation.Required | Must complete rehab quest |

## Key Classes Reference

| Class | Location | Purpose |
|-------|----------|---------|
| UFVoidQuestDefinition | Narrative/Quest/ | Quest template data asset |
| UFVoidQuestInstance | Narrative/Quest/ | Runtime quest state |
| UFVoidQuestObjective | Narrative/Quest/ | Objective base class |
| UFVoidQuestSubsystem | Systems/ | Quest management |
| UFVoidWorldStateSubsystem | Systems/ | World state and relationships |
| UFVoidMemoryFragment | Narrative/Memory/ | Memory data asset |

## Designer Workflow

### Creating a Quest

1. Right-click in Content Browser, select Miscellaneous then Data Asset
2. Select UFVoidQuestDefinition
3. Fill in identity fields: QuestId, DisplayName, Description
4. Set QuestType tag
5. Configure prerequisites
6. Add objectives as instanced subobjects
7. Define completion and failure consequences
8. Create associated dialogue in CSV tables

## See Also

- Dialogue System Documentation: ../DialogueSystem/README.md
- Ability System Documentation: ../AbilitySystem/README.md
