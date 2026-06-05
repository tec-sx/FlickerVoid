class UFVInAirMovementHandler : UFVMovementHandlerBase
{
	UFUNCTION(BlueprintOverride)
	void GenerateMovement(float DeltaTime)
	{
        if (MovementComponent.IsFalling())
        {
            MovementComponent.RotationRate = FRotator(0.f, 200.f, 0.f);
        }
        else
        {
            MovementComponent.RotationRate = FRotator(0.f, -1.f, 0.f);
        }
	}
}