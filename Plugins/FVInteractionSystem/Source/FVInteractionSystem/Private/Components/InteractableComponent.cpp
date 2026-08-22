#include "Components/InteractableComponent.h"
#include "Components/InteractorComponent.h"
#include "Components/PrimitiveComponent.h"
#include "FVInteractionSystem.h"
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

	FocusPrimitive = GetOwner()->FindComponentByClass<UPrimitiveComponent>();

	if (!FocusPrimitive.IsValid())
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' has an InteractableComponent but no PrimitiveComponent to derive focus bounds from. It will not be registered as interactable."),
			*GetOwner()->GetName());
		return;
	}

	FocusPrimitive->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Block);

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

FVector UInteractableComponent::GetFocusPoint() const
{
	if (const UPrimitiveComponent* Primitive = FocusPrimitive.Get())
	{
		return Primitive->Bounds.Origin;
	}

	return GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

void UInteractableComponent::SetFocused(bool bFocused)
{
	if (bIsInitialized && bFocused != bIsInFocus)
	{
		bIsInFocus = bFocused;
	}
}
