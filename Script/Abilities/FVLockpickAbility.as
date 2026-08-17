class UFVLockpickAbility : UFVInteractAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick")
	float Difficulty = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Lockpick")
	UAnimMontage LockpickMontage;

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
		Started.LockedActor = EngagedActor;
		Started.Difficulty = Difficulty;

		UGameplayMessageSubsystem::Get().BroadcastMessage(
			GameplayTags::Interaction_Event_LockpickStarted, Started);
	}

	UFUNCTION()
	void HandleLockpickEnded(FGameplayTag Channel, const FFVInteractionLockpickMessage& Message)
	{
		if (Message.LockedActor != EngagedActor)
		{
			return;
		}

		// TODO: apply the unlock to the target once a lock component exists.
		EndLockpick(!Message.bSucceeded);
	}

	private void EndLockpick(bool bWasCancelled)
	{
		UGameplayMessageSubsystem::Get().UnregisterListener(LockpickEndedHandle);

		if (bWasCancelled && EngagedActor != nullptr)
		{
			FFVInteractionLockpickMessage Aborted;
			Aborted.LockedActor = EngagedActor;
			Aborted.Difficulty = Difficulty;

			UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Interaction_Event_LockpickEnded, Aborted);
		}

		EndAbility();
	}
}
