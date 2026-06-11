#pragma once
#include "CoreMinimal.h"
#include "FVCharacterTypes.generated.h"

UENUM(BlueprintType)
enum class EFVGait: uint8
{
	Walking,
	Running,
	Sprinting
};

UENUM(BlueprintType)
enum class EFVStance : uint8
{
	Stand,
	Crouch
};

UENUM(BlueprintType)
enum class EFVMovementMode : uint8
{
	None,
	OnGround,
	InAir,
	Sliding,
	Traversing,
	Swimming
};

USTRUCT(BlueprintType)
struct FFVCharacterRuntimeState
{
    GENERATED_BODY()
    
	//~=============================================================================
	// Transform & Physics Data
	//~=============================================================================
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Transform")
    FTransform ActorTransform;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Physics")
    FVector Velocity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Physics")
    FVector InputAcceleration;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Physics")
    float GroundSpeed;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Physics")
    float CurrentMaxAcceleration;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Physics")
    float CurrentMaxDeceleration;
	
	//~=============================================================================
	// State
	//~=============================================================================
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Movement") 
	EFVGait Gait;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Movement") 
	EFVStance Stance;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Movement") 
	EFVMovementMode MovementMode;
	
    //~=============================================================================
    // Rotation Intent
    //~=============================================================================
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Rotation")
    FRotator OrientationIntent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Rotation")
    FRotator AimingDirection;

    //~=============================================================================
    // Landing Data
    //~=============================================================================

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Landing")
    FVector LandingVelocity;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Landing")
	bool bJustLanded;
	
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FVCharacter|Ground")
    FVector GroundNormal;
};