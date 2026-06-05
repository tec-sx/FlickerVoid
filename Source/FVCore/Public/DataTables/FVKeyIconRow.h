#pragma once

#include "CoreMinimal.h"
#include "FVKeyIconRow.generated.h"

USTRUCT(BlueprintType)
struct FFVKeyIconRow
{
    GENERATED_BODY()
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FKey Key;
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UTexture2D* Icon;
};