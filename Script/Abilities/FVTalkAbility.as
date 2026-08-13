class UFVTalkAbility : UFVGameplayAbility
{
	UPROPERTY(EditDefaultsOnly, Category = "Talk")
	FGameplayTag FlowNotifyTag;

	private FGameplayMessageListenerHandle DialogueEndedHandle;

	UFUNCTION(BlueprintOverride)
	void ActivateAbility(
		FGameplayAbilitySpecHandle Handle,
		FGameplayAbilityActorInfo ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		FGameplayEventData TriggerEventData)
	{
		UFlowComponent FlowComponent = ResolveEngagedFlowComponent(ActorInfo);

		if (FlowComponent == nullptr || !FlowNotifyTag.IsValid())
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
			return;
		}

		DialogueEndedHandle = UGameplayMessageSubsystem::Get().RegisterListener(
			GameplayTags::Dialogue_Ended,
			this,
			n"HandleDialogueEnded",
			FFVDialogueEndedMessage());

		FlowComponent.NotifyGraph(FlowNotifyTag);
	}

	UFUNCTION()
	void HandleDialogueEnded(FGameplayTag Channel, const FFVDialogueEndedMessage& Message)
	{
		EndAbility(CurrentAbilitySpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}

	UFUNCTION(BlueprintOverride)
	void EndAbility(
		FGameplayAbilitySpecHandle Handle,
		FGameplayAbilityActorInfo ActorInfo,
		FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled)
	{
		UGameplayMessageSubsystem::Get().UnregisterListener(DialogueEndedHandle);
	}

	private UFlowComponent ResolveEngagedFlowComponent(FGameplayAbilityActorInfo ActorInfo)
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

		if (Target == nullptr || Target.GetOwner() == nullptr)
		{
			return nullptr;
		}

		return Cast<UFlowComponent>(Target.GetOwner().GetComponentByClass(UFlowComponent));
	}
}
