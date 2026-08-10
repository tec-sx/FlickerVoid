#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTags.h"

#include "FVFactSaveGame.generated.h"

UCLASS()
class FLICKERVOIDCORE_API UFVFactSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY()
	TMap<FGameplayTag, int32> Facts;
};
