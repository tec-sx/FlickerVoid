#pragma once

#include "Abilities/FVGameplayAbility.h"
#include "Interactions/FVInteractionTypes.h"
#include "UI/FVInteractionInfo.h"
#include "FVInteractAbility.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

class UFVInteractionOfferComponent;
class UFVInteractionTargetComponent;

UCLASS(MinimalAPI, Abstract)
class UFVInteractAbility : public UFVGameplayAbility
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "Interaction")
	UE_API UFVInteractionOfferComponent* GetOfferComponent() const;

	UE_API virtual bool CanActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr,
		const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override;

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

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Interaction")
	void ActivateInteraction(UFVInteractionOfferComponent* Offers, AActor* InteractableActor);

private:
	EFVInteractionSlot ResolveSlotFromSpec(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo) const;
};

#undef UE_API
