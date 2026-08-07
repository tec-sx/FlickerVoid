#pragma once

#include "GameFramework/Character.h"
#include "FVCharacterTypes.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "GenericTeamAgentInterface.h"
#include "FVCharacter.generated.h"

class UFlowComponent;
class UFVTagComponent;
class UNavMoverComponent;
class UInputAction;
class UCharacterMoverComponent;
class UFVCharacterMovementComponent;
struct FInputActionValue;
class UFVAbilitySystemComponent;
class USpringArmComponent;

UCLASS(Config = Game)
class FLICKERVOIDCHARACTER_API AFVCharacter : public ACharacter, public IGameplayTagAssetInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AFVCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void BeginPlay() override;
	virtual void Landed(const FHitResult& Hit) override;
	
	virtual FGenericTeamId GetGenericTeamId() const override { return TeamId; }
	
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
	
	UFUNCTION(BlueprintCallable)
	void SetSprinting(const bool bValue) { bIsSprinting = bValue; }
	
	UFUNCTION(BlueprintPure)
	bool IsSprinting() const { return bIsSprinting; }
	
	UFUNCTION(BlueprintCallable)
	void SetWalking(const bool bValue) { bIsWalking = bValue; }
	
	UFUNCTION(BlueprintPure)
	bool IsWalking() const { return bIsWalking; }
	
	UFUNCTION(BlueprintPure)
	bool IsAiming() const { return bIsAiming; }
	
	UFUNCTION(BlueprintCallable)
	void SetTraversing(const bool bValue) { bIsTraversing = bValue; }
	
	UFUNCTION(BlueprintPure)
	bool IsTraversing() const { return bIsTraversing; }
	
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
	void RequestAim(const bool bValue) { bIsAiming = bValue; }
	
	UFUNCTION(BlueprintImplementableEvent)
	bool RequestTraverse();
	
	UFUNCTION(BlueprintPure, Category = "FlowGraph")
	FGameplayTagContainer GetIdentityTags() const;
	
	UFlowComponent* GetFlowComponent() const { return FlowComponent; }
	
	UFUNCTION(BlueprintCallable, Category = "Tags")
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;
	
	UFUNCTION(BlueprintCallable, Category = "Tags")
	void AddGameplayTag(FGameplayTag Tag);

	UFUNCTION(BlueprintCallable, Category = "Tags")
	void RemoveGameplayTag(FGameplayTag Tag);
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFlowComponent> FlowComponent;	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer OwnedTags;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGenericTeamId TeamId = FGenericTeamId(1);
private:
	// Intent Data
	FVector MovementDirection = FVector::ZeroVector;
	EFVGait DesiredGait = EFVGait::Walking;
	EFVStance DesiredStance = EFVStance::Stand;
	
	// Runtime State Data
	FVector Velocity;
	FVector Acceleration;
	bool bIsWalking;
	bool bIsSprinting;
	bool bIsTraversing;
	bool bIsAiming = false;
	bool bLockedOnTarget = false;
	FVector TargetedPosition = FVector::ZeroVector;
	FVector LandingVelocity = FVector::ZeroVector;
	bool bJustLanded = false;
	
	FTimerHandle JustLandedTimerHandle;
	
	EFVGait GetGait() const;
	EFVStance GetStance() const;
	EFVMovementMode GetMovementMode() const;
};

