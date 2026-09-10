#include "Components/InteractionResponderComponent.h"

#include "Components/InteractableComponent.h"
#include "Components/InteractionResponseComponent.h"
#include "FVInteractionSystem.h"
#include "GameFramework/Actor.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(InteractionResponderComponent)

UInteractionResponderComponent::UInteractionResponderComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractionResponderComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* OwningActor = GetOwner();
	if (!OwningActor)
	{
		return;
	}

	CachedResponse = UInteractionResponseComponent::Get(OwningActor);
	CachedInteractable = OwningActor->FindComponentByClass<UInteractableComponent>();

	if (CachedResponse.IsValid())
	{
		BindResponses(CachedResponse.Get());
	}

	if (CachedInteractable.IsValid())
	{
		BindInteractableResponses(CachedInteractable.Get());
	}

	if (!CachedResponse.IsValid() && !CachedInteractable.IsValid())
	{
		UE_LOG(LogFVInteraction, Warning,
			TEXT("'%s' on '%s' found neither a response component nor an interactable to bind to."),
			*GetName(), *OwningActor->GetName());
	}
}
