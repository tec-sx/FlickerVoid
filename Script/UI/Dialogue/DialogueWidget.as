// ─────────────────────────────────────────────────────────────────────────────
// Sample_DialogueWidget.as
//
// Three classes in one file, ordered by dependency:
//
//   1. UDialogueResponseButton  — one player-choice entry (sub-widget)
//   2. UDialogueWidget          — the main dialogue panel widget
//   3. UDialogueHUDManager      — owns the widget, lives on the HUD,
//                                  handles show/hide and subsystem wiring
//
// Layout is defined in Blueprint (WBP_DialogueWidget, WBP_ResponseButton).
// All runtime logic lives here. Blueprint only supplies named slots and
// animation handles via BlueprintEvent stubs at the bottom of each class.
//
// Intended widget hierarchy (WBP_DialogueWidget):
//
//   [Root Canvas]
//     ├─ DialoguePanel (VerticalBox, animated in/out)
//     │    ├─ SpeakerNameText     (TextBlock)   ← speaker display name
//     │    ├─ LineText            (RichTextBlock or TextBlock) ← line content
//     │    ├─ ContinuePrompt      (Image/Text)  ← "▼ continue" hint
//     │    └─ ResponseBox         (VerticalBox) ← response buttons go here
//     └─ [animations: Anim_Show, Anim_Hide, Anim_ResponsesIn]
//
// Intended widget hierarchy (WBP_ResponseButton):
//
//   [Root Button]   ← the UButton we bind OnClicked to
//     ├─ IconImage       (Image)     ← shown when IconTag is valid
//     ├─ ResponseText    (TextBlock) ← DisplayText
//     └─ LockedOverlay   (Image)     ← faded overlay when bIsAvailable=false
// ─────────────────────────────────────────────────────────────────────────────


// ═════════════════════════════════════════════════════════════════════════════
// 2. UDialogueWidget
//
// The main dialogue panel. Handles:
//   • Typewriter text reveal with configurable speed
//   • Auto-dismiss timer when no choices are present
//   • Dynamic response button creation and cleanup
//   • Speaker name display with optional NPC display name lookup
//   • Continue prompt visibility (shown when text is complete, no choices)
//   • Animation hooks for panel slide-in, choice list reveal
// ═════════════════════════════════════════════════════════════════════════════

const FString TypewriterTickFunctionName = "TypewriterTick";
const FString AutoDismissFunctionName = "AutoDismissFired";
const FString OnResponseButtonClickedFunctionName = "OnResponseButtonClicked";

class UDialogueWidget : UUserWidget
{
    // ── Named widget bindings ─────────────────────────────────────────────────

    UPROPERTY(BindWidget)
    UTextBlock SpeakerNameText;

    // Use RichTextBlock in practice for inline bold/color markup on key words
    UPROPERTY(BindWidget)
    UTextBlock LineText;

    // "▼ Press E to continue" — shown after typewriter completes, no choices
    UPROPERTY(BindWidget)
    UWidget ContinuePrompt;

    // Container where UDialogueResponseButton instances are added
    UPROPERTY(BindWidget)
    UVerticalBox ResponseBox;

    // ── Configuration ─────────────────────────────────────────────────────────

    /**
     * Characters revealed per second during typewriter effect.
     * Set to 0 to disable typewriter and show text instantly.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Typewriter")
    float TypewriterCharsPerSecond = 40.f;

    /**
     * Seconds after typewriter completes before auto-dismissing a line
     * that has no response options and no voice audio.
     * Set to 0 to require explicit player input for all lines.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|AutoDismiss")
    float AutoDismissDelay = 3.5f;

    /**
     * Blueprint class used to create each response button entry.
     * Assign WBP_ResponseButton here in the widget's Blueprint defaults.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Layout")
    TSubclassOf<UDialogueResponseButton> ResponseButtonClass;

    /**
     * Optional: a DataAsset or map that converts SpeakerID gameplay tags
     * to human-readable display names ("Dealer", "The Innkeeper", etc.).
     * If null, the raw tag leaf name is used as a fallback.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Layout")
    UDataAsset SpeakerNameRegistry;

    // ── Internal State ────────────────────────────────────────────────────────

    // Reference back to the subsystem — set by UDialogueHUDManager.Initialize()
    private UFVDialogueSubsystem DialogueSys;

    // Full text of the current line (typewriter reveals into LineText)
    private FText FullLineText;

    // How many characters have been revealed so far
    private int32 RevealedCharCount = 0;

    // Whether the typewriter has finished for the current line
    private bool bTypewriterComplete = false;

    // Whether response buttons are currently visible
    private bool bResponsesVisible = false;

    // Live response data — kept for the "skip reveals all + show choices" flow
    private TArray<FFVEvaluatedResponse> PendingResponses;

    private FTimerHandle TypewriterTimer;
    private FTimerHandle AutoDismissTimer;

    // ── Setup (called by UDialogueHUDManager) ─────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void Initialize(UFVDialogueSubsystem InDialogueSys)
    {
        DialogueSys = InDialogueSys;
        SetVisibility(ESlateVisibility::Collapsed);
        ContinuePrompt.SetVisibility(ESlateVisibility::Collapsed);
        ResponseBox.ClearChildren();
    }

    // ── Public API (called by UDialogueHUDManager's delegate handlers) ─────────

    /**
     * Show the widget and begin displaying a new NPC line.
     * Called when UDialogueSubsystem.OnLineStarted fires.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ShowLine(const FFVDialogueLineRow& LineRow, const FGameplayTag& SpeakerID)
    {
        // Cancel any running timers from a previous line
        CancelAllTimers();

        // Clear previous response buttons
        ClearResponseButtons();
        bResponsesVisible = false;
        PendingResponses.Reset();

        // ── Speaker name ──
        FText SpeakerDisplayName = ResolveSpeakerDisplayName(SpeakerID);
        SpeakerNameText.SetText(SpeakerDisplayName);

        // ── Prepare text ──
        FullLineText = LineRow.Text;
        RevealedCharCount = 0;
        bTypewriterComplete = false;
        LineText.SetText(FText::FromString(""));
        ContinuePrompt.SetVisibility(ESlateVisibility::Collapsed);

        // ── Show widget (plays BP animation Anim_Show) ──
        SetVisibility(ESlateVisibility::Visible);
        BP_PlayShowAnimation();

        // ── Start typewriter (or show instantly) ──
        if (TypewriterCharsPerSecond > 0.f)
        {
            float Interval = 1.f / TypewriterCharsPerSecond;
            TypewriterTimer = System::SetTimer(this, FName(TypewriterTickFunctionName), Interval, true);
        }
        else
        {
            // Instant reveal
            LineText.SetText(FullLineText);
            OnTypewriterComplete(LineRow);
        }
    }

    /**
     * Show player response choices below the current line.
     * Called when UDialogueSubsystem.OnResponsesReady fires.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ShowResponses(const TArray<FFVEvaluatedResponse>& Responses)
    {
        PendingResponses = Responses;

        // If typewriter isn't done yet, store the responses and show them
        // after it completes — avoids the jarring appearance of choices
        // popping in while text is still scrolling.
        if (!bTypewriterComplete)
        {
            return; // OnTypewriterComplete() will call BuildResponseButtons()
        }

        BuildResponseButtons();
    }

    /**
     * Hide response buttons (called when OnHideResponses fires).
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void HideResponses()
    {
        ClearResponseButtons();
        bResponsesVisible = false;
        BP_PlayResponsesOutAnimation();
    }

    /**
     * Hide the entire widget (called when OnSessionEnded fires).
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void HideWidget()
    {
        CancelAllTimers();
        ClearResponseButtons();
        BP_PlayHideAnimation();
        // SetVisibility(Collapsed) is called from BP after Anim_Hide completes
        // via a BP AnimationFinished event → Widget.SetVisibility(Collapsed)
    }

    // ── Player Input ──────────────────────────────────────────────────────────

    /**
     * Player pressed the "continue / skip" input while no choices are shown.
     * If the typewriter is still running → jump to full text immediately.
     * If the typewriter is complete   → dismiss the line.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnContinuePressed()
    {
        if (!bTypewriterComplete)
        {
            // Skip to end of typewriter
            SkipTypewriter();
        }
        else if (!bResponsesVisible)
        {
            // Dismiss the line
            CancelAllTimers();
            if (DialogueSys != nullptr)
            {
                DialogueSys.SubmitLineDismissed();
            }
        }
        // If responses ARE visible, continue-press does nothing —
        // the player must pick a response
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
            System::ClearTimer(this, TypewriterTickFunctionName);
            OnTypewriterCompleteNoRow();
        }
    }

    private void SkipTypewriter()
    {
        System::ClearTimer(this, TypewriterTickFunctionName);
        LineText.SetText(FullLineText);
        RevealedCharCount = FullLineText.ToString().Len();
        OnTypewriterCompleteNoRow();
    }

    // Called by ShowLine() when TypewriterCharsPerSecond == 0 (instant reveal)
    private void OnTypewriterComplete(const FFVDialogueLineRow& LineRow)
    {
        bTypewriterComplete = true;

        // If responses were already delivered while typewriter was running,
        // show them now
        if (PendingResponses.Num() > 0)
        {
            BuildResponseButtons();
            return;
        }

        // No choices — show continue prompt and start auto-dismiss
        ContinuePrompt.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        BP_PlayContinuePromptAnimation();
        StartAutoDismissTimer();
    }

    // No-row version called from TypewriterTick and SkipTypewriter
    private void OnTypewriterCompleteNoRow()
    {
        bTypewriterComplete = true;

        if (PendingResponses.Num() > 0)
        {
            BuildResponseButtons();
            return;
        }

        ContinuePrompt.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        BP_PlayContinuePromptAnimation();
        StartAutoDismissTimer();
    }

    // ── Auto-dismiss ──────────────────────────────────────────────────────────

    private void StartAutoDismissTimer()
    {
        if (AutoDismissDelay <= 0.f) return; // requires explicit player input
        if (bResponsesVisible)      return; // don't auto-dismiss during choices

        AutoDismissTimer = System::SetTimer(this, FName(AutoDismissFunctionName), AutoDismissDelay, false); // one-shot
    }

    UFUNCTION()
    private void AutoDismissFired()
    {
        if (bResponsesVisible) return;

        if (DialogueSys != nullptr)
        {
            DialogueSys.SubmitLineDismissed();
        }
    }

    // ── Response Button Construction ──────────────────────────────────────────

    private void BuildResponseButtons()
    {
        // Cancel auto-dismiss — player must now make a choice
        System::ClearTimer(this, AutoDismissFunctionName);

        // Hide the continue prompt — choices replace it
        ContinuePrompt.SetVisibility(ESlateVisibility::Collapsed);

        if (ResponseButtonClass == nullptr)
        {
            PrintWarning("DialogueWidget: ResponseButtonClass not set — cannot build response buttons.");
            return;
        }

        for (const FFVEvaluatedResponse& Response : PendingResponses)
        {
            UDialogueResponseButton Button = 
                Cast<UDialogueResponseButton>(WidgetBlueprint::CreateWidget(ResponseButtonClass, GetOwningPlayer()));

            if (Button == nullptr) continue;

            Button.SetupFromResponse(Response);

            Button.OnResponseClicked.BindUFunction(this, FName(OnResponseButtonClickedFunctionName));

            ResponseBox.AddChild(Button);
        }

        bResponsesVisible = true;
        BP_PlayResponsesInAnimation();
    }

    private void ClearResponseButtons()
    {
        ResponseBox.ClearChildren();
        bResponsesVisible = false;
    }

    UFUNCTION()
    private void OnResponseButtonClicked(FName ResponseID)
    {
        if (DialogueSys == nullptr) return;

        // Lock out further clicks immediately — prevent double-submit on fast input
        // The response buttons get cleared when HideResponses fires from the
        // subsystem anyway, but this is belt-and-suspenders
        for (int32 i = 0; i < ResponseBox.GetChildrenCount(); ++i)
        {
            UDialogueResponseButton Btn = Cast<UDialogueResponseButton>(ResponseBox.GetChildAt(i));
            if (Btn != nullptr)
            {
                Btn.SetIsEnabled(false);
            }
        }

        DialogueSys.SubmitPlayerChoice(ResponseID);
    }

    // ── Speaker Name Resolution ───────────────────────────────────────────────

    private FText ResolveSpeakerDisplayName(FGameplayTag SpeakerID)
    {
        // First try Blueprint lookup (DataAsset / localization table)
        // If BP_ResolveSpeakerName returns an empty text, fall through to the
        // leaf-name fallback so you always get something in the nameplate.
        FText BPResult = BP_ResolveSpeakerName(SpeakerID);
        if (!BPResult.IsEmpty())
        {
            return BPResult;
        }

        // Fallback: use the last segment of the tag as a display name
        // "Speaker.NPC.Dealer" → "Dealer"
        FString TagString = SpeakerID.ToString();
        FString _;
        FString LeafName;
        TagString.Split(".", _, LeafName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
        return FText::FromString(LeafName.IsEmpty() ? TagString : LeafName);
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

    /**
     * Resolve a SpeakerID tag to a display name string.
     * Override in WBP_DialogueWidget to look up from a DataAsset / 
     * localization table. Return empty FText to fall through to the tag
     * leaf name fallback in C++/AS.
     */
    UFUNCTION(BlueprintEvent, Category = "Dialogue|Content")
    FText BP_ResolveSpeakerName(FGameplayTag SpeakerID)
    {
        return FText::FromString("");
    }

    // ── Helpers ───────────────────────────────────────────────────────────────

    private void PrintWarning(FString Msg)
    {
        System::PrintString(Msg, true, true, FLinearColor::Yellow, 5.f);
    }
}