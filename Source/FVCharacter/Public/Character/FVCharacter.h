// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"
#include "CharacterTrajectoryComponent.h"
#include "GameplayTagContainer.h"
#include "Animation/FVAnimationTypes.h"
#include "Movement/FVCharacterMovementTypes.h"
#include "FVCharacterTypes.h"

#include "FVCharacter.generated.h"

class UNavMoverComponent;
class UInputAction;
class UCharacterMoverComponent;
class UFVCharacterMovementComponent;
struct FInputActionValue;
class UFVAbilitySystemComponent;
class USpringArmComponent;
class UFVInteractionComponent;

UCLASS(Config = Game)
class FLICKERVOIDCHARACTER_API AFVCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFVCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void Landed(const FHitResult& Hit) override;

	//~=============================================================================
	// Character Properties
	//~=============================================================================

	/** Get aim rotation (override for aim assist, target lock, etc.) */
	virtual FRotator GetAimRotation() const;
	
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	FVector GetMovementDirection() const { return MovementDirection; }
	
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	float GetGroundSpeed() const;

	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	UFVCharacterMovementComponent* GetFVCharacterMovement() const;

	UFUNCTION(BlueprintPure)
	FFVCharacterAnimationData GetAnimationData() const;
	
	UFUNCTION(BlueprintCallable)
	void SetSprinting(const bool bValue) { bIsSprinting = bValue; }
	
	UFUNCTION(BlueprintPure)
	bool IsSprinting() const { return bIsSprinting; }
	
	UFUNCTION(BlueprintCallable)
	void SetWalking(const bool bValue) { bIsWalking = bValue; }
	
	UFUNCTION(BlueprintPure)
	bool IsWalking() const { return bIsWalking; }
	
	UFUNCTION(BlueprintCallable)
	void SetTraversing(const bool bValue) { bIsTraversing = bValue; }
	
	UFUNCTION(BlueprintPure)
	bool IsTraversing() const { return bIsTraversing; }
	
	UFUNCTION(BlueprintPure)
	FFVCharacterIntent GetIntent() const;
	
	UFUNCTION(BlueprintPure)
	FFVCharacterRuntimeState GetRuntimeState() const;
	
	UFUNCTION(BlueprintPure)
	bool CanMove() const { return true; /* TODO: Check for movement restrictions, etc. */ }
	
	UFUNCTION(BlueprintPure)
	bool CanSprint() const;
	
	void RequestMove(const FVector& NewDirection) { MovementDirection = NewDirection; }
	void RequestWalk();
	void RequestCrouch();
	void RequestSprint(const bool bValue);
	void RequestJump();
	void RequestAim(const bool bValue) { bWantsToAim = bValue; }
	void RequestInteract(const bool bValue) { bWantsToInteract = bValue; }
	
	UFUNCTION(BlueprintImplementableEvent)
	bool RequestTraverse();

private:
	// Intent Data
	FVector MovementDirection = FVector::ZeroVector;
	EFVGait DesiredGait = EFVGait::Walking;
	EFVStance DesiredStance = EFVStance::Stand;
	bool bWantsToTraverse = false;
	bool bWantsToJump = false;
	bool bWantsToAim = false;
	bool bWantsToInteract = false;
	
	// Runtime State Data
	FVector Velocity;
	FVector Acceleration;
	bool bIsWalking;
	bool bIsSprinting;
	bool bIsTraversing;
	bool bLockedOnTarget = false;
	TObjectPtr<AActor> TargetedActor = nullptr;
	
	FTimerHandle JustLandedTimerHandle;
	FVector LandingVelocity = FVector::ZeroVector;
	bool bJustLanded = false;
};

