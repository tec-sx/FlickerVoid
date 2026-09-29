class AFVHUD : AHUD
{
	UPROPERTY(EditAnywhere, Category = "Configuration")
    TSubclassOf<UInteractionSetWidget> InteractionOptionsWidgetClass;

	UPROPERTY(Config, EditAnywhere, Category = "Configuration")
    TSubclassOf<UDialogueWidget> DialogueWidgetClass;
    
	UPROPERTY()
    private UInteractionSetWidget InteractionPromptWidget;

	UPROPERTY(VisibleAnywhere)
	private UFVDialogueUIManagerBase DialogueManager;

	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
        APlayerController PC = GetOwningPlayerController();

        if (PC != nullptr)
	    {
			if (InteractionOptionsWidgetClass.IsValid())
			{
				InteractionPromptWidget = WidgetBlueprint::CreateWidget(InteractionOptionsWidgetClass, PC);
				InteractionPromptWidget.AddToViewport(0);

				APlayerController PlayerController = GetOwningPlayerController();
				PC.OnPossessedPawnChanged.AddUFunction(this, n"UpdatePawn");
				UpdatePawn(nullptr, PC.GetControlledPawn());
			}

			if (DialogueWidgetClass.IsValid())
			{
				DialogueManager = NewObject(this, UDialogueHUDManager);
				DialogueManager.Initialize(DialogueWidgetClass.Get(), GetOwningPlayerController());
			}
	    }
	}

    UFUNCTION()
    private void UpdatePawn(APawn OldPawn, APawn NewPawn)
    {
		if (IsValid(OldPawn))
		{
			UFVInteractorResponseComponent_ShowPrompt ShowPromptResponse = OldPawn.GetComponentByClass(UFVInteractorResponseComponent_ShowPrompt);
			InteractionPromptWidget.UnbindResponse(ShowPromptResponse);
		}

		if (IsValid(NewPawn))
		{
			UFVInteractorResponseComponent_ShowPrompt ShowPromptResponse = NewPawn.GetComponentByClass(UFVInteractorResponseComponent_ShowPrompt);
			InteractionPromptWidget.BindResponse(ShowPromptResponse);
		}
    }

	UFUNCTION(BlueprintOverride)
    void EndPlay(EEndPlayReason EndPlayReason)
    {
		if (IsValid(DialogueManager))
		{
			DialogueManager.Deinitialize();
		}
    }
}