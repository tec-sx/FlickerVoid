class UFVInAirMovementHandler : UFVMovementHandlerBase
{
    UFUNCTION(BlueprintOverride)
    bool Resolve() const
    {
        return MovementComponent.MovementMode == EMovementMode::MOVE_Falling;
    }

    UFUNCTION(BlueprintOverride)
    void OnEnter()
    {
        Print("Entered InAir State");
    }

	UFUNCTION(BlueprintOverride)
	void GenerateMovement(float DeltaTime)
	{
        MovementComponent.RotationRate = FRotator(0.f, 200.f, 0.f);
	}
}