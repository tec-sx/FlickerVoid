// Abstract mini-game task base.
// Subclass to implement concrete mini-game logic.
//
// Designer workflow:
//   1. Subclass UFVMiniGameTask and add timing/difficulty UPROPERTYs
//   2. Implement OnStartMiniGame, OnMiniGameInput, OnMiniGameCancelled
//   3. Use this subclass as a task node in the action's State Tree
//
// Configure in the State Tree editor:
//   MaxAttempts — 0 = infinite
//   TimeLimit   — seconds (0 = no limit, NYI)
//   SuccessTag  — applied to instigator ASC on success (in addition to GrantedTagsOnSuccess)
//   FailureTag  — applied on permanent failure

class UFVMiniGameTask : UFVInteractionStateTreeTaskBase
{
    UPROPERTY(Category = "MiniGame")
    int MaxAttempts = 3;

    UPROPERTY(Category = "MiniGame")
    float TimeLimit = 30.f;

    UPROPERTY(Category = "MiniGame")
    FGameplayTag SuccessTag;

    UPROPERTY(Category = "MiniGame")
    FGameplayTag FailureTag;

    // Runtime
    AActor CachedOwnerActor;
    int AttemptsUsed = 0;
    bool bMiniGameActive = false;

    UFUNCTION(BlueprintOverride)
    EStateTreeStateChangeType ReceiveEnterState(AActor OwnerActor,
        EStateTreeActionType ActionType, FStateTreeTransitionResult Transition)
    {
        CachedOwnerActor = OwnerActor;
        AttemptsUsed = 0;
        bMiniGameActive = true;
        OnStartMiniGame(OwnerActor);
        return EStateTreeStateChangeType::Changed;
    }

    UFUNCTION(BlueprintOverride)
    void ReceiveExitState(AActor OwnerActor,
        EStateTreeActionType ActionType, FStateTreeTransitionResult Transition)
    {
        if (bMiniGameActive)
        {
            bMiniGameActive = false;
            OnMiniGameCancelled(OwnerActor);
        }
        CachedOwnerActor = nullptr;
    }

    //~=========================================================================
    // Implement in subclasses
    //~=========================================================================

    // Show the mini-game UI and begin the interaction
    UFUNCTION(BlueprintEvent)
    void OnStartMiniGame(AActor OwnerActor)
    {
    }

    // One timing attempt completed — bHitWindow true if the player hit the sweet spot
    UFUNCTION(BlueprintEvent)
    void OnMiniGameInput(bool bHitWindow)
    {
    }

    UFUNCTION(BlueprintEvent)
    void OnMiniGameCancelled(AActor OwnerActor)
    {
    }

    //~=========================================================================
    // Call from the mini-game widget
    //~=========================================================================

    // Attempt result — true if the player hit the window correctly.
    // Handles attempt counting and applies success/failure tags.
    UFUNCTION(BlueprintCallable)
    void ReportAttemptResult(bool bSuccess)
    {
        if (!bMiniGameActive)
        {
            return;
        }

        AActor Instigator = GetInstigator(CachedOwnerActor);

        if (bSuccess)
        {
            bMiniGameActive = false;

            if (SuccessTag.IsValid())
            {
                AbilitySystem::SendGameplayEventToActor(Instigator, SuccessTag, FGameplayEventData());
            }

            CompleteTask(CachedOwnerActor, true);
            return;
        }

        AttemptsUsed++;

        bool bOutOfAttempts = MaxAttempts > 0 && AttemptsUsed >= MaxAttempts;

        if (bOutOfAttempts)
        {
            bMiniGameActive = false;

            if (FailureTag.IsValid())
            {
                AbilitySystem::SendGameplayEventToActor(Instigator, FailureTag, FGameplayEventData());
            }

            CompleteTask(CachedOwnerActor, false);
        }
        // else: still has attempts remaining — UI continues
    }
}
