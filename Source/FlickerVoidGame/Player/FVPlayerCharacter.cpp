// Fill out your copyright notice in the Description page of Project Settings.


#include "FVPlayerCharacter.h"

#include "Abilities/FVAbilitySystemComponent.h"
#include "Interactions/FVInteractionInstigatorComponent.h"
#include "Interactions/FVInteractionOfferComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerCharacter)

AFVPlayerCharacter::AFVPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InteractionInstigator = CreateDefaultSubobject<UFVInteractionInstigatorComponent>(TEXT("InteractionInstigator"));
	InteractionOffers = CreateDefaultSubobject<UFVInteractionOfferComponent>(TEXT("InteractionOffers"));
	AbilitySystemComponent = CreateDefaultSubobject<UFVAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
}

UAbilitySystemComponent* AFVPlayerCharacter::GetAbilitySystemComponent() const
{
	return GetFVAbilitySystemComponent();
}

UFVInteractionInstigatorComponent* AFVPlayerCharacter::GetInteraction() const
{
	return InteractionInstigator;
}

UFVInteractionOfferComponent* AFVPlayerCharacter::GetOffers() const
{
	return InteractionOffers;
}
