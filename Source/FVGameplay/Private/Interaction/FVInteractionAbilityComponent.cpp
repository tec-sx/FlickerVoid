#include "Interaction/FVInteractionAbilityComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractionAbilityComponent)

UFVInteractionAbilityComponent::UFVInteractionAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UFVInteractionAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	Interactor = GetOwner()->FindComponentByClass<UFVInteractorComponent>();
	if (!ensureMsgf(Interactor, TEXT("%s on %s requires a UFVInteractorComponent on the same actor."), *GetName(), *GetNameSafe(GetOwner())))
	{
		return;
	}

	Interactor->InteractionCommitEnded.AddDynamic(this, &UFVInteractionAbilityComponent::OnInteractionCommited);
}

void UFVInteractionAbilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (Interactor)
	{
		Interactor->InteractionCommitEnded.RemoveDynamic(this, &UFVInteractionAbilityComponent::OnInteractionCommited);
		Interactor = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void UFVInteractionAbilityComponent::OnInteractionCommited(const FFVInteractionCommit& Commit, bool bSuccess)
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
