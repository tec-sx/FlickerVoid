# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

A non-combat, AAA-style narrative single-player game in UE5 focused on psychological state, identity, and consequence-driven progression. Core pillars: sanity system, substance abuse mechanics, social manipulation, and narrative choice. **Single-player only — no replication assumptions.**

Engine: Unreal Engine 5.7. Primary language: C++. Blueprints are for content assembly and design configuration only — not logic.

## Build & Development

**Build:** Open `FlickerVoid.sln` in Visual Studio 2022 → Build Solution, or compile via the Unreal Editor toolbar.

**Recompile AngelScript:** Script changes hot-reload automatically in the editor. C++ changes require a full or live-coding compile.

**Generate project files:** Right-click `FlickerVoid.uproject` → Generate Visual Studio Project Files.

## C++ Module Architecture

Nine runtime modules — each has `Public/` and `Private/` directories:

| Module folder | Prefix | Purpose |
|---|---|---|
| `Source/FlickerVoidGame/` | `FVoid` | Core game: modes, abilities, GAS setup |
| `Source/FVCore/` | `FVCore` | Foundation utilities |
| `Source/FVCharacter/` | `FV` | Character, movement, animation |
| `Source/FVGameplay/` | `FVG` | Gameplay mechanics |
| `Source/FVWorld/` | `FVW` | World and environment |
| `Source/FVItems/` | `FVI` | Inventory and items |
| `Source/FVNarrative/` | `FVN` | Dialogue and story |
| `Source/FVAI/` | `FVAI` | AI behavior and state trees |
| `Source/FVUI/` | `FVUI` | UI and UMG widgets |

The `FVCharacter` module is organized by domain context, not by function type. Always use context-qualified include paths:

```cpp
#include "Character/FVCharacter.h"         // not "FVCharacter.h"
#include "Movement/FVMovementHandlerBase.h"
#include "Animation/FVCharacterAnimInstance.h"
```

## AngelScript Scripting Layer

Game logic lives in `Script/` as AngelScript (`.as` files). C++ provides framework types; AngelScript provides game-specific implementations. AngelScript classes inherit from C++ UCLASS types and implement `BlueprintImplementableEvent` functions.

Key script directories:

- `Script/Core/Movement/` — movement handlers and config
- `Script/Core/Player/` — player character and controller
- `Script/Core/Interaction/Tasks/` — interaction State Tree tasks
- `Script/Animation/` — animation blueprint logic
- `Script/Environment/` — traversable world actors
- `Script/UI/Interaction/` — interaction prompt overlay widget
- `Script/UI/Dialogue/` — dialogue overlay and response entry widgets

AngelScript changes hot-reload in the editor; no compilation step needed.

## Movement System — Handler Pattern

The movement system is the most complex part of the codebase. Understand this before touching `FVCharacter` or `Script/Core/Movement/`.

**How it works:**

`UFVCharacterMovementComponent` manages a set of `UFVMovementHandlerBase` subclasses. Each tick:
1. All handlers evaluate their `Resolve()` → returns bool for whether they should be active.
2. The highest-priority handler with satisfied gameplay tag requirements becomes active.
3. Lifecycle: `OnInitialize` → `OnEnter` → `GenerateMovement(DeltaTime)` each frame → `OnExit`.

**Handler configuration** lives in `UFVMovementHandlerData` (Data Asset), which stores `FFVMovementHandlerInfo` structs. Each handler info defines:
- Required/blocked gameplay tags
- Priority (0–255; higher wins)
- Whether it can be interrupted
- Tags to grant while active

**Concrete handlers** (implemented in AngelScript):

| Handler | Active when | Responsibility |
|---|---|---|
| `FVGroundMovementHandler` | `MOVE_Walking` | Gait selection, acceleration, friction, rotation |
| `FVInAirMovementHandler` | `MOVE_Falling` | Airborne rotation rate |
| `FVTraverseMovementHandler` | Custom traversal logic | Sets `MOVE_Flying` during obstacle interactions |

**Gait config** (`FFVGaitConfig` in `FVMovementConfig.as`):
- Walk: 180 fwd / 160 strafe / 150 back
- Jog: 310 fwd / 250 strafe / 250 back
- Sprint: 700 all directions (reduced accel, less friction)

To add a new movement mode: create a handler class (C++ or AngelScript) inheriting `UFVMovementHandlerBase`, implement `Resolve()` and `GenerateMovement()`, add an entry to the Data Asset.

## Character Intent & State Flow

`FFVCharacterIntent` (in `FVCharacterTypes.h`) represents raw player input — direction, walk/sprint/crouch/aim/jump/traverse flags. `FFVCharacterRuntimeState` is the computed result per frame.

`AFVCharacter` owns both structs and exposes intent setters (`RequestMove`, `RequestSprint`, `RequestJump`, `RequestTraverse`). The movement component reads intent and runtime state; it does not directly read input.

## Traversal System

Entry point: `AFVPlayerCharacter::RequestTraverse()` → `FVTraversalComponent::TryTraversalAction()`.

Flow: forward raycast → detect `AFVTraversable` actor → query front/back ledge positions → clearance checks → floor detection → select animation via `FChooserTable` (keyed on obstacle height/depth, movement mode, character speed).

## Ability & Input System

Based on Lyra's GAS setup. Input actions map to Gameplay Tags via `FVoidInputConfig` (Data Asset). Tags are passed to GAS; abilities activate on matching tags. The `FVoidAbilitySet` Data Asset controls which abilities an actor receives.

`FVoidPlayerState` owns the Ability System Component. `FVoidProtagonistComponent` (mandatory on the player character) bridges input → character intent.

## Item System

Items are defined as `UFVoidItemDataAsset` (Data Asset) containing `FFVoidItemDataTemplate`. Behavior is composed from `UFVoidItemEffect` subclasses (Blueprint-extensible):

- `UFVoidConsumableEffect`, `UFVoidFoodEffect`, `UFVoidBeverageEffect`, `UFVoidDrugEffect`, `UFVoidEquipmentEffect`

Items use tag hierarchies for categorization (`Item.Category.*`, `Item.Rarity.*`, `Item.Property.*`). Effects handle behavior; tags handle categorization — these concerns are separate.

## Interaction System

All interaction code lives in `Source/FVGameplay/Public/Interaction/` and `Private/Interaction/`. Concrete tasks are in `Script/Core/Interaction/Tasks/`.

### Data flow

```
Physical Key → InputConfig data asset → InputTag (Input.Interact.Primary etc.)
    → GAS ability (GA_Interact* Blueprint) activates via Input_AbilityInputTagPressed
    → UFVInteractionComponent::RequestInteraction(InputTag)
    → UFVInteractableComponent::TryExecuteAction(InputTag, Instigator)
        → CheckRequirements()
        → UStateTreeComponent::SetStateTree(Action.ActionStateTree) + StartLogic()
            → UFVInteractionStateTreeTaskBase::ReceiveEnterState(OwnerActor, ...)
            → async tasks: widget/dialogue runs → CompleteTask(OwnerActor, bool) called
            → UFVInteractionStateTreeTaskBase::ReceiveTick polls IsActiveTaskDone()
        → UStateTreeComponent::OnStateTreeStopped → GrantedTagsOnSuccess applied to ASC
    → UFVInteractionComponent::BroadcastFocusState() → UI updated
```

### Adding a new action type

1. Create `UFVMyTask : UFVInteractionStateTreeTaskBase` in AngelScript (or C++)
2. Add UPROPERTY fields for configuration — the State Tree editor exposes them per-node
3. Override `ReceiveEnterState` to start the action; for sync actions call `CompleteTask(OwnerActor, true)` here
4. For async actions: open a widget via gameplay event, cache `OwnerActor`, call `CompleteTask(CachedOwnerActor, bool)` in the callback
5. Override `ReceiveExitState` for cleanup on cancel
6. Create a `UStateTree` asset, set schema to `UStateTreeComponentSchema`, add your task node
7. Assign the asset to `FFVInteractionAction.ActionStateTree` on the interactable

Tasks that need per-frame work (e.g. advancing a slider) override `ReceiveTick` and call `Super.ReceiveTick(OwnerActor, DeltaTime)` first to preserve the completion check.

### Task context helpers (on `UFVInteractionStateTreeTaskBase`)

All static, take the `OwnerActor` parameter passed by `ReceiveEnterState`/`ReceiveTick`:

```cpp
GetInstigator(OwnerActor)       // the player who triggered the action
GetInteractable(OwnerActor)     // UFVInteractableComponent on the actor
GetActionTag(OwnerActor)        // e.g. Interaction.Action.PickUp
GetInteractionPoint(OwnerActor) // world position for IK/VFX
CompleteTask(OwnerActor, bool)  // signal done; State Tree detects on next Tick
```

### Requirements

Attach any number of `UFVInteractionRequirement` subclasses (instanced, inline in editor) to an action's `Requirements` array. Checked in C++ before the State Tree starts. Built-in types:
- `UFVTagRequirement` — player must have a gameplay tag on their ASC
- `UFVAttributeRequirement` — player must have a GAS attribute ≥ a minimum value
- `UFVItemRequirement` — abstract, override `IsMet` in Blueprint/AngelScript

### Input slots → tags

| Physical key (example) | Tag |
|---|---|
| E press | `Input.Interact.Primary` |
| E hold | `Input.Interact.PrimaryHold` |
| F press | `Input.Interact.Secondary` |
| F hold | `Input.Interact.SecondaryHold` |

Only the `InputConfig` data asset knows physical keys. Everything downstream uses the tag.

### NPC character

`AFVNPCCharacter` (in `FVCharacter` module) — lightweight character with a `UFVInteractableComponent`. Add `UFVQuestGiverComponent` / `UFVDialogueComponent` (from `FVNarrative`) for narrative functionality. NPC identity and state are tag-driven via `NPCTags`.

### Concrete tasks (AngelScript, `Script/Core/Interaction/Tasks/`)

| Task | Action tag | Type |
|---|---|---|
| `UFVPickupTask` | `Interaction.Action.Pickup` | Sync — fires inventory event, hides actor |
| `UFVExamineTask` | `Interaction.Action.Examine` | Async — rotate overlay, fires secret tags at angle thresholds |
| `UFVLockpickTask` | (extends `UFVMiniGameTask`) | Async — slider ticks in `ReceiveTick`, timing mini-game |
| `UFVMiniGameTask` | any | Abstract base — attempt counting, success/failure tags |
| `UFVTalkTask` | `Interaction.Action.Talk` | Async — starts `UFVDialogueSubsystem` conversation, notifies `UFVQuestSubsystem` |

## HUD / UI System

`AFVHUD` (in `Source/FVUI/`) creates and manages both HUD overlays at `BeginPlay`. Assign widget class properties in the HUD Blueprint:
- `InteractionPromptWidgetClass` → `UFVInteractionPromptWidgetImpl` subclass (4-action overlay)
- `DialogueWidgetClass` → `UFVDialogueWidget` subclass (NPC conversation overlay)

`UFVInteractionPromptWidget` (C++ base in `FVUI`) automatically binds to the player pawn's `UFVInteractionComponent::OnFocusChanged` delegate and re-binds when the pawn changes. Implement `OnFocusChanged` in the AngelScript subclass.

`UFVDialogueWidget` (pure AngelScript) binds directly to `UFVDialogueSubsystem` delegates at `Construct()` — no character dependency.

## Narrative & Quest Systems

All narrative code lives in `Source/FVNarrative/`.

### Dialogue — `UFVDialogueSubsystem` (GameInstance subsystem)

Signal-based dialogue system similar to Halo's DRS. Dialogue data lives in `UDataTable` rows:
- `FFVDialogueLine` — one spoken line; conditions, effects, flow all inline-serialized as `TArray<FString>` in CSV-friendly format
- `FFVDialogueConditionRow` — reusable named conditions referenced by multiple lines
- `FFVDialogueResponse` — player response option (displayed in the choice list)
- `FFVSpeakerInfo` — NPC display name, subtitle color, voice pitch

**Key API:**
```cpp
// One-shot bark (NPC ambient line, item description)
DialogueSub.SendSignalFromActor("Greeting", NPCActor, PlayerActor);

// Structured conversation (multiple exchanges, player choices)
FFVDialogueContext Ctx; Ctx.SpeakerActor = NPC; Ctx.ListenerActor = Player;
DialogueSub.StartConversation("Conv_Barkeeper_FirstMeet", "Greeting", Ctx);
DialogueSub.SelectResponse(0); // player chose option 0
DialogueSub.EndConversation();
```

**Condition inline format** (in DataTable CSV):
- `"Tag:WorldState.MetBarkeeper:Present"` — gameplay tag must be on the ASC
- `"Attribute:Sanity:>=:0.3"` — character attribute threshold
- `"Quest:QuestId_01:Completed"` — quest completion state

**Effect inline format:**
- `"AddTag:WorldState.MetBarkeeper"` — apply tag to world state ASC
- `"ModifyRelationship:NPC_Barkeeper:0.1"` — change NPC relationship value
- `"StartQuest:QuestId_SideQuest"` — begin a quest

### Quests — `UFVQuestSubsystem` (GameInstance subsystem)

Quest definitions are `UFVQuestDefinition` UAssets. Runtime state is tracked in `UFVQuestInstance`.

**Notify the quest system** when gameplay events occur:
```cpp
QuestSub.NotifyTalkedToNPC(NPCId, DialogueNodeId);
QuestSub.NotifyItemObtained(ItemAssetId, Quantity);
QuestSub.NotifyLocationEntered(LocationId);
QuestSub.NotifyThresholdReached(ThresholdTag, TargetId, Value);
```

The `UFVTalkTask` calls `NotifyTalkedToNPC` automatically after each conversation ends.

### Memory Fragments — `UFVMemoryFragment`

Represents recovered identity/trauma memories. Used as condition checks in dialogue and quest definitions. Unlock via `Action.GrantedTagsOnSuccess` on interaction actions or gameplay effects.

## Key Architectural Principles

- **Tag-driven everywhere.** Gameplay Tags drive abilities, movement handler selection, state conditions, item categorization, and dialogue gating. Prefer adding tags over adding booleans or enums.
- **Data Asset configuration.** Systems are parameterized through Data Assets — not hardcoded values or Blueprint logic.
- **C++ framework, script behavior.** C++ defines the contract (`BlueprintImplementableEvent`, `BlueprintNativeEvent`); AngelScript or Blueprint provides the implementation.
- **Senior-level standards.** Prefer industry patterns, scalable architecture, and Unreal internals awareness. Avoid tutorial-level or single-use solutions.
