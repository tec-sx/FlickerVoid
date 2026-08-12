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
		APlayerCharacter Character = Cast<APlayerCharacter>(Gameplay::GetPlayerCharacter(0));

		if (Character == nullptr)
		{
			return EFVInteractionResult::NoInteractable;
		}

		UFVInteractionOfferComponent Offers = Character.InteractionOffers;

		if (Offers == nullptr || !Offers.HasActiveOffer())
		{
			return EFVInteractionResult::NoInteractable;
		}

		// The player tree owns the Interacting state; refuse to stack a second
		// interaction on top of the one it is already committed to.
		if (Offers.IsInteracting())
		{
			return EFVInteractionResult::Blocked;
		}

		UFVInteractionTargetComponent FocusedTarget = Offers.GetActiveOffer().Target;

		FFVResolvedInteraction Resolved = Offers.GetActiveSlot(Slot);

		if (Resolved.Action == nullptr)
		{
			return EFVInteractionResult::ActionNotFound;
		}

		if (!Resolved.Info.bAvailable)
		{
			return EFVInteractionResult::RequirementNotMet;
		}

		if (FocusedTarget == nullptr || FocusedTarget.IsInteractionInProgress())
		{
			return EFVInteractionResult::Blocked;
		}

		FocusedTarget.RunAction(Character, Resolved.Action);
		Offers.NotifyActiveOfferTaken();

		return EFVInteractionResult::Success;
	}
}