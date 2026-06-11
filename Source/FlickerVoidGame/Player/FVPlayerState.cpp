#include "FVPlayerState.h"
#include "FVPlayerController.h"
#include "Abilities/FVAbilitySystemComponent.h"
#include "Abilities/FVAbilitySet.h"
#include "Character/FVPawnData.h"
#include "Components/GameFrameworkComponentManager.h"
#include "Engine/World.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVPlayerState)

const FName AFVPlayerState::NAME_FVAbilityReady("FVAbilitiesReady");

AFVPlayerState::AFVPlayerState(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AbilitySystemComponent = ObjectInitializer.CreateDefaultSubobject<UFVAbilitySystemComponent>(this, TEXT("AbilitySystemComponent"));
}

AFVPlayerController* AFVPlayerState::GetFVPlayerController() const
{
	return Cast<AFVPlayerController>(GetOwner());
}

UAbilitySystemComponent* AFVPlayerState::GetAbilitySystemComponent() const
{
	return GetFVAbilitySystemComponent();
}

UFVInputConfig* AFVPlayerState::GetInputConfig() const
{
	if (PawnData)
	{
		return PawnData->InputConfig;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState [%s] has no PawnData set, cannot return InputConfig."), *GetNameSafe(this));
		return nullptr;
	}
}

void AFVPlayerState::SetPawnData(const UFVPawnData* InPawnData)
{
	check(InPawnData);

	if (GetLocalRole() != ROLE_Authority)
	{
		return;
	}

	if (PawnData)
	{
		UE_LOG(LogTemp, Error, TEXT("Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]."), *GetNameSafe(InPawnData), *GetNameSafe(this), *GetNameSafe(PawnData));
		return;
	}

	PawnData = InPawnData;

	for (const UFVAbilitySet* AbilitySet : PawnData->AbilitySets)
	{
		if (AbilitySet)
		{
			AbilitySet->PassToAbilitySystem(AbilitySystemComponent, nullptr);
		}
	}

	UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent(this, NAME_FVAbilityReady);
}
