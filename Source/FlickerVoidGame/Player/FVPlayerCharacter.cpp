// Fill out your copyright notice in the Description page of Project Settings.


#include "FVPlayerCharacter.h"

#include "Abilities/FVAbilitySystemComponent.h"
#include "Components/InteractionResponseComponent.h"
#include "Components/InteractorComponent.h"
#include "Interaction/FVInteractionAbilityResponder.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerCharacter)

AFVPlayerCharacter::AFVPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UFVAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	Interactor = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor"));
	InteractionResponse = CreateDefaultSubobject<UInteractionResponseComponent>(TEXT("InteractionResponse"));
	InteractionAbilityResponder = CreateDefaultSubobject<UFVInteractionAbilityResponder>(TEXT("InteractionAbilityResponder"));
}

UAbilitySystemComponent* AFVPlayerCharacter::GetAbilitySystemComponent() const
{
	return GetFVAbilitySystemComponent();
}

