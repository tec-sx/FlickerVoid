#pragma once
#include "CoreMinimal.h"
#include "FVCharacterTypes.generated.h"

USTRUCT(BlueprintType)
struct FLICKERVOIDCHARACTER_API FFVCharacterIntent
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    FVector Direction = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bWantsToWalk = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bWantsToSprint = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bWantsToCrouch = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bWantsToAim = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bWantsToInteract = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWantsToTraverse = false;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    bool bWantsToJump = false;
};

USTRUCT(BlueprintType)
struct FFVCharacterRuntimeState
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    FVector Velocity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    FVector Acceleration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bIsWalking;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bIsFalling;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bIsCrouching;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) 
    bool bIsSprinting;
};