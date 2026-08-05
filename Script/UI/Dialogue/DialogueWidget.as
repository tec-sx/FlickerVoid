class UDialogueWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UTextBlock SpeakerNameText;

    UPROPERTY(BindWidget)
    UTextBlock LineText;

    UPROPERTY(BindWidget)
    UVerticalBox ChoiceBox;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Typewriter")
    float TypewriterCharsPerSecond = 40.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|AutoDismiss")
    float AutoDismissDelay = 3.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Layout")
    TSubclassOf<UDialogueResponseButton> ResponseButtonClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Layout")
    UDataAsset SpeakerNameRegistry;

    private FText FullLineText;
    private int32 RevealedCharCount = 0;
    private bool bTypewriterComplete = false;
    private bool bChoicesVisible = false;
    private TArray<FFVDialogueChoiceMessage> PendingChoices;

    private FTimerHandle TypewriterTimer;
    private FTimerHandle AutoDismissTimer;

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void Initialize()
    {
        SetVisibility(ESlateVisibility::Collapsed);
        ChoiceBox.ClearChildren();
    }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ShowLine(const FText Text, const FText SpeakerName)
    {
        CancelAllTimers();

        ClearResponseButtons();
        bChoicesVisible = false;
        PendingChoices.Reset();

        SpeakerNameText.SetText(SpeakerName);

        FullLineText = Text;
        RevealedCharCount = 0;
        bTypewriterComplete = false;
        LineText.SetText(FText::FromString(""));

        SetVisibility(ESlateVisibility::Visible);
        BP_PlayShowAnimation();

        if (TypewriterCharsPerSecond > 0.f)
        {
            float Interval = 1.f / TypewriterCharsPerSecond;
            TypewriterTimer = System::SetTimer(this, n"TypewriterTick", Interval, true);
        }
        else
        {
            LineText.SetText(FullLineText);
            OnTypewriterComplete(Text);
        }
    }

    UFUNCTION(BlueprintCallable)
    void ShowChoices(const TArray<FFVDialogueChoiceMessage> Choices)
    {
        PendingChoices = Choices;

        if (!bTypewriterComplete)
        {
            return;
        }

        BuildResponseButtons();
    }

    UFUNCTION(BlueprintCallable)
    void HideChoices()
    {
        ClearResponseButtons();
        bChoicesVisible = false;
        BP_PlayResponsesOutAnimation();
    }

    UFUNCTION(BlueprintCallable)
    void HideWidget()
    {
        CancelAllTimers();
        ClearResponseButtons();
        BP_PlayHideAnimation();
        // SetVisibility(Collapsed) is called from BP after Anim_Hide completes
        // via a BP AnimationFinished event → Widget.SetVisibility(Collapsed)
    }

    // ── Player Input ──────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnContinuePressed()
    {
        if (!bTypewriterComplete)
        {
            SkipTypewriter();
        }
        else if (!bChoicesVisible)
        {
            CancelAllTimers();
        }
    }

    // ── Typewriter ────────────────────────────────────────────────────────────

    UFUNCTION()
    private void TypewriterTick()
    {
        FString FullString = FullLineText.ToString();
        int32 TotalChars = FullString.Len();

        RevealedCharCount = Math::Min(RevealedCharCount + 1, TotalChars);
        FString Revealed = FullString.Left(RevealedCharCount);
        LineText.SetText(FText::FromString(Revealed));

        if (RevealedCharCount >= TotalChars)
        {
            System::ClearTimer(this, "TypewriterTick");
            OnTypewriterCompleteNoRow();
        }
    }

    private void SkipTypewriter()
    {
        System::ClearTimer(this, "TypewriterTick");
        LineText.SetText(FullLineText);
        RevealedCharCount = FullLineText.ToString().Len();
        OnTypewriterCompleteNoRow();
    }

    private void OnTypewriterComplete(const FText& Text)
    {
        bTypewriterComplete = true;

        if (PendingChoices.Num() > 0)
        {
            BuildResponseButtons();
            return;
        }

        BP_PlayContinuePromptAnimation();
        StartAutoDismissTimer();
    }

    private void OnTypewriterCompleteNoRow()
    {
        bTypewriterComplete = true;

        if (PendingChoices.Num() > 0)
        {
            BuildResponseButtons();
            return;
        }

        BP_PlayContinuePromptAnimation();
        StartAutoDismissTimer();
    }

    // ── Auto-dismiss ──────────────────────────────────────────────────────────

    private void StartAutoDismissTimer()
    {
        if (AutoDismissDelay <= 0.f || bChoicesVisible)
            return;

        AutoDismissTimer = System::SetTimer(this, n"AutoDismissFired", AutoDismissDelay, false);
    }

    UFUNCTION()
    private void AutoDismissFired()
    {
        if (bChoicesVisible)
            return;
    }

    // ── Response Button Construction ──────────────────────────────────────────

    private void BuildResponseButtons()
    {
        // Cancel auto-dismiss — player must now make a choice
        System::ClearTimer(this, "AutoDismissFired");

        if (ResponseButtonClass == nullptr)
        {
            PrintWarning("DialogueWidget: ResponseButtonClass not set — cannot build response buttons.");
            return;
        }

        for (FFVDialogueChoiceMessage Choice : PendingChoices)
        {
            UDialogueResponseButton Button = Cast<UDialogueResponseButton>(WidgetBlueprint::CreateWidget(ResponseButtonClass, GetOwningPlayer()));

            if (Button == nullptr) continue;

            Button.SetupFromResponse(Choice);
            Button.OnResponseClicked.BindUFunction(this, n"OnChoiceButtonClicked");
            ChoiceBox.AddChild(Button);
        }

        BP_PlayResponsesInAnimation();
    }

    private void ClearResponseButtons()
    {
        ChoiceBox.ClearChildren();
    }

    UFUNCTION()
    private void OnChoiceButtonClicked(int Index)
    {
        for (int32 i = 0; i < ChoiceBox.GetChildrenCount(); ++i)
        {
            UDialogueResponseButton Btn = Cast<UDialogueResponseButton>(ChoiceBox.GetChildAt(i));
            if (Btn != nullptr)
            {
                Btn.SetIsEnabled(false);
            }
        }

        // FVDialogueMessage::BroadcastChoicesReady
    }

    // ── Timer Cleanup ─────────────────────────────────────────────────────────

    private void CancelAllTimers()
    {
        System::ClearAndInvalidateTimerHandle(TypewriterTimer);
        System::ClearAndInvalidateTimerHandle(AutoDismissTimer);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Blueprint Events — implement all visual/animation work in WBP_DialogueWidget
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * Play the panel slide-in animation (Anim_Show).
     * At the end of the animation, the panel should be fully visible.
     */
    UFUNCTION(BlueprintEvent, Category = "Dialogue|Animation")
    void BP_PlayShowAnimation() {}

    /**
     * Play the panel slide-out animation (Anim_Hide).
     * At the END of this animation, call SetVisibility(Collapsed) in Blueprint
     * so the widget doesn't block input after it's off-screen.
     */
    UFUNCTION(BlueprintEvent, Category = "Dialogue|Animation")
    void BP_PlayHideAnimation() {}

    /**
     * Animate the response box into view (e.g. fade + slide up from below
     * the line text). Called once all response buttons have been added.
     */
    UFUNCTION(BlueprintEvent, Category = "Dialogue|Animation")
    void BP_PlayResponsesInAnimation() {}

    /**
     * Animate the response box out of view before the next line appears.
     */
    UFUNCTION(BlueprintEvent, Category = "Dialogue|Animation")
    void BP_PlayResponsesOutAnimation() {}

    /**
     * Animate the "▼ continue" prompt pulsing in.
     * Typically a gentle fade-in + bob loop.
     */
    UFUNCTION(BlueprintEvent, Category = "Dialogue|Animation")
    void BP_PlayContinuePromptAnimation() {}

    // ── Helpers ───────────────────────────────────────────────────────────────

    private void PrintWarning(FString Msg)
    {
        System::PrintString(Msg, true, true, FLinearColor::Yellow, 5.f);
    }
}