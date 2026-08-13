class UFVExamineAbility : UFVGameplayAbility
{
	// Viewing orientations that hide a secret, matched within SecretAngleTolerance.
	UPROPERTY(EditDefaultsOnly, Category = "Examine")
	TArray<FRotator> SecretViewAngles;

	UPROPERTY(EditDefaultsOnly, Category = "Examine")
	float SecretAngleTolerance = 12.f;

	private FGameplayMessageListenerHandle ExamineHandle;
	private AActor ExaminedActor;
	private TArray<int> FoundSecrets;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility(
		FGameplayAbilitySpecHandle Handle,
		FGameplayAbilityActorInfo ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		FGameplayEventData TriggerEventData)
	{
		ExaminedActor = ResolveEngagedActor(ActorInfo);

		if (ExaminedActor == nullptr)
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

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
		if (Message.ExaminedActor != ExaminedActor)
		{
			return;
		}

		if (!Message.bVisible)
		{
			EndAbility(CurrentAbilitySpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
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
		Print("Examine: secret " + SecretIndex + " found on " + ExaminedActor.GetName());
	}

	UFUNCTION(BlueprintOverride)
	void EndAbility(
		FGameplayAbilitySpecHandle Handle,
		FGameplayAbilityActorInfo ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled)
	{
		UGameplayMessageSubsystem::Get().UnregisterListener(ExamineHandle);

		if (bWasCancelled && ExaminedActor != nullptr)
		{
			BroadcastExamineState(false, FRotator::ZeroRotator);
		}

		ExaminedActor = nullptr;
	}

	private void BroadcastExamineState(bool bVisible, FRotator ViewRotation)
	{
		FFVInteractionExamineMessage Message;
		Message.ExaminedActor = ExaminedActor;
		Message.bVisible = bVisible;
		Message.ViewRotation = ViewRotation;

		UGameplayMessageSubsystem::Get().BroadcastMessage(
			GameplayTags::Interaction_Event_ExamineStarted, Message);
	}

	private AActor ResolveEngagedActor(FGameplayAbilityActorInfo ActorInfo)
	{
		AActor Instigator = ActorInfo.AvatarActor;

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
