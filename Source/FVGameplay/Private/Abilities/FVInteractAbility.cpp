#include "Abilities/FVInteractAbility.h"

#include "AbilitySystemComponent.h"
#include "FVCoreTags.h"
#include "GameFramework/Actor.h"
#include "Interactions/FVInteractionOfferComponent.h"
#include "Interactions/FVInteractionTargetComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVInteractAbility)

UFVInteractionOfferComponent* UFVInteractAbility::GetOfferComponent() const
{
	const AActor* Avatar = GetAvatarActorFromActorInfo();

	return Avatar ? Avatar->FindComponentByClass<UFVInteractionOfferComponent>() : nullptr;
}

EFVInteractionSlot UFVInteractAbility::ResolveSlotFromSpec(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	const UAbilitySystemComponent* ASC = ActorInfo ? ActorInfo->AbilitySystemComponent.Get() : nullptr;
	const FGameplayAbilitySpec* Spec = ASC ? ASC->FindAbilitySpecFromHandle(Handle) : nullptr;

	if (!Spec)
	{
		return EFVInteractionSlot::MAX;
	}

	for (const FGameplayTag& Tag : Spec->GetDynamicSpecSourceTags())
	{
		const EFVInteractionSlot Slot = FVCoreTags::InputTagToSlot(Tag);

		if (Slot != EFVInteractionSlot::MAX)
		{
			return Slot;
		}
	}

	return EFVInteractionSlot::MAX;
}

bool UFVInteractAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if (!Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		return false;
	}

	const EFVInteractionSlot Slot = ResolveSlotFromSpec(Handle, ActorInfo);

	if (Slot == EFVInteractionSlot::MAX)
	{
		return false;
	}

	const UFVInteractionOfferComponent* Offers = GetOfferComponent();

	if (!Offers)
	{
		return false;
	}

	const FFVResolvedInteraction Resolved = Offers->GetActiveSlot(Slot);

	return Resolved.Info.bAvailable && GetAssetTags().HasTagExact(Resolved.Config.AbilityTag);
}

void UFVInteractAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	UFVInteractionOfferComponent* Offers = GetOfferComponent();
	const EFVInteractionSlot Slot = ResolveSlotFromSpec(Handle, ActorInfo);

	if (!Offers || Offers->BeginEngagement(Slot) != EFVInteractionResult::Success)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ActivateInteraction(Offers, Offers->GetEngagedActor());
}

void UFVInteractAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UFVInteractionOfferComponent* Offers = GetOfferComponent())
	{
		Offers->EndEngagement();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
