#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include <Subsystems/InteractionRegistrySubsystem.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractableComponent)

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UInteractableComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!ensure(GetOwner()))
	{
		return;
	}

	bIsInitialized = true;

	UInteractionRegistrySubsystem* Registry = GetWorld()->GetSubsystem<UInteractionRegistrySubsystem>();

	if (Registry)
	{
		Registry->Register(this);
	}
}

void UInteractableComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UInteractionRegistrySubsystem* Registry = GetWorld() 
		? GetWorld()->GetSubsystem<UInteractionRegistrySubsystem>() 
		: nullptr;

	if (Registry)
	{
		Registry->Unregister(this);
	}

	bIsInitialized = false;

	Super::EndPlay(EndPlayReason);
}

FVector UInteractableComponent::GetAimProbeLocation() const
{
	if (!AimProbeSocket.IsNone())
	{
		if (const USceneComponent* Mesh = GetOwner()->FindComponentByClass<USkeletalMeshComponent>())
		{
			if (Mesh->DoesSocketExist(AimProbeSocket))
			{
				return Mesh->GetSocketLocation(AimProbeSocket);
			}
		}
	}

	return GetOwner()->GetActorLocation() + AimProbeOffset;
}

void UInteractableComponent::SetFocused(bool bFocused)
{
	if (bIsInitialized && bFocused != bIsInFocus)
	{
		bIsInFocus = bFocused;
	}
}
