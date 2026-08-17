class UFVTalkAbility : UFVInteractAbility
{
	default AbilityTags.AddTag(GameplayTags::Interaction_Action_Talk);

	UFUNCTION(BlueprintOverride)
	void ActivateAbility()
	{
		UFVInteractionOfferComponent Offers = GetOfferComponent();

		if (IsValid(Offers))
		{
			Offers.BeginEngagement(Slot);

			if (IsValid(Offers.EngagedActor))
			{
				UFlowComponent FlowComponent = Offers.EngagedActor.GetComponentByClass(UFlowComponent);

				if (IsValid(FlowComponent))
				{
					Print("Notifying Graph on " + Offers.EngagedActor);
					
					FlowComponent.NotifyGraph(GameplayTags::Interaction_Action_Talk);
				}
			}
		}

		EndAbility();
	}
}
