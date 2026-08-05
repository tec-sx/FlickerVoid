class AFVHUD : AHUD
{
    UPROPERTY()
    private UUserWidget InteractionPromptWidget;

	UPROPERTY(VisibleAnywhere)
	private UFVDialogueUIManagerBase DialogueManager;

	UFUNCTION(BlueprintOverride)
	void BeginPlay()
	{
        APlayerController PC = GetOwningPlayerController();

        if (PC != nullptr)
	    {
			UPlayerSettings PlayerSettings = UPlayerSettings.GetDefaultObject();

			if (PlayerSettings.InteractionPromptWidgetClass.IsValid())
			{
				InteractionPromptWidget = WidgetBlueprint::CreateWidget(PlayerSettings.InteractionPromptWidgetClass.Get(), PC);
				InteractionPromptWidget.AddToViewport(0);
			}

			if (PlayerSettings.DialogueWidgetClass.IsValid())
			{
				DialogueManager = NewObject(this, UDialogueHUDManager);
				DialogueManager.Initialize(PlayerSettings.DialogueWidgetClass.Get(), GetOwningPlayerController());
			}
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