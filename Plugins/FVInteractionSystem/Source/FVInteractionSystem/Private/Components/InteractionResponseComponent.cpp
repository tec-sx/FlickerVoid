#include "Components/InteractionResponseComponent.h"

#include "Components/InteractorComponent.h"
#include "Components/InteractableComponent.h"
#include "FVInteractionSystem.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionResponseComponent)

UInteractionResponseComponent::UInteractionResponseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionResponseComponent::BeginPlay()
{
	Super::BeginPlay();

	const AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}
	
	Interactable = OwningActor->FindComponentByClass<UInteractableComponent>();

	if (!Interactable.IsValid())
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' on '%s' can not find an interactable to bind to."),
			*GetName(), *OwningActor->GetName());
	}
}
