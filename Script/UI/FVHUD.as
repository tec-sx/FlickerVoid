class AFVHUD : AHUD
{
    UPROPERTY(EditDefaultsOnly, Category = "UI|Interaction")
	TSubclassOf<UFVInteractionPromptWidget> InteractionPromptWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI|Dialogue")
	TSubclassOf<UFVDialogueWidget> DialogueWidgetClass;

    UPROPERTY()
    private UUserWidget InteractionPromptWidget;

	UPROPERTY(VisibleAnywhere)
	private UDialogueHUDManager DialogueManager;

	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
        APlayerController PC = GetOwningPlayerController();

        if (PC != nullptr)
	    {
			if (InteractionPromptWidgetClass != nullptr)
			{
				InteractionPromptWidget = WidgetBlueprint::CreateWidget(InteractionPromptWidgetClass, PC);
				InteractionPromptWidget.AddToViewport(0);
			}

			DialogueManager = NewObject(this, UDialogueHUDManager);
			
			//   DialogueManager.Initialize(DialogueWidgetClass, GetOwningPlayerController());
	    }
	}

	UFUNCTION()
	UDialogueHUDManager GetDialogueManager()
	{
		return DialogueManager;
	}
}