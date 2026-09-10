#include "Interaction/FVInteractionAbilityResponder.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/InteractableComponent.h"
#include "Components/InteractionResponseComponent.h"
#include "Abilities/GameplayAbilityTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionAbilityResponder)

void UFVInteractionAbilityResponder::BindResponses_Implementation(UInteractionResponseComponent* Response)
{
	Response->OnInteractionRequested.AddDynamic(this, &UFVInteractionAbilityResponder::HandleInteractionRequested);
}

void UFVInteractionAbilityResponder::HandleInteractionRequested(const FInteractionCommit& Commit)
{
	AActor* Instigator = GetOwner();
	if (!Instigator || !Commit.ActionTag.IsValid())
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = Commit.ActionTag;
	EventData.Instigator = Instigator;
	EventData.Target = Commit.Target;
	EventData.OptionalObject = Commit.Interactable;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Instigator, Commit.ActionTag, EventData);
}
