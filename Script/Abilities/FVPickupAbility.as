class UFVPickupAbility : UFVInteractAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	UAnimMontage PickupMontage;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		if (PickupMontage == nullptr)
		{
			FinishPickup();
			return;
		}

		UAbilityTask_PlayMontageAndWait MontageTask = AngelscriptAbilityTask::PlayMontageAndWait(this, n"Pickup", PickupMontage); 

		MontageTask.OnCompleted.AddUFunction(this, n"HandleMontageFinished");
		MontageTask.OnInterrupted.AddUFunction(this, n"HandleMontageFinished");
		MontageTask.OnCancelled.AddUFunction(this, n"HandleMontageFinished");
		MontageTask.OnBlendOut.AddUFunction(this, n"HandleMontageFinished");
		MontageTask.ReadyForActivation();
	}

	UFUNCTION()
	void HandleMontageFinished()
	{
		FinishPickup();
	}

	private void FinishPickup()
	{
		// TODO: Route pickup through FVInventoryEquipmentSystem.
		EndAbility();
	}
}
