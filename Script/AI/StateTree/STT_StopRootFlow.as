class USTT_StopRootFlow : UStateTreeTaskBlueprintBase
{
    UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	AFVAICharacter Actor;

    UFUNCTION(BlueprintOverride)
    void LatentEnterState(FStateTreeTransitionResult Transition)
    {
        if (IsValid(Actor))
        {
            UFlowComponent Flow = Actor.GetComponentByClass(UFlowComponent);

            if (IsValid(Flow) && Flow.IsActive())
            {
                FinishTask(true);
            }
        }

        FinishTask(false);
    }
}