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

	if (!FocusComponentTag.IsValid())
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' has an InteractableComponent but no FocusComponentTag value is set. It will not be registered as interactable."),
			*GetOwner()->GetName());
		return;
	}
	
	FocusPrimitive = GetOwner()->FindComponentByTag<UPrimitiveComponent>(FocusComponentTag);

	if (!FocusPrimitive.IsValid())
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' has an InteractableComponent but no PrimitiveComponent with the set tag value was found to derive focus bounds from. It will not be registered as interactable."),
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

const FInteractionOffer* UInteractableComponent::FindOffer(const FGameplayTag& InputTag) const
{
	return Offers.FindByPredicate([InputTag](const FInteractionOffer& Offer)
	{
		return Offer.InputTag == InputTag;
	});
}