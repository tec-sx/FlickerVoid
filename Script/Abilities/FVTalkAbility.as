class UFVTalkAbility : UFVInteractAbility
{
	default AbilityTags.AddTag(GameplayTags::Interaction_Action_Talk);

	UFUNCTION(BlueprintOverride)
	void ActivateInteraction(UFVInteractionOfferComponent Offers, AActor InteractableActor)
	{
		UFlowComponent FlowComponent = Offers.EngagedActor.GetComponentByClass(UFlowComponent);

		if (IsValid(FlowComponent))
		{
			FlowComponent.NotifyGraph(GameplayTags::Interaction_Action_Talk);
		}

		EndAbility();
	}
}
