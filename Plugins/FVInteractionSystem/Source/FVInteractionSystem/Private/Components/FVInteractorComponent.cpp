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
#include "TimerManager.h"
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
	
	SetDefaults();
	SetComponentTickInterval(TickInterval);

#if !UE_BUILD_SHIPPING
	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(GetWorld()))
	{
		DebugSubsystem = GameInstance->GetSubsystem<UFVInteractionDebugSubsystem>();
	}
#endif
}

void UFVInteractorComponent::TickComponent(
	float DeltaTime, 
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	if (State == EFVInteractorState::Idle || State == EFVInteractorState::Awake)
	{
		EFVInteractorState NewState = EFVInteractorState::Idle;

		if (IsValid(Registry) && Registry->GetActiveInteractables().Num() > 0)
		{
			NewState = EFVInteractorState::Awake;
		}

		SetState(NewState);
	}
	
	if (State != EFVInteractorState::Awake && State != EFVInteractorState::Interacting)
	{
		ClearFocusedInteractable();
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
	ClearFocusedInteractable();
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

const FFVInteractionOffer* UFVInteractorComponent::FindActiveOffer() const
{
	const UFVInteractableComponent* Target = ActiveCommit.Interactable;
	return Target ? Target->FindOffer(ActiveCommit.InputTag) : nullptr;
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

	UFVInteractableComponent* Target = FocusedInteractable.Get();
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
		if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
		{
			Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::Disabled);
		}
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
	if (State == EFVInteractorState::Interacting)
		return false;
	if (!Target.CanInteract())
		return false;

	ActiveCommit = FFVInteractionCommit();
	ActiveCommit.ActionTag = Offer.ActionTag;
	ActiveCommit.InputTag = Offer.InputTag;
	ActiveCommit.Interactable = &Target;
	ActiveCommit.Interactor = this;

	ActiveMode = Offer.InputMode == EFVInteractionInputMode::Default
		? EFVInteractionInputMode::Press
		: Offer.InputMode;

	ActiveDuration = Offer.InteractionPeriod < 0.f
		? UFVInteractionSystemSettings::Get().InteractableBaseSettings.DefaultInteractionPeriod
		: Offer.InteractionPeriod;

	ActiveElapsed = 0.f;
	ActivePresses = 0;
	ActiveRequiredPresses = FMath::Max(Offer.RequiredPresses, 1);
	
	SetState(EFVInteractorState::Interacting);
	Target.StartInteraction(ActiveCommit.ActionTag, this);
	InteractionCommitStarted.Broadcast(ActiveCommit);
	
	if (ActiveMode == EFVInteractionInputMode::Press || ActiveDuration <= 0.f)
	{
		CommitInteraction();
		return true;
	}

	const float UpdateRate = FMath::Max(UFVInteractionSystemSettings::Get().WidgetUpdateFrequency, 0.01f);

	GetWorld()->GetTimerManager().SetTimer(
		Timer_Interaction,
		FTimerDelegate::CreateUObject(this, &UFVInteractorComponent::TickInteraction),
		UpdateRate,
		true);

	return true;
}

void UFVInteractorComponent::TickInteraction()
{
	if (State != EFVInteractorState::Interacting)
	{
		return;
	}

	UFVInteractableComponent* Target = ActiveCommit.Interactable;
	if (!IsValid(Target) || Target != FocusedInteractable.Get())
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_FocusLost);
		return;
	}

	if (Target->GetState() == EFVInteractableState::Suppressed)
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_Suppressed);
		return;
	}
	
	const FFVInteractionOffer* Offer = FindActiveOffer();
	
	if (!Offer || !IsOfferAvailable(*Offer))
	{
		CancelInteraction(FVInteractionGameplayTags::Interaction_Cancel_RequirementFailed);
		return;
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
	if (State != EFVInteractorState::Interacting)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(Timer_Interaction);
	SetState(EFVInteractorState::Awake);
	
#if !UE_BUILD_SHIPPING
	if (UFVInteractionDebugSubsystem* Debug = DebugSubsystem.Get())
	{
		Debug->DebugInteractionOutcome(EFVDebugInteractionOutcome::Succeeded);
	}
#endif

	InteractionCommitEnded.Broadcast(ActiveCommit, true);
	
	if (UFVInteractableComponent* Target = ActiveCommit.Interactable)
	{
		Target->ConsumeOffer(ActiveCommit.InputTag);
		Target->EndInteraction(ActiveCommit.ActionTag, this, true);
	}
	
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

void UFVInteractorComponent::CancelInteraction(const FGameplayTag& Reason)
{
	if (State != EFVInteractorState::Interacting)
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(Timer_Interaction);
	SetState(EFVInteractorState::Awake);
	
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
	
	UFVInteractableComponent* BestInteractable = nullptr;
	float BestDetectionWeight = -1;
	
	for (FHitResult& HitResult : TraceData.HitResults)
	{	
		const AActor* HitActor = HitResult.GetActor();
		const UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		
		if (!IsValid(HitActor) || !IsValid(HitComponent))
			continue;
		
		TArray<UActorComponent*> InteractableComponents = 
			HitActor->GetComponentsByTag(UFVInteractableComponent::StaticClass(), TEXT("InteractableComponent"));
		
		if (InteractableComponents.IsEmpty())
			continue;
		
		for (UActorComponent* Component : InteractableComponents)
		{
			UFVInteractableComponent* InteractableComponent = Cast<UFVInteractableComponent>(Component);
			
			if (!InteractableComponent)
				continue;
			if (InteractableComponent->GetState() != EFVInteractableState::Awake)
				continue;
			if (InteractableComponent->GetCollisionChannel() != CollisionChannel)
				continue;
			if (!InteractableComponent->GetDetectablePrimitives().Contains(HitComponent))
				continue;
			if (InteractorTag.IsValid() && !InteractableComponent->GetCompatibleInteractorTags().HasTag(InteractorTag))
			{
				LOG_WARNING(
					TEXT("[PerformTrace] Interactor Tag %s is not compatible with %s Interactable on %s Actor"), 
					*InteractorTag.ToString(), 
					*InteractableComponent->GetName(), 
					*HitActor->GetName())
				continue;
			}
				
			const float CandidateDetectionWeight = InteractableComponent->GetDetectionWeight();

			if (CandidateDetectionWeight <= BestDetectionWeight)
				continue;
			if (PerformOcclusionTest(TraceData.StartLocation, HitResult.ImpactPoint, HitActor))
				continue;
			
			BestDetectionWeight = CandidateDetectionWeight;
			BestInteractable = InteractableComponent;
		}
	}

	if (BestInteractable != FocusedInteractable.Get())
	{
		ClearFocusedInteractable();
		
		if (IsValid(BestInteractable))
		{
			SetFocusedInteractable(BestInteractable);
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

void UFVInteractorComponent::SetFocusedInteractable(UFVInteractableComponent* NewInteractable)
{
	NewInteractable->InteractorFound.Broadcast(this);
	InteractableFound.Broadcast(NewInteractable);
	FocusedInteractable = NewInteractable;
}

void UFVInteractorComponent::ClearFocusedInteractable()
{
	if (UFVInteractableComponent* InteractableComponent = FocusedInteractable.Get())
	{
		InteractableComponent->InteractorLost.Broadcast(this);
		InteractableLost.Broadcast(InteractableComponent);
	}
	
	FocusedInteractable.Reset();
}

void UFVInteractorComponent::RefreshOffers(bool bForceBroadcast)
{
	const UFVInteractableComponent* Target = FocusedInteractable.Get();
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
