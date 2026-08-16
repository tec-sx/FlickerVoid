class UFVInteractAbility : UFVGameplayAbility
{
	// Which input slot this ability drives. The focused target decides what
	// actually occupies the slot; the ability never matches on ActionTag.
	UPROPERTY(EditDefaultsOnly)
	EFVInteractionSlot Slot = EFVInteractionSlot::Primary;

	UFUNCTION(BlueprintOverride)
	bool CanActivateAbility(
		FGameplayAbilityActorInfo InActorInfo,
		FGameplayAbilitySpecHandle Handle,
		FGameplayTagContainer& RelevantTags) const
	{
		return true;
	}

	UFUNCTION()
	EFVInteractionResult ExecuteInteraction()
	{
		AFVPlayerCharacter Character = Cast<AFVPlayerCharacter>(Gameplay::GetPlayerCharacter(0));

		if (Character == nullptr)
		{
			return EFVInteractionResult::NoInteractable;
		}

		if (Character.Offers == nullptr || !Character.Offers.HasActiveOffer())
		{
			return EFVInteractionResult::NoInteractable;
		}

		// The player tree owns the Interacting state; refuse to stack a second
		// interaction on top of the one it is already committed to.
		if (Character.Offers.IsInteracting())
		{
			return EFVInteractionResult::Blocked;
		}

		UFVInteractionTargetComponent FocusedTarget = Character.Offers.GetActiveOffer().Target;

		FFVResolvedInteraction Resolved = Character.Offers.GetActiveSlot(Slot);

		if (Resolved.Action == nullptr)
		{
			return EFVInteractionResult::ActionNotFound;
		}

		if (!Resolved.Info.bAvailable)
		{
			return EFVInteractionResult::RequirementNotMet;
		}

		if (FocusedTarget == nullptr)
		{
			return EFVInteractionResult::Blocked;
		}

		UFVAbilitySystemComponent ASC = GetAbilitySystemComponent();

		if (ASC == nullptr)
		{
			return EFVInteractionResult::Blocked;
		}

		FGameplayAbilitySpecHandle Dispatched = ASC.TryActivateAbilityByAssetTagAndGetHandle(Resolved.Action.AbilityTag);

		Character.Offers.NotifyActiveOfferTaken();
		Character.Offers.BeginEngagement(FocusedTarget, Dispatched);

		return EFVInteractionResult::Success;
	}
}
