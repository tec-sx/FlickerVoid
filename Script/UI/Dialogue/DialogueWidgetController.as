// DialogueWidgetController.as
// Angelscript — widget controller that bridges UDialogueSubsystem delegates
// to the dialogue UI widget. Attach to your dialogue HUD widget.
//
// Bind this controller in your HUD Blueprint or GameMode:
//   DialogueWidget.SetController(Controller)
//
// The widget itself is pure Blueprint data — layout, fonts, animations.
// All logic lives here.

class UDialogueWidgetController : UObject
{
    // ── References ────────────────────────────────────────────────────────────

    private UFVDialogueSubsystem DialogueSys;

    // ── Setup ─────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable)
    void Initialize(UFVDialogueSubsystem InDialogueSys)
    {
        DialogueSys = InDialogueSys;

        // Bind to all subsystem delegates
        DialogueSys.OnLineStarted.AddUFunction(this, n"OnLineStarted");
        DialogueSys.OnResponsesReady.AddUFunction(this, n"OnResponsesReady");
        DialogueSys.OnHideResponses.AddUFunction(this, n"OnHideResponses");
        DialogueSys.OnSessionEnded.AddUFunction(this, n"OnSessionEnded");
    }

    UFUNCTION(BlueprintCallable)
    void Deinitialize()
    {
        if (DialogueSys != nullptr)
        {
            DialogueSys.OnLineStarted.Unbind(this, n"OnLineStarted");
            DialogueSys.OnResponsesReady.Unbind(this, n"OnResponsesReady");
            DialogueSys.OnHideResponses.Unbind(this, n"OnHideResponses");
            DialogueSys.OnSessionEnded.Unbind(this, n"OnSessionEnded");
        }
    }

    // ── Delegate Handlers → Broadcast to Widget Blueprints ───────────────────

    UFUNCTION()
    void OnLineStarted(FName LineID, const FFVDialogueLineRow&in LineRow, const FFVDialogueContext&in Context)
    {
        // Broadcast to the Blueprint widget via event dispatchers
        BP_OnLineStarted(LineID, LineRow.Text, Context.SpeakerID);

        // Start audio if voice asset exists
        if (!LineRow.VoiceAsset.IsNull())
        {
            // Async load and play
            // FStreamableManager.RequestAsyncLoad(LineRow.VoiceAsset, ...)
            // On load: PlayDialogueAudio(LoadedSound, Context.TargetActor)
        }
        else
        {
            // No audio — start a timer for text display duration, then auto-dismiss
            // GetWorld().GetTimerManager().SetTimer(...)
        }
    }

    UFUNCTION()
    void OnResponsesReady(const TArray<FFVEvaluatedResponse>&in Responses)
    {
        // Pass to Blueprint widget for layout
        // The widget creates a button per response (FEvaluatedResponse.ShouldShow())
        // Greyed responses are shown but not clickable (bIsAvailable = false)
        BP_OnResponsesReady(Responses);
    }

    UFUNCTION()
    void OnHideResponses()
    {
        BP_OnHideResponses();
    }

    UFUNCTION()
    void OnSessionEnded()
    {
        BP_OnSessionEnded();
    }

    // ── Player Input → Subsystem ──────────────────────────────────────────────

    UFUNCTION(BlueprintCallable)
    void SubmitChoice(FName ResponseID)
    {
        if (DialogueSys != nullptr)
        {
            DialogueSys.SubmitPlayerChoice(ResponseID);
        }
    }

    UFUNCTION(BlueprintCallable)
    void DismissCurrentLine()
    {
        if (DialogueSys != nullptr)
        {
            DialogueSys.SubmitLineDismissed();
        }
    }

    // ── Blueprint Events (implemented in the widget Blueprint) ────────────────

    UFUNCTION(BlueprintEvent)
    void BP_OnLineStarted(FName LineID, FText LineText, FGameplayTag SpeakerID) {}

    UFUNCTION(BlueprintEvent)
    void BP_OnResponsesReady(const TArray<FFVEvaluatedResponse>& Responses) {}

    UFUNCTION(BlueprintEvent)
    void BP_OnHideResponses() {}

    UFUNCTION(BlueprintEvent)
    void BP_OnSessionEnded() {}
}
