#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"

#include "InteractAbility.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UInteractableComponent;
class UInteractorComponent;

UCLASS(MinimalAPI, Abstract)
class UInteractAbility : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UE_API virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

protected:
	UE_API virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	UE_API virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void ActivateInteraction(UInteractorComponent* Interactor, UInteractableComponent* Interactable);

private:
	UE_API UInteractorComponent* GetInteractorComponentFromActorInfo(const FGameplayAbilityActorInfo* ActorInfo) const;
};

#undef UE_API