#include "Subsystems/InteractionRegistrySubsystem.h"
#include "Components/InteractableComponent.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionRegistrySubsystem)

bool UInteractionRegistrySubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	const UWorld* World = Cast<UWorld>(Outer);
	return World && World->IsGameWorld();
}

void UInteractionRegistrySubsystem::Deinitialize()
{
	Interactables.Reset();

	Super::Deinitialize();
}

void UInteractionRegistrySubsystem::Register(UInteractableComponent* Interactable)
{
	if (IsValid(Interactable))
	{
		Interactables.AddUnique(Interactable);
	}
}

void UInteractionRegistrySubsystem::Unregister(UInteractableComponent* Interactable)
{
	Interactables.RemoveSingleSwap(Interactable, EAllowShrinking::No);
}

void UInteractionRegistrySubsystem::QueryInRange(const FVector& Origin, float MaxRadius, TArray<UInteractableComponent*>& OutResults) const
{
	const float MaxRadiusSq = FMath::Square(MaxRadius);

	OutResults.Reset();
	OutResults.Reserve(Interactables.Num());

	for (const TObjectPtr<UInteractableComponent>& Interactable : Interactables)
	{
		if (FVector::DistSquared(Origin, Interactable->GetFocusPoint()) <= MaxRadiusSq)
		{
			OutResults.Add(Interactable);
		}
	}
}