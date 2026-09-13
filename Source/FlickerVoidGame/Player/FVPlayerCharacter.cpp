// Fill out your copyright notice in the Description page of Project Settings.


#include "FVPlayerCharacter.h"

#include "Abilities/FVAbilitySystemComponent.h"
#include "Components/FVInteractorResponseComponent.h"
#include "Components/FVInteractorComponent.h"
#include "Interaction/FVInteractorResponseComponent_ActivateAbility.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerCharacter)

AFVPlayerCharacter::AFVPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UFVAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	Interactor = CreateDefaultSubobject<UFVInteractorComponent>(TEXT("Interactor"));
	InteractionResponse = CreateDefaultSubobject<UFVInteractorResponseComponent>(TEXT("InteractionResponse"));
	InteractionAbilityResponder = CreateDefaultSubobject<UFVInteractorResponseComponent_ActivateAbility>(TEXT("InteractionAbilityResponder"));
}

UAbilitySystemComponent* AFVPlayerCharacter::GetAbilitySystemComponent() const
{
	return GetFVAbilitySystemComponent();
}

