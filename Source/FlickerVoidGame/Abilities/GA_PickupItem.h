// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/FVGameplayAbility.h"
#include "GA_PickupItem.generated.h"

class AFVItemPickupActor;

/**
 * Gameplay Ability for picking up items from the world.
 * Triggered via interaction system when player interacts with AFVItemPickupActor.
 * 
 * Event Data:
 * - Target: The AFVItemPickupActor to pick up
 * - Instigator: The player character
 * - EventTag: Interaction.Action.Pickup
 */
UCLASS()
class FLICKERVOID_API UGA_PickupItem : public UFVGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_PickupItem(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	/** Try to execute the pickup */
	UFUNCTION(BlueprintCallable, Category = "FlickerVoid|Ability")
	bool TryPickupItem(AFVItemPickupActor* PickupActor);

	/** Called when pickup succeeds - override in Blueprint for custom feedback */
	UFUNCTION(BlueprintNativeEvent, Category = "FlickerVoid|Ability")
	void OnPickupSuccess(AFVItemPickupActor* PickupActor);

	/** Called when pickup fails - override in Blueprint for custom feedback */
	UFUNCTION(BlueprintNativeEvent, Category = "FlickerVoid|Ability")
	void OnPickupFailed(AFVItemPickupActor* PickupActor);
};