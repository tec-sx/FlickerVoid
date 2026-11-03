# Interaction Execution via Gameplay Events

How a committed interaction runs. The interactable advertises offers as data; the player's abilities do the work.
Setup steps are in `Docs/Setup/FVInteractionSystem.md` and, for items, `Docs/Setup/Items.md`.

## Flow

```
Player looks at an interactable
  -> UFVInteractorComponent picks the target and resolves its offers
     (the definition's Offers whose Conditions pass)
Input on an offer's Input Tag
  -> the offer's gesture runs (press, hold, mash, hover)
  -> completed: UFVInteractorComponent::FinishInteraction(true)
       Interactable.ExecuteAction(ActionTag, Interactor)   response components bound to that action
       Interactable.ConsumeOffer(InputTag)                  Remaining Uses
       Interactable.EndInteraction(...)                     the offer's Effects, then InteractionEnded
  -> Activate Ability effect (FFVEffect_ActivateAbility)
       sends the gameplay event EventTag to the instigator's ASC:
       Instigator = player, Target = interactable's owner, OptionalObject = its UFVInteractableComponent
  -> the ability that takes the event runs on the player
```

A gesture that fails or is cancelled before it completes (`CancelInteraction`, the target lost or out of reach) runs nothing.

## Offers are data, abilities are logic

Offers live on `UFVInteractableDefinition` (FVInteractionSystem): Input Tag, Action Tag, Gesture, Display, Conditions, Effects, Weight, Remaining Uses. The plugin knows nothing about GAS; the game module adds the **Activate Ability** effect. Offer effects run with Instigator = the interactor's owner and Target = the interactable's owner, so any other effect works there too (e.g. *FV Trade Effect*, which needs no ability).

Response components (`UFVInteractableResponseComponent`, bound with `BindResponse(ActionTag, Response)`) hold logic that belongs to the target itself, such as a door's toggle (`UFVInteractableResponseComponent_Toggle`). They run before the offer's effects.

Offer conditions decide whether an offer is available (hidden or shown locked); the ability's own tags (`ActivationRequiredTags`, `ActivationBlockedTags`, e.g. `Status.HandsBusy`) decide whether it can start.

## Starting an ability from the event

An ability takes the event in one of two ways:
- **Trigger Event Tag** on `UFVGameplayAbility` (AngelScript: `default TriggerEventTag = GameplayTags::Event_...;`). `UFVAbilitySystemComponent::OnGiveAbility` registers it as a gameplay event trigger, unless an Ability Trigger already fires the ability on that tag.
- An **Ability Trigger** of type Gameplay Event, set on the class or a Blueprint child.

A script ability overrides `ActivateAbilityFromEvent(EventData)` to read the event, or `ActivateAbility()` when the target is enough.

Grant each ability once, through the character's `UFVAbilitySet`: under *Granted Interaction Abilities* with its `Interaction.Action` tag, or under *Granted Gameplay Abilities* with an Input Tag. Each grant registers the trigger, so a second grant makes one event start the ability twice. An offer whose event no granted ability takes does nothing.

## The shared ability base

`UFVInteractAbility` (C++, `Abstract`) is the base of every ability an interaction starts. It provides `GetInteractor()` (the avatar's interactor component) and `GetInteractable()` (that interactor's current target; `Interactable` in AngelScript). The target can change while a long ability runs, so abilities keep the actor they started on.

## Item events

Item abilities also take events sent by `FVItem::SendItemEvent` (Use Held, inventory screens) or, in C++, `SendItemPayload` (the FV Give Item flow node). Those carry a `UFVItemEventPayload` as OptionalObject: item, instance id, world actor, other actor, quantity. `FVItem::GetItemPayload(EventData)` returns it, or none for an event from an offer, whose OptionalObject is the interactable.

## Where abilities live

**Every concrete interaction ability is AngelScript.** C++ provides the framework; the game detail is scripted.

| Layer | Owns |
| --- | --- |
| C++ | `UFVGameplayAbility` (Trigger Event Tag), `UFVInteractAbility`, the ASC (trigger registration, input tags, activation groups), the Activate Ability effect, the item library and payload, native tags, and message structs C++ sends or reads (`Items/FVItemMessages.h`) |
| AngelScript | One class per verb, plus message structs used only by those abilities and their widgets (`Script/Abilities/FVInteractionAbilityTypes.as`) |

The ability scripts are leaf nodes: nothing depends on them, they hot-reload, and they are where design iteration happens. If a script ability needs an API that isn't exposed, add a `UFUNCTION` to the relevant C++ type; don't move the ability into C++.

## Reference abilities

In `Script/Abilities/`. These set no Trigger Event Tag: give them (or Blueprint children) an Ability Trigger on the event their offer sends.

**`UFVTalkAbility`**: finds the target's `UFVDialogueParticipantComponent`, starts its dialogue with the player and ends when `UFVDialogueSubsystem` broadcasts `OnConversationEnded`.

**`UFVLockpickAbility`**: plays its optional montage and broadcasts `Interaction.Event.LockpickStarted` (`FFVInteractionLockpickMessage`, with the lock's Difficulty, else its own) to open the mini-game, then waits for the widget's `Interaction.Event.LockpickEnded` and unlocks the target's `UFVLockComponent` on success.

**`UFVExamineAbility`**: broadcasts `Interaction.Event.ExamineStarted` (`FFVInteractionExamineMessage`, Visible) to open the overlay. The widget owns dim/blur, display and rotation input and echoes the current `ViewRotation` on the same channel. The ability passes it to the examined actor's `UFVExamineSecretsComponent` (`TryFind`); once every secret of an unsolved `AFVReadable` is found it reads the note and ends. When it ends with the overlay still open, also when cancelled from outside, it broadcasts Visible false so the overlay closes.

In `Script/Items/Abilities/`, each with a Trigger Event Tag under `Event.Item`: pickup, use, wear, hold, use held, drop, throw, read, take all, pickpocket and give (table in `Docs/Setup/Items.md`, section 6). **`UFVPickupAbility`** plays the character's pickup montage for the item's height, warped to the item, and calls the target's `UFVPickupComponent::TryPickup` on the Apply notify; an `AFVMoneyPickup` is collected at once.

## Content setup

1. Interactable definitions with offers whose effect is **Activate Ability** with an `Event.` tag.
2. The player's ability set granting each ability once; each ability has a Trigger Event Tag or an Ability Trigger on that event.
3. The examine overlay and lockpick mini-game widgets, built against the message contracts above.
