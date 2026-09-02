class UFVExamineAbility : UFVInteractAbility
{
	// Viewing orientations that hide a secret, matched within SecretAngleTolerance.
	UPROPERTY(EditDefaultsOnly, Category = "Examine")
	TArray<FRotator> SecretViewAngles;

	UPROPERTY(EditDefaultsOnly, Category = "Examine")
	float SecretAngleTolerance = 12.f;

	private FGameplayMessageListenerHandle ExamineHandle;
	private TArray<int> FoundSecrets;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		FoundSecrets.Empty();

		ExamineHandle = UGameplayMessageSubsystem::Get().RegisterListener(
			GameplayTags::Interaction_Event_ExamineStarted,
			this,
			n"HandleExamineUpdate",
			FFVInteractionExamineMessage());

		BroadcastExamineState(true, FRotator::ZeroRotator);
	}

	UFUNCTION()
	void HandleExamineUpdate(FGameplayTag Channel, const FFVInteractionExamineMessage& Message)
	{
		if (Message.ExaminedActor != Interactable.GetOwner())
		{
			return;
		}

		if (!Message.bVisible)
		{
			EndExamine(false);
			return;
		}

		CheckForSecret(Message.ViewRotation);
	}

	private void CheckForSecret(FRotator ViewRotation)
	{
		for (int Index = 0; Index < SecretViewAngles.Num(); Index++)
		{
			if (FoundSecrets.Contains(Index))
			{
				continue;
			}

			if (IsWithinTolerance(ViewRotation, SecretViewAngles[Index]))
			{
				FoundSecrets.Add(Index);
				UnlockMemory(Index);
			}
		}
	}

	private bool IsWithinTolerance(FRotator A, FRotator B)
	{
		return Math::Abs(Math::FindDeltaAngleDegrees(A.Pitch, B.Pitch)) <= SecretAngleTolerance
			&& Math::Abs(Math::FindDeltaAngleDegrees(A.Yaw, B.Yaw)) <= SecretAngleTolerance
			&& Math::Abs(Math::FindDeltaAngleDegrees(A.Roll, B.Roll)) <= SecretAngleTolerance;
	}

	// TODO: unlock the memory tied to SecretViewAngles[SecretIndex].
	private void UnlockMemory(int SecretIndex)
	{
		Print("Examine: secret " + SecretIndex + " found on " + Interactable.GetOwner().GetName());
	}

	private void EndExamine(bool bWasCancelled)
	{
		UGameplayMessageSubsystem::Get().UnregisterListener(ExamineHandle);

		if (bWasCancelled && Interactable.GetOwner() != nullptr)
		{
			BroadcastExamineState(false, FRotator::ZeroRotator);
		}

		EndAbility();
	}

	private void BroadcastExamineState(bool bVisible, FRotator ViewRotation)
	{
		FFVInteractionExamineMessage Message;
		Message.ExaminedActor = Interactable.GetOwner();
		Message.bVisible = bVisible;
		Message.ViewRotation = ViewRotation;

		UGameplayMessageSubsystem::Get().BroadcastMessage(
			GameplayTags::Interaction_Event_ExamineStarted, Message);
	}
}
