#pragma once
#include "CoreMinimal.h"
#include "FVCharacterTypes.generated.h"

USTRUCT(BlueprintType)
struct FFVCharacterRuntimeState
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FVector Velocity;

    UPROPERTY(BlueprintReadOnly)
    FVector Acceleration;

    UPROPERTY(BlueprintReadOnly)
    bool bIsWalking;
    
    UPROPERTY(BlueprintReadOnly)
    bool bIsFalling;

    UPROPERTY(BlueprintReadOnly)
    bool bIsCrouching;

    UPROPERTY(BlueprintReadOnly)
    bool bIsSprinting;

    // Add more properties as needed
};