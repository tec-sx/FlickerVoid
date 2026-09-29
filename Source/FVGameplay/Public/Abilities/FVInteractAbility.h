#pragma once

#include "Abilities/FVGameplayAbility.h"
#include "FVInteractAbility.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

class UFVInteractorComponent;
class UFVInteractableComponent;

UCLASS(MinimalAPI, Abstract)
class UFVInteractAbility : public UFVGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API UFVInteractorComponent* GetInteractor() const;

	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API UFVInteractableComponent* GetInteractable() const;
	
private:
	UE_API UFVInteractorComponent* GetInteractor(const FGameplayAbilityActorInfo* ActorInfo) const;
};

#undef UE_API