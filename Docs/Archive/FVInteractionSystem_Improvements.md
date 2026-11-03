# FVInteractionSystem — Improvement Plan (Mountea patterns × Gameplay Ability System)

> Compares `Plugins/FVInteractionSystem/` against `Plugins/MounteaInteractionSystem/` and proposes a GAS-native evolution.
> Companion documents: `MounteaInteractionSystem_Analysis.md` (concept mining) and
> `FVInteractionSystem_Architecture.md` — **the authoritative layer/responsibility design.**
> Where the two disagree, the architecture doc wins.

---

## 1. Where FVInteractionSystem Stands Today

### 1.1 Current shape

| Piece | File | Role |
|---|---|---|
| `UInteractableComponent` | `Public/Components/InteractableComponent.h` | Holds `Type` tag, `DetectionRadius`, `FocusComponentTag`, and `TArray<FInteractionOffer> Offers`. |
| `UInteractorComponent` | `Public/Components/InteractorComponent.h` | Ticks, queries registry, sweeps, picks focus, caches `FInteraction` prompts, executes via `FExecuteInteractionAction`. |
| `UInteractionRegistrySubsystem` | `Public/Subsystems/` | World subsystem; `Register` / `Unregister` / `QueryInRange`. |
| `UInteractionRequirement` | `Public/Core/InteractionRequirement.h` | `EditInlineNew` predicate with `EInteractionGate::Hide/Disable`. |
| `FInteractionOffer` / `FInteraction` | `Public/Core/InteractionTypes.h` | Authoring data (InputTag + ActionTag + Requirements) and runtime prompt (InputTag + ActionTag + bCanExecute). |
| GAS bridge | `FVPlayerCharacter::BeginPlay` | `Interactor->ExecuteAction.BindWeakLambda(... ASC->ExecuteInteractionAction(ActionTag))`. |
| In-progress work | `Core/InteractionTypes.h` (commented out) | Draft `EInteractableLifecycle`, `EInteractorPrecision`, `EInteractorState`, `EInteractableState`, `EHighlightType`, `EHighlightSetupType`, `FInteractionHighlightSetup`. |
| In-progress work | `Interfaces/InteractableInterface.h`, delegates using `TScriptInterface<IInteractableInterface>` | **To be replaced** — see §2.1, interfaces are not usable from Angelscript. |
| `UFVInteractAbility` | `Source/FVGameplay/Private/Abilities/` | Ability helper that resolves interactor/interactable from actor info. |
| Editor validation | `FVInteractionSystemEditor/Validation/` | Blueprint compiler extension + rule registry (`ValidateInteractableSetupRule`). |

### 1.2 What FV already does better than Mountea

- **Multiple offers per interactable.** `TArray<FInteractionOffer>` means one component can expose "Open", "Lock", "Inspect". Mountea needs one component per verb.
- **GAS as the execution layer.** Actions are gameplay abilities → cost, cooldown, tag blocking, animation montages, prediction all come for free.
- **World registry subsystem** — `QueryInRange` is O(registered) instead of per-interactor physics overlap.
- **Composable requirements** as instanced `UObject`s with a Hide/Disable gate — a cleaner concept than Mountea's ad-hoc `CanInteract` overrides.
- **Editor compile-time validation** via a rule registry — Mountea only warns at runtime.
- **Lean, cheap runtime** — plain virtuals and raw structs, no `BlueprintNativeEvent` in the detection loop.

### 1.3 Concrete gaps

| # | Gap | Impact |
|---|---|---|
| G1 | No interactable state machine (`Awake/Active/Cooldown/Completed/Disabled/Suppressed`). | Every "already opened" / "on cooldown" case is re-implemented per actor. |
| G2 | No interactor state (`Awake/Suppressed/Disabled`) — only a `bEnabled` bool. | Cutscenes, UI, vehicles, multiple interactors all need bespoke code. |
| G3 | No arbitration beyond "closest hit". No weight, no tags, no nesting. | Drawer-vs-item-in-drawer is unsolvable. |
| G4 | Interaction is instantaneous. No hold / mash / hover / automatic. | Every timed interaction must be hand-built inside an ability. |
| G5 | No lifecycle / cooldown / cycle count on the interactable. | Charges, respawning pickups, one-shot levers all bespoke. |
| G6 | No decoupling layer — everything is one concrete `UActorComponent` with all behaviour inline. | Cannot compose interactable behaviour; every variant edits the same class. |
| G7 | No presentation contract: no highlight, no widget interface, no device-aware glyphs (`FInteractionKeyBinding` exists but is unused). | UI/VFX is glued on per Blueprint. |
| G8 | No replication. | **Accepted for now** — out of scope, see §2.2. |
| G9 | `ExecuteAction` is a single non-dynamic delegate bound in `AFVPlayerCharacter::BeginPlay`. | Fragile coupling; only one listener; not script-facing; the plugin secretly depends on the game module's ASC. |
| G10 | No occlusion/safety validation independent of the sweep; `bDebugHitOccluder` exists but occlusion is not part of the offer evaluation contract. | Interact-through-walls edge cases. |
| G11 | Requirements can't express GAS conditions (`CanActivateAbility`, tags, attributes) generically. | Duplicate gating logic between requirement objects and abilities. |
| G12 | Logging is `UE_LOG(LogTemp, ...)`; no category, no verbosity flags, no `ToString`. | Poor diagnosability. |

---

## 2. Design Principle for the Upgrade

> **Mountea supplies the *state, arbitration and presentation* model. GAS supplies the *execution* model. FV should own the seam between them and never duplicate either.**

Rules of thumb:

1. **The interactable owns state; the ability owns behaviour.** The interactable never plays montages, spawns effects, or applies effects — it publishes state and lets the ability drive.
2. **Anything that can be a Gameplay Tag should be a Gameplay Tag** — interactable state, interactor state, requirement conditions, suppression.
3. **Anything that can be a `GameplayAbility` should not be a component subclass** — hold/mash/press timing lives in the ability (via `AbilityTask`s), *not* in five interactable subclasses.
4. **Keep the detection loop free of `BlueprintNativeEvent`.** Use plain virtuals + an optional BP hook only on the low-frequency events (focus changed, executed, state changed).
5. **No `UINTERFACE` on any type that gameplay code touches** — see §2.1.
6. **No replication in this pass** — see §2.2.
7. **Reuse the types already drafted in `Core/InteractionTypes.h`.** `EInteractableState`, `EInteractorState`, `EInteractableLifecycle`, `EInteractorPrecision`, `EHighlightType`, `EHighlightSetupType` and `FInteractionHighlightSetup` are already authored (commented out). Uncomment them rather than introducing parallel definitions; the only new enum this plan adds is `EInteractionInputMode` (§3.5).

### 2.1 Constraint: Angelscript — response components instead of interfaces

The project is scripted in **UE5 Angelscript**, where `UINTERFACE`s are discouraged/unavailable. The idiomatic replacement is a **response component carrying multicast events**:

```cpp
// Instead of: IDamageable::TakeDamage(...)
UDamageResponseComponent* Comp = UDamageResponseComponent::Get(Actor);
Comp->OnDamageTaken.Broadcast(...);
```

The component normally implements *nothing but a set of events*. Two benefits that an interface cannot give:

1. **Responders live in their own components, not in the actor.** `UExplodeOnDamageComponent` binds to `OnDamageTaken`, owns all the explosion logic, and can be dropped onto any actor. Many components can listen to the same event.
2. **Bindings are dynamic.** Components can be added/removed at runtime to change what an actor does — dependency inversion at the component level.

This maps cleanly onto GAS: the *response component* is the notification surface, the *gameplay ability* is the behaviour, and neither knows about the other's class.

**Therefore:**

Use `UInteractableComponent*` in those signatures instead.
- Every event exposed to script

### 2.2 Constraint: no replication for now

The original plan's replication phase is **dropped**. Build the system single-player-first:

- No `Server_*` / `Client_*` RPCs, no `OnRep_`, no `GetLifetimeReplicatedProps` in the interaction plugin.
- Do **not** add `Replicated` specifiers speculatively.
- Keep the door open cheaply: route *all* state changes through a single `TrySetState(...)` funnel and *all* execution through one `TryExecuteInteraction(...)` entry point. When multiplayer is needed, those two functions are the only places that need a server hop, and GAS already handles prediction for the ability itself. Nothing else in the design needs to change.

---

## 3. Proposed Improvements

### 3.1 Interactable state machine (fixes G1, G5)

**Do not define new enums.** `Core/InteractionTypes.h` already contains the drafted, commented-out `EInteractableState` and `EInteractableLifecycle` — uncomment and use them as-is:

```cpp
UENUM(BlueprintType, meta=(ScriptName="InteractableState"))
enum class EInteractableState : uint8
{
	Idle		UMETA(DisplayName = "Idle", Tooltip = "Default state. Interactable is not in player range."),
	Awake		UMETA(DisplayName = "Awake", Tooltip = "Interactable can react to Interactor."),
	Suppressed	UMETA(DisplayName = "Suppressed", Tooltip = "Interactions are disabled. e.g. Cutscenes, etc."),
	Interacting	UMETA(DisplayName = "Interacting", ToolTip = "Interactable is in use."),
	Paused		UMETA(DisplayName = "Paused", ToolTip = "Interaction is paused, waiting for player input."),
	Cooldown	UMETA(DisplayName = "Cooldown", ToolTip = "Interactions are disabled during cooldown period"),
	Completed	UMETA(DisplayName = "Completed", ToolTip = "Interaction is disabled, Cannot be activated again."),
	Default		UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EInteractableLifecycle : uint8
{
	OneShot		UMETA(DisplayName = "OneShot", Tooltip="Once interacted, interaction is disabled."),
	Repeatable	UMETA(DisplayName = "Repeatable", Tooltip="Interaction can be repeated n times."),
	Default		UMETA(Hidden)
};
```

Mapping to the Mountea concepts referenced elsewhere in this document:

| Mountea | FV equivalent (already drafted) | Notes |
|---|---|---|
| `EIS_Awake` | `Awake` | In range, can react. |
| `EIS_Active` | `Interacting` | |
| `EIS_Cooldown` | `Cooldown` | |
| `EIS_Paused` | `Paused` | |
| `EIS_Completed` | `Completed` | |
| `EIS_Suppressed` | `Suppressed` | |
| `EIS_Disabled` / `EIS_Asleep` | `Idle` | FV folds "off" and "out of range" into `Idle`; `Suppressed` covers deliberate blocking. |
| `EIL_OnlyOnce` / `EIL_Cycled` | `OneShot` / `Repeatable` | |

The only fix needed on the draft is the `Interactiong` typo in the `Interacting` display name (present in both `EInteractableState` and `EInteractorState`).

> **Superseded detail (architecture §2.2.2):** the *registry subsystem* owns the `Idle <-> Awake` transition via in-range set diffing, and focus is a separate axis (`bIsInFocus`) that never writes `State`.

Note `Idle` is the *default* state: an interactable only moves to `Awake` when an interactor's registry query brings it in range.

On `UInteractableComponent`:

```cpp
UFUNCTION(BlueprintCallable, Category="Interactable|State")
bool TrySetState(EInteractableState NewState, FString& OutError);   // Mountea's error-message pattern

UPROPERTY(BlueprintAssignable) FOnInteractableStateChanged OnStateChanged;

UPROPERTY(EditAnywhere, Category="Interactable|Lifecycle") EInteractableLifecycle Lifecycle = EInteractableLifecycle::OneShot;
UPROPERTY(EditAnywhere, Category="Interactable|Lifecycle", meta=(EditCondition="Lifecycle==EInteractableLifecycle::Repeatable", ClampMin="0")) int32 MaxCycles = 1;
UPROPERTY(EditAnywhere, Category="Interactable|Lifecycle", meta=(EditCondition="Lifecycle==EInteractableLifecycle::Repeatable", ClampMin="0")) float CooldownPeriod = 0.f;
```

**Mirror the state into GAS.** When state changes, add/remove a loose gameplay tag on the *interactable's* ASC (or an owned tag container queried by requirements):

```
Interaction.State.Idle / .Awake / .Suppressed / .Interacting / .Paused / .Cooldown / .Completed
```

This lets designers gate abilities with normal `ActivationRequiredTags` / `ActivationBlockedTags` on the target, with no new plumbing. Generate the tag from the enum name so the two can never drift.

Keep the transition table in one place (a static `IsTransitionAllowed(From, To, FString& OutError)`), exactly like Mountea — and return *why* it failed so the debug component can print it. `Default` is never a valid target state; treat it as an assert.

### 3.2 Interactor state (fixes G2)

**Reuse the drafted `EInteractorState`** already sitting commented out in `Core/InteractionTypes.h` — do not invent a new one:

```cpp
UENUM(BlueprintType, meta=(ScriptName="InteractorState"))
enum class EInteractorState : uint8
{
	Idle		UMETA(DisplayName = "Idle", Tooltip = "Default state. No Interactables in range."),
	Awake		UMETA(DisplayName = "Awake", Tooltip = "Interactor is looking for Interactables."),
	Suppressed	UMETA(DisplayName = "Suppressed", Tooltip = "Interactions are disabled. e.g. Cutscenes, etc."),
	Interacting	UMETA(DisplayName = "Interacting", ToolTip = "Interactor is in use."),
	Default		UMETA(Hidden)
};
```

Replace `bool bEnabled` with this enum, plus:

```cpp
UFUNCTION(BlueprintCallable) bool TrySetInteractorState(EInteractorState NewState, FString& OutError);
UFUNCTION(BlueprintCallable) void AddInteractionDependency(UInteractorComponent* Other);
UFUNCTION(BlueprintCallable) void RemoveInteractionDependency(UInteractorComponent* Other);
void ProcessDependencies();   // suppress dependents while this one is Interacting
```

Mapping notes vs. Mountea: `EIS_Asleep`/`EIS_Disabled` both collapse into `Idle`, and `EIS_Active` is `Interacting`. `Idle` means "detection running, nothing found"; `Awake` means "candidates in range". Because there is no separate `Disabled`, an explicitly turned-off interactor uses `Suppressed` — which is also what dependencies and cutscenes use, so a suppression *reason* (a small `FGameplayTagContainer SuppressionReasons`) is worth adding so two systems can't un-suppress each other prematurely.

GAS synergy: suppression can also be driven by tags — while the ASC owns `State.Interacting` or `State.InCutscene`, the interactor is forced to `Suppressed`. Register a `RegisterGameplayTagEvent` listener instead of polling.

### 3.3 Offer-level arbitration (fixes G3)

Extend `FInteractionOffer` and `UInteractableComponent`:

```cpp
// FInteractionOffer
UPROPERTY(EditAnywhere) int32 Weight = 0;
UPROPERTY(EditAnywhere, meta=(Categories="Interaction.Interactor")) FGameplayTagContainer RequiredInteractorTags;
UPROPERTY(EditAnywhere) TSubclassOf<UGameplayAbility> AbilityOverride;  // optional, see 3.5

// UInteractableComponent
UPROPERTY(EditAnywhere, Category="Interactable|Arbitration") int32 InteractionWeight = 0;
```

`UInteractorComponent` gains `FGameplayTag InteractorTag` and a scoring function replacing "nearest wins":

```cpp
float ScoreCandidate(const UInteractableComponent& Candidate, const FHitResult& Hit) const;
// weight (dominant) -> screen-centre alignment -> distance -> tie-break by registration order
```

This directly solves the drawer/item case and enables multiple interactors per pawn (`Interactor.Hands`, `Interactor.Eyes`, `Interactor.Vehicle`).

### 3.4 Safety / occlusion trace as a first-class step (fixes G10)

> **Superseded by architecture §2.2.1:** detection uses the registry gate plus Mountea's `ProcessTrace` with `FTracingSetup` and `ESafetyTracingMode`. The struct below is kept only as the original rationale.

Promote the existing debug occluder logic into contract:

```cpp
USTRUCT(BlueprintType)
struct FSafetyTraceSetup
{
	UPROPERTY(EditAnywhere) bool bEnabled = true;
	UPROPERTY(EditAnywhere) TEnumAsByte<ECollisionChannel> Channel = ECC_Visibility;
	UPROPERTY(EditAnywhere) float StartOffset = 0.f;
	UPROPERTY(EditAnywhere) FName TargetSocket = NAME_None;
};

bool PerformSafetyTrace(const UInteractableComponent& Candidate) const;
```

Run it *after* scoring and *before* the candidate becomes the focused target, and record the result for the debug component.

### 3.5 Interaction verbs as Ability Tasks, not component subclasses (fixes G4 — the key GAS divergence)

> **Superseded by architecture §3.1–§3.3:** press/hold/mash timing lives in the **interactor** input FSM (`PushInput` / `EInteractionInputPhase`), which reports progress through the response component; the ability only receives the commit. Field names are `InteractionPeriod` / `RequiredPresses`, not `HoldDuration` / `MashWindow`.

Mountea solves press/hold/mash/hover with five interactable subclasses. **Do not copy that.** In a GAS project the verb belongs to the ability.

This is the **only genuinely new enum** in the plan — everything else already exists as a draft in `InteractionTypes.h`:

```cpp
UENUM(BlueprintType, meta=(ScriptName="InteractionInputMode"))
enum class EInteractionInputMode : uint8 { Press, Hold, Mash, Hover, Automatic, Default UMETA(Hidden) };

// FInteractionOffer
UPROPERTY(EditAnywhere) EInteractionInputMode InputMode = EInteractionInputMode::Press;
UPROPERTY(EditAnywhere, meta=(EditCondition="InputMode==EInteractionInputMode::Hold")) float HoldDuration = 1.f;
UPROPERTY(EditAnywhere, meta=(EditCondition="InputMode==EInteractionInputMode::Mash"))  int32 RequiredPresses = 5;
UPROPERTY(EditAnywhere, meta=(EditCondition="InputMode==EInteractionInputMode::Mash"))  float MashWindow = 2.f;
```

Then add to `FVGameplay`:

- `UAbilityTask_InteractionHold` — waits for release/duration, broadcasts `OnProgress(float 0..1)` each tick for the radial UI, `OnCompleted`, `OnCancelled`.
- `UAbilityTask_InteractionMash` — counts presses inside the window, broadcasts `OnProgress`.
- `UFVInteractAbility` reads `EInteractionInputMode` from the focused offer and spawns the right task automatically, so **most interact abilities need zero graph work**.

Benefits over Mountea's approach: cancellation, prediction, cost/cooldown, montage sync and replication all come from GAS for free, and one component still exposes many verbs.

### 3.6 Replace the raw `ExecuteAction` delegate with an interaction response component (fixes G9)

Current coupling is fragile:

```cpp
// FVPlayerCharacter.cpp — plugin depends on game module behaviour via a lambda
Interactor->ExecuteAction.BindWeakLambda(this, [this](const FGameplayTag& ActionTag, const FInteractionContext&)
{
	return AbilitySystemComponent->ExecuteInteractionAction(ActionTag);
});
```

Replace it with a **response component** (§2.1), owned by the plugin, carrying nothing but events:

```cpp
UCLASS(MinimalAPI, ClassGroup=(Interaction), NotBlueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UInteractionResponseComponent final : public UActorComponent
{
	GENERATED_BODY()
public:
	// Convenience getter, mirrors UDamageResponseComponent::Get(Actor)
	UFUNCTION(BlueprintPure, Category="Interaction", meta=(DefaultToSelf="Actor"))
	static UE_API UInteractionResponseComponent* Get(AActor* Actor);

	// Fired by the interactor when an offer is triggered. Responders do the work.
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionRequested OnInteractionRequested;   // (FGameplayTag ActionTag, const FInteractionContext& Context)

	// Fired when focus/prompt state changes, for UI and VFX responders.
	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionFocusChanged OnFocusChanged;

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionOffersChanged OnOffersChanged;

	UPROPERTY(BlueprintAssignable, Category="Interaction")
	FOnInteractionProgress OnInteractionProgress;     // (FGameplayTag ActionTag, float Progress01)
};
```

`UInteractorComponent` resolves `UInteractionResponseComponent::Get(GetOwner())` once and broadcasts through it. The plugin therefore has **zero** knowledge of GAS.

On the game side, add a small responder component in `FVGameplay`:

```cpp
// UFVInteractionAbilityResponseComponent — binds to OnInteractionRequested in BeginPlay
// and forwards the request into GAS.
```

Because `bCanExecute` gating already happened in the interactor (via requirements, §3.7), the responder does not need to return a value — which is exactly why a multicast event works here where an interface method would have been needed for a return code. If a hard failure signal is ever required, publish it back as `OnInteractionFailed` rather than making the event non-multicast.

**Benefits over the interface version:** several responders can react to the same interaction (ability activation + analytics + audio + tutorial hints) without touching each other; responders can be added/removed at runtime (e.g. a `UDisableInteractionsDuringCutscene` component); and everything is reachable from Angelscript.

Also send a proper GAS payload from the responder rather than a bare tag:

```cpp
FGameplayEventData Payload;
Payload.EventTag       = ActionTag;
Payload.Instigator     = Context.Interactor;
Payload.Target         = Context.Target;
Payload.OptionalObject = FocusedTarget.Get();          // the UInteractableComponent
Payload.TargetData     = MakeTargetDataFromHit(Context); // interaction point / hit
UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Avatar, ActionTag, Payload);
```

This gives the ability the interaction point, the interactable component and the target actor without any extra lookups (`UFVInteractAbility::GetInteractable()` currently has to walk back through the interactor).

### 3.7 GAS-aware requirements (fixes G11)

`UInteractionRequirement` stays, but ship built-in GAS subclasses so designers stop writing duplicate logic:

| Class | Checks |
|---|---|
| `UInteractionRequirement_AbilityCanActivate` | `ASC->CanActivateAbility(Spec)` for the offer's action tag — single source of truth for cost/cooldown/blocking. |
| `UInteractionRequirement_HasTags` | Interactor and/or interactable owned tags (`RequiredTags`, `BlockedTags`). |
| `UInteractionRequirement_AttributeThreshold` | e.g. stamina ≥ X. |
| `UInteractionRequirement_InteractableState` | Requires `EInteractableState::Awake`. |
| `UInteractionRequirement_Fact` | Bridges the existing FactDB (`FlickerVoidCoreEditor/FactDB`). |

Critically, `UInteractionRequirement_AbilityCanActivate` means an interaction prompt greys out **exactly** when the ability would fail — no drift between UI and gameplay.

Also make requirement evaluation cache-friendly: only re-evaluate on focus change, on interactable state change, and on relevant ASC tag/attribute delegates — not every `DetectionUpdateInterval` tick.

### 3.8 Decoupling without interfaces (fixes G6)

Mountea's flexibility comes from its interface layer. Achieve the same decoupling the Angelscript way:

- **`UInteractableComponent` stays the single concrete interactable type.** Do not add `IInteractableInterface`; delete the one currently drafted. "Any actor can be interactable" is satisfied by *adding the component*, which is cheap and is already how the registry subsystem discovers targets.
- **Behaviour variation moves into responder components**, not into interactable subclasses. `UInteractableComponent` broadcasts; small single-purpose components react:

| Responder (example) | Binds to | Does |
|---|---|---|
| `UInteractableHighlightComponent` | `OnFocusChanged` | Post-process / overlay-material highlight. |
| `UInteractableLifecycleComponent` | `OnInteractionExecuted` | Cooldown / cycle counting / `Completed` transition. |
| `UInteractableAudioComponent` | `OnInteractionStarted`, `OnInteractionExecuted` | SFX. |
| `UInteractableStateVFXComponent` | `OnStateChanged` | Visual state feedback. |

This is strictly more flexible than an interface: a designer composes an interactable by stacking components, and a component can be added or removed at runtime to change behaviour.

- **For polymorphic access, use static getters instead of `TScriptInterface`:** `UInteractableComponent::Get(AActor*)`, `UInteractorComponent::Get(AActor*)`, `UInteractionResponseComponent::Get(AActor*)`. Keep all delegate signatures typed as concrete component pointers.
- **If C++-internal polymorphism is genuinely needed** (e.g. a scoring strategy), a plain non-`UObject` C++ abstract class is fine — it just must never appear in a `UFUNCTION` signature, `UPROPERTY`, or delegate parameter.

### 3.9 Presentation via response events (fixes G7)

No widget interface. The UI binds to the interactor's response component:

```cpp
// Widget / HUD binds in construct:
UInteractionResponseComponent* Response = UInteractionResponseComponent::Get(OwningPawn);
Response->OnOffersChanged.AddDynamic(this, &UInteractionPromptWidget::HandleOffersChanged);
Response->OnInteractionProgress.AddDynamic(this, &UInteractionPromptWidget::HandleProgress);
Response->OnFocusChanged.AddDynamic(this, &UInteractionPromptWidget::HandleFocusChanged);
```

Input-device changes get their own event on the same component (`OnInputDeviceChanged(ECommonInputType, FName)`) so glyphs swap without the widget polling.

Highlighting becomes `UInteractableHighlightComponent`, driven by the drafted (currently commented-out) `EHighlightType` / `EHighlightSetupType` / `FInteractionHighlightSetup` types in `InteractionTypes.h` — uncomment and use them. It auto-collects `UMeshComponent`s per `EHighlightSetupType` and, mirroring Mountea's auto-setup, **caches and restores the original collision/material settings** on `EndPlay`.

Finally, wire the already-declared-but-unused `FInteractionKeyBinding` (Key + Glyph) into prompt building so the UI gets its glyph from the plugin instead of guessing.

### 3.10 Replication — deferred (G8)

**Out of scope for this pass** (§2.2). Single-player only. The only structural requirement is the funnelling described in §2.2: one `TrySetState` for all interactable state changes, one `TryExecuteInteraction` for all execution. Do not add RPCs, `OnRep_`, or `Replicated` properties now.

### 3.11 Diagnostics (fixes G12)

- Declare `DECLARE_LOG_CATEGORY_EXTERN(LogFVInteraction, Log, All)` and replace all `LogTemp` usage (`InteractorComponent::BeginPlay` currently warns via `LogTemp`).
- Add a bitflag verbosity enum on `UFVInteractionSystemSettings` (Info | Warning | Error), Mountea-style.
- Add `FString ToString() const` to `UInteractableComponent`, `UInteractorComponent`, `FInteractionOffer`, `FInteraction`.
- Extend `UInteractionDebugComponent` to draw: current state, score per candidate, failing requirement's class name and gate, safety-trace result, and the last `TrySetState` error string.
- Add compile rules to the existing `InteractionCompileRuleRegistry`: duplicate `InputTag` inside one `Offers` array, offer with no `ActionTag`, action tag with no matching granted ability, `Cooldown > 0` with `Lifecycle == OnlyOnce`.

---

## 4. Target Architecture (after the changes)

```
					┌──────────────────────────────┐
					│ InteractionRegistrySubsystem │  spatial query
					└───────────────┬──────────────┘
									│ QueryInRange
					┌───────────────▼──────────────┐
  input tag ───────▶│      UInteractorComponent    │  state, tag, dependencies
					│  detect → score → safetytrace│
					│  → focus → evaluate offers   │
					└───────┬──────────────┬───────┘
							│ prompts      │ UInteractionResponseComponent (events only)
			  ┌─────────────▼──────┐  ┌────▼───────────────────────────┐
			  │ Response listeners │  │ Game responder → ASC           │
			  │ (prompt/progress)  │  │  SendGameplayEvent(ActionTag)  │
			  └─────────▲──────────┘  └────┬───────────────────────────┘
						│ progress          │
						│            ┌──────▼─────────────────────────┐
						└────────────┤ UFVInteractAbility             │
									 │  AbilityTask_Hold / _Mash      │
									 │  cost, cooldown, montage, GEs  │
									 └──────┬─────────────────────────┘
											│ completed / cancelled
									 ┌──────▼─────────────────────────┐
									 │ UInteractableComponent         │
									 │ state machine + lifecycle      │
									 │ + offers + highlight           │
									 └────────────────────────────────┘
```

---

## 5. Suggested Implementation Order

| Phase | Work | Risk |
|---|---|---|
| **1** | `LogFVInteraction` category, `ToString()`, debug component expansion. | None — pure diagnostics, do it first so later phases are debuggable. |
| **2** | Remove `Interfaces/InteractableInterface.h` and retype the `TScriptInterface<IInteractableInterface>` delegates in `InteractionTypes.h` to `UInteractableComponent*`. | Low — do it before more code depends on the interface. |
| **3** | `UInteractionResponseComponent` (events only) + `::Get(AActor*)`; move the `ExecuteAction` lambda out of `AFVPlayerCharacter` into a game-side responder; `FGameplayEventData` payload with target data. | Low — isolated, immediately removes plugin↔game coupling. |
| **4** | Uncomment and finalise the drafted enums in `InteractionTypes.h`; add `EInteractableState` + `TrySetState(..., OutError)` + lifecycle/cooldown + state→gameplay-tag mirroring. | Medium — touches every interactable asset; add sensible defaults so existing content keeps working. |
| **5** | GAS requirement subclasses (`AbilityCanActivate`, `HasTags`, `InteractableState`) + event-driven requirement re-evaluation. | Low — additive; removes duplicated gating. |
| **6** | Arbitration: interactable/offer `Weight`, `InteractorTag`, `RequiredInteractorTags`, `ScoreCandidate`, safety trace (`EInteractorPrecision` already drafted). | Medium — changes focus selection; validate against existing levels. |
| **7** | `EInteractionInputMode` + `UAbilityTask_InteractionHold` / `_Mash` + auto-task selection in `UFVInteractAbility`; `OnInteractionProgress` broadcast. | Medium — new UI progress contract needed alongside. |
| **8** | Presentation responders: prompt widget bound to the response component, `UInteractableHighlightComponent` using `FInteractionHighlightSetup`, key-binding glyphs, `OnInputDeviceChanged`. | Low. |
| **9** | Interactor state machine + dependencies/suppression driven by ASC tag events. | Medium. |
| **10** | New compile rules in `InteractionCompileRuleRegistry`. | Low — ongoing. |
| *(deferred)* | Replication. | Out of scope — §2.2. |

---

## 6. Explicit Non-Goals

**Not copying from Mountea:**

1. **One component per interaction verb.** FV's offer array plus ability tasks is strictly better.
2. **`BlueprintNativeEvent` on every function.** Keep the detection/scoring loop as plain C++ virtuals; expose script hooks only on focus-changed, state-changed, requested, progress, executed.
3. **30+ dynamic multicast delegates.** Collapse to ~7: `OnFocusChanged`, `OnOffersChanged`, `OnInteractionRequested`, `OnInteractionStarted`, `OnInteractionProgress`, `OnInteractionExecuted`, `OnStateChanged`.
4. **Behaviour inside the interactable** (montages, effects, timers). That is the ability's or a responder component's job.
5. **Mountea's interface-first design.** Interfaces are unusable from Angelscript — response components replace them (§2.1).
6. **Mountea's dependency-suppression via manual arrays only.** Prefer gameplay-tag-driven suppression, with the array as a fallback.

**Not doing in this pass:**

7. **Replication** (§2.2).
8. **`TScriptInterface` / `UINTERFACE` anywhere in the public API**, including delegate parameters — including the `IInteractableInterface` currently drafted in the plugin, which should be removed.
9. **Non-dynamic delegates or templates in script-facing signatures.**
---

## 7. Quick Wins (under a day each)

1. Swap `UE_LOG(LogTemp, ...)` in `InteractorComponent::BeginPlay` for a real log category.
2. Use the unused `FInteractionKeyBinding` in prompt construction.
3. Return a meaningful result from `TryExecuteInteraction` — it currently returns `true` even when the offer was gated (`EDebugActionOutcome::Disabled`) and `false` only when there is no prompt.
4. Cache `MakeContext` output for the focused target instead of rebuilding per execution.
5. Only rebuild `CachedInteractions` when the focused target, its state, or a relevant tag actually changes — currently `RefreshOffers` runs every detection tick.
6. Add `int32 Weight` to `FInteractionOffer` and sort prompts by it — instant UI ordering improvement with no behavioural risk.
7. Retype `FInteractableSelected` / `FInteractableFound` / `FInteractableLost` in `InteractionTypes.h` from `TScriptInterface<IInteractableInterface>` to `UInteractableComponent*` and delete the forward declaration — removes the Angelscript blocker before anything binds to them.
