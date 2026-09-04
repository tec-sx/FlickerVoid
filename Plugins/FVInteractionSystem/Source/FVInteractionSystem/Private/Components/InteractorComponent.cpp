#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"
#include "Core/InteractionRequirement.h"
#include "GameFramework/PlayerController.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractorComponent)

UInteractorComponent::UInteractorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UInteractorComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = Cast<APawn>(GetOwner());
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction Component has invalid pawn owner."));
		return;
	}

	Registry = GetWorld()->GetSubsystem<UInteractionRegistrySubsystem>();

	bIsInitialized = Registry != nullptr;
	SetComponentTickEnabled(bIsInitialized);
}


void UInteractorComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsInitialized)
	{
		TimeSinceLastUpdate += DeltaTime;

		if (DetectionUpdateInterval <= 0.f || TimeSinceLastUpdate >= DetectionUpdateInterval)
		{
			TimeSinceLastUpdate = 0.f;
			DetectInteractables();
			RefreshOffers(false);
		}
	}
}

bool UInteractorComponent::TryExecuteAction(FGameplayTag InputTag)
{
#if !UE_BUILD_SHIPPING
	DebugLastInputTag = InputTag;
	DebugLastActionTime = FPlatformTime::Seconds();
	DebugLastOutcome = EDebugActionOutcome::NoPrompt;
#endif

	UInteractableComponent* Target = FocusedTarget.Get();

	if (!Target)
	{
		return false;
	}

	const FInteractionSlot* Prompt = CachedPrompts.FindByPredicate(
		[&InputTag](const FInteractionSlot& Candidate) { return Candidate.InputTag.MatchesTagExact(InputTag); });

	if (!Prompt)
	{
		return false;
	}

	if (Prompt->IsEnabled())
	{
		const FGameplayTag ActionTag = Prompt->ActionTag;
		const bool bExecuted = !ExecuteAction.IsBound() || ExecuteAction.Execute(ActionTag, MakeContext(*Target));

#if !UE_BUILD_SHIPPING
		DebugLastOutcome = bExecuted ? EDebugActionOutcome::Succeeded : EDebugActionOutcome::ExecuteFailed;
#endif

		Target->OnInteractionExecuted.Broadcast(Prompt->ActionTag, this);
		RefreshOffers();
	}
#if !UE_BUILD_SHIPPING
	else
	{
		DebugLastOutcome = EDebugActionOutcome::Disabled;
	}
#endif

	return true;
}

FInteractionContext UInteractorComponent::MakeContext(const UInteractableComponent& Target) const
{
	FInteractionContext Context;
	Context.Interactor = GetOwner();
	Context.Target = Target.GetOwner();
	Context.InteractionPoint = LastFocusImpactPoint.IsNearlyZero() ? Target.GetFocusPoint() : LastFocusImpactPoint;
	return Context;
}

bool UInteractorComponent::EvaluateRequirements(
	const FGameplayTag ActionTag, 
	const TArray<TObjectPtr<UInteractionRequirement>>& Requirements,
	bool& bOutHidden) const
{
	for (const UInteractionRequirement* Requirement : Requirements)
	{
		if (Requirement && !Requirement->IsMet(ActionTag, this, FocusedTarget.Get()))
		{
			bOutHidden = Requirement->Gate == EInteractionGate::Hide;
			return false;
		}
	}
		
	return true;
}

bool UInteractorComponent::GetAimPoint(FVector& OutOrigin, FVector& OutForward) const
{
	const APlayerController* PC = Owner ? Cast<APlayerController>(Owner->GetController()) : nullptr;
	if (!PC)
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;
	PC->GetPlayerViewPoint(ViewLocation, ViewRotation);

	OutOrigin = Owner->GetActorLocation() + Owner->GetActorRotation().RotateVector(AimOriginOffset);
	OutForward = ViewRotation.Vector();
	return true;
}

void UInteractorComponent::DetectInteractables()
{
	const FVector PawnLocation = Owner->GetActorLocation();

	FVector AimOrigin;
	FVector AimForward;
	if (!GetAimPoint(AimOrigin, AimForward))
	{
		Candidates.Reset();
		SetFocusedTarget(nullptr);
		return;
	}

	Registry->QueryInRange(PawnLocation, MaxDetectionRadius, Candidates);

	float GateRadius = 0.f;
	for (const UInteractableComponent* Candidate : Candidates)
	{
		if (FVector::Dist(Candidate->GetFocusPoint(), PawnLocation) <= Candidate->DetectionRadius)
		{
			GateRadius = FMath::Max(GateRadius, Candidate->DetectionRadius);
		}
	}

#if !UE_BUILD_SHIPPING
	DebugSweepDirection = AimForward;
	bDebugGateOpen = GateRadius > 0.f;
	bDebugHitOccluder = false;
	bDebugHasImpact = false;
#endif

	if (GateRadius <= 0.f)
	{
		SetFocusedTarget(nullptr);
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(FVInteractionSweep), false, Owner);

	FHitResult Hit;
	GetWorld()->SweepSingleByChannel(
		Hit,
		AimOrigin,
		AimOrigin + AimForward * GateRadius,
		FQuat::Identity,
		InteractionChannel,
		FCollisionShape::MakeSphere(AimSweepRadius),
		Params);

	UInteractableComponent* HitInteractable = nullptr;
	if (Hit.bBlockingHit)
	{
		if (const AActor* HitActor = Hit.GetActor())
		{
			HitInteractable = HitActor->FindComponentByClass<UInteractableComponent>();
		}

		if (HitInteractable && FVector::Dist(Hit.ImpactPoint, PawnLocation) > HitInteractable->DetectionRadius)
		{
			HitInteractable = nullptr;
		}

#if !UE_BUILD_SHIPPING
		DebugImpactPoint = Hit.ImpactPoint;
		bDebugHasImpact = true;
		bDebugHitOccluder = HitInteractable == nullptr;
#endif
	}

	LastFocusImpactPoint = Hit.bBlockingHit ? Hit.ImpactPoint : FVector::ZeroVector;
	SetFocusedTarget(HitInteractable);
}

void UInteractorComponent::SetFocusedTarget(UInteractableComponent* NewTarget)
{
	if (FocusedTarget.Get() == NewTarget)
	{
		return;
	}

	if (UInteractableComponent* Previous = FocusedTarget.Get())
	{
		Previous->SetFocused(false);
	}

	FocusedTarget = NewTarget;

	if (NewTarget)
	{
		NewTarget->SetFocused(true);
	}

	OnFocusChanged.Broadcast(NewTarget);
	RefreshOffers();
}

void UInteractorComponent::RefreshOffers(bool bForceBroadcast)
{
	const UInteractableComponent* Target = FocusedTarget.Get();
	TArray<FInteractionSlot> NewPrompts;

	if (Target)
	{
		for (const FInteractionOffer& Offer : Target->GetOffers())
		{
			if (!Offer.IsValid())
			{
				continue;
			}
			
			bool bHidden = false;
			const bool bRequirementsMet =  
				EvaluateRequirements(Offer.ActionTag, GlobalRequirements, bHidden) &&
				EvaluateRequirements(Offer.ActionTag, Offer.Requirements, bHidden);
			
			if (!bRequirementsMet && bHidden == true)
			{
				continue;
			}

			FInteractionSlot& Slot = NewPrompts.AddDefaulted_GetRef();
			Slot.InputTag = Offer.InputTag;
			Slot.ActionTag = Offer.ActionTag;
			Slot.bEnabled = bRequirementsMet;
		}
	}


	NewPrompts.Sort([](const FInteractionSlot& A, const FInteractionSlot& B)
	{
		return A.InputTag.ToString() < B.InputTag.ToString();
	});

	const bool bChanged = NewPrompts != CachedPrompts;
	CachedPrompts = MoveTemp(NewPrompts);

	if (bChanged || bForceBroadcast)
	{
		OnOffersChanged.Broadcast(CachedPrompts);
	}
}
