// Base of item abilities that play a montage. The item action runs on the montage's Apply notify (Event.Item.Notify.Apply), when the montage completes without one, or at once without a montage (or one that can't play); Attach and Release notifies bracket a prop in the hand. An interrupted montage ends the ability without the action.
UCLASS(Abstract)
class UFVItemMontageAbility : UFVInteractAbility
{
	private bool bApplied = false;
	private bool bHandsPutAway = false;
	private bool bWarping = false;

	// Puts away held items while the montage runs when PutsAwayHeldItems, then calls the item hooks from its notifies and ends the ability.
	void PlayItemMontage(UAnimMontage Montage)
	{
		bApplied = false;
		AActor Avatar = GetAvatarActorFromActorInfo();

		if (PutsAwayHeldItems())
		{
			FVItemAnim::SetHandsBusy(Avatar, true);
			bHandsPutAway = true;
		}

		if (Montage == nullptr)
		{
			RunWithoutMontage();
			return;
		}

		ListenForNotify(GameplayTags::Event_Item_Notify_Attach, n"HandleAttachNotify");
		ListenForNotify(GameplayTags::Event_Item_Notify_Apply, n"HandleApplyNotify");
		ListenForNotify(GameplayTags::Event_Item_Notify_Release, n"HandleReleaseNotify");

		UAbilityTask_PlayMontageAndWait MontageTask = FVItemAnim::PlayMontage(this, Montage);

		MontageTask.OnCompleted.AddUFunction(this, n"HandleMontageFinished");
		MontageTask.OnBlendOut.AddUFunction(this, n"HandleMontageFinished");
		MontageTask.OnInterrupted.AddUFunction(this, n"HandleMontageInterrupted");
		MontageTask.OnCancelled.AddUFunction(this, n"HandleMontageCancelled");
		MontageTask.ReadyForActivation();
	}

	// Motion warping target of the montage's warp window named Item; cleared when the ability ends.
	void SetItemWarpTarget(USceneComponent Target)
	{
		FVItemAnim::SetWarpTarget(GetAvatarActorFromActorInfo(), n"Item", Target);
		bWarping = Target != nullptr;
	}

	bool PutsAwayHeldItems()
	{
		return true;
	}

	void OnItemAttach()
	{
	}

	void OnItemApply()
	{
	}

	void OnItemRelease()
	{
	}

	// Always called when the ability ends, also when cancelled.
	void OnItemAbilityEnded(bool bWasApplied)
	{
	}

	UFUNCTION(BlueprintOverride)
	void OnEndAbility(bool bWasCancelled)
	{
		AActor Avatar = GetAvatarActorFromActorInfo();

		OnItemAbilityEnded(bApplied);

		if (bHandsPutAway)
		{
			FVItemAnim::SetHandsBusy(Avatar, false);
			bHandsPutAway = false;
		}

		if (bWarping)
		{
			FVItemAnim::ClearWarpTarget(Avatar, n"Item");
			bWarping = false;
		}

		bApplied = false;
	}

	private void ListenForNotify(FGameplayTag NotifyTag, FName Handler)
	{
		UAbilityTask_WaitGameplayEvent NotifyTask = FVItemAnim::WaitNotify(this, NotifyTag);

		NotifyTask.EventReceived.AddUFunction(this, Handler);
		NotifyTask.ReadyForActivation();
	}

	private void RunWithoutMontage()
	{
		OnItemAttach();
		ApplyOnce();
		OnItemRelease();
		EndAbility();
	}

	private void ApplyOnce()
	{
		if (bApplied)
		{
			return;
		}

		bApplied = true;
		OnItemApply();
	}

	UFUNCTION()
	private void HandleAttachNotify(FGameplayEventData Payload)
	{
		OnItemAttach();
	}

	UFUNCTION()
	private void HandleApplyNotify(FGameplayEventData Payload)
	{
		ApplyOnce();
	}

	UFUNCTION()
	private void HandleReleaseNotify(FGameplayEventData Payload)
	{
		OnItemRelease();
	}

	UFUNCTION()
	private void HandleMontageFinished()
	{
		ApplyOnce();
		OnItemRelease();
		EndAbility();
	}

	UFUNCTION()
	private void HandleMontageInterrupted()
	{
		EndAbility();
	}

	// The montage couldn't play, e.g. on another skeleton.
	UFUNCTION()
	private void HandleMontageCancelled()
	{
		RunWithoutMontage();
	}
}
