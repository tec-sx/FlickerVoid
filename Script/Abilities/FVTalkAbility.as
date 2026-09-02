class UFVTalkAbility : UFVInteractAbility
{
	default AbilityTags.AddTag(GameplayTags::Interaction_Action_Talk);

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		if (IsValid(Interactable))
		{
			UFlowComponent FlowComponent = Interactable.GetOwner().GetComponentByClass(UFlowComponent);
	
			if (IsValid(FlowComponent))
			{
				FlowComponent.NotifyGraph(GameplayTags::Interaction_Action_Talk);
			}
	
			EndAbility();
		}
	}
}
