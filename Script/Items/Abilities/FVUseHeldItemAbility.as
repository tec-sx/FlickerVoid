// Uses what the character holds, the right hand first, then the left (e.g. switches a flashlight). Also bindable to an input tag.
class UFVUseHeldItemAbility : UFVInteractAbility
{
	default TriggerEventTag = GameplayTags::Event_Item_UseHeld;
	default ActivationBlockedTags.AddTag(GameplayTags::Status_HandsBusy);

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		UseHeld();
	}

	UFUNCTION(BlueprintOverride)
	void ActivateAbilityFromEvent(FGameplayEventData EventData)
	{
		UseHeld();
	}

	private void UseHeld()
	{
		UFVHeldItemComponent Held = UFVHeldItemComponent::Get(GetAvatarActorFromActorInfo());

		if (IsValid(Held) && !Held.UseHeld(true))
		{
			Held.UseHeld(false);
		}

		EndAbility();
	}
}
