// Fill out your copyright notice in the Description page of Project Settings.

#include "GA_PickupItem.h"
#include "FVItemsTags.h"
#include "Items/FVItemPickupActor.h"
#include "AbilitySystemComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GA_PickupItem)

UGA_PickupItem::UGA_PickupItem(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// This ability is triggered by interaction events, not input
	ActivationPolicy = EFVAbilityActivationPolicy::OnInputTriggered;
	ActivationGroup = EFVAbilityActivationGroup::Independent;

	// Set up ability tags
	AbilityTags.AddTag(FVItemsTags::Interaction_Action_Pickup);

	// Ability triggers on the Pickup event tag
	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FVItemsTags::Interaction_Action_Pickup;
	TriggerData.TriggerSource = EGameplayAbilityTriggerSource::GameplayEvent;
	AbilityTriggers.Add(TriggerData);
}

void UGA_PickupItem::ActivateAbility(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (!TriggerEventData)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Get the pickup actor from the event target
	AActor* TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
	AFVItemPickupActor* PickupActor = Cast<AFVItemPickupActor>(TargetActor);

	if (!PickupActor)
	{
		OnPickupFailed(nullptr);
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Attempt pickup
	if (TryPickupItem(PickupActor))
	{
		OnPickupSuccess(PickupActor);
	}
	else
	{
		OnPickupFailed(PickupActor);
	}

	// End ability immediately (instant action)
	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

bool UGA_PickupItem::TryPickupItem(AFVItemPickupActor* PickupActor)
{
	if (!PickupActor)
	{
		return false;
	}

	//return PickupActor->ExecutePickup(Inventory);

	return false;
}

void UGA_PickupItem::OnPickupSuccess_Implementation(AFVItemPickupActor* PickupActor)
{
	// Default implementation - can be overridden in Blueprint
	// Could play sound, show UI notification, etc.
}

void UGA_PickupItem::OnPickupFailed_Implementation(AFVItemPickupActor* PickupActor)
{
	// Default implementation - can be overridden in Blueprint
	// Could play error sound, show "inventory full" message, etc.
}