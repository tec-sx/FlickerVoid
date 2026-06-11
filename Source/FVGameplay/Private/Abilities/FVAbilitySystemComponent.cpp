// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/FVAbilitySystemComponent.h"
#include "Abilities/FVAbilityTagRelationshipMap.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVAbilitySystemComponent)

UE_DEFINE_GAMEPLAY_TAG(TAG_Gameplay_AbilityInputBlocked, "Gameplay.AbilityInputBlocked");

UFVAbilitySystemComponent::UFVAbilitySystemComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	FMemory::Memset(ActivationGroupCounts, 0, sizeof(ActivationGroupCounts));
}

//void UFVAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
//{
//	return void();
//}

void UFVAbilitySystemComponent::InitAbilityActorInfo(AActor* InOwnerActor, AActor* InAvatarActor)
{
	FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
	check(ActorInfo);
	check(InOwnerActor);

	Super::InitAbilityActorInfo(InOwnerActor, InAvatarActor);

	TryActivateAbilitiesOnSpawn();
}

void UFVAbilitySystemComponent::TryActivateAbilitiesOnSpawn()
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (const UFVGameplayAbility* AbilityCDO = Cast<UFVGameplayAbility>(AbilitySpec.Ability))
		{
			AbilityCDO->TryActivateAbilityOnSpawn(AbilityActorInfo.Get(), AbilitySpec);
		}
	}
}

void UFVAbilitySystemComponent::AbilitySpecInputPressed(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputPressed(Spec);

	if (Spec.IsActive())
	{
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, Spec.Handle, FPredictionKey());
	}
}

void UFVAbilitySystemComponent::AbilitySpecInputReleased(FGameplayAbilitySpec& Spec)
{
	Super::AbilitySpecInputReleased(Spec);

	if (Spec.IsActive())
	{
		InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, Spec.Handle, FPredictionKey());
	}
}

void UFVAbilitySystemComponent::CancelAbilitiesByFunc(TShouldCancelAbilityFunc ShouldCancelFunc)
{
	ABILITYLIST_SCOPE_LOCK();
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (!AbilitySpec.IsActive())
		{
			continue;
		}

		UFVGameplayAbility* LyraAbilityCDO = Cast<UFVGameplayAbility>(AbilitySpec.Ability);
		if (!LyraAbilityCDO)
		{
			UE_LOG(LogTemp, Error, TEXT("CancelAbilitiesByFunc: Non-FVoiidGameplayAbility %s was Granted to ASC. Skipping."), *AbilitySpec.Ability.GetName());
			continue;
		}

		PRAGMA_DISABLE_DEPRECATION_WARNINGS
			ensureMsgf(AbilitySpec.Ability->GetInstancingPolicy() != EGameplayAbilityInstancingPolicy::NonInstanced, TEXT("CancelAbilitiesByFunc: All Abilities should be Instanced (NonInstanced is being deprecated due to usability issues)."));
		PRAGMA_ENABLE_DEPRECATION_WARNINGS

			// Cancel all the spawned instances.
			TArray<UGameplayAbility*> Instances = AbilitySpec.GetAbilityInstances();
		for (UGameplayAbility* AbilityInstance : Instances)
		{
			UFVGameplayAbility* FVAbilityInstance = CastChecked<UFVGameplayAbility>(AbilityInstance);

			if (ShouldCancelFunc(FVAbilityInstance, AbilitySpec.Handle))
			{
				if (FVAbilityInstance->CanBeCanceled())
				{
					FVAbilityInstance->CancelAbility(AbilitySpec.Handle, AbilityActorInfo.Get(), FVAbilityInstance->GetCurrentActivationInfo(), false);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("CancelAbilitiesByFunc: Can't cancel ability [%s] because CanBeCanceled is false."), *FVAbilityInstance->GetName());
				}
			}
		}
	}
}

void UFVAbilitySystemComponent::CancelInputActivatedAbilities()
{
	auto ShouldCancelFunc = [this](const UFVGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
		{
			const EFVAbilityActivationPolicy ActivationPolicy = Ability->GetActivationPolicy();
			return ((ActivationPolicy == EFVAbilityActivationPolicy::OnInputTriggered) || (ActivationPolicy == EFVAbilityActivationPolicy::WhileInputActive));
		};

	CancelAbilitiesByFunc(ShouldCancelFunc);
}

void UFVAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// Find and activate abilities with this input tag
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			const UFVGameplayAbility* AbilityCDO = CastChecked<UFVGameplayAbility>(AbilitySpec.Ability);
			
			if (AbilitySpec.IsActive())
			{
				// Ability is already active, send input pressed event to it
				AbilitySpecInputPressed(*const_cast<FGameplayAbilitySpec*>(&AbilitySpec));
			}
			else
			{
				// Try to activate based on activation policy
				const EFVAbilityActivationPolicy ActivationPolicy = AbilityCDO->GetActivationPolicy();
				
				if (ActivationPolicy == EFVAbilityActivationPolicy::OnInputTriggered ||
					ActivationPolicy == EFVAbilityActivationPolicy::WhileInputActive)
				{
					// Activate the ability immediately
					TryActivateAbility(AbilitySpec.Handle);
				}
			}
		}
	}
}

void UFVAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid())
	{
		return;
	}

	// Find abilities with this input tag and handle release
	for (const FGameplayAbilitySpec& AbilitySpec : ActivatableAbilities.Items)
	{
		if (AbilitySpec.Ability && AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			if (AbilitySpec.IsActive())
			{
				const UFVGameplayAbility* AbilityCDO = CastChecked<UFVGameplayAbility>(AbilitySpec.Ability);
				
				// Send input released event to active ability
				AbilitySpecInputReleased(*const_cast<FGameplayAbilitySpec*>(&AbilitySpec));
				
				// Auto-cancel WhileInputActive abilities when input is released
				if (AbilityCDO->GetActivationPolicy() == EFVAbilityActivationPolicy::WhileInputActive)
				{
					CancelAbilityHandle(AbilitySpec.Handle);
				}
			}
		}
	}
}

void UFVAbilitySystemComponent::ProcessAbilityInput(float DeltaTime, bool bGamePaused)
{
	// Simplified - now only used to block input if needed
	// The actual activation happens immediately in AbilityInputTagPressed
	
	if (HasMatchingGameplayTag(TAG_Gameplay_AbilityInputBlocked))
	{
		// Cancel all input-activated abilities when input is blocked
		CancelInputActivatedAbilities();
	}
}

bool UFVAbilitySystemComponent::IsActivationGroupBlocked(EFVAbilityActivationGroup Group) const
{
	bool bBlocked = false;

	switch (Group)
	{
	case EFVAbilityActivationGroup::Independent:
		// Independent abilities are never blocked.
		bBlocked = false;
		break;

	case EFVAbilityActivationGroup::Exclusive_Replace:
	case EFVAbilityActivationGroup::Exclusive_Block:
		// Exclusive abilities can activate if nothing is blocking.
		bBlocked = (ActivationGroupCounts[(uint8)EFVAbilityActivationGroup::Exclusive_Block] > 0);
		break;

	default:
		checkf(false, TEXT("IsActivationGroupBlocked: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	return bBlocked;
}

void UFVAbilitySystemComponent::AddAbilityToActivationGroup(EFVAbilityActivationGroup Group, UFVGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] < INT32_MAX);

	ActivationGroupCounts[(uint8)Group]++;

	const bool bReplicateCancelAbility = false;

	switch (Group)
	{
	case EFVAbilityActivationGroup::Independent:
		// Independent abilities do not cancel any other abilities.
		break;

	case EFVAbilityActivationGroup::Exclusive_Replace:
	case EFVAbilityActivationGroup::Exclusive_Block:
		CancelActivationGroupAbilities(EFVAbilityActivationGroup::Exclusive_Replace, Ability);
		break;

	default:
		checkf(false, TEXT("AddAbilityToActivationGroup: Invalid ActivationGroup [%d]\n"), (uint8)Group);
		break;
	}

	const int32 ExclusiveCount = ActivationGroupCounts[(uint8)EFVAbilityActivationGroup::Exclusive_Replace] + ActivationGroupCounts[(uint8)EFVAbilityActivationGroup::Exclusive_Block];
	if (!ensure(ExclusiveCount <= 1))
	{
		UE_LOG(LogTemp, Error, TEXT("AddAbilityToActivationGroup: Multiple exclusive abilities are running."));
	}
}

void UFVAbilitySystemComponent::RemoveAbilityFromActivationGroup(EFVAbilityActivationGroup Group, UFVGameplayAbility* Ability)
{
	check(Ability);
	check(ActivationGroupCounts[(uint8)Group] > 0);

	ActivationGroupCounts[(uint8)Group]--;
}

void UFVAbilitySystemComponent::CancelActivationGroupAbilities(EFVAbilityActivationGroup Group, UFVGameplayAbility* IgnoreAbility)
{
	auto ShouldCancelFunc = [this, Group, IgnoreAbility](const UFVGameplayAbility* Ability, FGameplayAbilitySpecHandle Handle)
		{
			return ((Ability->GetActivationGroup() == Group) && (Ability != IgnoreAbility));
		};

	CancelAbilitiesByFunc(ShouldCancelFunc);
}

void UFVAbilitySystemComponent::GetAbilityTargetData(const FGameplayAbilitySpecHandle AbilityHandle, FGameplayAbilityActivationInfo ActivationInfo, FGameplayAbilityTargetDataHandle& OutTargetDataHandle)
{
	TSharedPtr<FAbilityReplicatedDataCache> ReplicatedData = AbilityTargetDataMap.Find(FGameplayAbilitySpecHandleAndPredictionKey(AbilityHandle, ActivationInfo.GetActivationPredictionKey()));
	if (ReplicatedData.IsValid())
	{
		OutTargetDataHandle = ReplicatedData->TargetData;
	}
}

void UFVAbilitySystemComponent::SetTagRelationshipMap(UFVAbilityTagRelationshipMap* NewMap)
{
	TagRelationshipMap = NewMap;
}

void UFVAbilitySystemComponent::GetAdditionalActivationTagRequirements(const FGameplayTagContainer& AbilityTags, FGameplayTagContainer& OutActivationRequired, FGameplayTagContainer& OutActivationBlocked) const
{
	if (TagRelationshipMap)
	{
		TagRelationshipMap->GetRequiredAndBlockedActivationTags(AbilityTags, &OutActivationRequired, &OutActivationBlocked);
	}
}


void UFVAbilitySystemComponent::NotifyAbilityActivated(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability)
{
	Super::NotifyAbilityActivated(Handle, Ability);

	if (UFVGameplayAbility* FVAbility = Cast<UFVGameplayAbility>(Ability))
	{
		AddAbilityToActivationGroup(FVAbility->GetActivationGroup(), FVAbility);
	}
}

void UFVAbilitySystemComponent::NotifyAbilityFailed(const FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, const FGameplayTagContainer& FailureReason)
{
	Super::NotifyAbilityFailed(Handle, Ability, FailureReason);

	//UE_LOG(LogLyraAbilitySystem, Warning, TEXT("Ability %s failed to activate (tags: %s)"), *GetPathNameSafe(Ability), *FailureReason.ToString());

	if (const UFVGameplayAbility* FVAbility = Cast<const UFVGameplayAbility>(Ability))
	{
		FVAbility->OnAbilityFailedToActivate(FailureReason);
	}
}

void UFVAbilitySystemComponent::NotifyAbilityEnded(FGameplayAbilitySpecHandle Handle, UGameplayAbility* Ability, bool bWasCancelled)
{
	Super::NotifyAbilityEnded(Handle, Ability, bWasCancelled);

	if (UFVGameplayAbility* FVAbility = Cast<UFVGameplayAbility>(Ability))
	{
		RemoveAbilityFromActivationGroup(FVAbility->GetActivationGroup(), FVAbility);
	}
}

void UFVAbilitySystemComponent::ApplyAbilityBlockAndCancelTags(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bEnableBlockTags, const FGameplayTagContainer& BlockTags, bool bExecuteCancelTags, const FGameplayTagContainer& CancelTags)
{
	FGameplayTagContainer ModifiedBlockTags = BlockTags;
	FGameplayTagContainer ModifiedCancelTags = CancelTags;

	if (TagRelationshipMap)
	{
		// Use the mapping to expand the ability tags into block and cancel tag
		TagRelationshipMap->GetAbilityTagsToBlockAndCancel(AbilityTags, &ModifiedBlockTags, &ModifiedCancelTags);
	}

	Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags, ModifiedBlockTags, bExecuteCancelTags, ModifiedCancelTags);

	//@TODO: Apply any special logic like blocking input or movement
}

void UFVAbilitySystemComponent::HandleChangeAbilityCanBeCanceled(const FGameplayTagContainer& AbilityTags, UGameplayAbility* RequestingAbility, bool bCanBeCanceled)
{
	Super::HandleChangeAbilityCanBeCanceled(AbilityTags, RequestingAbility, bCanBeCanceled);

	//@TODO: Apply any special logic like blocking input or movement
}
