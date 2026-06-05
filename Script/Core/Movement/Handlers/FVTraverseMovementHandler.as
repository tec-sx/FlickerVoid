class UFVTraverseMovementHandler : UFVMovementHandlerBase
{

	UFUNCTION(BlueprintOverride)
	void GenerateMovement(float DeltaTime)
	{
        MovementComponent.SetMovementMode(EMovementMode::MOVE_Flying);
	}
};