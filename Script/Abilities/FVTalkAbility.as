class UFVTalkAbility : UFVInteractAbility
{
	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		Print("Talk ability activated!");
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
