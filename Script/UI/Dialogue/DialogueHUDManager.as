class UDialogueHUDManager : UFVDialogueUIManagerBase
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSubclassOf<UUserWidget> DialogueWidgetClass;

    private UDialogueWidget DialogueWidget;
    private APlayerController OwningController;
    private USUDSDialogue ActiveDialogue;

    UFUNCTION(BlueprintOverride)
    void OnManagerInitialized(TSubclassOf<UUserWidget> InDialogueWidgetClass, APlayerController InPC)
    {
        OwningController = InPC;

        if (InDialogueWidgetClass != nullptr)
        {
            DialogueWidgetClass = InDialogueWidgetClass;
        }

        if (DialogueWidgetClass == nullptr)
        {
            System::PrintString(
				"DialogueHUDManager: No DialogueWidgetClass set — dialogue UI will not appear.",
                true,
				true,
				FLinearColor::Red,
				10.f);
            return;
        }

        DialogueWidget = Cast<UDialogueWidget>(WidgetBlueprint::CreateWidget(DialogueWidgetClass, InPC));

        if (DialogueWidget == nullptr)
        {
            System::PrintString(
				"DialogueHUDManager: Failed to create DialogueWidget.",
                true,
				true,
				FLinearColor::Red,
				10.f);
            return;
        }

        DialogueWidget.Initialize();
        DialogueWidget.AddToViewport(100);
    }

    
    UFUNCTION(BlueprintOverride)
    void OnManagerDeinitialized()
    {
        if (DialogueWidget != nullptr)
        {
            DialogueWidget.RemoveFromParent();
            DialogueWidget = nullptr;
        }
    }

    // ── Delegate Handlers ─────────────────────────────────────────────────────

    UFUNCTION(BlueprintOverride)
    void OnDialogueLineReady(FFVDialogueLineMessage Message)
    {
        if (IsValid(DialogueWidget))
        {
            DialogueWidget.ShowLine(Message.Text, Message.SpeakerDisplayName);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnDialogueEvent(FFVDialogueEventMessage Message)
    {
    }

    UFUNCTION(BlueprintOverride)
    void OnDialogueEnded(FFVDialogueEndedMessage Message)
    {
        Print("Dialogue End");
        if (IsValid(DialogueWidget))
        {
            DialogueWidget.HideWidget();
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnDialogueChoicesReady(FFVDialogueChoicesMessage Message)
    {
        if (IsValid(DialogueWidget))
        {
            DialogueWidget.ShowChoices(Message.Choices);
        }
    }


    UFUNCTION()
    void DisplayChoices(const TArray<FSUDSScriptEdge> Choices)
    {
    }

    UFUNCTION()
    void HandleHideChoices()
    {
        if (IsValid(DialogueWidget))
        {
            DialogueWidget.HideChoices();
        }
    }
}
