class UFVLockpickAbility : UFVGameplayAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Lockpick")
	float Difficulty = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Lockpick")
	UAnimMontage LockpickMontage;

	private FGameplayMessageListenerHandle LockpickEndedHandle;
	private AActor LockedActor;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		LockedActor = ResolveEngagedActor(ActorInfo);

		if (LockedActor == nullptr)
		{
			EndLockpick(true);
			return;
		}

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
		Started.LockedActor = LockedActor;
		Started.Difficulty = Difficulty;

		UGameplayMessageSubsystem::Get().BroadcastMessage(
			GameplayTags::Interaction_Event_LockpickStarted, Started);
	}

	UFUNCTION()
	void HandleLockpickEnded(FGameplayTag Channel, const FFVInteractionLockpickMessage& Message)
	{
		if (Message.LockedActor != LockedActor)
		{
			return;
		}

		// TODO: apply the unlock to the target once a lock component exists.
		EndLockpick(!Message.bSucceeded);
	}

	private void EndLockpick(bool bWasCancelled)
	{
		UGameplayMessageSubsystem::Get().UnregisterListener(LockpickEndedHandle);

		if (bWasCancelled && LockedActor != nullptr)
		{
			FFVInteractionLockpickMessage Aborted;
			Aborted.LockedActor = LockedActor;
			Aborted.Difficulty = Difficulty;

			UGameplayMessageSubsystem::Get().BroadcastMessage(GameplayTags::Interaction_Event_LockpickEnded, Aborted);
		}

		LockedActor = nullptr;
		EndAbility();
	}

	private AActor ResolveEngagedActor(FGameplayAbilityActorInfo InActorInfo)
	{
		AActor Instigator = InActorInfo.AvatarActor;

		if (Instigator == nullptr)
		{
			return nullptr;
		}

		UFVInteractionOfferComponent Offers =
			Cast<UFVInteractionOfferComponent>(Instigator.GetComponentByClass(UFVInteractionOfferComponent));

		if (Offers == nullptr)
		{
			return nullptr;
		}

		UFVInteractionTargetComponent Target = Offers.GetEngagedTarget();

		return Target != nullptr ? Target.GetOwner() : nullptr;
	}
}
