#include "Interaction/FVInteractionComponent.h"
#include "Interaction/FVInteractableComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"

UFVInteractionComponent::UFVInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFVInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Populate defaults after engine is fully initialized
	if (DetectionObjectTypes.IsEmpty())
	{
		DetectionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
		DetectionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	}
}

void UFVInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	TimeSinceLastUpdate += DeltaTime;

	if (FocusUpdateInterval <= 0.f || TimeSinceLastUpdate >= FocusUpdateInterval)
	{
		TimeSinceLastUpdate = 0.f;
		UpdateFocus();
	}
}

//~=============================================================================
// Focus Detection
//~=============================================================================

void UFVInteractionComponent::UpdateFocus()
{
	UFVInteractableComponent* Best = FindBestInteractable();
	SetFocusedInteractable(Best);
}

UFVInteractableComponent* UFVInteractionComponent::FindBestInteractable() const
{
	AActor* Owner = GetOwner();
	if (!Owner || !GetWorld())
	{
		return nullptr;
	}

	// Determine the view direction — prefer controller if available
	FVector ViewLocation = Owner->GetActorLocation();
	FVector ViewForward  = Owner->GetActorForwardVector();

	if (APawn* Pawn = Cast<APawn>(Owner))
	{
		if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
			ViewForward = ViewRotation.Vector();
		}
	}

	// Collect all overlapping actors
	TArray<FOverlapResult> Overlaps;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionFocus), false, Owner);

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Owner->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(DetectionObjectTypes),
		FCollisionShape::MakeSphere(DetectionRadius),
		Params);

	UFVInteractableComponent* BestComponent = nullptr;
	float BestScore = -1.f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		const AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == Owner)
		{
			continue;
		}

		UFVInteractableComponent* Interactable = HitActor->FindComponentByClass<UFVInteractableComponent>();
		if (!Interactable)
		{
			continue;
		}

		FVector ToActor = (HitActor->GetActorLocation() - ViewLocation);
		const float Distance  = ToActor.Size();

		if (Distance > Interactable->FocusRadius)
		{
			continue;
		}

		const float DotProduct = FVector::DotProduct(ViewForward, ToActor.GetSafeNormal());
		if (DotProduct < FocusConeCosine)
		{
			continue;
		}

		// Score = dot (angle quality) + proximity bonus
		float Score = DotProduct + (1.f - Distance / DetectionRadius);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestComponent = Interactable;
		}
	}

	return BestComponent;
}

void UFVInteractionComponent::SetFocusedInteractable(UFVInteractableComponent* NewInteractable)
{
	if (FocusedInteractable.Get() == NewInteractable)
	{
		return;
	}

	// Clear previous focus
	if (UFVInteractableComponent* Previous = FocusedInteractable.Get())
	{
		Previous->SetFocused(false);
	}

	FocusedInteractable = NewInteractable;

	if (NewInteractable)
	{
		NewInteractable->SetFocused(true);
	}

	BroadcastFocusState();
}

void UFVInteractionComponent::BroadcastFocusState() const
{
	UFVInteractableComponent* Current = FocusedInteractable.Get();
	TArray<FFVInteractionActionDisplay> Actions;

	if (Current)
	{
		Actions = Current->GetActionDisplayData(GetOwner());
	}

	OnFocusChanged.Broadcast(Current, Actions);
}

//~=============================================================================
// Input Routing
//~=============================================================================

EFVInteractionResult UFVInteractionComponent::RequestInteraction(const FGameplayTag& InputTag)
{
	UFVInteractableComponent* Current = FocusedInteractable.Get();
	if (!Current)
	{
		return EFVInteractionResult::NoInteractable;
	}

	EFVInteractionResult Result = Current->TryExecuteAction(InputTag, GetOwner());

	// Re-broadcast so the UI can reflect updated availability after execution
	if (Result == EFVInteractionResult::Success)
	{
		BroadcastFocusState();
	}

	return Result;
}

void UFVInteractionComponent::CancelCurrentInteraction()
{
	if (UFVInteractableComponent* Current = FocusedInteractable.Get())
	{
		Current->CancelActiveInteraction();
	}
}
