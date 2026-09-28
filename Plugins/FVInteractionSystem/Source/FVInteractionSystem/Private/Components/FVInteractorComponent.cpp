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
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/FVGestureComponent.h"

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
		const bool bHasInteractablesInRange = IsValid(Registry) && !Registry->GetActiveInteractables().IsEmpty();
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
	CancelInteraction(FGameplayTag());
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

void UFVInteractorComponent::AddSuppression(const FGameplayTag Reason)
{
	if (!Reason.IsValid() || SuppressionReasons.HasTagExact(Reason))
	{
		return;
	}

	SuppressionReasons.AddTag(Reason);
	CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Suppressed);
	SetState(EFVInteractorState::Suppressed);
}

void UFVInteractorComponent::RemoveSuppression(const FGameplayTag Reason)
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

void UFVInteractorComponent::SetState(const EFVInteractorState NewState)
{
	if (State == NewState)
	{
		return;
	}
	
	State = NewState;
	StateChanged.Broadcast(State);
}

bool UFVInteractorComponent::BeginInteraction(const FGameplayTag InputTag)
{
#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->DebugInput(InputTag, FPlatformTime::Seconds());
		Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::NoPrompt);
	}
#endif
	
	if (State != EFVInteractorState::Awake)
	{
		return false;
	}

	UFVInteractableComponent* Target = TargetInteractable.Get();
	if (!Target || !Target->CanInteract())
	{
		return false;
	}

	const FFVInteractionOffer* Offer = FindOffer(InputTag);
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

	UFVGestureComponent* Gesture = ResolveGestureComponent();
	if (!Gesture || !Gesture->BeginGesture(InputTag, Offer->Gesture))
	{
		return false;
	}

	Gesture->PushInput(InputTag, EFVInputPhase::Pressed);

	ActiveCommit = FFVInteractionCommit();
	ActiveCommit.ActionTag = Offer->ActionTag;
	ActiveCommit.InputTag = Offer->InputTag;
	ActiveCommit.Interactable = Target;
	ActiveCommit.Interactor = this;
	
	SetState(EFVInteractorState::Interacting);
	SetComponentTickInterval(0.f);
	Target->StartInteraction(ActiveCommit.ActionTag, this);
	InteractionCommitStarted.Broadcast(ActiveCommit);

	TickInteraction(0.f);
	return true;
}

void UFVInteractorComponent::TickInteraction(const float DeltaTime)
{
	UFVGestureComponent* Gesture = GestureComponent.Get();
	if (!Gesture || !ValidateActiveInteraction())
	{
		FinishInteraction(false);
		return;
	}

	const EFVGestureStatus Status = Gesture->UpdateGesture(DeltaTime);
	if (Status != EFVGestureStatus::Running)
	{
		FinishInteraction(Status == EFVGestureStatus::Completed);
		return;
	}

	const float Progress = Gesture->GetProgress();
	ActiveCommit.Interactable->ProgressInteraction(ActiveCommit.ActionTag, this, Progress);
	InteractionCommitProgressed.Broadcast(ActiveCommit, Progress);
}

bool UFVInteractorComponent::ValidateActiveInteraction()
{
	const UFVInteractableComponent* Target = TargetInteractable.Get();
	if (!Target || Target != ActiveCommit.Interactable || !InteractableIsInReach(Target))
	{
		return false;
	}

	if (Target->GetState() != EFVInteractableState::Interacting)
	{
		return false;
	}

	const FFVInteractionOffer* Offer = FindOffer(ActiveCommit.InputTag);
	return Offer && Offer->CanExecute();
}

void UFVInteractorComponent::CancelInteraction(const FGameplayTag Reason)
{
	if (State == EFVInteractorState::Interacting)
	{
		FinishInteraction(false);
	}
}

void UFVInteractorComponent::FinishInteraction(const bool bSuccess)
{
	if (UFVGestureComponent* Gesture = GestureComponent.Get())
	{
		Gesture->ResetGesture();
	}

#if !UE_BUILD_SHIPPING
	if (bSuccess)
	{
		if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
		{
			Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::Succeeded);
		}
	}
#endif

	const FFVInteractionCommit Commit = ActiveCommit;
	ActiveCommit = FFVInteractionCommit();

	SetState(EFVInteractorState::Awake);
	SetComponentTickInterval(TickInterval);
	
	if (UFVInteractableComponent* Target = ActiveCommit.Interactable)
	{
		if (bSuccess)
		{
			Target->ConsumeOffer(ActiveCommit.InputTag);
		}

		Target->EndInteraction(ActiveCommit.ActionTag, this, bSuccess);
	}
	
	InteractionCommitEnded.Broadcast(ActiveCommit, bSuccess);
	RefreshOffers();
}

const FFVInteractionOffer* UFVInteractorComponent::FindOffer(const FGameplayTag& InputTag)
{
	return CachedOffers.FindByPredicate([&InputTag](const FFVInteractionOffer& Candidate)
	{
		return Candidate.InputTag.MatchesTagExact(InputTag);
	});
}

UFVGestureComponent* UFVInteractorComponent::ResolveGestureComponent()
{
	if (UFVGestureComponent* Cached = GestureComponent.Get())
	{
		return Cached;
	}

	const APawn* Pawn = Cast<APawn>(GetOwner());
	if (!Pawn)
	{
		return nullptr;
	}

	UFVGestureComponent* Found = Pawn->FindComponentByClass<UFVGestureComponent>();
	if (!Found && Pawn->GetController())
	{
		Found = Pawn->GetController()->FindComponentByClass<UFVGestureComponent>();
	}

	ensureMsgf(Found, TEXT("%s requires a UFVGestureComponent on its pawn or controller."), *GetNameSafe(GetOwner()));
	GestureComponent = Found;
	return Found;
}

bool UFVInteractorComponent::IsOfferAvailable(const FFVInteractionOffer& Offer) const
{
	if (BlockedActionTags.HasTag(Offer.ActionTag))
	{
		return false;
	}

	return Offer.AreTagsSatisfied(GrantedTags);
}

void UFVInteractorComponent::GrantTag(const FGameplayTag NewTag)
{
	if (!NewTag.IsValid() || GrantedTags.HasTagExact(NewTag))
	{
		return;
	}

	GrantedTags.AddTag(NewTag);
	RefreshOffers();
}

void UFVInteractorComponent::RemoveTag(const FGameplayTag OldTag)
{
	if (!GrantedTags.HasTagExact(OldTag))
	{
		return;
	}

	GrantedTags.RemoveTag(OldTag);
	RefreshOffers();
}

bool UFVInteractorComponent::PushInput(const FGameplayTag InputTag, const EFVInputPhase Phase)
{
	if (State == EFVInteractorState::Interacting)
	{
		UFVGestureComponent* Gesture = GestureComponent.Get();
		return Gesture && Gesture->PushInput(InputTag, Phase);
	}

	return Phase == EFVInputPhase::Pressed && BeginInteraction(InputTag);
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

bool UFVInteractorComponent::PerformOcclusionTest(const FVector& Start, const FVector& End, const AActor* Target) const
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
