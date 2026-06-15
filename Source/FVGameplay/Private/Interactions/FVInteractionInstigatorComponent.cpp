#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"
#include "Interactions/FVInteractionInstigatorConfig.h"
#include "Subsystems/FVInteractionSubsystem.h"

UFVInteractionInstigatorComponent::UFVInteractionInstigatorComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFVInteractionInstigatorComponent::BeginPlay()
{
	Super::BeginPlay();

	Owner = GetOwner();
	if (!Owner)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction Component has invalid owner."));
		return;
	}

	if (!Config)
	{
		UE_LOG(LogTemp, Warning, TEXT("Interaction Instigator Configuration not found. creating default object."));
		Config = NewObject<UFVInteractionInstigatorConfig>();
	}

	if (Config->DetectionObjectTypes.IsEmpty())
	{
		Config->DetectionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
		Config->DetectionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	}

	bIsInitialized = true;
}

void UFVInteractionInstigatorComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsInitialized)
	{
		TimeSinceLastUpdate += DeltaTime;

		if (Config->FocusUpdateInterval <= 0.f || TimeSinceLastUpdate >= Config->FocusUpdateInterval)
		{
			TimeSinceLastUpdate = 0.f;
			UpdateFocus();
		}
	}
}

//~=============================================================================
// Focus Detection
//~=============================================================================

void UFVInteractionInstigatorComponent::UpdateFocus() const
{
	UFVInteractionTargetComponent* NewTarget = FindBestTarget();
	
	GetInteractionSubsystem()->UpdateFocus(NewTarget);
}

UFVInteractionTargetComponent* UFVInteractionInstigatorComponent::FindBestTarget() const
{
	// Determine the view direction — prefer controller if available
	FVector ViewLocation = Owner->GetActorLocation();
	FVector ViewForward = Owner->GetActorForwardVector();

	if (const APawn* Pawn = Cast<APawn>(Owner))
	{
		if (const APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
		{
			FRotator ViewRotation;
			PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
			ViewForward = ViewRotation.Vector();
		}
	}

	// Collect all overlapping actors
	TArray<FOverlapResult> Overlaps;
	const FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractionFocus), false, Owner);

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		Owner->GetActorLocation(),
		FQuat::Identity,
		FCollisionObjectQueryParams(Config->DetectionObjectTypes),
		FCollisionShape::MakeSphere(Config->DetectionRadius),
		Params);

	UFVInteractionTargetComponent* BestTarget = nullptr;
	float BestScore = -1.f;

	for (const FOverlapResult& Overlap : Overlaps)
	{
		const AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActor == Owner)
		{
			continue;
		}

		UFVInteractionTargetComponent* Target = HitActor->FindComponentByClass<UFVInteractionTargetComponent>();
		if (!Target)
		{
			continue;
		}

		FVector TargetLocation = (HitActor->GetActorLocation() - ViewLocation);
		const float Distance = TargetLocation.Size();

		if (Distance > Target->GetFocusRadius())
		{
			continue;
		}

		const float DotProduct = FVector::DotProduct(ViewForward, TargetLocation.GetSafeNormal());
		if (DotProduct < Config->FocusConeCosine)
		{
			continue;
		}

		// Score = dot (angle quality) + proximity bonus
		const float Score = DotProduct + (1.f - Distance / Config->DetectionRadius);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}

	return BestTarget;
}

//~=============================================================================
// Input Routing
//~=============================================================================

EFVInteractionResult UFVInteractionInstigatorComponent::RequestInteraction(const FGameplayTag& InputTag)
{
	UFVInteractionTargetComponent* Target = InteractionSubsystem->GetFocusedTarget();
	if (!Target)
	{
		return EFVInteractionResult::NoInteractable;
	}

	const EFVInteractionResult Result = Target->TryExecuteAction(InputTag, GetOwner(), InstigatorTags);

	// Re-broadcast so the UI can reflect updated availability after execution
	if (Result == EFVInteractionResult::Success)
	{
		GetInteractionSubsystem()->OnFocusChanged.Broadcast(Target);
	}

	return Result;
}

void UFVInteractionInstigatorComponent::CancelCurrentInteraction() const
{
	if (UFVInteractionTargetComponent* Current = InteractionSubsystem->GetFocusedTarget())
	{
		Current->CancelActiveInteraction();
	}
}

UFVInteractionSubsystem* UFVInteractionInstigatorComponent::GetInteractionSubsystem() const
{
	// Fast-path: return cached if valid.
	if (InteractionSubsystem.IsValid())
	{
		return InteractionSubsystem.Get();
	}

	if (const APlayerController* PC = Cast<APlayerController>(Owner->GetInstigatorController()))
	{
		if (const ULocalPlayer* LP = PC->GetLocalPlayer())
		{
			if (UFVInteractionSubsystem* Sub = LP->GetSubsystem<UFVInteractionSubsystem>())
			{
				InteractionSubsystem = Sub;
				return Sub;
			}
		}
	}
	
	return nullptr;
}
