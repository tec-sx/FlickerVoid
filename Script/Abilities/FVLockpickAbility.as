class UFVLockpickAbility : UFVInteractAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick")
	float Difficulty = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Lockpick")
	UAnimMontage LockpickMontage;

	default ActivationOwnedTags.AddTag(GameplayTags::Status_Interacting);

	private FGameplayMessageListenerHandle LockpickEndedHandle;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		LockpickEndedHandle = UGameplayMessageSubsystem::Get().RegisterListener(
		GameplayTags::Interaction_Event_LockpickEnded,
			this,
			n"HandleLockpickEnded",
			FFVInteractionLockpickMessage());

		if (LockpickMontage != nullptr)
		{
			UAbilityTask_PlayMontageAndWait MontageTask = AngelscriptAbilityTask::PlayMontageAndWait(this, n"Lockpick", LockpickMontage);
			MontageTask.ReadyForActivation();
		}

		FFVInteractionLockpickMessage Started;
		Started.LockedActor = Interactable.GetOwner();
		Started.Difficulty = GetLockDifficulty();

		UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Interaction_Event_LockpickStarted, Started);
	}

	UFUNCTION()
	void HandleLockpickEnded(FGameplayTag Channel, const FFVInteractionLockpickMessage& Message)
	{
		if (Message.LockedActor != Interactable.GetOwner())
		{
			return;
		}

		if (Message.bSucceeded)
		{
			UFVLockComponent Lock = UFVLockComponent::Get(Interactable.GetOwner());

			if (IsValid(Lock))
			{
				Lock.Unlock(GetAvatarActorFromActorInfo());
			}
		}

		EndLockpick(!Message.bSucceeded);
	}

	private float GetLockDifficulty()
	{
		UFVLockComponent Lock = UFVLockComponent::Get(Interactable.GetOwner());

		if (IsValid(Lock))
		{
			return Lock.GetDifficulty();
		}

		return Difficulty;
	}

	private void EndLockpick(bool bWasCancelled)
	{
		UGameplayMessageSubsystem::Get().UnregisterListener(LockpickEndedHandle);

		if (bWasCancelled && Interactable.GetOwner() != nullptr)
		{
			FFVInteractionLockpickMessage Aborted;
			Aborted.LockedActor = Interactable.GetOwner();
			Aborted.Difficulty = GetLockDifficulty();

			UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Interaction_Event_LockpickEnded, Aborted);
		}

		EndAbility();
	}
}
