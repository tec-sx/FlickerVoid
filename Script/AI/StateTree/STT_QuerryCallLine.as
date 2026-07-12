class USTT_QuerryCallLine : UStateTreeTaskBlueprintBase
{

    UPROPERTY(EditAnywhere, Category = Output, meta = (Output))
    FText SelectedLine;

	UFUNCTION(BlueprintOverride)
	void LatentEnterState(FStateTreeTransitionResult Transition)
	{
        auto DD = Subsystem::GetGameInstanceSubsystem(UFVDialogueDatabaseSubsystem);

	}

    
	UFUNCTION(BlueprintOverride)
	FText GetDescription(EStateTreeNodeFormatting Formatting) const
	{
        return NSLOCTEXT("FlickerVoid", "QuerryCallLine", "Queries the line that will be used to call the player.");
	}
}