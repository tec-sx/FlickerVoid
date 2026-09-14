#include "Components/FVInteractableResponseComponent.h"

#include "FVInteractionSystem.h"
#include "Components/FVInteractableComponent.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractableResponseComponent)

UFVInteractableResponseComponent::UFVInteractableResponseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVInteractableResponseComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwningActor = GetOwner();
	UFVInteractableComponent* Interactable = OwningActor
		? OwningActor->FindComponentByClass<UFVInteractableComponent>()
		: nullptr;

	if (!Interactable)
	{
		UE_LOG(
			LogFVInteraction, 
			Warning,
			TEXT("'%s' requires a UFVInteractableComponent on '%s' but none was found."),
			*GetName(),
			*OwningActor->GetName());

		return;
	}

	BindEvents(Interactable);
}

void UFVInteractableResponseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (const AActor* OwningActor = GetOwner())
	{
		if (UFVInteractableComponent* Interactable = OwningActor->FindComponentByClass<UFVInteractableComponent>())
		{
			UnbindEvents(Interactable);
		}
	}

	Super::EndPlay(EndPlayReason);
}