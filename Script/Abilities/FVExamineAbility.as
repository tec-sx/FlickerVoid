// Opens the examine view on the targeted actor and finds its secrets (UFVExamineSecretsComponent) as the widget turns it. Finding the last secret of an unsolved readable reads it and closes the view; cancelling keeps found clues and leaves the note unsolved.
class UFVExamineAbility : UFVInteractAbility
{
	default ActivationOwnedTags.AddTag(GameplayTags::Status_Interacting);

	private FGameplayMessageListenerHandle ExamineHandle;
	private AActor ExaminedActor;
	private bool bListening = false;
	private bool bViewOpen = false;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		ExaminedActor = IsValid(Interactable) ? Interactable.GetOwner() : nullptr;

		if (ExaminedActor == nullptr)
		{
			EndAbility();
			return;
		}

		ExamineHandle = UGameplayMessageSubsystem::Get().RegisterListener(
			GameplayTags::Interaction_Event_ExamineStarted,
			this,
			n"HandleExamineUpdate",
			FFVInteractionExamineMessage());

		bListening = true;
		bViewOpen = true;
		BroadcastExamineState(true, FRotator::ZeroRotator);
	}

	UFUNCTION(BlueprintOverride)
	void OnEndAbility(bool bWasCancelled)
	{
		if (bListening)
		{
			UGameplayMessageSubsystem::Get().UnregisterListener(ExamineHandle);
			bListening = false;
		}

		if (bViewOpen && ExaminedActor != nullptr)
		{
			BroadcastExamineState(false, FRotator::ZeroRotator);
		}

		bViewOpen = false;
		ExaminedActor = nullptr;
	}

	UFUNCTION()
	void HandleExamineUpdate(FGameplayTag Channel, const FFVInteractionExamineMessage& Message)
	{
		if (ExaminedActor == nullptr || Message.ExaminedActor != ExaminedActor)
		{
			return;
		}

		if (!Message.bVisible)
		{
			bViewOpen = false;
			EndAbility();
			return;
		}

		UFVExamineSecretsComponent Secrets = UFVExamineSecretsComponent::Get(ExaminedActor);

		if (!IsValid(Secrets))
		{
			return;
		}

		Secrets.TryFind(Message.ViewRotation);
		TrySolveReadable(Secrets);
	}

	private void TrySolveReadable(UFVExamineSecretsComponent Secrets)
	{
		AFVReadable Readable = Cast<AFVReadable>(ExaminedActor);

		if (Readable == nullptr || Readable.IsSolved() || !Secrets.HasSecrets() || !Secrets.AllFound())
		{
			return;
		}

		Readable.MarkRead(GetAvatarActorFromActorInfo());
		EndAbility();
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
}
