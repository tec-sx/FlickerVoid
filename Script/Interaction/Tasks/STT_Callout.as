class USTT_Callout : UStateTreeTaskBlueprintBase
{
    UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	AActor Actor;

    UFUNCTION(BlueprintOverride)
    void LatentEnterState(FStateTreeTransitionResult Transition)
    {
    }
}