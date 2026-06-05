// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "GameFramework/Character.h"
#include "CharacterTrajectoryComponent.h"
#include "GameplayTagContainer.h"
#include "Animation/FVAnimationTypes.h"
#include "Movement/FVMovementTypes.h"
#include "FVCharacterTypes.h"

#include "FVCharacter.generated.h"

class UNavMoverComponent;
class UInputAction;
class UCharacterMoverComponent;
class UFVCharacterMovementComponent;
struct FInputActionValue;
class UFVAbilitySystemComponent;
class USpringArmComponent;
class UFVCharacterStateManager;
class UFVCharacterArchetypeData;

UCLASS(Config = Game)
class FLICKERVOIDCHARACTER_API AFVCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFVCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~APawn interface
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	//~End of APawn interface
	
	//~ACharacter overrides
	virtual void Landed(const FHitResult& Hit) override;
	//~End of ACharacter overrides

	//~=============================================================================
	// Character Properties
	//~=============================================================================

	/** Get aim rotation (override for aim assist, target lock, etc.) */
	virtual FRotator GetAimRotation() const;
	
	
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	FVector GetMovementDirection() const { return MovementDirection; }
	
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	float GetGroundSpeed() const;
	
	/** Get the FV character movement component */
	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	UFVCharacterMovementComponent* GetFVCharacterMovement() const;
	
	/** Get animation data snapshot (physics + tags) */
	UFUNCTION(BlueprintPure, Category = "Character|Animation")
	FFVCharacterAnimationData GetAnimationData() const;
	
	void RequestMove(const FVector& NewDirection) { MovementDirection = NewDirection; }
	void RequestWalk() { bWantsToWalk = !State.bIsWalking; }
	void RequestCrouch() { bWantsToCrouch = !State.bIsCrouching; }
	void RequestSprint(bool bValue) { bWantsToSprint = bValue; }
	void RequestJump() { bWantsToJump = true; }
	void RequestTraverse() { bWantsToTraverse = true; }
	void RequestAim(bool bValue) { bWantsToAim = bValue; }
	void RequestInteract(bool bValue) { bWantsToInteract = bValue; }
	//~=============================================================================
	// Archetype Configuration
	//~=============================================================================

	/** Character archetype data (set in editor or on spawn) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character|Archetype")
	TObjectPtr<UFVCharacterArchetypeData> ArchetypeData;

	/** Apply archetype configuration (called automatically in BeginPlay) */
	UFUNCTION(BlueprintCallable, Category = "Character|Archetype")
	void ApplyArchetype(UFVCharacterArchetypeData* NewArchetype);

protected:
	/** State management object (THE single source of truth for character state) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|State")
	TObjectPtr<UFVCharacterStateManager> StateManager;

private:
	FFVCharacterRuntimeState State;

	// Intent Data
	FVector MovementDirection = FVector::ZeroVector;
	bool bWantsToWalk = false;
	bool bWantsToSprint = false;
	bool bWantsToCrouch = false;
	bool bWantsToTraverse = false;
	bool bWantsToJump = false;
	bool bWantsToAim = false;
	bool bWantsToInteract = false;

	bool bLockedOnTarget = false;
	TObjectPtr<AActor> TargetedActor = nullptr;
	
	FTimerHandle JustLandedTimerHandle;
	FVector LandingVelocity = FVector::ZeroVector;
	bool bJustLanded = false;
};

