# Mountea Interaction System — Comprehensive Analysis

> Source analysed: `Plugins/MounteaInteractionSystem/`
> Author of plugin: Dominik Morse (Pavlicek), 2023–2024.

---

## 1. Module Layout

| Module | Type | Purpose |
|---|---|---|
| `MounteaInteractionSystem` | Runtime | All gameplay classes: interfaces, interactable/interactor components, settings, BFL, logging. |
| `MounteaInteractionSystemEditor` | Editor | Asset factories, class-viewer filters, details panel customization, help button/popup, editor settings. |
| `MounteaInteractionSystemEditorNotifications` | Editor | Standalone notification/update helper (`EditorHelper`). |

Runtime folder structure:

```
Public/
  Interfaces/    MounteaInteractableInterface.h, MounteaInteractorInterface.h, MounteaInteractionWidget.h
  Components/
	Interactable/ Base, Automatic, Hold, Hover, Mash, Press
	Interactor/   Base, Overlap, Trace
  Helpers/       Helpers, HelperEvents, GeneralDataTypes, FunctionLibrary, BFL, SettingsConfig, Settings, Log
```

---

## 2. Core Architecture

### 2.1 Interface-first design

The system is built on two `UINTERFACE(Blueprintable)` contracts:

- `IMounteaInteractableInterface` (~1000 lines) — everything an interactable must expose.
- `IMounteaInteractorInterface` (~560 lines) — everything an interactor must expose.

Every single API entry is declared as:

```cpp
UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Mountea|Interaction|Interactable")
bool CanInteract() const;
virtual bool CanInteract_Implementation() const = 0;
```

**Consequences (the good parts):**

- Any `UObject`/`AActor`/`UActorComponent` can be an interactable or interactor — you are not forced to inherit from a concrete component class.
- Blueprint users can implement/override *every* piece of behaviour without C++.
- Callers talk through `TScriptInterface<IMounteaInteractableInterface>`, so runtime coupling between the interactor and interactable is zero.
- Pure-virtual `_Implementation` forces derived C++ classes to be explicit — no silent inherited defaults.

**Cost:** `BlueprintNativeEvent` dispatch is significantly more expensive than a direct virtual call, and the interfaces are very large (a full implementation is ~80 functions).

### 2.2 Event-handle pattern

Instead of exposing delegates directly, the interfaces expose *handle getters*:

```cpp
virtual FInteractableSelected& GetOnInteractableSelectedHandle() override { return OnInteractableUpdated; }
virtual FInteractableFound&    GetOnInteractableFoundHandle()    override { return OnInteractableFound; }
virtual FStateChanged&         GetOnStateChangedHandle()         override { return OnStateChanged; }
```

This lets external systems bind through the interface without knowing the concrete class — a clean way to make delegates polymorphic in UE, which normally can't be virtualised.

### 2.3 Three-layer event chain

For each meaningful moment the plugin runs a strict chain:

```
Multicast delegate  ->  BlueprintNativeEvent (C++ logic)  ->  BlueprintImplementableEvent (designer hook)
OnInteractionStarted -> InteractionStarted()             -> OnInteractionStartedEvent()
OnInteractorFound    -> InteractorFound()                -> OnInteractorFoundEvent()
OnCooldownCompleted  -> InteractionCooldownCompleted()   -> OnCooldownCompletedEvent()
```

This is the single most valuable convention in the plugin: designers always have a hook, C++ always has an override point, and external systems always have a delegate — without any of the three fighting each other.

---

## 3. State Machines

### 3.1 Interactable — `EInteractableStateV2`

| State | Meaning |
|---|---|
| `EIS_Active` | Being interacted with right now. |
| `EIS_Awake` | Ready, reacting to interactors. |
| `EIS_Cooldown` | Temporarily disabled after a cycle; auto-returns to Awake. |
| `EIS_Paused` | Interaction paused but progress preserved. |
| `EIS_Completed` | Finished permanently, cannot be re-activated. |
| `EIS_Disabled` | Off, but can be woken. |
| `EIS_Suppressed` | Suppressed by a higher-weight interactable/dependency. |
| `EIS_Asleep` | Hidden default. |

### 3.2 Interactor — `EInteractorStateV2`

`EIS_Awake`, `EIS_Asleep`, `EIS_Suppressed`, `EIS_Active`, `EIS_Disabled`.

### 3.3 Transition guarding

State changes never happen by raw assignment. Instead:

```cpp
bool ActivateInteractable(FString& ErrorMessage);
bool WakeUpInteractable(FString& ErrorMessage);
bool CompleteInteractable(FString& ErrorMessage);
void DeactivateInteractable();
void PauseInteraction(const float ExpirationTime, const TScriptInterface<IMounteaInteractorInterface>& CausingInteractor);
```

Every transition returns `bool` **plus a human-readable `ErrorMessage`**. This is excellent for debuggability: a failed interaction always tells you *why*, and the same message surfaces in the editor validation and debug draw.

---

## 4. Interaction Types via Subclassing

Rather than one mega-component with a "mode" enum, Mountea ships one subclass per interaction verb:

| Class | Behaviour |
|---|---|
| `UMounteaInteractableComponentPress` | Single key press completes instantly. |
| `UMounteaInteractableComponentHold` | Key must be held for a period; starts a timer in `InteractionStarted_Implementation`, completes via `OnInteractionCompletedCallback`. |
| `UMounteaInteractableComponentMash` | Requires N presses inside a time window. |
| `UMounteaInteractableComponentHover` | Completes on focus alone, no key. |
| `UMounteaInteractableComponentAutomatic` | Triggers by proximity/overlap with no input. |

Each subclass only overrides the two or three functions it needs (e.g. Hold overrides just `InteractionStarted_Implementation`). This keeps each type small, self-documenting and independently Blueprint-subclassable.

Interactor side mirrors this: `UMounteaInteractorComponentTrace` (line/box trace with `EInteractorPrecision::EIP_High/EIP_Low`) and `UMounteaInteractorComponentOverlap` (collision overlap driven), both derived from `UMounteaInteractorComponentBase`.

---

## 5. Selection / Arbitration Model

The interactor does not simply take the first hit. Flow:

```
Detection (trace or overlap)
  -> OnInteractableFound broadcast
  -> InteractableFound()
  -> EvaluateInteractable(FoundInteractable)      // comparison / arbitration
  -> SetActiveInteractable() + OnInteractableUpdated (a.k.a. "Selected")
```

Arbitration inputs:

- **Interaction Weight** (`int32`, `OnInteractableWeightChanged`) — higher weight wins. Documented example: a chest of drawers where the drawer outranks the items inside it.
- **Interactor Tag** (`FGameplayTag`, `GetInteractorTag` / `SetInteractorTag`) vs. interactable compatible tags — lets you have several interactors on one pawn (hands, eyes, vehicle) that only see their own class of interactables.
- **Ignored Actors** list — per-interactor filtering (`AddIgnoredActor(s)` / `RemoveIgnoredActor(s)`).
- **Interaction Dependencies** — `AddInteractionDependency` / `ProcessDependencies`: while this interactor is Active, dependent interactors are forced to `EIS_Suppressed`. This is how "you cannot use the hover-interactor while holding an object" is expressed declaratively.
- **Safety trace** — `FSafetyTracingSetup` + `PerformSafetyTrace(const AActor*)`: a secondary occlusion check between the interactor and the candidate, so a wall between you and the item invalidates it even if the primary overlap succeeded.

---

## 6. Lifecycle & Cooldown

`EInteractableLifecycle`:

- `EIL_OnlyOnce` — after completion the interactable goes to `EIS_Completed` forever.
- `EIL_Cycled` — after each completion it goes to `EIS_Cooldown` for `CooldownPeriod`, then back to `EIS_Awake`; optionally limited by a max lifecycle count.

The chain is explicit and observable:

```
InteractionStarted -> InteractionCycleCompleted (n times) -> InteractionLifecycleCompleted
														  -> InteractionCooldownCompleted -> Awake
```

`OnInteractionCycleCompleted` even carries `RemainingLifecycles`, so UI can display "3 charges left" without extra plumbing.

---

## 7. Auto-Setup & Presentation

`AutoSetup()` + `ESetupType` remove almost all manual wiring:

- Walks the owning actor, finds the parent/primitive component, registers it as the **collision component**.
- Finds mesh components and registers them as **highlightable components**.
- Caches original collision settings in `FCollisionShapeCache` (`bGenerateOverlapEvents`, `CollisionEnabled`, `CollisionResponse`) so `CleanupComponent()` can restore the actor exactly to its pre-interaction state. This is a detail most in-house systems get wrong.

Highlighting is first-class: `EHighlightType::EHT_PostProcessing` or `EHT_OverlayMaterial`, with `OnHighlightTypeChanged` / `OnHighlightMaterialChanged` events, plus dynamic add/remove of highlightable and collision components at runtime.

Widget integration goes through `IMounteaInteractionWidget` + `UpdateInteractionWidget()`, and `FInteractionDeviceChanged(ECommonInputType, FName)` lets the prompt swap glyphs when the player switches between gamepad and keyboard.

---

## 8. Networking

The interactor base is genuinely replication-aware:

```cpp
UFUNCTION(Server, Reliable)   void SetState_Server(const EInteractorStateV2 NewState);
UFUNCTION(Server, Reliable)   void StartInteraction_Server(const float StartTime);
UFUNCTION(Server, Unreliable) void AddIgnoredActor_Server(AActor*);
UFUNCTION(Client, Reliable)   void SetActiveInteractable_Client(...);
UFUNCTION()                   void OnRep_InteractorState();
UFUNCTION()                   void OnRep_ActiveInteractable();
virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&) const override;
```

Pattern: **reliable RPC for state/interaction, unreliable for bookkeeping (ignored actors, dependencies, channels), `OnRep_` + `ProcessStateChanged_Client()` for cosmetic client reaction.** Note the header comment on some classes still says "Networking is not implemented" — the interactor base is ahead of its own documentation.

---

## 9. Configuration & Tooling

- `UMounteaInteractionSystemSettings` — project-level developer settings, plus `UMounteaInteractionSettingsConfig` as a **data asset** so different games/levels can swap whole configuration sets.
- `EMounteaInteractionLoggingVerbosity` — bitflag enum (`Info | Warning | Error`) so verbosity is selectable per-category in the editor, not a single global level.
- `FDebugSettings { DebugMode : 1; EditorDebugMode : 1; }` — separates *runtime* debug draw from *editor* validation warnings.
- `ToString_Implementation()` on both interfaces — every object can describe itself for logs.
- Editor module: custom details panel (`MounteaInteractableBase_DetailsPanel`), class viewer filters, asset factories for config assets and interactable component blueprints, and an in-editor help/wiki button.

---

## 10. Strengths Summary

1. **Interface-driven** — zero hard coupling, everything overridable in BP.
2. **Explicit state machines** with error-message-returning transitions.
3. **Three-layer event chain** giving delegate + C++ + BP hooks uniformly.
4. **One subclass per interaction verb** instead of an enum-driven mega class.
5. **Real arbitration** (weight, tags, dependencies, ignore list, safety trace).
6. **Lifecycle/cooldown/pause** built in rather than bolted on per-actor.
7. **Auto-setup with collision caching and restore.**
8. **Replication patterns already in place.**
9. **Presentation (highlight, widget, input-device glyphs) is part of the contract.**
10. **First-class debug/logging/editor tooling.**

## 11. Weaknesses Summary

1. **Interface surface is enormous** — ~80 pure-virtual functions to implement a custom interactable from scratch.
2. **`BlueprintNativeEvent` everywhere** costs performance on hot paths (evaluation happens per detection tick).
3. **Delegate explosion** — 30+ dynamic multicast delegate types in one header; heavy compile-time and cognitive load.
4. **No Gameplay Ability System integration** — interactions are hard-coded behaviours, not abilities; no cost/cooldown/tag-blocking from GAS, no prediction.
5. **Interaction "actions" are not data** — an interactable is one interaction, so multi-choice prompts ("Open / Lock / Inspect") require multiple components.
6. **No spatial registry** — detection relies on traces/overlaps per interactor rather than a queryable world subsystem.
7. Documentation drift ("Networking is not implemented" comments on networked classes).

---

## 12. What Is Worth Stealing

| Feature | Why |
|---|---|
| Interactable state machine + `ErrorMessage` transitions | Debuggability and correct gating for free. |
| Interaction weight + interactor tag arbitration | Solves nested/overlapping interactables properly. |
| Lifecycle / cooldown / cycle-count | Removes per-actor timer boilerplate. |
| Interaction dependencies (suppression) | Declarative interactor conflict resolution. |
| Safety trace | Prevents interacting through walls. |
| Auto-setup with collision caching | Massive designer-time saver. |
| Three-layer event chain | Consistent extension surface. |
| Highlight abstraction + device-aware widget updates | Presentation without per-actor Blueprint spaghetti. |
| Per-category logging verbosity + `ToString()` | Cheap, high-value diagnostics. |
| Trace pipeline (`ProcessTrace` precise/loose, re-armed timer, safety trace) | Adopted wholesale — see architecture §2.2.1. |

## 13. What We Deliberately Did NOT Take

| Rejected | Reason |
|---|---|
| Interface-first design (`UINTERFACE` / `TScriptInterface`) | Unusable from UE5 Angelscript. Replaced by a response-component mailbox plus responder components (architecture §2.3–§2.4). |
| One component per interaction verb (Press/Hold/Mash/Hover/Automatic) | FV keeps one `UInteractableComponent` with an offer array; the verb is `EInteractionInputMode` on the offer and the timing FSM lives in the interactor (architecture §3.1–§3.3). |
| Replication patterns | Out of scope this pass; funnelled through `TrySetState` / commit so it can be added later. |
| `BlueprintNativeEvent` on hot paths | Plain virtuals in detection/scoring; script hooks only on low-frequency events. |
| 30+ dynamic multicast delegates | Collapsed to ~7 on the response component. |
| Per-component `DebugMode` UPROPERTY | Debug is gated exclusively by CVars (architecture §6.4.3). |
| Overlap-based interactor | Broad phase is the registry subsystem's in-range diffing, not per-actor overlap volumes. |

See `FVInteractionSystem_Improvements.md` for the gap analysis and `FVInteractionSystem_Architecture.md` for the authoritative design.
