# Interaction Execution via Ability Self-Selection

How an interaction actually *runs*. The target advertises; the instigator executes.
Nothing on the interactable side executes interaction logic anymore.

## Naming

Only two terms are used:

- **Interaction** — what a target advertises. Data only: `FFVInteractionConfig`, surfaced to UI as `FFVInteractionInfo`.
- **Ability** — what actually executes on the instigator's ASC. A `UFVInteractAbility` subclass.

The term *Action* is no longer used anywhere in the interaction system.

## The core idea

The player has abilities bound to input tags (`InputTag.Action.Primary`, etc.), Lyra-style.
But the primary slot means something different for every target: pick up an item, talk to
an NPC, pry open a door. Nothing dispatches by tag. The input fans out to every ability
bound to it and each one decides for itself whether it is the interaction the current offer
advertises for that slot.

```
'E' pressed
  -> InputTag.Action.Primary
  -> ASC fans out to every ability bound to that input tag
  -> UFVInteractAbility::CanActivateAbility
       input tag -> slot -> Offers.GetActiveSlot(Slot)
       true only if the resolved AbilityTag is this ability's own asset tag
         and the interaction is available
  -> winner's ActivateAbility
       Offers.BeginEngagement(Slot)
       -> ActivateInteraction(Offers, InteractableActor)   [the real work, in script]
```

Because selection happens inside `CanActivateAbility`, exactly one ability can win and
recursive self-activation is impossible.

`BeginEngagement(EFVInteractionSlot)` returns an `EFVInteractionResult` and only publishes
the engaged target and takes the active offer. It never activates anything.

## The shared ability base

`UFVInteractAbility` (C++, `Abstract`) is the base for every ability that represents an
interaction. It implements `CanActivateAbility` (self-selection), `ActivateAbility`
(engagement plus the `ActivateInteraction` BlueprintImplementableEvent) and `EndAbility`
(release), and provides `GetOfferComponent()`. Concrete AngelScript abilities implement
`ActivateInteraction` only.

## The interaction asset is advertisement only

`FFVInteractionConfig` is pure data held by a `UFVInteractionSet` data asset. It carries no
logic, no requirements, no graph.

| Property | Role |
| --- | --- |
| `AbilityTag` | Asset tag of the ability that implements it (`Ability.Interact.*`). |
| `DisplayName` | Prompt label. |
| `Icon` | Prompt icon. |

`UFVInteractionSet` has exactly three fields — `Primary`, `Secondary`, `Ternary`. Field
position *is* the slot, so a slot can never be duplicated or overflow. A target config
references one set, which allows generic sets to be shared and swapped at runtime later.

`bIsSimple`, `FlowGraph`, `CheckRequirements`, `GetGrantedTags` and the
`RequiredTags` / `BlockedByTags` / `GrantedTags` containers are all gone. Requirements now
live where they belong: on the ability itself, as `ActivationRequiredTags` and
`ActivationBlockedTags`.

## Availability comes from the ability, not the asset

`ResolveInteractions` asks the instigator's `UFVAbilitySystemComponent`:

```cpp
bool QueryAbilityAvailabilityByTag(const FGameplayTag& AbilityTag,
								   bool& OutAvailable,
								   FGameplayTag& OutFailureTag) const;
```

It returns `false` when the ability is **not granted**, and `true` with `OutAvailable`
describing whether it could activate right now. This produces a three-state model:

| Ability state | Resolver behaviour | UI |
| --- | --- | --- |
| Not granted | Slot is skipped entirely | Nothing shown |
| Granted, blocked | `bAvailable = false`, `UnavailableReason` set from the failure tag | Greyed out with tooltip |
| Granted, ready | `bAvailable = true` | Normal, activatable |

Granting is therefore the switch that decides whether an action is *known* to the player at
all. Ungranted means invisible, not greyed out. This is why abilities are granted and
removed manually rather than always-on.

> `UGameplayAbility` keeps `ActivationRequiredTags` / `ActivationBlockedTags` protected, so
> `UFVGameplayAbility` exposes `GetActivationRequiredTags()` / `GetActivationBlockedTags()`
> for the ASC to read.

## Engagement lifetime

Engagement is bounded by the running ability, not by the target.

`ActivateAbility` calls `BeginEngagement(Slot)`; `EndAbility` calls `EndEngagement()`. Since
the ability that owns the engagement is the one that opened it, no handle bookkeeping or
`OnAbilityEnded` subscription is needed.

**Interaction scripts must never call `BeginEngagement` or `EndEngagement` themselves.**
Implement `ActivateInteraction` and end the ability; both sides are automatic.

## Cancellation

`AbortEngagedInteraction(Reason)` maps the reason to an `Interaction.Cancel.*` tag, applies
it as a loose tag on the instigator ASC, cancels every ability tagged `Ability.Interact`,
then removes the tag and releases engagement. The ability observes the reason during its
`EndAbility` if it cares.

| Reason | Tag |
| --- | --- |
| `WalkedAway` | `Interaction.Cancel.WalkedAway` |
| `HigherPriorityOffer` | `Interaction.Cancel.HigherPriorityOffer` |
| `OfferExpired` | `Interaction.Cancel.OfferExpired` |
| `CombatStarted` | `Interaction.Cancel.CombatStarted` |
| `Death` | `Interaction.Cancel.Death` |
| `Scripted` | `Interaction.Cancel.Scripted` |

Walk-away and offer withdrawal route through `FinishOffer`, which aborts the engagement when
the finished offer is the engaged one.

## Where abilities live

**Every concrete interaction ability is AngelScript.** C++ provides the framework; the game
detail is scripted. The boundary:

| Layer | Owns |
| --- | --- |
| C++ | `UFVGameplayAbility` base, the ASC, `UFVInteractAbility` self-selection, engagement lifetime, availability queries, native tags, and message structs crossing a module boundary |
| AngelScript | One class per interaction verb — pickup, talk, lockpick, examine — plus message structs used only by those abilities and their widgets |

The ability scripts are leaf nodes: nothing depends on them, they hot-reload, and they are
where design iteration happens. If a script ability needs an API that isn't exposed, the fix
is to add a `UFUNCTION` to the relevant C++ type — **not** to move the ability into C++.

The same test applies to message structs. `FFVInteractionOfferChangedMessage` is C++ because
the C++ UI router consumes it. `FFVInteractionExamineMessage` and
`FFVInteractionLockpickMessage` are AngelScript (`Script/Abilities/FVInteractionAbilityTypes.as`)
because only a script ability and a widget ever touch them.

## Reference abilities

All live in `Script/Abilities/`.

**`UFVPickupAbility`** — fire-and-forget. Optionally plays a montage, then resolves the
engaged target's `AFVItemPickup` and calls `ExecutePickup` against the instigator's
inventory, then ends.

**`UFVTalkAbility`** — long-running, externally terminated. Finds the engaged target's
`UFlowComponent`, calls `NotifyGraph(FlowNotifyTag)`, then listens for
`Dialogue.Ended` on the message bus and ends when the conversation finishes.

**`UFVLockpickAbility`** — widget-driven mini-game. Broadcasts
`Interaction.Event.LockpickStarted` with a `Difficulty` scalar to open the mini-game, then
waits for the widget to report the outcome on `Interaction.Event.LockpickEnded`.

**`UFVExamineAbility`** — widget-driven inspection. Broadcasts
`Interaction.Event.ExamineStarted` to open the overlay; the widget owns dim/blur, item
display and rotation input, echoing the current `ViewRotation` back on the same channel.
The ability matches that rotation against `SecretViewAngles` within `SecretAngleTolerance`
and fires each secret once.

Both widget-driven abilities re-broadcast their close message when cancelled externally, so
the overlay cannot be orphaned by a walk-away or combat interrupt.

### Unimplemented placeholders

- `UFVExamineAbility::UnlockMemory` only logs. The memory/clue system does not exist yet.
- `UFVLockpickAbility` does not apply an unlock to the target; there is no lock component yet.
- The examine overlay and lockpick mini-game widgets are not built.

## Content setup

Code alone is not enough. In-editor you must:

1. Author `UFVInteractionSet` assets, filling the `Primary` / `Secondary` / `Ternary` entries with valid `AbilityTag` values.
2. Reference the set from the target's `UFVInteractionTargetConfig`.
3. Add abilities whose asset tags match those `AbilityTag` values to the player's ability set, each granted with the input tag of the slot it serves.
4. Build the examine overlay and lockpick mini-game widgets against the message contracts above.

An interaction whose `AbilityTag` matches no granted ability will simply never appear in the prompt.

## What was removed

The interactable no longer owns a `UStateTreeComponent` for execution. `RunAction`,
`CompleteActiveTask`, `CancelActiveInteraction`, `IsInteractionInProgress`, the `Active*`
context fields, `OnStateTreeStatusChanged` and `UFVInteractionStateTaskBase` are all deleted.

The player-side State Tree is untouched — `UFVInteractionPerceptionEvaluator` and
`UFVInteractingStateTask` still drive camera, movement and presentation. See
[FC_Interaction_PlayerStateTree.md](FC_Interaction_PlayerStateTree.md).
