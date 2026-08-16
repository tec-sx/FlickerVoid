class UFVPickupAbility : UFVGameplayAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Pickup")
	UAnimMontage PickupMontage;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		if (PickupMontage == nullptr)
		{
			FinishPickup(ActorInfo);
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
		FinishPickup(ActorInfo);
	}

	private void FinishPickup(FGameplayAbilityActorInfo InActorInfo)
	{
		AActor Instigator = InActorInfo.AvatarActor;

		UFVInteractionOfferComponent Offers =
			Cast<UFVInteractionOfferComponent>(Instigator.GetComponentByClass(UFVInteractionOfferComponent));

		if (Offers != nullptr)
		{
			UFVInteractionTargetComponent Target = Offers.GetEngagedTarget();

			if (Target != nullptr)
			{
				AFVItemPickup Pickup = Cast<AFVItemPickup>(Target.GetOwner());
				UFVInventoryComponent Inventory =
					Cast<UFVInventoryComponent>(Instigator.GetComponentByClass(UFVInventoryComponent));

				if (Pickup != nullptr && Inventory != nullptr)
				{
					Pickup.ExecutePickup(Inventory);
				}
			}
		}

		EndAbility();
	}
}
