// Handler for NPC conversation actions.
//
// Connects the interaction system to UFVDialogueSubsystem without a hard
// module dependency — all coupling is through gameplay tags and the subsystem API.
//
// Sync vs async:
//   - If ConversationId is set, a structured conversation opens and the handler
//     stays "executing" until OnConversationEnded fires.
//   - If only InitialSignal is set, a single bark line plays and the handler
//     completes immediately (synchronous).
//
// Quest integration:
//   UFVQuestSubsystem::NotifyTalkedToNPC is called after the conversation ends
//   so quest objectives that require speaking to this NPC are automatically tracked.

class UFVTalkHandlerConfig : UFVInteractionHandlerConfig
{
    // Signal to send at the start of this interaction.
    // Every entry in the dialogue DataTable has a SignalName — this identifies
    // which set of lines the subsystem should query.
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    FName InitialSignal = n"Greeting";

    // Set this to open a structured conversation (with player responses).
    // Leave empty for single-bark interactions (ambient, item descriptions, etc.)
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    FName ConversationId;

    // NPC identifier passed to the quest subsystem.
    // Match this to the SpeakerId in the dialogue DataTable.
    UPROPERTY(EditAnywhere, Category = "Quest")
    FName NPCId;
}

class UFVTalkHandler : UFVInteractionActionHandler
{
    UFVTalkHandlerConfig TalkConfig;
    bool bIsConversation = false;

    UFUNCTION(BlueprintOverride)
    void OnInitialize(UFVInteractionHandlerConfig Config)
    {
        TalkConfig = Cast<UFVTalkHandlerConfig>(Config);
        if (TalkConfig == nullptr)
        {
            Warning(GetName() + ": no UFVTalkHandlerConfig assigned, using defaults.");
            TalkConfig = NewObject(this, UFVTalkHandlerConfig);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnExecute(FFVInteractionContext Context)
    {
        UFVDialogueSubsystem DialogueSub = GetDialogueSubsystem(Context.Instigator);
        if (DialogueSub == nullptr)
        {
            Warning(GetName() + ": UFVDialogueSubsystem not found.");
            CompleteExecution(false);
            return;
        }

        FFVDialogueContext DialogCtx;
        DialogCtx.SpeakerActor  = Context.TargetActor;
        DialogCtx.ListenerActor = Context.Instigator;

        if (TalkConfig.ConversationId != n"None")
        {
            // Structured conversation — wait for OnConversationEnded before completing
            bIsConversation = true;
            DialogueSub.OnConversationEnded.AddUFunction(this, n"HandleConversationEnded");

            DialogueSub.StartConversation(TalkConfig.ConversationId, TalkConfig.InitialSignal, DialogCtx);
        }
        else
        {
            // Single bark — fire and forget, complete immediately
            DialogueSub.SendSignalFromActor(TalkConfig.InitialSignal, Context.TargetActor, Context.Instigator);
            NotifyQuestSubsystem(Context.Instigator, n"None");
            CompleteExecution(true);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnCancel(FFVInteractionContext Context)
    {
        if (bIsConversation)
        {
            UFVDialogueSubsystem DialogueSub = GetDialogueSubsystem(Context.Instigator);
            if (DialogueSub != nullptr)
            {
                DialogueSub.OnConversationEnded.Unbind(this, n"HandleConversationEnded");
                DialogueSub.EndConversation();
            }
            bIsConversation = false;
        }
    }

    UFUNCTION()
    void HandleConversationEnded()
    {
        // We don't have direct access to the original context here, so
        // we use the C++ base's GetActiveContext() to retrieve the instigator.
        FFVInteractionContext Ctx = GetActiveContext();
        UFVDialogueSubsystem DialogueSub = GetDialogueSubsystem(Ctx.Instigator);
        if (DialogueSub != nullptr)
        {
            DialogueSub.OnConversationEnded.Unbind(this, n"HandleConversationEnded");
        }

        NotifyQuestSubsystem(Ctx.Instigator, n"None");
        bIsConversation = false;
        CompleteExecution(true);
    }

    // =========================================================================
    // Helpers
    // =========================================================================

    private UFVDialogueSubsystem GetDialogueSubsystem(AActor ContextActor)
    {
        // TODO: Check if it is requered to pass the ContextActor (ContextActor, UFVQuestSubsystem)
        return Cast<UFVDialogueSubsystem>(Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem));
    }

    private void NotifyQuestSubsystem(AActor ContextActor, FName DialogueNodeId)
    {
        if (ContextActor == nullptr)
        {
            return;
        }

        // TODO: Check if it is requered to pass the ContextActor (ContextActor, UFVQuestSubsystem)
        UFVQuestSubsystem QuestSub = Cast<UFVQuestSubsystem>(Subsystem::GetGameInstanceSubsystem(UFVQuestSubsystem));

        if (QuestSub != nullptr)
        {
            QuestSub.NotifyTalkedToNPC(TalkConfig.NPCId, DialogueNodeId);
        }
    }
}
