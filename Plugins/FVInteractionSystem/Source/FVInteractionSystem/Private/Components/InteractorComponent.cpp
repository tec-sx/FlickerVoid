#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/InteractionTags.h"
#include "Engine/World.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ScopeExit.h"
#include "Subsystems/InteractionRegistrySubsystem.h"
#include "TimerManager.h"
#include "Components/InteractionSignalComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractorComponent)

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	const FInteractorSettings& Defaults = UFVInteractionSystemSettings::Get().InteractorDefaultSettings;

	if (Precision == EInteractorPrecision::Default)
	{
		Precision = Defaults.DefaultPrecision;
	}

	if (InteractorTags.IsEmpty() && Defaults.InteractorTag.IsValid())
	{
		InteractorTags.AddTag(Defaults.InteractorTag);
	}

	if (TracingSetup.SafetyTracingMode == ESafetyTracingMode::Default)
	{
		TracingSetup = Defaults.TracingSetup;
	}

	Owner = Cast<APawn>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogFVInteraction, Warning, TEXT("Interaction Component has invalid pawn owner."));
		return;
	}

	Registry = GetWorld()->GetSubsystem<UInteractionRegistrySubsystem>();

	if (Registry)
	{
		Registry->OnInRangeSetChanged.AddDynamic(this, &UInteractorComponent::HandleInRangeSetChanged);
		Registry->RegisterInteractor(this);
	}

	bIsInitialized = Registry != nullptr;
}

void UInteractorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisableTracing();

	if (Registry)
	{
		Registry->OnInRangeSetChanged.RemoveDynamic(this, &UInteractorComponent::HandleInRangeSetChanged);
		Registry->UnregisterInteractor(this);
	}

	bIsInitialized = false;

	Super::EndPlay(EndPlayReason);
}

void UInteractorComponent::HandleInRangeSetChanged(bool bHasAnyInRange)
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

void UInteractorComponent::EnableTracing()
{
	if (!bIsInitialized || bIsTracing || IsSuppressed())
	{
		return;
	}

	bIsTracing = true;
	ProcessTrace();
	UpdateState();
}

void UInteractorComponent::DisableTracing()
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

void UInteractorComponent::AddSuppression(FGameplayTag Reason)
{
	if (!Reason.IsValid() || SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.AddTag(Reason);

	CancelInteraction(InteractionTags::Interaction_Cancel_Suppressed);
	DisableTracing();
}

void UInteractorComponent::RemoveSuppression(FGameplayTag Reason)
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

void UInteractorComponent::UpdateState()
{
	if (IsSuppressed())
	{
		SetState(EInteractorState::Suppressed);
		return;
	}

	if (bIsInteracting)
	{
		SetState(EInteractorState::Interacting);
		return;
	}

	SetState(bIsTracing ? EInteractorState::Awake : EInteractorState::Idle);
}

void UInteractorComponent::SetState(EInteractorState NewState)
{
	if (State == NewState)
	{
		return;
	}

	State = NewState;
	OnStateChanged.Broadcast(State);
}

void UInteractorComponent::ArmNextTrace()
{
	UWorld* World = GetWorld();
	if (!World || !bIsTracing)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		TraceTimer,
		FTimerDelegate::CreateUObject(this, &UInteractorComponent::ProcessTrace),
		FMath::Max(TracingSetup.TracingInterval, 0.01f),
		false);
}

const FInteractionOffer* UInteractorComponent::FindActiveOffer() const
{
	const UInteractableComponent* Target = ActiveCommit.Interactable;
	return Target ? Target->FindOffer(ActiveCommit.InputTag) : nullptr;
}

bool UInteractorComponent::PushInput(FGameplayTag InputTag, EInteractionInputPhase Phase)
{
#if !UE_BUILD_SHIPPING
	DebugLastInputTag = InputTag;
	DebugLastActionTime = FPlatformTime::Seconds();
	DebugLastOutcome = EDebugActionOutcome::NoPrompt;
#endif

	if (bIsInteracting && ActiveCommit.InputTag.MatchesTagExact(InputTag))
	{
		if (Phase == EInteractionInputPhase::Cancelled)
		{
			CancelInteraction(InteractionTags::Interaction_Cancel_Player);
			return true;
		}

		if (Phase == EInteractionInputPhase::Released && ActiveMode == EInteractionInputMode::Hold)
		{
			CancelInteraction(InteractionTags::Interaction_Cancel_Released);
			return true;
		}

		if (Phase == EInteractionInputPhase::Pressed && ActiveMode == EInteractionInputMode::Mash)
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

	if (Phase != EInteractionInputPhase::Pressed)
	{
		return false;
	}

	UInteractableComponent* Target = FocusedTarget.Get();
	if (!Target)
	{
		return false;
	}

	const FInteractionOffer* Interaction = CachedOffers.FindByPredicate([&InputTag](const FInteractionOffer& Candidate)
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

	const FInteractionOffer* Offer = Target->FindOffer(InputTag);
	if (!Offer || Offer->IsExhausted())
	{
		return false;
	}

	return BeginInteraction(*Offer, *Target);
}

bool UInteractorComponent::BeginInteraction(const FInteractionOffer& Offer, UInteractableComponent& Target)
{
	if (!Target.SetState(EInteractableState::Interacting))
	{
		return false;
	}

	ActiveCommit = FInteractionCommit();
	ActiveCommit.ActionTag = Offer.ActionTag;
	ActiveCommit.InputTag = Offer.InputTag;
	ActiveCommit.Interactable = &Target;
	ActiveCommit.Interactor = this;
	ActiveCommit.InteractionPoint = LastFocusImpactPoint.IsNearlyZero()
		? Target.GetFocusPoint()
		: LastFocusImpactPoint;

	ActiveMode = Offer.InputMode == EInteractionInputMode::Default
		? EInteractionInputMode::Press
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
	
	if (const UInteractionSignalComponent* Signal = UInteractionSignalComponent::Get(GetOwner()))
	{
		Signal->InteractionStarted.Broadcast(ActiveCommit);
	}

	if (ActiveMode == EInteractionInputMode::Press || ActiveDuration <= 0.f)
	{
		CommitInteraction();
		return true;
	}

	const float UpdateRate = FMath::Max(UFVInteractionSystemSettings::Get().WidgetUpdateFrequency, 0.01f);

	GetWorld()->GetTimerManager().SetTimer(
		InteractionTimer,
		FTimerDelegate::CreateUObject(this, &UInteractorComponent::TickInteraction),
		UpdateRate,
		true);

	return true;
}

void UInteractorComponent::TickInteraction()
{
	if (!bIsInteracting)
	{
		return;
	}

	UInteractableComponent* Target = ActiveCommit.Interactable;
	if (!IsValid(Target) || Target != FocusedTarget.Get())
	{
		CancelInteraction(InteractionTags::Interaction_Cancel_FocusLost);
		return;
	}

	if (Target->GetState() == EInteractableState::Suppressed)
	{
		CancelInteraction(InteractionTags::Interaction_Cancel_Suppressed);
		return;
	}

	if (const FInteractionOffer* Offer = FindActiveOffer())
	{
		if (!IsOfferAvailable(*Offer))
		{
			CancelInteraction(InteractionTags::Interaction_Cancel_RequirementFailed);
			return;
		}
	}

	const float UpdateRate = FMath::Max(UFVInteractionSystemSettings::Get().WidgetUpdateFrequency, 0.01f);
	ActiveElapsed += UpdateRate;

	if (ActiveMode == EInteractionInputMode::Mash)
	{
		if (ActiveElapsed >= ActiveDuration)
		{
			CancelInteraction(InteractionTags::Interaction_Cancel_Timeout);
			return;
		}
		
		if (const UInteractionSignalComponent* Signal = UInteractionSignalComponent::Get(GetOwner()))
		{
			Signal->InteractionProgress.Broadcast(ActiveCommit, ActivePresses / static_cast<float>(ActiveRequiredPresses))
		}

		return;
	}

	const float Progress = FMath::Clamp(ActiveElapsed / ActiveDuration, 0.f, 1.f);

	if (const UInteractionSignalComponent* Signal = UInteractionSignalComponent::Get(GetOwner()))
	{
		Signal->InteractionProgress.Broadcast(ActiveCommit, Progress);
	}

	if (Progress >= 1.f)
	{
		CommitInteraction();
	}
}

void UInteractorComponent::CommitInteraction()
{
	if (!bIsInteracting)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(InteractionTimer);
	bIsInteracting = false;
	UpdateState();

	if (const UInteractionSignalComponent* Signal = UInteractionSignalComponent::Get(GetOwner()))
	{
		Signal->InteractionRequested.Broadcast(ActiveCommit);
	}

#if !UE_BUILD_SHIPPING
	DebugLastOutcome = EDebugActionOutcome::Succeeded;
#endif

	if (UInteractableComponent* Target = ActiveCommit.Interactable)
	{
		Target->ConsumeOffer(ActiveCommit.ActionTag);
		Target->SetState(EInteractableState::Awake);
	}
	
	RefreshOffers();
}

void UInteractorComponent::CancelInteraction(const FGameplayTag& Reason)
{
	if (!bIsInteracting)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(InteractionTimer);
	bIsInteracting = false;
	UpdateState();

	if (const UInteractionSignalComponent* Signal = UInteractionSignalComponent::Get(GetOwner()))
	{
		Signal->InteractionCancelled.Broadcast(ActiveCommit, Reason);
	}

	if (UInteractableComponent* Target = ActiveCommit.Interactable)
	{
		Target->SetState(EInteractableState::Awake);
	}

	RefreshOffers();
}

bool UInteractorComponent::TryExecuteInteraction(FGameplayTag InputTag)
{
	return PushInput(InputTag, EInteractionInputPhase::Pressed);
}

bool UInteractorComponent::IsOfferAvailable(const FInteractionOffer& Offer) const
{
	if (BlockedActionTags.HasTag(Offer.ActionTag))
	{
		return false;
	}

	return Offer.AreTagsSatisfied(InteractorTags);
}

FVector UInteractorComponent::GetDetectionOrigin() const
{
	return Owner ? Owner->GetActorLocation() : FVector::ZeroVector;
}

void UInteractorComponent::AddInteractorTag(FGameplayTag NewTag)
{
	if (!NewTag.IsValid() || InteractorTags.HasTagExact(NewTag))
	{
		return;
	}

	InteractorTags.AddTag(NewTag);
	RefreshOffers();
}

void UInteractorComponent::RemoveInteractorTag(FGameplayTag OldTag)
{
	if (!InteractorTags.HasTagExact(OldTag))
	{
		return;
	}

	InteractorTags.RemoveTag(OldTag);
	RefreshOffers();
}

bool UInteractorComponent::GetTraceOrigin(FVector& OutOrigin, FVector& OutForward) const
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

	if (TracingSetup.SafetyTracingMode == ESafetyTracingMode::Socket && !TracingSetup.StartSocketName.IsNone())
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

bool UInteractorComponent::PerformSafetyTrace(const FVector& Origin, const UInteractableComponent& Candidate) const
{
	if (TracingSetup.SafetyTracingMode == ESafetyTracingMode::None)
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

void UInteractorComponent::ProcessTrace()
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

	if (Precision == EInteractorPrecision::Trace)
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
		UInteractableComponent* Interactable;
		FVector ImpactPoint;
	};

	TArray<FRankedCandidate> Ranked;

	for (const FHitResult& Hit : Hits)
	{
		const AActor* HitActor = Hit.GetActor();
		UInteractableComponent* Interactable = HitActor
			? HitActor->FindComponentByClass<UInteractableComponent>()
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

void UInteractorComponent::SetFocusedTarget(UInteractableComponent* NewTarget)
{
	if (FocusedTarget.Get() == NewTarget)
	{
		return;
	}

	if (UInteractableComponent* Previous = FocusedTarget.Get())
	{
		Previous->StateChanged.RemoveDynamic(this, &UInteractorComponent::HandleFocusedStateChanged);
		Previous->SetFocused(false, this);
	}

	FocusedTarget = NewTarget;

	if (NewTarget)
	{
		NewTarget->SetFocused(true, this);
		NewTarget->StateChanged.AddDynamic(this, &UInteractorComponent::HandleFocusedStateChanged);
	}

	if (const UInteractionSignalComponent* Signal = UInteractionSignalComponent::Get(GetOwner()))
	{
		Signal->FocusChanged.Broadcast(NewTarget);
	}

	RefreshOffers();
}

void UInteractorComponent::HandleFocusedStateChanged(EInteractableState NewState)
{
	RefreshOffers();
}

void UInteractorComponent::RefreshOffers(bool bForceBroadcast)
{
	const UInteractableComponent* Target = FocusedTarget.Get();
	TArray<FInteractionOffer> NewOffers;

	if (Target)
	{
		for (const FInteractionOffer& Offer : Target->GetOffers())
		{
			if (!Offer.IsValid())
			{
				continue;
			}

			const bool bRequirementsMet = IsOfferAvailable(Offer);

			if (!bRequirementsMet && Offer.RequirementGate == EInteractionGate::Hide)
			{
				continue;
			}

			FInteractionOffer& Slot = NewOffers.Add_GetRef(Offer);
			Slot.bRequirementsMet = bRequirementsMet;
		}
	}


	NewOffers.Sort([](const FInteractionOffer& A, const FInteractionOffer& B)
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
		if (const UInteractionSignalComponent* Signal = UInteractionSignalComponent::Get(GetOwner()))
		{
			Signal->OffersChanged.Broadcast(CachedOffers);
		}
	}
}
