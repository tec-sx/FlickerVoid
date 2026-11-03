# FVoid Dialogue System

A Dynamic Response System (DRS) inspired dialogue framework built on Unreal Engine DataTables.

## Overview

The dialogue system enables designers to create rich, context-aware conversations using CSV/Excel files while the runtime handles condition evaluation, response ranking, and effect application.

## Architecture

See flowcharts:

- [System Architecture](FC_DialogueSystem_SystemArchitecture.md)
- [Signal Flow](FC_DialogueSystem_SignalFlow.md)
- [Condition Evaluation](FC_DialogueSystem_ConditionEvaluation.md)
- [Response Selection](FC_DialogueSystem_ResponseSelection.md)
- [Effect Application](FC_DialogueSystem_EffectApplication.md)
- [Conversation State Machine](FC_DialogueSystem_ConversationStateMachine.md)

## Core Concepts

### Signals

Signals are named events that trigger dialogue queries (similar to CryEngine DRS).

| Signal | When Triggered |
|--------|----------------|
| Greeting | Initiating conversation |
| Bark_Hostile | NPC becomes hostile |
| Bark_Suspicious | NPC notices something odd |
| Quest_Accept | Accepting a quest |
| Quest_Complete | Completing a quest |

### Conditions

Conditions determine which dialogue lines are valid for a given context.

| Type | Format | Example |
|------|--------|---------|
| WorldState | WorldState:TagName:Present/Absent | WorldState:Chapter1:Present |
| Relationship | Relationship:NPCId:Comparison:Value | Relationship:NPC_Barkeeper:>=:0.5 |
| Attribute | Attribute:AttrName:Comparison:Value | Attribute:Sanity:>=:0.3 |
| Quest | Quest:QuestId:State | Quest:MainQuest_01:Completed |
| Addiction | Addiction:Type:Comparison:Value | Addiction:Alcohol:>=:0.7 |
| Memory | Memory:Type/Total:Comparison:Value | Memory:Total:<=:5 |

### Effects

Effects modify game state after dialogue is played.

| Type | Format | Example |
|------|--------|---------|
| AddTag | AddTag:TagName | AddTag:WorldState.MetBarkeeper |
| RemoveTag | RemoveTag:TagName | RemoveTag:Quest.Available |
| ModifyRelationship | ModifyRelationship:NPCId:Delta | ModifyRelationship:NPC_Barkeeper:0.1 |
| ModifyAttribute | ModifyAttribute:AttrName:Delta | ModifyAttribute:Sanity:-0.05 |

### Priority and Specificity

When multiple lines match a signal:

1. Priority field determines explicit ordering (higher wins)
2. Condition weight determines specificity (more conditions equals more specific)
3. Cooldown prevents repetition within a time window
4. MaxUseCount limits total uses (0 means unlimited)

## DataTable Structure

### Dialogue Lines (FFVoidDialogueLine)

Main table containing all NPC and player dialogue.

| Column | Type | Description |
|--------|------|-------------|
| LineId | FName | Unique identifier |
| SignalName | FName | Trigger signal |
| SpeakerId | FName | Who speaks this line |
| GroupId | FName | For organizing related lines |
| Text | FText | Displayed dialogue text |
| AlternativeText | FText | Low sanity version |
| LineType | Enum | Speech/Thought/Whisper/Shout/Communication |
| DurationOverride | Float | Override auto-calculated duration |
| RequiredConditionIds | Array | References to conditions table |
| ConditionOperator | Enum | And/Or for combining conditions |
| InlineConditions | Array | Conditions in CSV string format |
| Priority | Int | Selection priority (higher wins) |
| CooldownHours | Float | Game hours before reuse |
| MaxUseCount | Int | Max times line can be used |
| InlineEffects | Array | Effects to apply |
| NextSignal | FName | Continue conversation flow |
| NextSignalDelay | Float | Delay before next signal |
| bEndsConversation | Bool | Ends dialogue when played |
| CameraAngle | FName | Camera shot identifier |
| Expression | FName | Facial expression override |

### Responses (FFVoidDialogueResponse)

Player response options shown during conversations.

| Column | Type | Description |
|--------|------|-------------|
| ResponseId | FName | Unique identifier |
| TriggerSignal | FName | When to show this response |
| Text | FText | Button/option text |
| ConditionOperator | Enum | And/Or for conditions |
| InlineConditions | Array | Availability conditions |
| Priority | Int | Display order priority |
| InlineEffects | Array | Effects when chosen |
| NextSignal | FName | Follow-up signal |
| ResponseType | Enum | Neutral/Aggressive/Friendly/Curious/Vulnerable/Determined |
| bEndsConversation | Bool | Ends conversation when selected |
| TooltipText | FText | Hover tooltip explanation |

### Speakers (FFVoidSpeakerInfo)

Speaker configuration for UI presentation.

| Column | Type | Description |
|--------|------|-------------|
| SpeakerId | FName | Unique identifier |
| DisplayName | FText | Shown in subtitles |
| SubtitleColor | LinearColor | Text color RGBA |
| VoicePitch | Float | Audio pitch modifier |
| CharacterData | SoftObjectPtr | Reference to character data asset |
| Tags | TagContainer | Speaker classification tags |

### Conditions (FFVoidDialogueConditionRow)

Reusable condition definitions.

| Column | Type | Description |
|--------|------|-------------|
| ConditionId | FName | Reference identifier |
| ConditionType | Enum | WorldState/Attribute/Relationship/Quest/Tag |
| TargetKey | FName | What to check |
| TargetId | FName | Optional secondary target |
| Comparison | Enum | Equal/NotEqual/LessThan/GreaterThan/etc |
| CompareValue | Float | Value to compare against |
| bRequireTagPresent | Bool | For tag-based conditions |
| bNegate | Bool | Invert the result |
| Weight | Float | Specificity weight for ranking |

## Example Files

Located in this directory:

| File | Description |
|------|-------------|
| EG_DialogueLines.csv | Sample dialogue lines |
| EG_DialogueResponses.csv | Sample player responses |
| EG_Speakers.csv | Sample speaker definitions |
| EG_Conditions.csv | Sample reusable conditions |

## Key Classes

| Class | Location | Purpose |
|-------|----------|---------|
| UFVoidDialogueSubsystem | Systems/ | Central dialogue manager |
| UFVoidDialogueQuery | Narrative/Dialogue/ | Finds matching lines |
| UFVoidDialogueConditionEvaluator | Narrative/Dialogue/ | Evaluates conditions |
| FFVoidDialogueLine | Narrative/Dialogue/FVoidDialogueTypes.h | DataTable row struct |
| FFVoidDialogueResponse | Narrative/Dialogue/FVoidDialogueTypes.h | Player response struct |
| FFVoidSpeakerInfo | Narrative/Dialogue/FVoidDialogueTypes.h | Speaker config struct |

## Integration Points

| System | Class | Integration |
|--------|-------|-------------|
| Quest System | UFVoidQuestSubsystem | Quest state conditions |
| World State | UFVoidWorldStateSubsystem | World tags and NPC relationships |
| Attributes | UFVoidProtagonistAttributeSet | Sanity/addiction checks |
| Memory System | UFVoidMemoryFragment | Memory-based conditions |

## Designer Workflow

1. Edit CSV files in Excel or any spreadsheet application
2. Import CSVs as DataTables in Unreal Editor
3. Assign tables to UFVoidDialogueSubsystem via Blueprint or C++
4. Test signals using SendSignal in gameplay

## See Also

- Story System Documentation: ../StorySystem/README.md
- Ability System Documentation: ../AbilitySystem/README.md
