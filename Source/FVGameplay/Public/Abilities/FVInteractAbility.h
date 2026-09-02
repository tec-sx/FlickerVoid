#pragma once

#include "Abilities/FVGameplayAbility.h"
#include "FVInteractAbility.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

class UInteractorComponent;
class UInteractableComponent;

UCLASS(MinimalAPI, Abstract)
class UFVInteractAbility : public UFVGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API UInteractorComponent* GetInteractor() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API UInteractableComponent* GetInteractable() const;
	
private:
	UE_API UInteractorComponent* GetInteractor(const FGameplayAbilityActorInfo* ActorInfo) const;
};

#undef UE_API