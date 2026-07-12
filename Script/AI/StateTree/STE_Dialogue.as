UCLASS()
class USTE_Dialogue : UStateTreeEvaluatorBlueprintBase
{
    UPROPERTY(VisibleAnywhere, Category = Context)
	TObjectPtr<AFVAICharacter> Character;

	UPROPERTY(VisibleAnywhere, Category = Context)
	TWeakObjectPtr<AFVAICharacterController> AIController;

	UPROPERTY(VisibleAnywhere)
	TArray<FFVDialogueLineRow> CallRows;

	UFUNCTION(BlueprintOverride)
	void TreeStart()
	{
		auto DD = Subsystem::GetGameInstanceSubsystem(UFVDialogueDatabaseSubsystem);
	}

	UFUNCTION(BlueprintOverride)
	void TreeStop()
	{
	}

    UFUNCTION(BlueprintOverride)
	FText GetDescription(EStateTreeNodeFormatting Formatting) const
	{
        return NSLOCTEXT(
			"STE_Dialogue",
			"DialogueEvaluatorDesc",
			"Generates Dialogue Data"
		);
	}
}