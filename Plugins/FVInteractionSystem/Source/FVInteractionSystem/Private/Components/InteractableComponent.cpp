#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "FVInteractionSystem.h"
#include "FVInteractionSystemSettings.h"
#include <Subsystems/InteractionRegistrySubsystem.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractableComponent)

namespace
{
	constexpr float MinFocusExtent = 10.f;
}

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

	FocusPrimitive = GetOwner()->FindComponentByClass<UPrimitiveComponent>();

	if (!FocusPrimitive.IsValid())
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' has an InteractableComponent but no PrimitiveComponent to derive focus bounds from. It will not be registered as interactable."),
			*GetOwner()->GetName());
		return;
	}

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

FVector UInteractableComponent::GetFocusPoint() const
{
	if (const UPrimitiveComponent* Primitive = FocusPrimitive.Get())
	{
		return Primitive->Bounds.Origin;
	}

	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

FVector UInteractableComponent::GetClosestFocusPoint(const FVector& FromLocation) const
{
	const UPrimitiveComponent* Primitive = FocusPrimitive.Get();

	if (!Primitive)
	{
		return GetFocusPoint();
	}

	const FBoxSphereBounds& Bounds = Primitive->Bounds;
	const FVector ClampedExtent = FVector::Max(Bounds.BoxExtent, FVector(MinFocusExtent));

	return FBox(Bounds.Origin - ClampedExtent, Bounds.Origin + ClampedExtent).GetClosestPointTo(FromLocation);
}

void UInteractableComponent::SetFocused(bool bFocused)
{
	if (bIsInitialized && bFocused != bIsInFocus)
	{
		bIsInFocus = bFocused;
	}
}
