// Dialogue overlay displayed during NPC conversations.
//
// Lifecycle:
//   Appears on UFVDialogueSubsystem::OnDialogueLineStarted.
//   Hides on UFVDialogueSubsystem::OnConversationEnded.
//
// UMG Designer layout (suggested):
//   - SpeakerNameText   (UTextBlock — NPC name)
//   - LineText          (UTextBlock — dialogue body, word-wrap on)
//   - ResponsesContainer (UVerticalBox — populated with UFVResponseEntryWidget children)
//   - SkipHintText      (optional UTextBlock — "Press [E] to skip / continue")
//
// Set-up:
//   1. Create a Widget Blueprint inheriting UFVDialogueWidget.
//   2. Add the named child widgets listed above.
//   3. Set ResponseEntryClass to your UFVResponseEntryWidget subclass.
//   4. Assign the Widget Blueprint to AFVHUD::DialogueWidgetClass.

class UFVDialogueWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UTextBlock SpeakerNameText;

    UPROPERTY(BindWidget)
    UTextBlock LineText;

    UPROPERTY(BindWidget)
    UVerticalBox ResponsesContainer;

    UPROPERTY(BindWidgetOptional)
    UTextBlock SkipHintText;

    // Assign your UFVResponseEntryWidget subclass in the widget blueprint
    UPROPERTY(EditDefaultsOnly, Category = "Dialogue|Responses")
    TSubclassOf<UFVResponseEntryWidget> ResponseEntryClass;

    UFVDialogueSubsystem DialogueSubsystem;

    UFUNCTION(BlueprintOverride)
    void Construct()
    {
        // SetVisibility(ESlateVisibility::Hidden);

        // DialogueSubsystem = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);

        // if (DialogueSubsystem == nullptr)
        // {
        //     Warning("UFVDialogueWidget: UFVDialogueSubsystem not found.");
        //     return;
        // }

        // DialogueSubsystem.OnDialogueLineStarted.AddUFunction(this, n"HandleLineStarted");
        // DialogueSubsystem.OnDialogueResponsesAvailable.AddUFunction(this, n"HandleResponsesAvailable");
        // DialogueSubsystem.OnConversationEnded.AddUFunction(this, n"HandleConversationEnded");
    }

    UFUNCTION(BlueprintOverride)
    void Destruct()
    {
        // if (DialogueSubsystem != nullptr)
        // {
        //     DialogueSubsystem.OnDialogueLineStarted.Unbind(this, n"HandleLineStarted");
        //     DialogueSubsystem.OnDialogueResponsesAvailable.Unbind(this, n"HandleResponsesAvailable");
        //     DialogueSubsystem.OnConversationEnded.Unbind(this, n"HandleConversationEnded");
        // }
    }

    // =========================================================================
    // Subsystem callbacks
    // =========================================================================

    UFUNCTION()
    void HandleLineStarted(const FFVDialogueLineRow&in Line, const FFVDialogueContext&in Context)
    {
        // SetVisibility(ESlateVisibility::HitTestInvisible);
        // ResponsesContainer.ClearChildren();

        // FFVSpeakerInfo SpeakerInfo;
        // FText SpeakerName = DialogueSubsystem.GetSpeakerInfo(Line.SpeakerId, SpeakerInfo)
        //     ? SpeakerInfo.DisplayName
        //     : FText::FromName(Line.SpeakerId);

        // SpeakerNameText.SetText(SpeakerName);
        // LineText.SetText(Line.Text);

        // if (SkipHintText != nullptr)
        // {
        //     SkipHintText.SetVisibility(ESlateVisibility::HitTestInvisible);
        // }
    }

    UFUNCTION()
    private void HandleResponsesAvailable(const TArray<FFVEvaluatedResponse>&in Responses)
    {
        // if (SkipHintText != nullptr)
        // {
        //     SkipHintText.SetVisibility(ESlateVisibility::Collapsed);
        // }

        // ResponsesContainer.ClearChildren();

        // if (ResponseEntryClass == nullptr)
        // {
        //     Warning("UFVDialogueWidget: ResponseEntryClass not set.");
        //     return;
        // }

        // for (int32 i = 0; i < Responses.Num(); ++i)
        // {
        //     UFVResponseEntryWidget Entry = Cast<UFVResponseEntryWidget>(
        //         WidgetBlueprint::CreateWidget(ResponseEntryClass, GetOwningPlayer()));

        //     if (Entry != nullptr)
        //     {
        //         // Pass the subsystem so the entry can route the selection back directly
        //         Entry.Initialize(DialogueSubsystem, Responses[i], i);
        //         ResponsesContainer.AddChildToVerticalBox(Entry);
        //     }
        // }
    }

    UFUNCTION()
    void HandleConversationEnded()
    {
        SetVisibility(ESlateVisibility::Hidden);
        ResponsesContainer.ClearChildren();
    }

    // Bind this to the skip/continue input action in your player controller or input config
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SkipCurrentLine()
    {
        // if (DialogueSubsystem != nullptr && DialogueSubsystem.IsInConversation())
        // {
        //     DialogueSubsystem.SkipCurrentLine();
        // }
    }
}

// =========================================================================
// Response entry — one selectable option in the response list.
//
// UMG Designer: create a Widget Blueprint inheriting UFVResponseEntryWidget with:
//   ResponseText  → UTextBlock (the full response sentence)
//   ShortText     → UTextBlock (optional — for a wheel/compact layout)
//   TypeIndicator → UWidget  (optional — tint by ResponseType tag for tone cues)
//
// In the blueprint, wire a Button.OnClicked → SelectThisResponse().
// =========================================================================

class UFVResponseEntryWidget : UUserWidget
{
    UPROPERTY(BindWidget)
    UTextBlock ResponseText;

    UPROPERTY(BindWidgetOptional)
    UTextBlock ShortText;

    private UFVDialogueSubsystem BoundSub;
    private int32 StoredIndex = -1;

    void Initialize(UFVDialogueSubsystem Sub, FFVEvaluatedResponse Response, int32 Index)
    {
        // BoundSub    = Sub;
        // StoredIndex = Index;

        // ResponseText.SetText(Response.ResponseText);

        // if (ShortText != nullptr && !Response.ShortText.IsEmpty())
        // {
        //     ShortText.SetText(Response.ShortText);
        // }
    }

    // Wire this to a button's OnClicked event in the widget blueprint
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SelectThisResponse()
    {
        // if (BoundSub != nullptr && StoredIndex >= 0)
        // {
        //     BoundSub.SelectResponse(StoredIndex);
        // }
    }
}
