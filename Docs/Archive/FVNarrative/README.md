# FVoid Story System

A comprehensive narrative framework for tracking quests, psychological state, memories, and world progression.

## Overview

The story system manages:

- Quests: Main story, side quests, rehabilitation quests
- Psychological State: Sanity, addiction, withdrawal
- Memory System: Identity recovery through discovered fragments
- World State: Persistent flags, NPC relationships, consequences

## Architecture

Quest and chapter flow: [Quest/Flow/README.md](../Public/Quest/Flow/README.md)

See flowcharts:

- [Psychological System](FC_StorySystem_PsychologicalSystem.md)
- [Sanity Flow](FC_StorySystem_SanityFlow.md)
- [Memory System](FC_StorySystem_MemorySystem.md)
- [Memory Clarity](FC_StorySystem_MemoryClarity.md)
- [Progression Blockers](FC_StorySystem_ProgressionBlockers.md)

## Quest System

Quests and chapters are authored as Flow graphs backed by the fact database, not as data assets or C++ subsystems. See [Quest/Flow/README.md](../Public/Quest/Flow/README.md) for the fact layout, node catalogue, and authoring examples.

### Quest Types

| Tag | Description |
|-----|-------------|
| Quest.Type.MainStory | Critical path quests |
| Quest.Type.Side | Optional side content |
| Quest.Type.Memory | Memory recovery quests |
| Quest.Type.Rehabilitation | Addiction recovery quests |
| Quest.Type.Errand | Simple fetch/delivery tasks |
| Quest.Type.Social | Relationship building |

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

World state, NPC relationships, location discovery, and time are tracked as facts, not through a dedicated subsystem. See `FVWorldTags` (`Fact.World.*`, `Fact.Location.*`) and the fact API on `UFVFactSubsystem`.

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
| UFVQuestFactHelpers | Narrative/Quest/ | Builds chapter/quest/objective fact tags |
| UFVFactSubsystem | Core/FactDB/ | Stores and evaluates all facts |
| UFVoidMemoryFragment | Narrative/Memory/ | Memory data asset |

## Designer Workflow

### Creating a Quest

1. Add the chapter and every quest tag to `Config/Tags/Facts/Quests/<ChapterId>.ini`
2. Author the chapter's Flow graph, gating on required quests only
3. Author each quest's Flow graph using Advance Objective, Set Quest Stage, and Wait For Fact Condition
4. Attach Set Fact AddOns for outcomes
5. Create associated dialogue in CSV tables

See [Quest/Flow/README.md](../Public/Quest/Flow/README.md) for the full authoring checklist.

## See Also

- Dialogue System Documentation: ../DialogueSystem/README.md
- Ability System Documentation: ../AbilitySystem/README.md
