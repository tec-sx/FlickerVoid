# FVInteractionSystem — Architecture & Responsibilities

> Deep design analysis for the **rewrite**. Mountea is a source of *concepts only* — no dependency, no class parity, no naming parity.
> Companion docs: `MounteaInteractionSystem_Analysis.md` (concept mining), `FVInteractionSystem_Improvements.md` (feature backlog).

---

## 0. Reading Your Draft — Where You're Already Heading

The commented-out code in `Core/InteractionTypes.h` and `FVInteractionSystemSettings.h` already implies most of the architecture. Made explicit:

| Draft | What it implies |
|---|---|
| `FInteractorSettings` / `FInteractableSettings` in `UDeveloperSettings` | **Project-wide defaults, per-instance overrides.** Components should not carry hardcoded literals; they resolve from settings at construction. |
| `FTracingSetup { ValidationCollisionChannel, ActorMeshName="CharacterMesh0", StartSocketName="head" }` | Aim/occlusion tracing originates from a **mesh socket**, not the current `AimOriginOffset` vector hack, and validation uses a **separate channel** from detection. |
| `DefaultInteractionPeriod = 3.f` | Interactions have **duration**. Hold is a first-class concept, defaulted globally. |
| `DefaultLifecycles = -1`, `DefaultCooldownPeriod`, `EInteractableLifecycle` | **Repeatable interactions with cooldown and (optionally infinite) charges.** |
| `DefaultInteractableWeight = 1` | **Arbitration**, not "nearest wins". |
| `FInteractionHighlightSetup`, `EHighlightSetupType` | **Presentation is configured data, auto-set up**, not per-Blueprint wiring. |
| `WidgetUpdateFrequency = 0.05f` | UI is **pushed at a throttled rate**, never polled per-frame by the widget. |
| `InteractableMainTag`, `InteractorTag`, `FInteractorTagChanged` | **Tag-driven identity and matching** on both sides. |
| `FInteractionKeyPressed/Released(const float& Time)` | The interactor is aware of **input phases and their timestamps** — i.e. it owns press/hold/release timing. |
| `FIgnoredActorAdded/Removed`, `FCollisionChanged` | Runtime-mutable filtering, everything change-notified. |
| `EInteractorPrecision { High=LineTrace, Low=BoxOverlap }` | Detection strategy is **configuration**, not a subclass. |

Two things in the draft must change (see §7): the `TScriptInterface<IInteractableInterface>` delegate signatures (Angelscript), and `FInteractionKeyPressed/Released` as *interactor-level* events (they should be an input *entry point*, not an output event).

---

## 1. The Five Layers

```
+-----------------------------------------------------------------------+
| L0  INPUT                              (game module - FlickerVoidGame) |
|     EnhancedInput -> FGameplayTag InputTag + phase. Nothing else.      |
+-----------------------------------------------------------------------+
					| PushInput(InputTag, Phase, Time)
					v
+-----------------------------------------------------------------------+
| L1  INTERACTOR                                      (plugin - runtime) |
|     "What can I do right now, and did the player just commit to one?"  |
|     registry gate - trace - arbitration - focus - offers - input FSM   |
+-----------------------------------------------------------------------+
					| broadcast (never a direct call)
					v
+-----------------------------------------------------------------------+
| L2  MAILBOX            UInteractionResponseComponent (events only)     |
|     Zero logic. Decouples "who detects" from "who reacts".             |
+-----------------------------------------------------------------------+
		  |                          |                         |
		  v                          v                         v
+------------------+  +---------------------------+  +--------------------+
| L3a UI RESPONDER |  | L3b ABILITY RESPONDER     |  | L3c ANY RESPONDER  |
| prompts, glyphs, |  | (game) sends GameplayEvent|  | audio, analytics,  |
| progress bar     |  | -> GAS activates ability  |  | tutorial, quest    |
+------------------+  +---------------------------+  +--------------------+
									 |
									 v
+-----------------------------------------------------------------------+
| L4  INTERACTABLE                                    (plugin - runtime) |
|     Declaration + state. NO behaviour.                                 |
|     offers (tags + params) | state machine | lifecycle | focus point   |
|     is itself the target-side event hub for its own responders         |
+-----------------------------------------------------------------------+
					^
					| Register / Unregister / QueryInRange
+-----------------------------------------------------------------------+
| L5  REGISTRY SUBSYSTEM               UInteractionRegistrySubsystem     |
|     Spatial index of interactables. Knows nothing about interactors.   |
+-----------------------------------------------------------------------+

  Cross-cutting:  UInteractionRequirement (policy objects, L1 consults them)
				  UFVInteractionSystemSettings (defaults for L1 and L4)
```

**The two invariants that keep this clean:**

1. **The interactor never calls a responder, and no responder holds a pointer to the interactor.** All traffic goes through the mailbox (L2).
2. **The interactable never executes behaviour.** It declares and it remembers; responders act.

---

## 2. Responsibility Contracts

### 2.1 L0 — Input (game module)

**Owns:** `UInputAction`, `UInputMappingContext`, `FKey`, device detection, `AFVPlayerController`.
**Produces:** an `FGameplayTag` and a phase. Nothing else crosses the boundary.

```cpp
// AFVPlayerController — the ONLY place EnhancedInput appears
void AFVPlayerController::Input_AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (InputTag.MatchesTag(FVCoreTags::InputTag_Interaction))
	{
		if (UInteractorComponent* Interactor = UInteractorComponent::Get(CachedCharacter.Get()))
		{
			Interactor->PushInput(InputTag, EInteractionInputPhase::Pressed);
			return;                                     // consumed
		}
	}
	FVPlayer->GetFVAbilitySystemComponent()->AbilityInputTagPressed(InputTag);
}

void AFVPlayerController::Input_AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (InputTag.MatchesTag(FVCoreTags::InputTag_Interaction))
	{
		if (UInteractorComponent* Interactor = UInteractorComponent::Get(CachedCharacter.Get()))
		{
			Interactor->PushInput(InputTag, EInteractionInputPhase::Released);
			return;
		}
	}
	...
}
```

> **Fix an existing bug while you're here:** `Input_AbilityInputTagReleased` currently forwards *every* release to the ASC, including interaction tags whose press was consumed by the interactor. Release must be routed symmetrically or hold/mash can never work.

**Rule: the plugin has zero EnhancedInput dependency.** `FVInteractionSystem.Build.cs` must not reference `EnhancedInput`. `FKey` appears in the plugin only inside `FInteractionKeyBinding`, as inert display data (§5.3).

### 2.2 L1 — `UInteractorComponent`

**Single question it answers:** *"Given where I'm looking and who I am, what interactions are available, and has the player just committed to one?"*

| Responsibility | Detail |
|---|---|
| Detection | Two-phase: registry broad-phase gate, then timer-driven multi-hit trace. See §2.2.1. |
| Validation | Safety/occlusion line trace on `ValidationCollisionChannel`, plus ignored-actor filtering. |
| Arbitration | Score candidates: weight, then interactor-tag compatibility, then view alignment, then distance. |
| Focus | Owns exactly one `FocusedTarget`. Focus is **not** a state — see §2.2.2. |
| Offer evaluation | Runs `UInteractionRequirement`s to produce `TArray<FInteraction>` with `bCanExecute` / hidden. |
| **Input intent FSM** | Consumes `PushInput(Tag, Phase)`, times hold/mash, produces *one* commit event. |
| Progress | Publishes `Progress01` for the in-flight verb, throttled by `WidgetUpdateFrequency`. |
| State | Owns `EInteractorState` + suppression reasons. |

**It does NOT:** know about GAS, abilities, widgets, montages, highlighting, or what an action *means*.

#### 2.2.1 Detection — Registry Gate + Mountea-Style Tracing

Our current `DetectInteractables()` has a redundancy: it calls `Registry->QueryInRange(...)` to build `Candidates`, then loops the candidates doing a **second** per-interactable distance check to compute `GateRadius`, then sweeps, then does a **third** distance check on the hit. Three range tests for one decision. It also uses `SweepSingleByChannel`, so a non-interactable prop in front of a door swallows the hit entirely and the door is never seen.

Mountea's tracing is better in four specific ways, all of which we take:

| Mountea does | Why it's better |
|---|---|
| `LineTraceMultiByChannel` / `SweepMultiByChannel` | Multi-hit. Something in front of the interactable doesn't kill the trace; occlusion is decided deliberately by the safety trace, not accidentally by hit ordering. |
| `GetActorEyesViewPoint()` (or a custom transform) | Correct eye origin for free, no hand-tuned `AimOriginOffset` vector. |
| Timer-driven at `TracingInterval`, re-armed at the end of `ProcessTrace` | Non-reentrant, cost is explicit, and it can be paused/resumed/disabled as a unit. Not tick. |
| Picks best by **weight** across all hits, then `PerformSafetyTrace` on the winner | Arbitration and occlusion are separate, ordered steps. |

**Our one improvement over Mountea:** Mountea traces continuously whenever the interactor is awake. We don't need to — the registry already knows whether anything is nearby. So the registry becomes a **pure on/off gate for the trace timer**, and the redundant per-candidate and post-hit range checks disappear.

```
  Registry broad-phase (cheap, on a slow timer ~0.2s, or event-driven)
  ────────────────────────────────────────────────────────────────────
      QueryInRange(PawnLocation, MaxDetectionRadius) -> Candidates
                              |
          any candidate with EInteractorPrecision::High ?
                  |                              |
                 yes                             no
                  |                              |
            EnableTracing()               DisableTracing()
            (start/resume timer)          (clear timer, clear focus,
                  |                        broadcast OnFocusChanged(null))
                  v
  Narrow-phase ProcessTrace() @ TracingInterval
  ────────────────────────────────────────────────────────────────────
      origin  = FTracingSetup socket, else GetActorEyesViewPoint()
      end     = origin + forward * TracingRange
      High -> LineTraceMultiByChannel
      Low  -> SweepMultiByChannel (box, TracingShapeHalfSize)
                              |
      for each hit: resolve UInteractableComponent
                    reject: not registered / channel mismatch
                            / state can't be triggered
                            / RequiredInteractorTags vs InteractorTag mismatch
                              |
      keep highest InteractionWeight
                              |
      PerformSafetyTrace(winner)  ── line trace, ValidationCollisionChannel,
                              |     must hit the interactable actor first
                              |     (if blocked -> try next-best candidate)
                              v
      SetFocusedTarget(winner) -> RefreshOffers() -> broadcast
                              |
                    re-arm timer, return
```

Consequences for our code:

- **One range test, not three.** The registry query is the *only* place a radius is compared. `DetectInteractables`'s `GateRadius` loop and the post-sweep `Distance > DetectionRadius` rejection both get deleted. Whether a hit is "in range" is answered by `TracingRange` — which is what the trace already enforces geometrically.
- **`Single` → `Multi`.** Required for the weight-based arbitration and safety trace to mean anything.
- **`AimOriginOffset` → `FTracingSetup`.** `ESafetyTracingMode { None, Location, Socket }` mirrored from Mountea; socket mode resolves `StartSocketName` on `ActorMeshName` (your drafted `"head"` / `"CharacterMesh0"` defaults) and falls back to actor location if the socket is missing.
- **Tick → timer.** `PrimaryComponentTick` is disabled entirely on the interactor. `EnableTracing` / `DisableTracing` / `PauseTracing` / `ResumeTracing` become the public control surface, and suppression (§4.3) simply calls `PauseTracing`.
- **Zero cost when the world is empty.** No interactables nearby = no trace at all. Mountea can't say that.
- **`EInteractorPrecision` picks the trace function, not a subclass** — Mountea splits `UMounteaInteractorComponentTrace` / `...Overlap`; we keep one final component and switch inside `ProcessTrace`.

> The broad-phase gate should be driven by the registry itself where possible — `OnInteractableRegistered` / `OnInteractableUnregistered` plus a slow poll — rather than a fast timer. The point of the gate is that it is much cheaper than the thing it gates.

#### 2.2.2 `Idle` vs `Awake` — the registry's job, and why focus is not a state

Yes — that is exactly what those two members are for, and it makes the two phases map cleanly onto the two enum values:

| State | Means | Set by | Phase |
|---|---|---|---|
| `Idle` | Out of range. Nobody could interact with me even if they aimed perfectly. Costs nothing, participates in nothing. | broad-phase | registry gate |
| `Awake` | In range. I am a live candidate — trace against me, evaluate my offers, let me highlight. | broad-phase | registry gate |
| `Interacting` / `Paused` / `Cooldown` / `Completed` / `Suppressed` | Everything else | itself / responders | — |

So the **registry drives `Idle <-> Awake`, not the interactor.** That's a correction to what I wrote earlier — I had the interactor doing it, which is wrong for three reasons:

1. **N interactors.** With a player and a companion, "am I in someone's range" is a set membership question, not a single-owner one. If the interactor flipped the state, two interactors would fight over it. The registry answers it once for everyone.
2. **It's the gate's own output.** The broad-phase already computes exactly this set every cycle. Having it publish the result as state is free; making the interactor recompute it is the third redundant range check we just deleted.
3. **Focus is transient, range is not.** Looking away from a door must not put it back to `Idle` — it's still in range, still a candidate, still needs to be traced next cycle. If focus wrote to state, the door would thrash `Awake -> Idle -> Awake` every time you glanced sideways, dragging highlight and prompt logic with it.

**Therefore focus is deliberately not a state.** It's a separate boolean/event on the interactable:

```cpp
// STATE — where am I in my own lifecycle (registry + responders write this)
EInteractableState State;                      // Idle / Awake / Interacting / ...

// FOCUS — is someone currently aiming at me (interactor writes this, per-interactor)
bool bIsInFocus;
FOnInteractableFocusChanged OnFocusStateChanged;   // (bool bFocused, UInteractorComponent* By)
```

Two orthogonal axes. `Awake + not focused` = "in range, glowing faintly / no prompt". `Awake + focused` = "highlighted, prompt on screen". `Idle` = invisible to the whole system regardless of focus. Trying to encode both on one enum is what forces the `Awake -> Idle` thrash.

This also gives the state machine a clean meaning for the transitions the interactor *does* own — it never writes `Idle` or `Awake`, only `Awake -> Interacting` on commit and `Interacting -> Awake` on finish/cancel:

```
     [Idle] ──── entered range (registry) ────► [Awake] ──── commit ────► [Interacting]
        ▲                                          ▲   │                       │
        └──────── left range (registry) ───────────┘   │                       │
                                                       └──── finish/cancel ────┘
                                                              (interactor)

  bIsInFocus toggles freely while Awake or Interacting — it never changes State.
```

Practical consequences:

- **`Idle` is the cheap default.** An interactable spawns `Idle` and stays there for most of the game. Highlight components, prompt evaluation and offer refresh can all early-out on `State == Idle`.
- **The registry needs enter/leave events, not just `QueryInRange`.** It should track the previous in-range set per interactor and diff it, so it can call `TrySetState(Awake)` / `TrySetState(Idle)` on the deltas only. Union across all interactors: an interactable stays `Awake` while *any* interactor has it in range.
- **`DetectionRadius` on the interactable is what the registry compares against** — that's its only remaining use, and it's the one range test we kept.
- **The gate condition gets simpler:** "are there any `Awake` high-precision interactables?" instead of re-deriving range in the interactor.
- **`Suppressed` is the manual override of this axis** — a cutscene forces an in-range interactable out of consideration without lying about its range.


### 2.3 L2 — `UInteractionResponseComponent` (instigator-side mailbox)

Logic-free. Exists so L1 and L3 never see each other.

```cpp
UCLASS(MinimalAPI, final, ClassGroup=(Interaction), NotBlueprintable,
       meta=(BlueprintSpawnableComponent, DisplayName="Interaction Response"))
class UInteractionResponseComponent final : public UActorComponent
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category="Interaction", meta=(DefaultToSelf="Actor"))
	static UE_API UInteractionResponseComponent* Get(AActor* Actor);

	// --- L1 -> L3 -------------------------------------------------------
	UPROPERTY(BlueprintAssignable) FOnInteractionFocusChanged   OnFocusChanged;    // (UInteractableComponent* Target)
	UPROPERTY(BlueprintAssignable) FOnInteractionOffersChanged  OnOffersChanged;   // (const TArray<FInteraction>&)
	UPROPERTY(BlueprintAssignable) FOnInteractionStarted        OnInteractionStarted;   // (FInteractionCommit)
	UPROPERTY(BlueprintAssignable) FOnInteractionProgress       OnInteractionProgress;  // (FGameplayTag Action, float P01)
	UPROPERTY(BlueprintAssignable) FOnInteractionRequested      OnInteractionRequested; // (FInteractionCommit) <-- the commit
	UPROPERTY(BlueprintAssignable) FOnInteractionCancelled      OnInteractionCancelled; // (FGameplayTag, FGameplayTag Reason)

	// --- L3 -> L2 (responder-driven duration, see 3.3) -------------------
	UFUNCTION(BlueprintCallable, Category="Interaction")
	UE_API void ReportInteractionProgress(FGameplayTag ActionTag, float Progress01);

	UFUNCTION(BlueprintCallable, Category="Interaction")
	UE_API void ReportInteractionFinished(FGameplayTag ActionTag, bool bSucceeded);
};
```

Why a separate component instead of putting these on the interactor: responders (UI, audio, quest) must survive the interactor being disabled/suppressed/absent; an AI or a cutscene can drive the same mailbox with no interactor at all; and it enforces invariant #1 structurally rather than by convention.

### 2.4 L3 — Responders

A responder is **a component that binds to a mailbox event in `BeginPlay` and owns one concern**. That is the whole contract. Add/remove at runtime to change behaviour.

| Responder | Side | Binds | Owns |
|---|---|---|---|
| `UFVInteractionAbilityResponder` | instigator (game) | `OnInteractionRequested` | Builds `FGameplayEventData`, sends to ASC. **Only place GAS is touched.** |
| `UInteractionPromptResponder` | instigator | `OnFocusChanged`, `OnOffersChanged`, `OnInteractionProgress` | Drives the widget. |
| `UInteractionAudioResponder` | instigator | `OnInteractionStarted/Requested/Cancelled` | SFX. |
| `UInteractableHighlightResponder` | target | `OnFocusStateChanged` | Post-process / overlay material. |
| `UInteractableLifecycleResponder` | target | `OnInteractionCompleted` | Cooldown timer, charge count, `Completed` transition. |
| `UInteractableStateVFXResponder` | target | `OnStateChanged` | Visual state feedback. |
| `UOpenDoorOnInteract` etc. | target | `OnInteractionCompleted` | Actual gameplay result, *when GAS is overkill*. |

The last row matters: for a light switch you do **not** need an ability. A target-side responder listening for `Interaction.Action.Toggle` is enough. GAS is for interactions that need cost/cooldown/montage/prediction.

#### 2.4.1 "Response component" vs "responder" — they are different things

Two distinct roles that unfortunately sound alike. Renaming for clarity:

| | `UInteractionResponseComponent` (L2) | `UInteractionResponderComponent` subclasses (L3) |
|---|---|---|
| Count per actor | **exactly one** | **zero or many** |
| Contains | `UPROPERTY(BlueprintAssignable)` delegates + `Report*` functions. **No logic.** | `BeginPlay` binding + one behaviour. **All the logic.** |
| Who broadcasts | the interactor writes into it | never broadcasts, only listens |
| Analogy | the mailbox bolted to the house | the people who check the mail |
| Subclassed? | never — mark `final` | always — that's its only purpose |

The mailbox exists so responders don't need a pointer to the interactor and don't need to be present at the same time. Add a responder at runtime, it starts working; remove it, it stops; the interactor never knew either happened.

**On the target side there is no separate mailbox** — `UInteractableComponent` *is* the mailbox, because it's mandatory on an interactable actor anyway. Target-side responders bind to it directly.

#### 2.4.2 What setup actually looks like

The base class is thin enough that a responder is ~10 lines:

```cpp
UCLASS(Abstract, ClassGroup=(Interaction), meta=(BlueprintSpawnableComponent))
class UInteractionResponderComponent : public UActorComponent
{
    GENERATED_BODY()
protected:
    UE_API virtual void BeginPlay() override final;   // finds mailbox, calls BindResponses

    // implement one of these
    UFUNCTION(BlueprintNativeEvent, Category="Interaction")
    void BindResponses(UInteractionResponseComponent* Mailbox);          // instigator side
    UFUNCTION(BlueprintNativeEvent, Category="Interaction")
    void BindInteractableResponses(UInteractableComponent* Interactable); // target side

    UPROPERTY(BlueprintReadOnly) TObjectPtr<UInteractionResponseComponent> Mailbox;
};
```

**Instigator side — the GAS bridge (the only GAS-aware class in the whole design):**

```cpp
UCLASS(ClassGroup=(FV), meta=(BlueprintSpawnableComponent))
class UFVInteractionAbilityResponder final : public UInteractionResponderComponent
{
    GENERATED_BODY()
protected:
    virtual void BindResponses_Implementation(UInteractionResponseComponent* InMailbox) override
    {
        InMailbox->OnInteractionRequested.AddDynamic(this, &ThisClass::HandleRequested);
    }

    UFUNCTION()
    void HandleRequested(const FInteractionCommit& Commit)
    {
        FGameplayEventData Payload;
        Payload.EventTag       = Commit.ActionTag;
        Payload.Instigator     = GetOwner();
        Payload.Target         = Commit.Context.Target;
        Payload.OptionalObject = Commit.Interactable;
        UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), Commit.ActionTag, Payload);
    }
};
```

**Target side — a door that opens itself, no GAS:**

```cpp
UCLASS(ClassGroup=(FV), meta=(BlueprintSpawnableComponent))
class UOpenDoorResponder final : public UInteractionResponderComponent
{
    GENERATED_BODY()
protected:
    virtual void BindInteractableResponses_Implementation(UInteractableComponent* Interactable) override
    {
        Interactable->OnInteractionCompleted.AddDynamic(this, &ThisClass::HandleCompleted);
    }

    UFUNCTION()
    void HandleCompleted(FGameplayTag ActionTag, AActor* Instigator, bool bSucceeded)
    {
        if (bSucceeded && ActionTag.MatchesTagExact(FVTags::Interaction_Action_Open))
        {
            // play the timeline
        }
    }
};
```

**Editor setup, start to finish:**

```
BP_PlayerCharacter                        BP_Door
├── UInteractorComponent          (1)     ├── UInteractableComponent            (1, is its own mailbox)
├── UInteractionResponseComponent (1)     │     Type = Interactable.Type.Door
├── UFVInteractionAbilityResponder        │     Offers = [ Primary -> Open   (Press)
├── UInteractionPromptResponder           │                Secondary -> Lockpick (Hold 4s, needs lockpick) ]
└── UInteractionAudioResponder            ├── UOpenDoorResponder                (no GAS needed)
                                          ├── UInteractableHighlightResponder
                                          └── UInteractableLifecycleResponder
```

Note there is no wiring step. Nobody assigns a reference to anybody. Each responder finds its mailbox on `BeginPlay` via `UInteractionResponseComponent::Get(GetOwner())` or `GetOwner()->FindComponentByClass<UInteractableComponent>()`. Drag the component on, it works; delete it, that behaviour is gone. Blueprint responders subclass `UInteractionResponderComponent` and override `BindResponses` — same contract, no C++.

> **Rule of thumb:** if it *broadcasts*, it's the mailbox (one). If it *binds*, it's a responder (many). Nothing does both.

### 2.5 L4 — `UInteractableComponent`

**Declaration + state. Zero behaviour.** It is simultaneously the target-side event hub (no second mailbox component — it must exist anyway, and making responders bind to it keeps interactable actors at one mandatory component).

```cpp
// DECLARATION
FGameplayTag                Type;               // Interactable.Type.Door
TArray<FInteractionOffer>   Offers;             // what can be done here
int32                       InteractionWeight;  // arbitration
float                       DetectionRadius;
FName                       FocusComponentTag;
FInteractionHighlightSetup  HighlightSetup;
EHighlightSetupType         HighlightSetupType;

// STATE
EInteractableState          State;              // Idle/Awake/Suppressed/Interacting/Paused/Cooldown/Completed
EInteractableLifecycle      Lifecycle;
int32                       RemainingLifecycles;// -1 = infinite

// EVENTS (target-side mailbox)
FOnInteractableStateChanged     OnStateChanged;
FOnInteractableFocusChanged     OnFocusStateChanged;
FOnInteractableInteractionBegan OnInteractionBegan;
FOnInteractableInteractionEnded OnInteractionCompleted;   // (ActionTag, Instigator, bSucceeded)
```

**It does NOT:** trace, score, time input, open doors, play montages, or know a single thing about the interactor's implementation.

### 2.6 L5 — `UInteractionRegistrySubsystem`

Spatial index only. Today it is a flat array + `QueryInRange`; when it becomes hot, swap the internals for a grid/hash. **Nothing outside it may cache the array.** It must never know what an interactor is.

---

## 3. Input: Tag-Only, and Who Times What

### 3.1 The input contract

```cpp
UENUM(BlueprintType, meta=(ScriptName="InteractionInputPhase"))
enum class EInteractionInputPhase : uint8
{
	Pressed     UMETA(DisplayName="Pressed"),
	Released    UMETA(DisplayName="Released"),
	Cancelled   UMETA(DisplayName="Cancelled", ToolTip="Focus lost, interactor suppressed, input device change, etc."),
	Default     UMETA(Hidden)
};

UFUNCTION(BlueprintCallable, Category="Interaction")
bool PushInput(FGameplayTag InputTag, EInteractionInputPhase Phase);
```

That is the **entire** input surface. Yes — tag-based only. No `UInputAction`, no `FKey`, no `FInputActionValue` anywhere in the plugin.

This is also why `FInteractionKeyPressed` / `FInteractionKeyReleased` in your draft should be **deleted as interactor output events**: raw key phases are an *input* to L1, not something L2/L3 should ever see. What L3 wants is `OnInteractionStarted` / `OnInteractionProgress` / `OnInteractionRequested` / `OnInteractionCancelled` — semantic, not mechanical.

### 3.2 Press / Hold / Mash — the interactor owns the timing

The verb is declared **per offer**, resolved from settings when unset:

```cpp
// FInteractionOffer
UPROPERTY(EditAnywhere, meta=(Categories="InputTag.Interaction")) FGameplayTag InputTag;
UPROPERTY(EditAnywhere, meta=(Categories="Interaction.Action"))   FGameplayTag ActionTag;
UPROPERTY(EditAnywhere) EInteractionInputMode InputMode = EInteractionInputMode::Press;
UPROPERTY(EditAnywhere, meta=(EditCondition="InputMode==EInteractionInputMode::Hold", Units="s"))
float InteractionPeriod = -1.f;                 // -1 = use Settings.DefaultInteractionPeriod
UPROPERTY(EditAnywhere, meta=(EditCondition="InputMode==EInteractionInputMode::Mash"))
int32 RequiredPresses = 5;
UPROPERTY(EditAnywhere) int32 Weight = 0;
UPROPERTY(EditAnywhere, Instanced) TArray<TObjectPtr<UInteractionRequirement>> Requirements;
```

The interactor runs a small FSM per focused offer:

```
				 PushInput(Tag, Pressed)
						  |
			 +------------+-------------+
			 |            |             |
		  [Press]      [Hold]        [Mash]
			 |            |             |
		commit now   start timer   count++, window timer
			 |            |             |
			 |     tick -> OnInteractionProgress(p01)
			 |            |             |
			 |   Released early?   window expired?
			 |     -> Cancelled     -> Cancelled
			 |            |             |
			 |     period reached  count reached
			 |            |             |
			 +------------+-------------+
						  v
			  OnInteractionRequested(FInteractionCommit)
```

`Hover` and `Automatic` never consult input at all — they commit from focus/proximity, which is why they belong in the same enum rather than being separate component classes.

**Why the interactor and not a GAS AbilityTask:**

- The prompt and its progress ring must behave identically whether the responder is GAS, a plain target-side responder, or nothing at all. Progress cannot depend on an ability existing.
- Cancellation sources are all interactor-side facts (focus lost, occluded, interactor suppressed, requirement became unmet mid-hold). The interactor already knows them the same frame; an ability would have to be told.
- It keeps *one* commit event, so responders never have to distinguish "press interaction" from "hold interaction".
- Requirements can be re-checked *during* the hold and cancel it — impossible if the ability owns the timer and hasn't activated yet.

Trade-off accepted: no GAS prediction on the hold itself. Irrelevant while single-player (§ no-replication decision), and the *ability* still predicts normally once committed.

### 3.3 When the responder owns the duration instead

Some interactions are long *after* commit (lockpicking minigame, channelled ritual, hacking). Those declare `InputMode = Press`, and the **ability** owns the duration, reporting back through the same channel:

```cpp
// inside the ability's tick/task
UInteractionResponseComponent::Get(GetAvatarActorFromActorInfo())
	->ReportInteractionProgress(ActionTag, Elapsed / Duration);
```

The UI is bound to one event and cannot tell the difference. That is the point.

---

## 4. Tags: The Complete Taxonomy and What Each One Is For

Tags are the only vocabulary shared across layers. Five distinct roles — **do not let them bleed into each other**:

```
InputTag.Interaction.Primary            L0->L1  "which button"        (game owns the mapping)
InputTag.Interaction.Secondary
InputTag.Interaction.Alternate

Interaction.Action.Open                 L4->L3  "what should happen"  (matches ability / responder)
Interaction.Action.Take
Interaction.Action.Talk
Interaction.Action.Lockpick

Interactable.Type.Door                  L4      "what kind of thing"  (UI verb text, analytics, quests)
Interactable.Type.Container
Interactable.Type.NPC

Interaction.State.Awake                 L4      mirrored from EInteractableState, for gating
Interaction.State.Cooldown              (generate from the enum name so they cannot drift)
Interaction.State.Completed  ...

Interaction.Suppression.Cutscene        L1/L4   *reasons*, as a container, not a bool
Interaction.Suppression.Menu
Interaction.Suppression.Vehicle
Interaction.Suppression.Dependency

Interactor.Tag.Hands                    L1      "which interactor am I"
Interactor.Tag.Eyes                     (offer declares RequiredInteractorTags to match)
Interactor.Tag.Vehicle
```

### 4.1 InputTag -> ActionTag is the whole indirection

`InputTag` answers *"which button"*. `ActionTag` answers *"what happens"*. The offer is the mapping, authored per interactable. That is why the same E key opens a door and takes a bottle with no branching anywhere.

### 4.2 ActionTag is also the GAS trigger tag

Replace the manual spec scan in `UFVAbilitySystemComponent::ExecuteInteractionAction` (which iterates `ActivatableAbilities` comparing asset tags) with a plain gameplay event:

```cpp
// UFVInteractionAbilityResponder::HandleInteractionRequested
FGameplayEventData Payload;
Payload.EventTag       = Commit.ActionTag;
Payload.Instigator     = Commit.Context.Interactor;
Payload.Target         = Commit.Context.Target;
Payload.OptionalObject = Commit.Interactable;          // UInteractableComponent*
Payload.TargetData     = MakeTargetDataFromContext(Commit.Context);
UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Avatar, Commit.ActionTag, Payload);
```

Abilities then declare `AbilityTriggers` on `Interaction.Action.X`. Benefits: no O(n) scan, designers add an interaction verb by creating one ability asset, `UFVInteractAbility::GetInteractable()` stops walking back through the interactor, and the game code shrinks to one small responder.

### 4.3 Suppression is a tag container, not a bool

```cpp
UFUNCTION(BlueprintCallable) void AddSuppression(FGameplayTag Reason);
UFUNCTION(BlueprintCallable) void RemoveSuppression(FGameplayTag Reason);
// Interactor is Suppressed while SuppressionReasons.Num() > 0
```

Your `EInteractorState` has no `Disabled` member, so cutscenes, menus, vehicles and dependency-suppression all share `Suppressed`. Without reasons, whichever system clears first wrongly re-enables interaction. This is the single highest-value tag decision in the design.

### 4.4 State mirroring into GAS

On every `TrySetState`, update a loose tag on the interactable's tag container. Then `UInteractionRequirement_InteractableState` and ability `ActivationBlockedTags` share one truth. Derive the tag name from the enum so `Interaction.State.*` can never desync.

---

## 5. UI: Progress and Glyphs Without Coupling

### 5.1 Prompt data

The interactor publishes `TArray<FInteraction>`; extend it to what UI actually needs, all still tag-only:

```cpp
USTRUCT(BlueprintType)
struct FInteraction
{
	FGameplayTag InputTag;            // -> glyph lookup
	FGameplayTag ActionTag;           // -> verb text lookup
	FGameplayTag InteractableType;    // -> noun text / icon lookup
	EInteractionInputMode InputMode;  // -> hold ring vs. tap pip vs. mash pips
	int32 Weight;                     // -> display order
	bool bCanExecute;                 // -> greyed out
};
```

Note it carries **no strings, no textures, no keys**. Localisation and glyphs are game-side lookups keyed by tag.

### 5.2 Progress

One event, `OnInteractionProgress(FGameplayTag ActionTag, float Progress01)`, broadcast at `WidgetUpdateFrequency` (your existing setting) — not per frame, and only while a verb is in flight. Emitted by the interactor for interactor-timed verbs and by responders via `ReportInteractionProgress` for ability-timed ones (§3.3).

### 5.3 Getting the key/glyph to the UI without EnhancedInput in the plugin

Keep `FInteractionKeyBinding { FKey Key; TSoftObjectPtr<UTexture2D> Glyph; }` in the plugin as **inert display data**, and put the *resolution* in the game module:

```
UI widget receives FInteraction (InputTag only)
		|
		v
IFVInputGlyphProvider  -- game module, wraps EnhancedInput + CommonUI
   ResolveGlyph(InputTag, CurrentDevice) -> FInteractionKeyBinding
```

Implement it as a game subsystem, or (better, given §2.1's no-interface rule) as a `UFVInputGlyphSubsystem` with a `UFUNCTION` lookup. On device change the subsystem broadcasts `OnInputDeviceChanged`; the prompt responder re-resolves. The plugin never learns what a mapping context is.

---

## 6. Composition: How You Actually Author an Interactable Now

### 6.1 The recipe (no C++, no subclassing)

**A door that opens (ability-backed) and can be locked (needs a key):**

1. Add `UInteractableComponent` to `BP_Door`.
2. `Type = Interactable.Type.Door`, `Weight = 1`.
3. Offers:
   - `InputTag.Interaction.Primary` -> `Interaction.Action.Open`, `InputMode = Press`
   - `InputTag.Interaction.Secondary` -> `Interaction.Action.Lockpick`, `InputMode = Hold`, `InteractionPeriod = 4`, Requirement: `HasItem(Lockpick)` gate `Hide`
4. Add `UInteractableHighlightResponder` (or leave it to `EHighlightSetupType::Quick` auto-setup).
5. Add `UInteractableLifecycleResponder` if it should have a cooldown.
6. `GA_OpenDoor` triggers on `Interaction.Action.Open`; `GA_Lockpick` on `Interaction.Action.Lockpick`.

Zero code. Zero inheritance.

**A light switch (no GAS):** `UInteractableComponent` with one `Press` offer -> `Interaction.Action.Toggle`, plus a `UToggleLightOnInteract` responder bound to `OnInteractionCompleted`. No ability at all.

**A drawer inside a cabinet:** both get `UInteractableComponent`; the cabinet's `InteractionWeight = 0`, the drawer's `= 10`. Arbitration handles the rest.

### 6.2 Inheritance rules — enforce them with `final`

"Not designed for subclassing" is a comment nobody reads. Make it a compile error.

| Class | Declaration | Why |
|---|---|---|
| `UInteractorComponent` | `final` | Variation = `EInteractorPrecision` + `FTracingSetup` + settings. |
| `UInteractableComponent` | `final` | Variation = `Offers` (data) + responders (composition). |
| `UInteractionResponseComponent` | `final` | Pure mailbox. A subclass could only add logic, which is the one thing it must not have. |
| `UInteractionRegistrySubsystem` | `final` | One spatial index per world. |
| `UFVInteractionSystemSettings` | `final` | UDeveloperSettings singleton. |
| Concrete responders (`UOpenDoorResponder`, `UFVInteractionAbilityResponder`, …) | `final` | Each is a leaf owning one concern; deepen by *adding* a responder, not by subclassing one. |
| `UInteractionRequirement` | **not** final | Extension point. `Abstract`. |
| `UInteractionResponderComponent` | **not** final | Extension point. `Abstract`. |

C++ and UCLASS specifiers must agree, or Blueprint will happily offer a child class that C++ rejects:

```cpp
// sealed
UCLASS(MinimalAPI, final, ClassGroup=(Interaction), NotBlueprintable,
       meta=(BlueprintSpawnableComponent, DisplayName="Interactor"))
class UInteractorComponent final : public UActorComponent
{ GENERATED_BODY() /* ... */ };

// extension point
UCLASS(MinimalAPI, Abstract, Blueprintable, ClassGroup=(Interaction),
       meta=(BlueprintSpawnableComponent))
class UInteractionResponderComponent : public UActorComponent
{ GENERATED_BODY() /* ... */ };
```

Specifier notes:
- `final` (UCLASS) + `final` (C++) — both are needed; the UCLASS one blocks Blueprint children, the C++ one blocks native ones.
- `NotBlueprintable` on the sealed classes; `Blueprintable` only on `UInteractionResponderComponent` and `UInteractionRequirement`.
- Keep `BlueprintSpawnableComponent` on the sealed components — designers still *add* them, they just can't *derive* from them.
- `BlueprintType` stays everywhere so they can be passed around as variables.
- Everything sealed also means no `virtual` in their public API — if a `virtual` looks necessary on `UInteractableComponent`, that's the signal it should have been a responder or a requirement.

Restated as rules:
- Detection strategy is `EInteractorPrecision`, not a subclass (unlike Mountea's Trace/Overlap split).
- Interaction verbs are `EInteractionInputMode`, not subclasses (unlike Mountea's five interactable classes).
- The only sanctioned inheritance points are `UInteractionRequirement` (policy) and `UInteractionResponderComponent` (behaviour).

### 6.3 Why this beats Mountea's model for you

| | Mountea | FV rewrite |
|---|---|---|
| Multiple verbs on one object | multiple components | one component, N offers |
| New verb | new C++/BP subclass | new row in `Offers` + one ability asset |
| New reaction | edit the actor / interface impl | drop in a responder component |
| Runtime behaviour change | not really possible | add/remove responder |
| Angelscript | interfaces, unusable | components + multicast events |
| Consequences | inside the interactable | GAS or responder |

### 6.4 Debugging: Mountea's Visuals + Our Range/Registry Layer, All CVar-Gated

Two complementary halves. Mountea debugs **the trace** from inside the component; we debug **the world** from outside it. Keep both, put each in the layer that owns the data, and — unlike Mountea — drive all of it from CVars.

#### 6.4.1 What each side actually has

| | Mountea | Ours | Verdict |
|---|---|---|---|
| Trace start/end markers | `DrawTracingDebugStart` — blue line (Precise) or blue sphere (Loose) at origin, red sphere at end | none | **take Mountea's** |
| Per-hit impact markers | `DrawTracingDebugEnd` — green sphere on **every** `HitResult.ImpactPoint` | single impact point via `bDebugHasImpact` | **take Mountea's** (multi-hit needs multi-marker) |
| Draw lifetime tied to trace cadence | `false, TraceInterval, 0, 0.25f` — each draw lives exactly one trace period | `-1.f` persistent, redrawn every tick | **take Mountea's** — the single best trick they have (§6.4.4) |
| Safety trace visualisation | blue box at origin, red box at target, purple arrow between | none | **take Mountea's** |
| `ToString()` state dump | `ToString_Implementation`, base + derived composition | none | **take Mountea's** (§6.4.5) |
| Editor property validation | `PostEditChangeChainProperty` clamps + `FEditorHelper::DisplayEditorNotification` toasts | compile-time rules in `FVInteractionSystemEditor` | **keep ours** (compile rules are stronger); optionally add their toasts |
| Detection radius rings | none | `DrawDebugCircle` for `MaxDetectionRadius` + per-interactable `DetectionRadius` | **keep ours** |
| Registry-wide view | none | iterates `Registry->GetAll()`, colours by focused / in-range / out-of-range | **keep ours** |
| Focused-target bounds box | none | `DrawDebugBox` on the focused actor's bounds | **keep ours** |
| On-screen HUD readout | none (log only) | `UDebugDrawService` canvas: candidates + distances, gate state, prompts, outcome | **keep ours** |
| Last-action outcome | none | `EDebugActionOutcome` + age-faded line | **keep ours** |
| **Gating mechanism** | `uint8 DebugMode : 1` on the component — must be set per-asset, ships in content | `FVCvar.Interaction.Debug.Draw` / `.HUD` | **keep ours, and extend it** (§6.4.3) |

Summary: **Mountea is better at the narrow phase, we are better at the broad phase and at gating.** Neither replaces the other.

#### 6.4.2 Where each half lives

The split follows the layer that owns the data — a debug component on the player cannot draw a trace it did not perform.

```
UInteractorComponent (L1)                    UInteractionDebugComponent (opt-in)
─────────────────────────                    ──────────────────────────────────
draws, inside ProcessTrace:                  draws, from outside, per tick:
  • trace ray / sweep shape                    • MaxDetectionRadius ring
  • end marker                                 • per-interactable DetectionRadius ring
  • every hit's ImpactPoint                    • colour by State (Idle/Awake) + focus
  • safety trace + verdict                     • focused actor bounds box
  • lifetime == TracingInterval                • in-range set membership, weights
                                               • HUD: candidates, prompts, outcome
      gated by FVCvar.Interaction.Debug.Trace      gated by FVCvar.Interaction.Debug.Draw / .HUD
```

Rationale for keeping trace drawing **in the interactor** rather than moving it into the debug component:

- The debug component would have to re-run the trace to draw it — wasteful, and a lie: it would visualise a *different* trace than the one that made the decision.
- `TracingInterval` is what makes the lifetime trick work, and only the interactor knows it.
- Multi-hit results are transient locals inside `ProcessTrace`. Exposing them as debug state would mean a far wider `#if !UE_BUILD_SHIPPING` accessor surface than the four getters we already regret.

#### 6.4.3 CVars are the gate — not a per-component bool

This is our deliberate divergence from Mountea. Their `DebugSettings.DebugMode` is a `UPROPERTY` on the component, which means toggling debug requires editing an asset, that edit is checked in, and turning it on for *all* interactables means touching every one of them.

Ours stays console-driven, extended to cover the new draws:

```cpp
// Private/Debug/InteractionDebugCVars.cpp  — one home for all of them
TAutoConsoleVariable<bool> CVarInteractionDebugDraw(
    TEXT("FVCvar.Interaction.Debug.Draw"), false,
    TEXT("Draw detection radii, candidate rings and focus highlights."));

TAutoConsoleVariable<bool> CVarInteractionDebugHUD(
    TEXT("FVCvar.Interaction.Debug.HUD"), false,
    TEXT("On-screen readout of candidates, prompts, state and last outcome."));

TAutoConsoleVariable<bool> CVarInteractionDebugTrace(
    TEXT("FVCvar.Interaction.Debug.Trace"), false,
    TEXT("Draw the interaction trace ray/sweep, all hit impact points and the safety trace."));

TAutoConsoleVariable<bool> CVarInteractionDebugState(
    TEXT("FVCvar.Interaction.Debug.State"), false,
    TEXT("Draw per-interactable state, weight and lifecycle text above each interactable."));

TAutoConsoleVariable<int32> CVarInteractionDebugVerbosity(
    TEXT("FVCvar.Interaction.Debug.Verbosity"), 0,
    TEXT("0=off 1=errors 2=warnings 3=info 4=verbose. Filters LogFVInteraction output."));
```

Rules:

- **Every debug draw in the plugin is behind one of these.** No exceptions, including the ones inside `ProcessTrace`.
- **`.Trace` is separate from `.Draw`** — the trace is high-frequency and visually noisy; you usually want one or the other, not both.
- **Read once per frame, not per draw call.** `const bool bDrawTrace = CVarInteractionDebugTrace.GetValueOnGameThread();` at the top of `ProcessTrace`, then branch on the local. `GetValueOnGameThread()` is cheap but not free, and `ProcessTrace` can issue dozens of draws.
- **Wrap in `#if !UE_BUILD_SHIPPING`**, matching the existing `UInteractionDebugComponent` convention, so shipping builds carry neither the CVars nor the draw code.
- **No debug `UPROPERTY` on components.** If per-actor isolation is ever needed, add `FVCvar.Interaction.Debug.FilterActor "BP_Door_2"` rather than a checkbox — it stays out of content and out of source control.

Keep Mountea's `bEditorDebugMode` idea *only* for editor-time property-validation toasts, since those fire in the editor where CVars are awkward. That is an editor-module concern and belongs next to the existing compile rules, not on the runtime component.

#### 6.4.4 The trace-lifetime rule

The detail most worth copying. Every debug draw issued from inside `ProcessTrace` uses:

```cpp
DrawDebugX(World, ..., /*bPersistent*/ false, /*LifeTime*/ TracingInterval, /*DepthPriority*/ 0, /*Thickness*/ 0.25f);
```

Because each draw lives exactly as long as the gap until the next trace, the visualisation is **continuous but never accumulates** — you see the current trace and only the current trace. Our present `-1.f` persistent draws redrawn every tick both flicker and pile up.

Colour convention, taken from Mountea and extended for our broad phase:

| Colour | Meaning |
|---|---|
| Blue | trace origin / ray |
| Red | trace end point, or safety trace blocked |
| Green | valid hit impact point, or safety trace clear |
| Purple | safety trace segment |
| Yellow | `Awake` (in range) but not focused |
| Grey | registered but `Idle` (out of range) |

#### 6.4.5 `ToDebugString()` for state dumps

Mountea's `ToString_Implementation` pattern is genuinely useful for HUD and log output. We take the idea, but as a plain `UFUNCTION` on sealed classes rather than a `BlueprintNativeEvent` chain (no virtuals on sealed types, §6.2):

- `UInteractorComponent::ToDebugString()` → state, suppression reasons, focused target, tracing on/off, precision, interval, range, interactor tag.
- `UInteractableComponent::ToDebugString()` → state, weight, lifecycle + remaining charges, offer count, focus flag, suppression reasons.

The HUD prints these rather than hand-assembling strings, so a newly added field appears everywhere at once.

#### 6.4.6 What our debug component must gain

Because the registry now owns `Idle <-> Awake` (§2.2.2), the debug component's job shifts:

- **Colour rings by `State`, not by a recomputed distance test.** It must not do its own range maths any more than the interactor does — `DrawVisualizer` currently computes `FVector::Dist(FocusPoint, PawnLocation) <= DetectionRadius`, which becomes a fourth redundant range check the moment the registry is authoritative.
- Show **in-range set membership per interactor** — now authoritative registry data.
- Show **`InteractionWeight`**, since arbitration is weight-first and "why did it pick that one?" is the most common question.
- Replace the `bDebugGateOpen` line with **tracing state** (`Disabled / Paused / Active`) and the reason.
- Keep the existing accessor surface (`GetDebugCandidates`, `GetDebugLastOutcome`, `IsDebugGateOpen`, …) compiling through the detection rewrite — update the component and the interactor in the same commit (see the risk note in the implementation plan).


---

## 7. Concrete Cleanup of the Current Draft

| Current | Action |
|---|---|
| `Interfaces/InteractableInterface.h` | **Delete.** Angelscript. |
| `FInteractableSelected/Found/Lost` using `TScriptInterface<IInteractableInterface>` | Retype to `UInteractableComponent*`. `Selected` and `Found` collapse into `OnFocusChanged`; `Lost` becomes `OnFocusChanged(nullptr)`. |
| `FInteractionKeyPressed` / `FInteractionKeyReleased` | **Delete as events.** Replaced by `PushInput(Tag, Phase)` as an entry point (§3.1). |
| `FStateChanged` (commented) | Uncomment, one per side: `FOnInteractorStateChanged`, `FOnInteractableStateChanged`. |
| `FCollisionChanged`, `FIgnoredActorAdded/Removed` | Keep — they are legitimate low-frequency config-change notifications. |
| `FInteractorTagChanged` | Keep; prompts must refresh when it fires. |
| `Interactiong` typo in both state enums | Fix to `Interacting`. |
| `FTracingSetup` empty body | Uncomment; use it to replace `AimOriginOffset` in `GetAimPoint`. |
| `FInteractorSettings` / `FInteractableSettings` empty bodies | Uncomment; add a `ResolveDefaults()` step in each component's constructor/`BeginPlay`. |
| `UInteractorComponent::ExecuteAction` (`FExecuteInteractionAction`) | **Delete.** Replaced by mailbox `OnInteractionRequested`. Also removes the lambda in `AFVPlayerCharacter::BeginPlay`. |
| `TryExecuteInteraction` returning `true` when the offer was gated | Return the real outcome, or return `void` and broadcast `OnInteractionCancelled(Reason)`. |
| `RefreshOffers` every detection tick | Re-evaluate on: focus change, interactable state change, interactor tag change, ASC tag/attribute delegate. Not on tick. |
| `UE_LOG(LogTemp, ...)` | `LogFVInteraction`. |
| `DetectInteractables` finding interactables via `FindComponentByClass` on the hit actor | Cache the component pointer on the registry entry — it runs per trace hit. |
| `SweepSingleByChannel` | → `SweepMultiByChannel` / `LineTraceMultiByChannel` (§2.2.1). Single-hit breaks weight arbitration and safety tracing. |
| `GateRadius` loop in `DetectInteractables` + post-sweep `Distance > DetectionRadius` rejection | **Delete both.** Redundant with the registry query and with `TracingRange`. One range test total. |
| `AimOriginOffset` / `GetAimPoint()` | Replace with `FTracingSetup` + `ESafetyTracingMode { None, Location, Socket }`, falling back to `GetActorEyesViewPoint()`. |
| Tick-driven detection (`DetectionUpdateInterval`, `PrimaryComponentTick`) | Timer-driven `ProcessTrace` re-armed at the end of itself; `PrimaryComponentTick` disabled. Public surface = `EnableTracing` / `DisableTracing` / `PauseTracing` / `ResumeTracing`, gated by the registry. |
| `UInteractorComponent`, `UInteractableComponent`, `UInteractionResponseComponent`, `UInteractionRegistrySubsystem`, `UFVInteractionSystemSettings`, concrete responders | Mark `final` in **both** C++ and UCLASS, plus `NotBlueprintable` (§6.2). |
| `DrawVisualizer` recomputing `FVector::Dist(...) <= DetectionRadius` per interactable | Colour by `State` instead — the registry is authoritative for range (§6.4.6). |
| Debug draws using `-1.f` persistent lifetime redrawn every tick | Lifetime = `TracingInterval`, `bPersistent = false` (§6.4.4). |
| No trace visualisation at all | Add Mountea-style ray/sweep, per-hit impact and safety-trace draws inside `ProcessTrace`, gated by `FVCvar.Interaction.Debug.Trace` (§6.4.3). |

---

## 8. End-to-End Trace (hold-to-lockpick, single frame by frame)

```
 t=0.00  Player walks into the area
		 L5 Registry broad-phase: Door enters range set
		 L4 Door.TrySetState(Idle -> Awake)      -> OnStateChanged
												 -> HighlightResponder enables faint glow
		 L1 EnableTracing() -> ProcessTrace timer armed at TracingInterval

 t=0.10  Player looks at door
		 L1 ProcessTrace: origin = "head" socket, LineTraceMultiByChannel
			hits [Crate, DoorMesh] -> Crate has no interactable, skipped
			DoorMesh -> Door, Awake, channel ok, tags ok, weight 1 -> best
		 L1 PerformSafetyTrace(Door) on ValidationCollisionChannel: clear
		 L1 SetFocusedTarget(Door)
		 L4 Door.bIsInFocus = true                -> OnFocusStateChanged(true)
												  -> HighlightResponder goes full highlight
												  (State stays Awake)
		 L1 evaluate offers (Open: ok, Lockpick: HasItem ok)
		 L2 OnFocusChanged(Door), OnOffersChanged([Open/Press, Lockpick/Hold])
		 L3 PromptResponder -> widget shows 2 rows; asks GlyphSubsystem for
			InputTag.Interaction.Primary/Secondary glyphs for current device

 t=1.00  Player presses Secondary
		 L0 PC routes InputTag.Interaction.Secondary, Pressed
		 L1 PushInput -> offer InputMode=Hold, InteractionPeriod=4
		 L1 starts hold, state -> Interacting
		 L4 Door.TrySetState(Awake -> Interacting)
		 L2 OnInteractionStarted(Lockpick)

 t=1.05  L2 OnInteractionProgress(Lockpick, 0.0125)   (WidgetUpdateFrequency)
   ...   L3 widget animates ring

 t=2.20  Player turns away -> focus lost
		 L1 cancels hold
		 L2 OnInteractionCancelled(Lockpick, Interaction.Cancel.FocusLost)
		 L4 Door.TrySetState(Interacting -> Awake -> Idle)

 (alternate) t=5.00  hold completes
		 L2 OnInteractionRequested(Commit{Lockpick, Door, hit point})
		 L3 AbilityResponder -> SendGameplayEventToActor(Interaction.Action.Lockpick)
			GAS -> GA_Lockpick (cost, montage, prediction)
		 GA finishes -> ReportInteractionFinished(Lockpick, true)
		 L4 Door.OnInteractionCompleted -> LifecycleResponder:
			Lifecycle=OneShot -> TrySetState(Completed); offer disappears from prompts
```

Note how many layers know about GAS: **one** (`UFVInteractionAbilityResponder`). And how many know about EnhancedInput: **one** (`AFVPlayerController`).

---

## 9. Build Order

| # | Deliverable | Unlocks |
|---|---|---|
| 1 | Cleanup pass from §7 (delete interface, retype delegates, log category, `final` + `NotBlueprintable` on sealed classes) | Nothing blocks on stale API; architecture is compiler-enforced |
| 2 | Uncomment enums + `FTracingSetup` + settings structs; `ResolveDefaults()` | Everything below reads config, not literals |
| 3 | `EInteractableState` + `TrySetState(..., OutError)` + tag mirroring + `bIsInFocus` as a separate axis (§2.2.2) | Gating, prompts that reflect reality |
| 4 | Rewrite detection: registry enter/leave diffing drives `Idle <-> Awake`, gate + timer-driven multi-trace + safety trace (§2.2.1); delete `GateRadius` and the redundant range checks | Correct occlusion, no tick, zero cost when idle |
| 5 | `UInteractionResponseComponent` + `UInteractionResponderComponent` base + `UFVInteractionAbilityResponder`; delete `ExecuteAction` | Decoupled GAS, event-triggered abilities |
| 6 | `PushInput(Tag, Phase)` + `EInteractionInputMode` FSM + `OnInteractionProgress` | Hold / mash / hover / automatic |
| 7 | Arbitration (`Weight`, `InteractorTag`, `RequiredInteractorTags`, scoring) | Nested interactables, multi-interactor |
| 8 | Requirement subclasses (`AbilityCanActivate`, `HasTags`, `InteractableState`) + event-driven re-eval | Prompts match ability truth; no per-tick eval |
| 9 | Prompt responder + glyph subsystem + `FInteraction` extension | Real UI |
| 10 | Target-side responders (highlight, lifecycle, audio) + `EHighlightSetupType` auto-setup | Designer-authored interactables |
| 11 | `EInteractorState` + suppression reasons + dependencies (suppression = `PauseTracing`) | Cutscenes, vehicles, menus |
| 12 | Compile rules + debug component expansion | Safety net |

Replication remains out of scope; the funnels (`TrySetState`, `PushInput`, `OnInteractionRequested`) are the only three places it would ever need to be added.

---

## 9.1 Coding Standard

Applies to every step in §9 and to every file this rewrite touches.

**Keep the code clean and simple. Do not add comments.**

- **No explanatory comments.** Names carry the intent. If a line needs a comment to be understood, rename the symbol or extract a well-named function instead.
- Only two exceptions are permitted:
  - `meta=(Tooltip="...")` designer-facing text on `UPROPERTY` / `UFUNCTION`.
  - A single `//` where a non-obvious engine constraint would otherwise read as a bug (for example, why `PerformSafetyTrace` must run on a separate channel).
- No file-header banners, no `// ---- Section ----` dividers, no comments restating what the next line does, no `/** */` doc blocks on self-evident accessors.
- Small functions, early returns, no nesting deeper than the logic requires.
- No speculative abstraction — add an indirection only once a second concrete caller exists.
- No dead code and no commented-out code left behind. The drafted blocks in `InteractionTypes.h` get uncommented **or** deleted, never left half-present.
- Prefer deleting over deprecating. Nothing in the §7 cleanup table is kept "just in case".

The design carries the explanation; the code carries the behaviour. If something needs justifying, it belongs in this document, not in a comment.
