event void FFVOnHeldItemUsed(AActor User);
event void FFVOnHeldItemToggled(bool bOn);

// Goes on a held item's actor class (e.g. a flashlight Blueprint): using the held item broadcasts On Used, and a toggle flips On and broadcasts On Toggled for the Blueprint to switch its light.
class UFVHeldItemBehaviourComponent : UActorComponent
{
	UPROPERTY(EditAnywhere, Category = "Held Item", meta = (ToolTip = "Each use flips On, e.g. a flashlight's light."))
	bool bToggleState = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item", meta = (ToolTip = "Current toggle state; also the starting one."))
	bool bOn = false;

	UPROPERTY(Category = "Held Item", meta = (ToolTip = "The held item was used, with the character using it."))
	FFVOnHeldItemUsed OnUsed;

	UPROPERTY(Category = "Held Item", meta = (ToolTip = "On changed (Toggle State only)."))
	FFVOnHeldItemToggled OnToggled;

	UFUNCTION(meta = (ToolTip = "Called by the holder's Held Item component: flips On for a toggle, then broadcasts On Used."))
	void Use(AActor User)
	{
		if (bToggleState)
		{
			bOn = !bOn;
			OnToggled.Broadcast(bOn);
		}

		OnUsed.Broadcast(User);
	}
}
