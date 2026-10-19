#pragma once

#include "GameFramework/Pawn.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "MoverSimulationTypes.h"
#include "FVCharacter.generated.h"

class UCapsuleComponent;
class UCharacterMoverComponent;
class UFlowComponent;
class UFVLocomotionComponent;
class USkeletalMeshComponent;

/**
 * Pawn shape shared by FlickerVoid characters: capsule, skeletal mesh, Mover's Character Mover Component and the locomotion
 * component. The pawn is Mover's input producer and hands that job to the locomotion component, so the Mover interface
 * stays in C++.
 */
UCLASS(Config = Game)
class FLICKERVOIDCHARACTER_API AFVCharacter : public APawn, public IGameplayTagAssetInterface, public IMoverInputProducerInterface
{
	GENERATED_BODY()

public:
	AFVCharacter(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostInitializeComponents() override;
	virtual FVector GetVelocity() const override;

	virtual void ProduceInput_Implementation(int32 SimTimeMs, FMoverInputCmdContext& InputCmdResult) override;

	UFUNCTION(BlueprintPure, Category = "Character")
	UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComponent; }

	UFUNCTION(BlueprintPure, Category = "Character")
	USkeletalMeshComponent* GetMesh() const { return Mesh; }

	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	UCharacterMoverComponent* GetMover() const { return Mover; }

	UFUNCTION(BlueprintPure, Category = "Character|Movement")
	UFVLocomotionComponent* GetLocomotion() const { return Locomotion; }

	UFUNCTION(BlueprintImplementableEvent, Category = "Character|Movement", meta = (ToolTip = "Tries a traversal action (vault, mantle, hurdle) in front of the character. Returns true when one started."))
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
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USkeletalMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCharacterMoverComponent> Mover;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFVLocomotionComponent> Locomotion;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Components, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UFlowComponent> FlowComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
	FGameplayTagContainer OwnedTags;
};
