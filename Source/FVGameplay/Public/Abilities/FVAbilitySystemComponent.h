// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Abilities/FVGameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "NativeGameplayTags.h"

#include "FVAbilitySystemComponent.generated.h"

#define UE_API FLICKERVOIDGAMEPLAY_API

class AActor;
class UGameplayAbility;
class UFVAbilityTagRelationshipMap;
class UObject;
struct FFrame;
struct FGameplayAbilityTargetDataHandle;

FLICKERVOIDGAMEPLAY_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(TAG_Gameplay_AbilityInputBlocked)

UCLASS(MinimalAPI)
class UFVAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	UFVAbilitySystemComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	////~UActorComponent interface
	//UE_API virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	////~End of UActorComponent interface

	UE_API virtual void InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor) override;

	typedef TFunctionRef<bool(const UFVGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)> TShouldCancelAbilityFunc;
	UE_API void CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc);

	UE_API void CancelInputActivatedAbilities();

	UE_API void AbilityInputTagPressed(const FGameplayTag& InputTag);
	UE_API void AbilityInputTagReleased(const FGameplayTag& InputTag);

	UE_API void ProcessAbilityInput(float DeltaTime, bool bGamePaused);

	UE_API bool IsActivationGroupBlocked(EFVAbilityActivationGroup Group) const;
	UE_API void AddAbilityToActivationGroup(EFVAbilityActivationGroup Group, UFVGameplayAbility* Ability);
	UE_API void RemoveAbilityFromActivationGroup(EFVAbilityActivationGroup Group, UFVGameplayAbility* Ability);
	UE_API void CancelActivationGroupAbilities(EFVAbilityActivationGroup Group, UFVGameplayAbility* IgnoreAbility);

	/** Gets the ability target data associated with the given ability handle and activation info */
	UE_API void GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle);

	/** Sets the current tag relationship mapping, if null it will clear it out */
	UE_API void SetTagRelationshipMap(UFVAbilityTagRelationshipMap* NewMap);

	/** Looks at ability tags and gathers additional required and blocking tags */
	UE_API void GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const;

	UE_API void TryActivateAbilitiesOnSpawn();

protected:
	UE_API virtual void AbilitySpecInputPressed(FGameplayAbilitySpec& Spec) override;
	UE_API virtual void AbilitySpecInputReleased(FGameplayAbilitySpec& Spec) override;
	
	UE_API virtual void NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability) override;
	UE_API virtual void NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason) override;
	UE_API virtual void NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled) override;
	UE_API virtual void ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags) override;
	UE_API virtual void HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled) override;

protected:

	// If set, this table is used to look up tag relationships for activate and cancel
	UPROPERTY()
	TObjectPtr<UFVAbilityTagRelationshipMap> TagRelationshipMap;

	// Number of abilities running in each activation group.
	int32 ActivationGroupCounts[(uint8)EFVAbilityActivationGroup::MAX];
};

#undef UE_API
