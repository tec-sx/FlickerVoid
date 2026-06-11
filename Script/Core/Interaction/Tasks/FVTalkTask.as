// NPC conversation task.
//
// Sync vs async:
//   ConversationId set  → structured conversation, stays Executing until OnConversationEnded.
//   ConversationId empty → single bark, completes immediately.
//
// Quest integration:
//   UFVQuestSubsystem::NotifyTalkedToNPC is called after every conversation ends.
//
// Configure in the State Tree editor:
//   InitialSignal  — signal name in the dialogue DataTable (e.g. n"Greeting")
//   ConversationId — leave empty for a bark; set for a structured conversation
//   NPCId          — passed to the quest subsystem

class UFVTalkTask : UFVInteractionStateTreeTaskBase
{
    UPROPERTY(EditAnywhere, Category = "Dialogue")
    FName InitialSignal = n"Greeting";

    UPROPERTY(EditAnywhere, Category = "Dialogue")
    FName ConversationId;

    UPROPERTY(EditAnywhere, Category = "Quest")
    FName NPCId;

    // Cached at ReceiveEnterState — needed by async callback which has no OwnerActor param
    AActor CachedOwnerActor;
    bool bIsConversation = false;

    UFUNCTION(BlueprintOverride)
    EStateTreeStateChangeType ReceiveEnterState(AActor OwnerActor,
        EStateTreeActionType ActionType, FStateTreeTransitionResult Transition)
    {
        CachedOwnerActor = OwnerActor;

        UFVDialogueSubsystem DialogueSub = GetDialogueSubsystem();
        if (DialogueSub == nullptr)
        {
            Warning(GetName() + ": UFVDialogueSubsystem not found.");
            CompleteTask(OwnerActor, false);
            return EStateTreeStateChangeType::Changed;
        }

        FFVDialogueContext DialogCtx;
        DialogCtx.SpeakerActor  = OwnerActor;
        DialogCtx.ListenerActor = GetInstigator(OwnerActor);

        if (ConversationId != n"None" && ConversationId != n"")
        {
            bIsConversation = true;
            DialogueSub.OnConversationEnded.AddUFunction(this, n"HandleConversationEnded");
            DialogueSub.StartConversation(ConversationId, InitialSignal, DialogCtx);
        }
        else
        {
            DialogueSub.SendSignalFromActor(InitialSignal, OwnerActor, GetInstigator(OwnerActor));
            NotifyQuestSubsystem(n"None");
            CompleteTask(OwnerActor, true);
        }

        return EStateTreeStateChangeType::Changed;
    }

    UFUNCTION(BlueprintOverride)
    void ReceiveExitState(AActor OwnerActor,
        EStateTreeActionType ActionType, FStateTreeTransitionResult Transition)
    {
        if (bIsConversation)
        {
            UFVDialogueSubsystem DialogueSub = GetDialogueSubsystem();
            if (DialogueSub != nullptr)
            {
                DialogueSub.OnConversationEnded.Unbind(this, n"HandleConversationEnded");
                DialogueSub.EndConversation();
            }
            bIsConversation = false;
        }
        CachedOwnerActor = nullptr;
    }

    UFUNCTION()
    void HandleConversationEnded()
    {
        UFVDialogueSubsystem DialogueSub = GetDialogueSubsystem();
        if (DialogueSub != nullptr)
        {
            DialogueSub.OnConversationEnded.Unbind(this, n"HandleConversationEnded");
        }

        bIsConversation = false;
        NotifyQuestSubsystem(n"None");
        CompleteTask(CachedOwnerActor, true);
        CachedOwnerActor = nullptr;
    }

    // =========================================================================
    // Helpers
    // =========================================================================

    private UFVDialogueSubsystem GetDialogueSubsystem()
    {
        return Cast<UFVDialogueSubsystem>(Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem));
    }

    private void NotifyQuestSubsystem(FName DialogueNodeId)
    {
        UFVQuestSubsystem QuestSub = Cast<UFVQuestSubsystem>(
            Subsystem::GetGameInstanceSubsystem(UFVQuestSubsystem));

        if (QuestSub != nullptr)
        {
            QuestSub.NotifyTalkedToNPC(NPCId, DialogueNodeId);
        }
    }
}
