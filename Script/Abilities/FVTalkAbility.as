class UFVTalkAbility : UFVInteractAbility
{
	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		if (!IsValid(Interactable))
		{
			EndAbility();
			return;
		}

		UFVDialogueParticipantComponent Participant = UFVDialogueParticipantComponent::Get(Interactable.GetOwner());

		if (!IsValid(Participant) || !Participant.HasDialogue())
		{
			EndAbility();
			return;
		}

		UFVDialogueSubsystem::Get().OnConversationEnded.AddUFunction(this, n"OnConversationEnded");

		if (!Participant.StartDialogue(GetAvatarActorFromActorInfo()))
		{
			EndAbility();
		}
	}

	UFUNCTION(BlueprintOverride)
	void OnEndAbility(bool bWasCancelled)
	{
		UFVDialogueSubsystem::Get().OnConversationEnded.UnbindObject(this);
	}

	UFUNCTION()
	private void OnConversationEnded()
	{
		EndAbility();
	}
}
