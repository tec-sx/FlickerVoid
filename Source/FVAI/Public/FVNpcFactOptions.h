#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FVNpcFactOptions.generated.h"

UCLASS()
class FLICKERVOIDAI_API UFVNpcFactOptions : public UObject
{
GENERATED_BODY()

public:
UFUNCTION()
static TArray<FName> GetAspects();
};