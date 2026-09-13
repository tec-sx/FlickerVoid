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

	const AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}

	if (UFVInteractableComponent* Interactable = GetOwner()->FindComponentByClass<UFVInteractableComponent>())
	{
		BindEvents(Interactable);	
	}
	else
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' on '%s' can not find an interactable signal component to bind to."),
			*GetName(), 
			*OwningActor->GetName());
	}
}

void UFVInteractableResponseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UFVInteractableComponent* Interactable = GetOwner()->FindComponentByClass<UFVInteractableComponent>())
	{
		UnbindEvents(Interactable);	
	}
	
	Super::EndPlay(EndPlayReason);
}