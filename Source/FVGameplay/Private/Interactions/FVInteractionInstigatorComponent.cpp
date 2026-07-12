#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Engine/OverlapResult.h"

static TAutoConsoleVariable CVarInteractionDebug(TEXT("FVCvar.Interaction.Debug"), false, TEXT("Debug the interaction system"));

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

	if (DetectionObjectTypes.IsEmpty())
	{
		DetectionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
		DetectionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
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

		if (DetectionUpdateInterval <= 0.f || TimeSinceLastUpdate >= DetectionUpdateInterval)
		{
			TimeSinceLastUpdate = 0.f;
			DetectInteractables();
		}
	}
}

//~=============================================================================
// Focus Detection
//~=============================================================================

void UFVInteractionInstigatorComponent::DetectInteractables() const
{
	UFVInteractionTargetComponent* NewTarget = FindBestTarget();
	
	if (FocusedTarget.Get() == NewTarget)
	{
		return;
	}
	
	if (UFVInteractionTargetComponent* Previous = FocusedTarget.Get())
	{
		Previous->SetFocused(false);
	}

	FocusedTarget = NewTarget;
	
	if (NewTarget)
	{
		NewTarget->SetFocused(true);
	}
	
	OnFocusChanged.Broadcast(NewTarget);
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
		FCollisionObjectQueryParams(DetectionObjectTypes),
		FCollisionShape::MakeSphere(DetectionRadius),
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
		if (DotProduct < DetectionConeAngle)
		{
			continue;
		}

		// Score = dot (angle quality) + proximity bonus
		const float Score = DotProduct + (1.f - Distance / DetectionRadius);
		if (Score > BestScore)
		{
			BestScore = Score;
			BestTarget = Target;
		}
	}
	
#if UE_BUILD_DEVELOPMENT
	if (CVarInteractionDebug.GetValueOnGameThread() == true)
	{
		FVector TargetOrigin = FVector::ZeroVector;
		FVector TargetBoxExtent = FVector::ZeroVector;
		
		BestTarget->GetOwner()->GetActorBounds(false, TargetOrigin, TargetBoxExtent);
		DrawDebugSphere(
			GetWorld(), 
			BestTarget->GetOwner()->GetActorLocation(),
			TargetBoxExtent.GetMax(),
			8,
			FColor::Red);
	}
#endif
	
	return BestTarget;
}
