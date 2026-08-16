class UFVTalkAbility : UFVGameplayAbility
{
	private FGameplayMessageListenerHandle DialogueEndedHandle;

	default AbilityTags.AddTag(GameplayTags::Interaction_Action_Talk);

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		UFlowComponent FlowComponent = ResolveEngagedFlowComponent(ActorInfo);

		if (IsValid(FlowComponent))
		{
			DialogueEndedHandle = UGameplayMessageSubsystem::Get().RegisterListener(
				GameplayTags::Dialogue_Ended,
				this,
				n"HandleDialogueEnded",
				FFVDialogueEndedMessage());

			FlowComponent.NotifyGraph(GameplayTags::Interaction_Action_Talk);
		}
		else
		{
			EndTalk();
		}
	}

	UFUNCTION()
	void HandleDialogueEnded(FGameplayTag Channel, const FFVDialogueEndedMessage& Message)
	{
		EndTalk();
	}

	private void EndTalk()
	{
		UGameplayMessageSubsystem::Get().UnregisterListener(DialogueEndedHandle);
		EndAbility();
	}

	private UFlowComponent ResolveEngagedFlowComponent(FGameplayAbilityActorInfo InActorInfo)
	{
		AFVPlayerController PC = Cast<AFVPlayerController>(InActorInfo.AvatarActor);

		if (IsValid(PC))
		{
			AFVPlayerCharacter Character = PC.GetControlledCharacter();

			if (IsValid(Character))
			{
				UFVInteractionTargetComponent Target = Character.Offers.GetEngagedTarget();
				
				Print(Character.Offers.GetEngagedTarget().ToString());
				if (IsValid(Target) && IsValid(Target.GetOwner()))
				{
					return Cast<UFlowComponent>(Target.GetOwner().GetComponentByClass(UFlowComponent));
				}
			}
		}

		return nullptr;
	}
}
