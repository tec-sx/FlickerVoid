#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "InteractAbility.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

UCLASS(MinimalAPI, Abstract)
class UInteractAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UInteractAbility();
	~UInteractAbility();
};

#undef UE_API