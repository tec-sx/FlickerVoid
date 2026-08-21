#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include "FVInteractionSystemSettings.h"
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

	ResolveFocusProfile();

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

#if WITH_EDITOR
void UInteractableComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	ResolveFocusProfile();
}
#endif

const FInteractionFocusProfile& UInteractableComponent::GetFocusProfile() const
{
	if (CachedFocusProfile)
	{
		return *CachedFocusProfile;
	}

	return GetDefault<UFVInteractionSystemSettings>()->GetFocusProfile(FocusProfileName);
}

void UInteractableComponent::ResolveFocusProfile()
{
	CachedFocusProfile = &GetDefault<UFVInteractionSystemSettings>()->GetFocusProfile(FocusProfileName);
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
