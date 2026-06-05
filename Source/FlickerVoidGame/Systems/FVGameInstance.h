#pragma once
#include "Engine/GameInstance.h"

#include "FVGameInstance.generated.h"

#define UE_API

UCLASS(MinimalAPI, Config = Game)
class UFVGameInstance : public UGameInstance
{
	GENERATED_BODY()

protected:

	UE_API virtual void Init() override;
	UE_API virtual void Shutdown() override;
};

#undef UE_API