# Interaction Execution via GAS Dispatch

How an interaction actually *runs*. The target advertises; the instigator executes.
Nothing on the interactable side executes interaction logic anymore.

## Naming

Only two terms are used:

- **Interaction** — what a target advertises. Data only: `UFVInteractionConfig`, surfaced to UI as `FFVInteractionInfo`.
- **Ability** — what actually executes on the instigator's ASC. A `UFVInteractAbility` subclass.

The term *Action* is no longer used anywhere in the interaction system.

## The core idea

The player has abilities bound to input tags (`InputTag.Action.Primary`, etc.), Lyra-style.
But the primary slot means something different for every target: pick up an item, talk to
an NPC, pry open a door. Rather than binding many abilities to one input tag and guessing,
the input calls a single entry point that asks the current offer what that slot resolves to
and activates the corresponding ability on the instigator's own ASC.

```
'E' pressed
  -> InputTag.Action.Primary
  -> UFVInteractAbility::TryBeginInteraction(Primary)
  -> Offers.BeginEngagement(Primary)             [does everything below]
       resolve slot -> check availability
       -> publish EngagedTarget + subscribe OnAbilityEnded
       -> take the active offer
       -> dispatch the resolved AbilityTag
  -> UFVPickupAbility / UFVTalkAbility / ...      [the real work]
```

`BeginEngagement(EFVInteractionSlot)` returns an `EFVInteractionResult` and is the *only*
public way to start an interaction. Ability dispatch and offer bookkeeping are private to
the offer component, so ordering cannot be got wrong from script or Blueprint.

Ordering matters: `TryActivateAbility` runs the ability synchronously, so the engaged target
is published *before* dispatch. A dispatched ability can therefore read `GetEngagedTarget()`
immediately inside `ActivateAbility`. If dispatch fails, the engagement is rolled back and
`Blocked` is returned.

## The shared ability base

`UFVInteractAbility` (C++, `Abstract`) is the base for every ability that represents an
interaction. It provides `GetOfferComponent()`, `GetEngagedTarget()`, `GetEngagedActor()`
and `TryBeginInteraction(Slot)`, so concrete AngelScript abilities never re-walk the avatar
actor by hand.

## The interaction asset is advertisement only

`UFVInteractionConfig` is pure data. It carries no logic, no requirements, no graph.

| Property | Role |
| --- | --- |
| `AbilityTag` | Asset tag of the ability to dispatch (`Ability.Interact.*`). |
| `DisplayName` | Prompt label. |
| `Icon` | Prompt icon. |
| `Slot` | Which input slot advertises it. |

`bIsSimple`, `FlowGraph`, `CheckRequirements`, `GetGrantedTags` and the
`RequiredTags` / `BlockedByTags` / `GrantedTags` containers are all gone. Requirements now
live where they belong: on the ability itself, as `ActivationRequiredTags` and
`ActivationBlockedTags`.

## Availability comes from the ability, not the asset

`UFVInteractionResolver` asks the instigator's `UFVAbilitySystemComponent`:

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

Engagement is bounded by the dispatched ability, not by the target.

`BeginEngagement(Target, Handle)` stores the spec handle and subscribes to
`ASC->OnAbilityEnded`. When the ability with that handle ends — for any reason, success,
failure or cancellation — `HandleAbilityEnded` calls `EndEngagement()` automatically.

**Interaction abilities must never call `EndEngagement()` themselves.** Just end the
ability; release is automatic.

## Cancellation

`AbortEngagedInteraction(Reason)` maps the reason to an `Interaction.Cancel.*` tag, applies
it as a loose tag on the instigator ASC, cancels the ability by handle, then removes the tag
and releases engagement. The ability observes the reason during its `EndAbility` if it cares.

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
| C++ | `UFVGameplayAbility` base, the ASC, `UFVInteractAbility` dispatch, engagement lifetime, availability queries, native tags, and message structs crossing a module boundary |
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

1. Author `UFVInteractionConfig` assets with a valid `AbilityTag` and `Slot`.
2. Add abilities whose asset tags match those `AbilityTag` values to the player's ability set.
3. Grant the slot dispatcher `UFVInteractAbility` once per slot, bound to the matching input tag.
4. Build the examine overlay and lockpick mini-game widgets against the message contracts above.

An action whose `AbilityTag` matches no granted ability will simply never appear in the prompt.

## What was removed

The interactable no longer owns a `UStateTreeComponent` for execution. `RunAction`,
`CompleteActiveTask`, `CancelActiveInteraction`, `IsInteractionInProgress`, the `Active*`
context fields, `OnStateTreeStatusChanged` and `UFVInteractionStateTaskBase` are all deleted.

The player-side State Tree is untouched — `UFVInteractionPerceptionEvaluator` and
`UFVInteractingStateTask` still drive camera, movement and presentation. See
[FC_Interaction_PlayerStateTree.md](FC_Interaction_PlayerStateTree.md).
