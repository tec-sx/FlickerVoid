#include "Components/FVInteractorComponent.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorResponseComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Core/FVInteractionGameplayTags.h"
#include "Engine/World.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include "GameFramework/PlayerController.h"
#include "Misc/ScopeExit.h"
#include "Subsystems/FVInteractionRegistrySubsystem.h"
#include "Core/FVInteractionLogger.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"

#if !UE_BUILD_SHIPPING
#include "Subsystems/FVInteractionDebugSubsystem.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractorComponent)

UFVInteractorComponent::UFVInteractorComponent()
	: CollisionChannel(ECC_Camera)
	, OcclusionChannel(ECC_Camera)
	, State(EFVInteractorState::Idle)
	, InteractorTag(FVInteractionGameplayTags::Interactor_Tag_Player)
	, TraceRadius(15.f)
	, TickInterval(0.1f)
	, TraceRange(250.f)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UFVInteractorComponent::BeginPlay()
{
	Super::BeginPlay();
	
	if (!Cast<APawn>(GetOwner()))
	{
		UE_LOG(LogFVInteraction, Warning, TEXT("Interaction Component has invalid pawn owner."));
		return;
	}

	Registry = GetWorld()->GetSubsystem<UFVInteractionRegistrySubsystem>();
	
	if (IsValid(Registry))
	{
		Registry->RegisterInteractor(this);
	}
	
	SetComponentTickInterval(TickInterval);

#if !UE_BUILD_SHIPPING
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		DebugSubsystem = GameInstance->GetSubsystem<UFVInteractionDebugSubsystem>();
	}
#endif
}

void UFVInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (State == EFVInteractorState::Interacting)
	{
		TickInteraction(DeltaTime);
		return;
	}

	if (State == EFVInteractorState::Idle || State == EFVInteractorState::Awake)
	{
		const bool bHasInteractablesInRange = IsValid(Registry) && Registry->GetActiveInteractables().Num() > 0;
		SetState(bHasInteractablesInRange ? EFVInteractorState::Awake : EFVInteractorState::Idle);
	}
	
	if (State != EFVInteractorState::Awake)
	{
		if (TargetInteractable.IsValid())
		{
			ReleaseTargetInteractable();
			RefreshOffers();
		}
		return;
	}
	
	PerformTrace();
}

void UFVInteractorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DisableTracing();

	if (IsValid(Registry))
	{
		Registry->UnregisterInteractor();
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInteractorComponent::EnableTracing()
{
	PrimaryComponentTick.SetTickFunctionEnable(true);
}

void UFVInteractorComponent::DisableTracing()
{
	ReleaseTargetInteractable();
	RefreshOffers();
	PrimaryComponentTick.SetTickFunctionEnable(false);
}

void UFVInteractorComponent::BindResponse(UFVInteractorResponseComponent* Response)
{
	if (!IsValid(Response))
	{
		return;
	}

	Response->BindEvents(this);
}

void UFVInteractorComponent::UnbindResponse(UFVInteractorResponseComponent* Response)
{
	if (!IsValid(Response))
	{
		return;
	}

	Response->UnbindEvents(this);
}

void UFVInteractorComponent::AddSuppression(FGameplayTag Reason)
{
	if (!Reason.IsValid() || SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.AddTag(Reason);
	CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Suppressed);
	SetState(EFVInteractorState::Suppressed);
}

void UFVInteractorComponent::RemoveSuppression(FGameplayTag Reason)
{
	if (!SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.RemoveTag(Reason);

	if (SuppressionReasons.IsEmpty() && State == EFVInteractorState::Suppressed)
	{
		EFVInteractorState NewState = EFVInteractorState::Idle;

		if (IsValid(Registry) && Registry->GetActiveInteractables().Num() > 0)
		{
			NewState = EFVInteractorState::Awake;
		}

		SetState(NewState);
	}
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

bool UFVInteractorComponent::PushInput(FGameplayTag InputTag, EFVInteractionInputPhase Phase)
{
#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->DebugInput(InputTag, FPlatformTime::Seconds());
		Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::NoPrompt);
	}
#endif

	if (State == EFVInteractorState::Interacting)
	{
		if (!ActiveCommit.InputTag.MatchesTagExact(InputTag))
		{
			return false;
		}
		
		PendingPresses += Phase == EFVInteractionInputPhase::Pressed ? 1 : 0;
		bPendingRelease |= Phase == EFVInteractionInputPhase::Released;
		bPendingCancel |= Phase == EFVInteractionInputPhase::Cancelled;
		return true;
	}

	if (State != EFVInteractorState::Awake || Phase != EFVInteractionInputPhase::Pressed)
	{
		return false;
	}

	UFVInteractableComponent* Target = TargetInteractable.Get();
	if (!Target || !Target->CanInteract())
	{
		return false;
	}

	const FFVInteractionOffer* Offer = CachedOffers.FindByPredicate([&InputTag](const FFVInteractionOffer& Candidate)
	{
		return Candidate.InputTag.MatchesTagExact(InputTag);
	});

	if (!Offer || !Offer->CanExecute())
	{
#if !UE_BUILD_SHIPPING
		if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
		{
			Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::Disabled);
		}
#endif
		return false;
	}

	ActiveCommit = FFVInteractionCommit();
	ActiveCommit.ActionTag = Offer->ActionTag;
	ActiveCommit.InputTag = Offer->InputTag;
	ActiveCommit.Interactable = Target;
	ActiveCommit.Interactor = this;

	ActiveMode = Offer->InputMode == EFVInteractionInputMode::Default
		? EFVInteractionInputMode::Press
		: Offer->InputMode;
	ActiveDuration = Offer->InteractionPeriod < 0.f
		? UFVInteractionSystemSettings::Get().InteractableBaseSettings.DefaultInteractionPeriod
		: Offer->InteractionPeriod;
	ActiveElapsed = 0.f;
	ActivePresses = 0;
	ActiveRequiredPresses = FMath::Max(Offer->RequiredPresses, 1);
	PendingPresses = 0;
	bPendingRelease = false;
	bPendingRelease = false;

	SetState(EFVInteractorState::Interacting);
	SetComponentTickInterval(0.f);
	Target->StartInteraction(ActiveCommit.ActionTag, this);
	InteractionCommitStarted.Broadcast(ActiveCommit);

	return true;
}

void UFVInteractorComponent::TickInteraction(float DeltaTime)
{
	ActiveElapsed += DeltaTime;

	const int32 Presses = PendingPresses;
	const bool bReleased = bPendingRelease;
	const bool bCancelled = bPendingCancel;
	PendingPresses = 0;
	bPendingRelease = false;
	bPendingCancel = false;

	UFVInteractableComponent* Target = ActiveCommit.Interactable;
	if (!IsValid(Target) || !InteractableIsInReach(Target))
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_FocusLost);
		return;
	}

	if (Target->GetState() == EFVInteractableState::Suppressed)
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Suppressed);
		return;
	}

	const FFVInteractionOffer* Offer = Target->FindOffer(ActiveCommit.InputTag);
	if (!Offer || !IsOfferAvailable(*Offer))
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_RequirementFailed);
		return;
	}

	if (bCancelled)
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Player);
		return;
	}

	float Progress = 1.f;

	switch (ActiveMode)
	{
	case EFVInteractionInputMode::Hold:
		if (bReleased)
		{
			CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Released);
			return;
		}
		Progress = ActiveDuration > 0.f ? FMath::Clamp(ActiveElapsed / ActiveDuration, 0.f, 1.f) : 1.f;
		break;

	case EFVInteractionInputMode::Mash:
		ActivePresses += Presses;
		if (ActiveDuration > 0.f && ActiveElapsed >= ActiveDuration && ActivePresses < ActiveRequiredPresses)
		{
			CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Timeout);
			return;
		}
		Progress = FMath::Clamp(ActivePresses / static_cast<float>(ActiveRequiredPresses), 0.f, 1.f);
		break;

	default:
		break;
	}

	ProgressInteraction(Progress);

	if (Progress >= 1.f)
	{
		FinishInteraction(true);
	}
}

void UFVInteractorComponent::CancelInteraction(const FGameplayTag& Reason)
{
	if (State != EFVInteractorState::Interacting)
	{
		return;
	}

	FinishInteraction(false);
}

void UFVInteractorComponent::FinishInteraction(bool bSuccess)
{
#if !UE_BUILD_SHIPPING
	if (bSuccess)
	{
		if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
		{
			Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::Succeeded);
		}
	}
#endif

	SetState(EFVInteractorState::Awake);
	SetComponentTickInterval(TickInterval);
	InteractionCommitEnded.Broadcast(ActiveCommit, bSuccess);
	
	if (UFVInteractableComponent* Target = ActiveCommit.Interactable)
	{
		if (bSuccess)
		{
			Target->ConsumeOffer(ActiveCommit.InputTag);
		}

		Target->EndInteraction(ActiveCommit.ActionTag, this, bSuccess);
	}
	
	ActiveCommit = FFVInteractionCommit();
	RefreshOffers();
}

void UFVInteractorComponent::ProgressInteraction(const float Progress)
{
	InteractionCommitProgressed.Broadcast(ActiveCommit, Progress);
	
	if (UFVInteractableComponent* Target = ActiveCommit.Interactable)
	{
		Target->ProgressInteraction(ActiveCommit.ActionTag, this, Progress);
	}
}

bool UFVInteractorComponent::IsOfferAvailable(const FFVInteractionOffer& Offer) const
{
	if (BlockedActionTags.HasTag(Offer.ActionTag))
	{
		return false;
	}

	return Offer.AreTagsSatisfied(GrantedTags);
}

void UFVInteractorComponent::GrantTag(FGameplayTag NewTag)
{
	if (!NewTag.IsValid() || GrantedTags.HasTagExact(NewTag))
	{
		return;
	}

	GrantedTags.AddTag(NewTag);
	RefreshOffers();
}

void UFVInteractorComponent::RemoveTag(FGameplayTag OldTag)
{
	if (!GrantedTags.HasTagExact(OldTag))
	{
		return;
	}

	GrantedTags.RemoveTag(OldTag);
	RefreshOffers();
}

void UFVInteractorComponent::PerformTrace()
{
	FTraceData TraceData;
	{
		TraceData.CollisionChannel = CollisionChannel;
		TraceData.CollisionParams.AddIgnoredActor(GetOwner());
		TraceData.CollisionParams.AddIgnoredActors(IgnoredActors);
		TraceData.CollisionParams.MobilityType = EQueryMobilityType::Any;
		TraceData.CollisionParams.bReturnPhysicalMaterial = true;
		
		GetOwner()->GetActorEyesViewPoint(TraceData.StartLocation, TraceData.TraceRotation);
		FVector DirectionVector = UKismetMathLibrary::GetForwardVector(TraceData.TraceRotation);
		TraceData.EndLocation = DirectionVector * TraceRange + TraceData.StartLocation;
	}
	
	const FCollisionShape CollisionShape = FCollisionShape::MakeSphere(TraceRadius);

	GetWorld()->SweepMultiByChannel
	(
		TraceData.HitResults,
		TraceData.StartLocation,
		TraceData.EndLocation,
		TraceData.TraceRotation.Quaternion(),
		TraceData.CollisionChannel,
		CollisionShape,
		TraceData.CollisionParams
	);

	UFVInteractableComponent* BestInteractableCandidate = nullptr;
	float BestDetectionWeight = -1;
	
	for (FHitResult& HitResult : TraceData.HitResults)
	{	
		const AActor* HitActor = HitResult.GetActor();
		const UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		
		if (!IsValid(HitActor) || !IsValid(HitComponent))
			continue;
		
		UFVInteractableComponent* InteractableCandidate = HitActor->FindComponentByClass<UFVInteractableComponent>();
		
		if (!IsValid(InteractableCandidate))
			continue;
		if (!InteractableCandidate->CanInteract())
			continue;
		if (InteractableCandidate->GetCollisionChannel() != CollisionChannel)
			continue;
		if (!InteractableCandidate->GetDetectablePrimitives().Contains(HitComponent))
			continue;
		if (!InteractableCandidate->GetCompatibleInteractorTags().HasTag(InteractorTag))
			continue;
			
		const float CandidateDetectionWeight = InteractableCandidate->GetDetectionWeight();

		if (CandidateDetectionWeight <= BestDetectionWeight)
			continue;
		if (PerformOcclusionTest(TraceData.StartLocation, HitResult.ImpactPoint, HitActor))
			continue;
		
		BestDetectionWeight = CandidateDetectionWeight;
		BestInteractableCandidate = InteractableCandidate;
	}

	if (BestInteractableCandidate != TargetInteractable.Get())
	{
		ReleaseTargetInteractable();
		
		if (IsValid(BestInteractableCandidate))
		{
			AcquireInteractable(BestInteractableCandidate);
		}

		RefreshOffers();
	}

#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->VisualizeTrace(GetWorld(), TraceData, TraceRadius, TickInterval);
		Debug->DebugTrace(TraceData.HitResults);
	}
#endif
}

bool UFVInteractorComponent::PerformOcclusionTest(const FVector& Start, const FVector& End, const AActor* Target)
{
	FHitResult OcclusionHit;
	FCollisionQueryParams QueryParams;
	{
		QueryParams.AddIgnoredActor(GetOwner());
	}
	
	GetWorld()->LineTraceSingleByChannel(OcclusionHit, Start, End, OcclusionChannel, QueryParams);
	
#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->DebugOcclusion(OcclusionHit);
	}
#endif
	
	return OcclusionHit.IsValidBlockingHit() && OcclusionHit.GetActor() != Target; 
}

void UFVInteractorComponent::AcquireInteractable(UFVInteractableComponent* NewInteractable)
{
	if (IsValid(NewInteractable))
	{
		TargetInteractable = NewInteractable;
	
		if (UFVInteractableComponent* Interactable = TargetInteractable.Get())
		{
			Interactable->AcquireInteractor(this);
			InteractableFound.Broadcast(Interactable);
		}
	}
}

void UFVInteractorComponent::ReleaseTargetInteractable()
{
	if (UFVInteractableComponent* Interactable = TargetInteractable.Get())
	{
		Interactable->ReleaseInteractor(this);
		InteractableLost.Broadcast(Interactable);
	}
	
	TargetInteractable.Reset();
}

void UFVInteractorComponent::RefreshOffers(bool bForceBroadcast)
{
	const UFVInteractableComponent* Target = TargetInteractable.Get();
	TArray<FFVInteractionOffer> NewOffers;

	if (Target)
	{
		for (const FFVInteractionOffer& Offer : Target->GetOffers())
		{
			if (!Offer.IsValid())
				continue;

			const bool bRequirementsMet = IsOfferAvailable(Offer);

			if (!bRequirementsMet && Offer.RequirementGate == EFVInteractionGate::Hide)
				continue;
			
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

bool UFVInteractorComponent::InteractableIsInReach(const UFVInteractableComponent* Target) const
{
	const AActor* TargetActor = Target->GetOwner();
	return IsValid(TargetActor) && FVector::DistSquared(GetOwner()->GetActorLocation(), TargetActor->GetActorLocation()) <= FMath::Square(TraceRange * 1.5f);
}
