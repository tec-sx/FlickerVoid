class UFVInteractAbility : UFVGameplayAbility
{
	UPROPERTY(EditDefaultsOnly, Meta = (Categories = "Interaction.Action"))
	FGameplayTag ActionTag;

	UFUNCTION(BlueprintOverride)
	bool CanActivateAbility(
		FGameplayAbilityActorInfo InActorInfo,
		FGameplayAbilitySpecHandle Handle,
		FGameplayTagContainer& RelevantTags) const
	{
		return ActionTag.IsValid();
	}

	UFUNCTION()
	EFVInteractionResult ExecuteInteraction()
	{
		APlayerCharacter Character = Cast<APlayerCharacter>(Gameplay::GetPlayerCharacter(0));

		if (Character == nullptr)
		{
			return EFVInteractionResult::NoInteractable;
		}

		if (Character.InteractionInstigator.HasFocus())
		{
			UFVInteractionTargetComponent FocusedTarget = Character.InteractionInstigator.FocusedTarget;
			FGameplayTagContainer CharacterTags;
			Character.GetOwnedGameplayTags(CharacterTags);
			TArray<UFVInteractionAction> AvailableActions = FocusedTarget.GetAvailableActions();
			UFVInteractionAction SelectedAction = nullptr;

			for (int i = 0; i < AvailableActions.Num(); i++)
			{
				UFVInteractionAction CurrentAction = AvailableActions[i];

				if (CurrentAction.ActionTag.MatchesTagExact(ActionTag))
				{
					SelectedAction = CurrentAction;
					break;
				}
			}

			if (SelectedAction == nullptr)
			{
				return EFVInteractionResult::ActionNotFound;
			}

			if (SelectedAction.CheckRequirements(CharacterTags))
			{
				UFVInteractionInstigatorComponent Instigator = Character.InteractionInstigator;

				if (Instigator.FocusedTarget.IsInteractionInProgress())
				{
					return EFVInteractionResult::Blocked;
				}
				
				Instigator.FocusedTarget.RunAction(Character, SelectedAction);
				Instigator.OnFocusChanged.Broadcast(Instigator.FocusedTarget);

				return EFVInteractionResult::Success;
			}
			else
			{
				return EFVInteractionResult::RequirementNotMet;
			}
		}
		else
		{
			return EFVInteractionResult::NoInteractable;
		}
	}
}