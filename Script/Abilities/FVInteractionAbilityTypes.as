struct FFVInteractionExamineMessage
{
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	AActor ExaminedActor;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	bool bVisible = false;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	FRotator ViewRotation;
};

struct FFVInteractionLockpickMessage
{
	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	AActor LockedActor;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	float Difficulty = 0.f;

	UPROPERTY(BlueprintReadWrite, Category = "Interaction")
	bool bSucceeded = false;
};
