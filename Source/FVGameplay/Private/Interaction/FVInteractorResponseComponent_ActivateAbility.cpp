#include "Interaction/FVInteractorResponseComponent_ActivateAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorResponseComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "AbilitySystemComponent.h"
#include "Components/FVInteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractorResponseComponent_ActivateAbility)

void UFVInteractorResponseComponent_ActivateAbility::BindEvents_Implementation(UFVInteractorComponent* Interactor)
{
	Interactor->InteractionCommitEnded.AddDynamic(this, &UFVInteractorResponseComponent_ActivateAbility::OnInteractionCommited);
}

void UFVInteractorResponseComponent_ActivateAbility::UnbindEvents_Implementation(UFVInteractorComponent* Interactor)
{
	Interactor->InteractionCommitEnded.RemoveDynamic(this, &UFVInteractorResponseComponent_ActivateAbility::OnInteractionCommited);
}

void UFVInteractorResponseComponent_ActivateAbility::OnInteractionCommited(const FFVInteractionCommit& Commit, bool bSuccess) const
{
	AActor* Instigator = GetOwner();
	if (!bSuccess || !Instigator || !Commit.ActionTag.IsValid() || !Commit.Interactable)
	{
		return;
	}

	FGameplayEventData EventData;
	EventData.EventTag = Commit.ActionTag;
	EventData.Instigator = Instigator;
	EventData.Target = Commit.Interactable->GetOwner();
	EventData.OptionalObject = Commit.Interactable;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Instigator, Commit.ActionTag, EventData);
}
