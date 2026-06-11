// Abstract mini-game handler base.
// Subclass this (in AngelScript or Blueprint) to add concrete mini-game logic.
//
// Designer workflow:
//   1. Subclass UFVMiniGameHandlerConfig and add timing/difficulty parameters
//   2. Subclass UFVMiniGameHandler and implement OnStartMiniGame / OnMiniGameInput
//   3. Assign your subclass as the HandlerClass on the interactable action slot

class UFVMiniGameHandlerConfig : UFVInteractionHandlerConfig
{
    // Number of attempts before permanent failure (0 = infinite)
    UPROPERTY(Category = "MiniGame")
    int MaxAttempts = 3;

    UPROPERTY(Category = "MiniGame")
    float TimeLimit = 30.f;

    // Tag sent on success (on top of GrantedTagsOnSuccess from action)
    UPROPERTY(Category = "MiniGame")
    FGameplayTag SuccessTag;

    // Tag sent on permanent failure
    UPROPERTY(Category = "MiniGame")
    FGameplayTag FailureTag;
}

class UFVMiniGameHandler : UFVInteractionActionHandler
{
    UFVMiniGameHandlerConfig MiniGameConfig;
    FFVInteractionContext MiniGameContext;
    int AttemptsUsed = 0;
    bool bMiniGameActive = false;

    UFUNCTION(BlueprintOverride)
    void OnInitialize(UFVInteractionHandlerConfig Config)
    {
        MiniGameConfig = Cast<UFVMiniGameHandlerConfig>(Config);
        if (MiniGameConfig == nullptr)
        {
            Warning(GetName() + ": no UFVMiniGameHandlerConfig assigned.");
            MiniGameConfig = NewObject(this, UFVMiniGameHandlerConfig);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnExecute(FFVInteractionContext Context)
    {
        MiniGameContext = Context;
        AttemptsUsed = 0;
        bMiniGameActive = true;
        OnStartMiniGame(Context, MiniGameConfig);
    }

    UFUNCTION(BlueprintOverride)
    void OnCancel(FFVInteractionContext Context)
    {
        bMiniGameActive = false;
        OnMiniGameCancelled(Context);
    }

    //~=========================================================================
    // Implement these in subclasses
    //~=========================================================================

    // Show the mini-game UI and begin the interaction
    UFUNCTION(BlueprintEvent)
    void OnStartMiniGame(FFVInteractionContext Context, UFVMiniGameHandlerConfig Config)
    {
    }

    // The player hit the timing window or performed the required input
    UFUNCTION(BlueprintEvent)
    void OnMiniGameInput(bool bHitWindow)
    {
    }

    UFUNCTION(BlueprintEvent)
    void OnMiniGameCancelled(FFVInteractionContext Context)
    {
    }

    //~=========================================================================
    // Call from the mini-game widget / subclass
    //~=========================================================================

    // Attempt result — true if the player hit the window correctly
    UFUNCTION(BlueprintCallable)
    void ReportAttemptResult(bool bSuccess)
    {
        if (!bMiniGameActive)
        {
            return;
        }

        if (bSuccess)
        {
            bMiniGameActive = false;

            if (MiniGameConfig.SuccessTag.IsValid())
            {
                AbilitySystem::SendGameplayEventToActor(
                    MiniGameContext.Instigator,
                    MiniGameConfig.SuccessTag,
                    FGameplayEventData());
            }

            CompleteExecution(true);
            return;
        }

        AttemptsUsed++;

        bool bOutOfAttempts = MiniGameConfig.MaxAttempts > 0 && AttemptsUsed >= MiniGameConfig.MaxAttempts;

        if (bOutOfAttempts)
        {
            bMiniGameActive = false;

            if (MiniGameConfig.FailureTag.IsValid())
            {
                AbilitySystem::SendGameplayEventToActor(
                    MiniGameContext.Instigator,
                    MiniGameConfig.FailureTag,
                    FGameplayEventData());
            }

            CompleteExecution(false);
        }
        // else: still has attempts remaining — UI continues
    }
}
