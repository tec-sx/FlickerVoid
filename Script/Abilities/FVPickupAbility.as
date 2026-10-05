class UFVPickupAbility : UFVInteractAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	UAnimMontage PickupMontage;

	// Captured on activation so a focus change during the montage can't redirect the pickup.
	private AActor PickupTarget;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		PickupTarget = IsValid(Interactable) ? Interactable.GetOwner() : nullptr;

		if (PickupMontage == nullptr)
		{
			FinishPickup();
			return;
		}

		UAbilityTask_PlayMontageAndWait MontageTask = AngelscriptAbilityTask::PlayMontageAndWait(this, n"Pickup", PickupMontage); 

		MontageTask.OnCompleted.AddUFunction(this, n"HandleMontageFinished");
		MontageTask.OnInterrupted.AddUFunction(this, n"HandleMontageInterrupted");
		MontageTask.OnCancelled.AddUFunction(this, n"HandleMontageInterrupted");
		MontageTask.OnBlendOut.AddUFunction(this, n"HandleMontageFinished");
		MontageTask.ReadyForActivation();
	}

	UFUNCTION()
	void HandleMontageFinished()
	{
		FinishPickup();
	}

	UFUNCTION()
	void HandleMontageInterrupted()
	{
		PickupTarget = nullptr;
		EndAbility();
	}

	private void FinishPickup()
	{
		if (IsValid(PickupTarget))
		{
			UFVPickupComponent Pickup = UFVPickupComponent::Get(PickupTarget);

			if (IsValid(Pickup))
			{
				Pickup.TryPickup(GetAvatarActorFromActorInfo());
			}
		}

		PickupTarget = nullptr;

		EndAbility();
	}
}
