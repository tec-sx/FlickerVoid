#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractableComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/FVInteractionGameplayTags.h"
#include "Engine/World.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ScopeExit.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "TimerManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractorComponent)

UFVInteractorComponent::UFVInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UFVInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	const FFVInteractorSettings& Defaults = UFVInteractionSystemSettings::Get().InteractorDefaultSettings;

	if (Precision == EFVInteractableDetectionMode::Default)
	{
		Precision = Defaults.DefaultPrecision;
	}

	if (InteractorTags.IsEmpty() && Defaults.InteractorTag.IsValid())
	{
		InteractorTags.AddTag(Defaults.InteractorTag);
	}

	if (TracingSetup.SafetyTracingMode == EFVOcclusionDetectionMode::Default)
	{
		TracingSetup = Defaults.DetectionSetup;
	}

	Owner = Cast<APawn>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogFVInteraction, Warning, TEXT("Interaction Component has invalid pawn owner."));
		return;
	}

	Registry = GetWorld()->GetSubsystem<UFVInteractionRegistrySubsystem>();

	if (Registry)
	{
		Registry->OnInRangeSetChanged.AddDynamic(this, &UFVInteractorComponent::HandleInRangeSetChanged);
		Registry->RegisterInteractor(this);
	}

	bIsInitialized = Registry != nullptr;
}

void UFVInteractorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisableTracing();

	if (Registry)
	{
		Registry->OnInRangeSetChanged.RemoveDynamic(this, &UFVInteractorComponent::HandleInRangeSetChanged);
		Registry->UnregisterInteractor(this);
	}

	bIsInitialized = false;

	Super::EndPlay(EndPlayReason);
}

void UFVInteractorComponent::HandleInRangeSetChanged(bool bHasAnyInRange)
{
	if (bHasAnyInRange)
	{
		EnableTracing();
	}
	else
	{
		DisableTracing();
	}
}

void UFVInteractorComponent::EnableTracing()
{
	if (!bIsInitialized || bIsTracing || IsSuppressed())
	{
		return;
	}

	bIsTracing = true;
	ProcessTrace();
	UpdateState();
}

void UFVInteractorComponent::DisableTracing()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TraceTimer);
	}

	bIsTracing = false;
	Candidates.Reset();
	SetFocusedTarget(nullptr);
	UpdateState();
}

void UFVInteractorComponent::AddSuppression(FGameplayTag Reason)
{
	if (!Reason.IsValid() || SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.AddTag(Reason);

	CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Suppressed);
	DisableTracing();
}

void UFVInteractorComponent::RemoveSuppression(FGameplayTag Reason)
{
	if (!SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.RemoveTag(Reason);

	if (Registry && Registry->GetInRangeSet(this).Num() > 0)
	{
		EnableTracing();
	}

	UpdateState();
}

void UFVInteractorComponent::UpdateState()
{
	if (IsSuppressed())
	{
		SetState(EFVInteractorState::Suppressed);
		return;
	}

	if (bIsInteracting)
	{
		SetState(EFVInteractorState::Interacting);
		return;
	}

	SetState(bIsTracing ? EFVInteractorState::Awake : EFVInteractorState::Idle);
}

void UFVInteractorComponent::SetState(EFVInteractorState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
	StateChanged.Broadcast(State);
}

void UFVInteractorComponent::ArmNextTrace()
{
	UWorld* World = GetWorld();
	if (!World || !bIsTracing)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		TraceTimer,
		FTimerDelegate::CreateUObject(this, &UFVInteractorComponent::ProcessTrace),
		FMath::Max(TracingSetup.TracingInterval, 0.01f),
		false);
}

const FFVInteractionOffer* UFVInteractorComponent::FindActiveOffer() const
{
	const UFVInteractableComponent* Target = ActiveCommit.Interactable;
	return Target ? Target->FindOffer(ActiveCommit.InputTag) : nullptr;
}

bool UFVInteractorComponent::PushInput(FGameplayTag InputTag, EFVInteractionInputPhase Phase)
{
#if !UE_BUILD_SHIPPING
	DebugLastInputTag = InputTag;
	DebugLastActionTime = FPlatformTime::Seconds();
	DebugLastOutcome = EDebugActionOutcome::NoPrompt;
#endif

	if (bIsInteracting && ActiveCommit.InputTag.MatchesTagExact(InputTag))
	{
		if (Phase == EFVInteractionInputPhase::Cancelled)
		{
			CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Player);
			return true;
		}

		if (Phase == EFVInteractionInputPhase::Released && ActiveMode == EFVInteractionInputMode::Hold)
		{
			CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Released);
			return true;
		}

		if (Phase == EFVInteractionInputPhase::Pressed && ActiveMode == EFVInteractionInputMode::Mash)
		{
			++ActivePresses;

			if (ActivePresses >= ActiveRequiredPresses)
			{
				CommitInteraction();
			}

			return true;
		}

		return true;
	}

	if (Phase != EFVInteractionInputPhase::Pressed)
	{
		return false;
	}

	UFVInteractableComponent* Target = FocusedTarget.Get();
	if (!Target)
	{
		return false;
	}

	const FFVInteractionOffer* Interaction = CachedOffers.FindByPredicate([&InputTag](const FFVInteractionOffer& Candidate)
	{
		return Candidate.InputTag.MatchesTagExact(InputTag);
	});

	if (!Interaction)
	{
		return false;
	}

	if (!Interaction->CanExecute())
	{
#if !UE_BUILD_SHIPPING
		DebugLastOutcome = EDebugActionOutcome::Disabled;
#endif
		return true;
	}

	const FFVInteractionOffer* Offer = Target->FindOffer(InputTag);
	if (!Offer || Offer->IsExhausted())
	{
		return false;
	}

	return BeginInteraction(*Offer, *Target);
}

bool UFVInteractorComponent::BeginInteraction(const FFVInteractionOffer& Offer, UFVInteractableComponent& Target)
{
	if (!Target.SetState(EFVInteractableState::Interacting))
	{
		return false;
	}

	ActiveCommit = FFVInteractionCommit();
	ActiveCommit.ActionTag = Offer.ActionTag;
	ActiveCommit.InputTag = Offer.InputTag;
	ActiveCommit.Interactable = &Target;
	ActiveCommit.Interactor = this;
	ActiveCommit.InteractionPoint = LastFocusImpactPoint.IsNearlyZero()
		? Target.GetFocusPoint()
		: LastFocusImpactPoint;

	ActiveMode = Offer.InputMode == EFVInteractionInputMode::Default
		? EFVInteractionInputMode::Press
		: Offer.InputMode;

	ActiveDuration = Offer.InteractionPeriod < 0.f
		? UFVInteractionSystemSettings::Get().InteractableBaseSettings.DefaultInteractionPeriod
		: Offer.InteractionPeriod;

	ActiveElapsed = 0.f;
	LastProgressBroadcast = 0.f;
	ActivePresses = 0;
	ActiveRequiredPresses = FMath::Max(Offer.RequiredPresses, 1);
	bIsInteracting = true;
	UpdateState();
	
	InteractionCommitStarted.Broadcast(ActiveCommit);
	Target.StartInteraction(ActiveCommit.ActionTag, this);
	
	if (ActiveMode == EFVInteractionInputMode::Press || ActiveDuration <= 0.f)
	{
		CommitInteraction();
		return true;
	}

	const float UpdateRate = FMath::Max(UFVInteractionSystemSettings::Get().WidgetUpdateFrequency, 0.01f);

	GetWorld()->GetTimerManager().SetTimer(
		InteractionTimer,
		FTimerDelegate::CreateUObject(this, &UFVInteractorComponent::TickInteraction),
		UpdateRate,
		true);

	return true;
}

void UFVInteractorComponent::TickInteraction()
{
	if (!bIsInteracting)
	{
		return;
	}

	UFVInteractableComponent* Target = ActiveCommit.Interactable;
	if (!IsValid(Target) || Target != FocusedTarget.Get())
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_FocusLost);
		return;
	}

	if (Target->GetState() == EFVInteractableState::Suppressed)
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Suppressed);
		return;
	}

	if (const FFVInteractionOffer* Offer = FindActiveOffer())
	{
		if (!IsOfferAvailable(*Offer))
		{
			CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_RequirementFailed);
			return;
		}
	}

	const float UpdateRate = FMath::Max(UFVInteractionSystemSettings::Get().WidgetUpdateFrequency, 0.01f);
	ActiveElapsed += UpdateRate;

	if (ActiveMode == EFVInteractionInputMode::Mash)
	{
		if (ActiveElapsed >= ActiveDuration)
		{
			CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Timeout);
			return;
		}
		
		const float Progress = ActivePresses / static_cast<float>(ActiveRequiredPresses);
		ProgressInteraction(Progress);

		return;
	}

	const float Progress = FMath::Clamp(ActiveElapsed / ActiveDuration, 0.f, 1.f);
	ProgressInteraction(Progress);

	if (Progress >= 1.f)
	{
		CommitInteraction();
	}
}

void UFVInteractorComponent::CommitInteraction()
{
	if (!bIsInteracting)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(InteractionTimer);
	bIsInteracting = false;
	UpdateState();

	
#if !UE_BUILD_SHIPPING
	DebugLastOutcome = EDebugActionOutcome::Succeeded;
#endif

	InteractionCommitEnded.Broadcast(ActiveCommit, true);
	
	if (UFVInteractableComponent* Target = ActiveCommit.Interactable)
	{
		Target->EndInteraction(ActiveCommit.ActionTag, this, true);
	}
	
	RefreshOffers();
}

void UFVInteractorComponent::ProgressInteraction(const float Progress)
{
	InteractionCommitProgress.Broadcast(ActiveCommit, Progress);
	
	if (UFVInteractableComponent* Target = ActiveCommit.Interactable)
	{
		Target->ProgressInteraction(ActiveCommit.ActionTag, this, Progress);
	}
}

void UFVInteractorComponent::CancelInteraction(const FGameplayTag& Reason)
{
	if (!bIsInteracting)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(InteractionTimer);
	bIsInteracting = false;
	UpdateState();
	
	InteractionCommitEnded.Broadcast(ActiveCommit, false);
	
	if (UFVInteractableComponent* Target = ActiveCommit.Interactable)
	{
		Target->EndInteraction(ActiveCommit.ActionTag, this, false);
	}

	RefreshOffers();
}

bool UFVInteractorComponent::IsOfferAvailable(const FFVInteractionOffer& Offer) const
{
	if (BlockedActionTags.HasTag(Offer.ActionTag))
	{
		return false;
	}

	return Offer.AreTagsSatisfied(InteractorTags);
}

FVector UFVInteractorComponent::GetDetectionOrigin() const
{
	return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
}

void UFVInteractorComponent::AddInteractorTag(FGameplayTag NewTag)
{
	if (!NewTag.IsValid() || InteractorTags.HasTagExact(NewTag))
	{
		return;
	}

	InteractorTags.AddTag(NewTag);
	RefreshOffers();
}

void UFVInteractorComponent::RemoveInteractorTag(FGameplayTag OldTag)
{
	if (!InteractorTags.HasTagExact(OldTag))
	{
		return;
	}

	InteractorTags.RemoveTag(OldTag);
	RefreshOffers();
}

bool UFVInteractorComponent::GetTraceOrigin(FVector& OutOrigin, FVector& OutForward) const
{
	if (!Owner)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	if (const APlayerController* PC = Cast<APlayerController>(Owner->GetController()))
	{
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	else
	{
		Owner->GetActorEyesViewPoint(ViewLocation, ViewRotation);
	}

	OutOrigin = ViewLocation;
	OutForward = ViewRotation.Vector();

	if (TracingSetup.SafetyTracingMode == EFVOcclusionDetectionMode::Socket && !TracingSetup.StartSocketName.IsNone())
	{
		const USkeletalMeshComponent* Mesh = Cast<USkeletalMeshComponent>(
			Owner->GetDefaultSubobjectByName(TracingSetup.ActorMeshName));

		if (Mesh && Mesh->DoesSocketExist(TracingSetup.StartSocketName))
		{
			OutOrigin = Mesh->GetSocketLocation(TracingSetup.StartSocketName);
		}
	}

	return true;
}

bool UFVInteractorComponent::PerformSafetyTrace(const FVector& Origin, const UFVInteractableComponent& Candidate) const
{
	if (TracingSetup.SafetyTracingMode == EFVOcclusionDetectionMode::None)
	{
		return true;
	}

	const AActor* CandidateOwner = Candidate.GetOwner();
	if (!CandidateOwner)
	{
		return false;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FVInteractionSafety), false, Owner);
	Params.AddIgnoredActor(CandidateOwner);

	FHitResult Hit;
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Origin,
		Candidate.GetFocusPoint(),
		TracingSetup.ValidationCollisionChannel,
		Params);

	return !bBlocked;
}

void UFVInteractorComponent::ProcessTrace()
{
	ON_SCOPE_EXIT
	{
		ArmNextTrace();
	};

	FVector Origin;
	FVector Forward;

	if (!GetTraceOrigin(Origin, Forward))
	{
		SetFocusedTarget(nullptr);
		return;
	}

	const FVector End = Origin + Forward * TracingSetup.TracingRange;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FVInteractionTrace), false, Owner);

	TArray<FHitResult> Hits;

	if (Precision == EFVInteractableDetectionMode::Trace)
	{
		GetWorld()->LineTraceMultiByChannel(Hits, Origin, End, InteractionChannel, Params);
	}
	else
	{
		GetWorld()->SweepMultiByChannel(
			Hits,
			Origin,
			End,
			FQuat::Identity,
			InteractionChannel,
			FCollisionShape::MakeBox(FVector(TracingSetup.TracingShapeHalfSize)),
			Params);
	}

#if !UE_BUILD_SHIPPING
	DebugSweepDirection = Forward;
	bDebugGateOpen = true;
	bDebugHitOccluder = false;
	bDebugHasImpact = false;
#endif

	Candidates.Reset();

	struct FRankedCandidate
	{
		UFVInteractableComponent* Interactable;
		FVector ImpactPoint;
	};

	TArray<FRankedCandidate> Ranked;

	for (const FHitResult& Hit : Hits)
	{
		const AActor* HitActor = Hit.GetActor();
		UFVInteractableComponent* Interactable = HitActor
			? HitActor->FindComponentByClass<UFVInteractableComponent>()
			: nullptr;

		if (Interactable && Interactable->CanBeInteractedWith())
		{
			Candidates.AddUnique(Interactable);
			Ranked.Add({ Interactable, Hit.ImpactPoint });
		}
	}

	Ranked.Sort([](const FRankedCandidate& A, const FRankedCandidate& B)
	{
		return A.Interactable->InteractionWeight > B.Interactable->InteractionWeight;
	});

	for (const FRankedCandidate& Candidate : Ranked)
	{
		if (!PerformSafetyTrace(Origin, *Candidate.Interactable))
		{
#if !UE_BUILD_SHIPPING
			bDebugHitOccluder = true;
#endif
			continue;
		}

#if !UE_BUILD_SHIPPING
		DebugImpactPoint = Candidate.ImpactPoint;
		bDebugHasImpact = true;
#endif

		LastFocusImpactPoint = Candidate.ImpactPoint;
		SetFocusedTarget(Candidate.Interactable);
		return;
	}

	LastFocusImpactPoint = FVector::ZeroVector;
	SetFocusedTarget(nullptr);
}

void UFVInteractorComponent::SetFocusedTarget(UFVInteractableComponent* NewTarget)
{
	if (FocusedTarget.Get() == NewTarget)
	{
		return;
	}
	
	if (UFVInteractableComponent* Previous = FocusedTarget.Get())
	{
		Previous->StateChanged.RemoveDynamic(this, &UFVInteractorComponent::HandleFocusedStateChanged);
		Previous->SetFocused(false, this);
	}

	FocusedTarget = NewTarget;

	if (NewTarget)
	{
		NewTarget->SetFocused(true, this);
		NewTarget->StateChanged.AddDynamic(this, &UFVInteractorComponent::HandleFocusedStateChanged);
	}

	FocusChanged.Broadcast(NewTarget);

	RefreshOffers();
}

void UFVInteractorComponent::HandleFocusedStateChanged(EFVInteractableState NewState)
{
	RefreshOffers();
}

void UFVInteractorComponent::RefreshOffers(bool bForceBroadcast)
{
	const UFVInteractableComponent* Target = FocusedTarget.Get();
	TArray<FFVInteractionOffer> NewOffers;

	if (Target)
	{
		for (const FFVInteractionOffer& Offer : Target->GetOffers())
		{
			if (!Offer.IsValid())
			{
				continue;
			}

			const bool bRequirementsMet = IsOfferAvailable(Offer);

			if (!bRequirementsMet && Offer.RequirementGate == EFVInteractionGate::Hide)
			{
				continue;
			}
			
			FFVInteractionOffer& Slot = NewOffers.Add_GetRef(Offer);
			Slot.bRequirementsMet = bRequirementsMet;
		}
	}


	NewOffers.Sort([](const FFVInteractionOffer& A, const FFVInteractionOffer& B)
	{
		if (A.Weight != B.Weight)
		{
			return A.Weight > B.Weight;
		}

		return A.InputTag.ToString() < B.InputTag.ToString();
	});

	const bool bChanged = NewOffers != CachedOffers;
	CachedOffers = MoveTemp(NewOffers);

	if (bChanged || bForceBroadcast)
	{
		OffersChanged.Broadcast(CachedOffers);
	}
}
