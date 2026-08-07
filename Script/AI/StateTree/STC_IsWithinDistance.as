class USTC_IsWithinDistance : UStateTreeConditionBlueprintBase
{
    UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	AFVAICharacter Actor;
	
	UPROPERTY(BlueprintReadOnly, Category = Context, meta = (Context))
	AFVAICharacterController AIController;

    UPROPERTY(EditAnywhere)
    float DistanceThreshold = 800.0f;

    UPROPERTY(BlueprintReadOnly)
	bool bInvert = false;

    UFUNCTION(BlueprintOverride)
    bool TestCondition() const
    {
        bool bResult = false;

        if (IsValid(Actor) && IsValid(AIController) && IsValid(AIController.AcquiredTarget))
        {
            float Distance = Actor.GetActorLocation().Distance(AIController.AcquiredTarget.GetActorLocation());
            bResult = Distance <= DistanceThreshold;
        }

        return bInvert ? !bResult : bResult;
    }
}