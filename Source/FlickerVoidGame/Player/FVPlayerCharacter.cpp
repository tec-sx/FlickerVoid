// Fill out your copyright notice in the Description page of Project Settings.


#include "FVPlayerCharacter.h"

#include "Abilities/FVAbilitySystemComponent.h"
#include "Components/InteractorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerCharacter)

AFVPlayerCharacter::AFVPlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = CreateDefaultSubobject<UFVAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	Interactor = CreateDefaultSubobject<UInteractorComponent>(TEXT("Interactor"));
}

UAbilitySystemComponent* AFVPlayerCharacter::GetAbilitySystemComponent() const
{
	return GetFVAbilitySystemComponent();
}

void AFVPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	Interactor->ResolveAction.BindWeakLambda(this, [this](const FGameplayTag& ActionTag)
	{
		FInteractionAvailabilityResult Result;

		bool bAvailable = false;
		FGameplayTag FailureTag;
		FGameplayTag InputTag;

		if (!AbilitySystemComponent->QueryAbilityAvailabilityByTag(ActionTag, bAvailable, FailureTag, InputTag))
		{
			Result.Availability = EInteractionAvailability::RequirementNotMet;
			return Result;
		}

		Result.Availability = bAvailable ? EInteractionAvailability::Available : EInteractionAvailability::Blocked;
		Result.FailureTag = FailureTag;
		return Result;
	});

	Interactor->ExecuteAction.BindWeakLambda(this, [this](const FGameplayTag& ActionTag, const FInteractionContext&)
	{
		return AbilitySystemComponent->ExecuteInteractionAction(ActionTag);
	});
}
