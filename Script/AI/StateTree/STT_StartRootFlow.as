class USTT_StartRootFlow : UStateTreeTaskBlueprintBase
{
    UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	AFVAICharacter Actor;

    UFUNCTION(BlueprintOverride)
    void LatentEnterState(FStateTreeTransitionResult Transition)
    {
        if (IsValid(Actor))
        {
            UFlowComponent Flow = Actor.GetComponentByClass(UFlowComponent);

            if (IsValid(Flow))
            {
                Flow.StartRootFlow();
                FinishTask(true);
            }
        }

        FinishTask(false);
    }
}