// Fill out your copyright notice in the Description page of Project Settings.


#include "FVPlayerCharacter.h"

#include "Abilities/FVAbilitySystemComponent.h"
#include "Components/FVInteractableComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Interaction/FVInteractionAbilityComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerCharacter)

AFVPlayerCharacter::AFVPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UFVAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	InteractorComponent = CreateDefaultSubobject<UFVInteractorComponent>(TEXT("InteractorComponent"));
	InteractionAbilityComponent = CreateDefaultSubobject<UFVInteractionAbilityComponent>(TEXT("InteractionAbilityComponent"));
}

UAbilitySystemComponent* AFVPlayerCharacter::GetAbilitySystemComponent() const
{
	return GetFVAbilitySystemComponent();
}

UFVInteractableComponent* AFVPlayerCharacter::GetFocusedInteractable() const
{
	return InteractorComponent->GetTargetInteractable();
}

AActor* AFVPlayerCharacter::GetFocusedActor() const
{
	if (const UFVInteractableComponent* Interactable = InteractorComponent->GetTargetInteractable())
	{
		return  Interactable->GetOwner();
	}
	
	return nullptr;
}

