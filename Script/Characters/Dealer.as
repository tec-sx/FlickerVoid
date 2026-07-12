// ─────────────────────────────────────────────────────────────────────────────
// ADealer.as
//
// Sample NPC — "Deale", the merchant from the worked examples.
// Demonstrates:
//   • Registering DT_DialogueLines with UDialogueDatabaseSubsystem on spawn
//   • Sphere overlap → greeting signal
//   • Player interaction key → interact signal  
//   • Responding to EventOnPlay / EventOnComplete gameplay tags via GAS
//   • Farewell on the player leaving radius
//   • Combat state preventing dialogue
//   • Tearing down registration on destroy (level streaming safe)
// ─────────────────────────────────────────────────────────────────────────────

class ADealer : AFVAICharacter
{
    // ── Editable Config (set per-NPC in Blueprint defaults) ───────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTag SpeakerID = GameplayTags::Dialogue_Speaker_Dealer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSoftObjectPtr<UDataTable> DialogueLineTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float GreetingCooldownSeconds = 12.f;

    /**
     * Personality and state tags granted to this NPC's ASC at spawn.
     * These feed into FDialogueContext.TargetTags and are read by
     * FTagRequirementCondition rows in the data table.
     *
     * Example defaults for Deale:
     *   Personality.Friendly
     *   NPC.Role.Merchant
     *   NPC.State.Idle
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FGameplayTagContainer InitialPersonalityTags;

    // ── Internal State ────────────────────────────────────────────────────────

    private float LastGreetingTimestamp = -999.f;
    private bool  bPlayerInGreetingRadius = false;
    private bool  bIsInCombat = false;

    // ─────────────────────────────────────────────────────────────────────────
    // Lifecycle
    // ─────────────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        InitialPersonalityTags.AddTag(GameplayTags::Dialogue_Speaker_Dealer);

        // 1. Grant personality tags to the ASC so condition rows can read them
        if (InitialPersonalityTags.IsValid())
        {
            AbilitySystem::AddLooseGameplayTags(this, InitialPersonalityTags);
        }

        // 2. Register this NPC's line table with the database subsystem.
        //    Safe to call even if the table is a soft reference not yet loaded —
        //    the subsystem handles async load internally.
        RegisterDialogueTable();

        AFVAICharacterController AIC = Cast<AFVAICharacterController>(GetController());
        
        if (AIC != nullptr)
        {
            UFVStateTreeAIComponent StateTreeComponent = AIC.GetComponentByClass(UFVStateTreeAIComponent);
        
            if (StateTreeComponent != nullptr)
            {
            }
        }
    }

    UFUNCTION()
    private void StartPatrol(AActor Actor, const FAIStimulus&in Stimulus)
    {
        Print("OnStartPatrol");
        AFVAICharacterController AIC = Cast<AFVAICharacterController>(GetController());
        
        if (AIC != nullptr)
        {
            UFVStateTreeAIComponent StateTreeComponent = AIC.GetComponentByClass(UFVStateTreeAIComponent);
        
            if (StateTreeComponent != nullptr)
            {
                Print("Patrol");
                // StateTreeComponent.SendEvent(GameplayTags::AI_State_Event_Patrol);
            }
        }
    }

    UFUNCTION(BlueprintOverride)
    void EndPlay(EEndPlayReason EndPlayReason)
    {
        // Unregister when NPC despawns / level unloads — prevents stale row
        // pointers surviving in the database index after the UDataTable GCs.
        UnregisterDialogueTable();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Table Registration
    // ─────────────────────────────────────────────────────────────────────────

    private void RegisterDialogueTable()
    {
        UFVDialogueDatabaseSubsystem DB = Subsystem::GetGameInstanceSubsystem(UFVDialogueDatabaseSubsystem);
        if (DB == nullptr || DialogueLineTable.IsNull())
        {
            PrintWarning(f"DealeNPC: DialogueLineTable not assigned on {GetName()} — no lines will play.");
            return;
        }

        FFVDialogueTableRegistration Reg;
        Reg.LineTable = DialogueLineTable;
        // Narrow this registration to only Dealer's speaker tag so the DB
        // doesn't scan this table for signals from other speakers.
        Reg.RestrictedToSpeakerIDs.AddTag(SpeakerID);

        DB.RegisterLineTable(Reg);
        Print(f"DealeNPC: Registered dialogue table '{DialogueLineTable.GetAssetName()}'");
    }

    private void UnregisterDialogueTable()
    {
        UFVDialogueDatabaseSubsystem DB = Subsystem::GetGameInstanceSubsystem(UFVDialogueDatabaseSubsystem);
        UDataTable LoadedTable = DialogueLineTable.Get();
        if (DB != nullptr && LoadedTable != nullptr)
        {
            DB.UnregisterLineTable(LoadedTable);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Greeting Sphere (larger — NPC "notices" player at distance)
    // ─────────────────────────────────────────────────────────────────────────

    UFUNCTION()
    void OnGreetingSphereEnter(
        UPrimitiveComponent OverlappedComp,
        AActor OtherActor,
        UPrimitiveComponent OtherComp,
        int OtherBodyIndex,
        bool bFromSweep,
        const FHitResult&in SweepResult)
    {
        if (!IsPlayerPawn(OtherActor)) return;
        bPlayerInGreetingRadius = true;
        TryFireGreeting(OtherActor);
    }

    UFUNCTION()
    void OnGreetingSphereExit(
        UPrimitiveComponent OverlappedComp,
        AActor OtherActor,
        UPrimitiveComponent OtherComp,
        int32 OtherBodyIndex)
    {
        if (!IsPlayerPawn(OtherActor)) return;
        bPlayerInGreetingRadius = false;
        TryFireFarewell(OtherActor);
    }

    private void TryFireGreeting(AActor PlayerActor)
    {
        // Guard: combat blocks dialogue
        if (bIsInCombat)
        {
            Print("DealeNPC: Suppressing greeting — NPC is in combat.");
            return;
        }

        // Guard: cooldown prevents greeting spam
        float Now = GetWorld().GetTimeSeconds();
        if (Now - LastGreetingTimestamp < GreetingCooldownSeconds) return;

        LastGreetingTimestamp = Now;
        BroadcastSignal(GameplayTags::Dialogue_Signal_Greeting, PlayerActor);
    }

    private void TryFireFarewell(AActor PlayerActor)
    {
        // Only fire farewell if we actually greeted recently (avoids cold farewell
        // firing when the NPC spawns inside the player's existing radius)
        float Now = GetWorld().GetTimeSeconds();
        if (Now - LastGreetingTimestamp > 60.f) return;

        // Don't fire a farewell signal if a session is already active —
        // the session is controlling itself; the NPC component shouldn't
        // interrupt it just because the player took a step backward.
        UFVDialogueSubsystem DS = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);
        if (DS != nullptr && DS.IsSessionActive()) return;

        BroadcastSignal(GameplayTags::Dialogue_Signal_Farewell, PlayerActor);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Interact Sphere (smaller — player presses E to start conversation)
    // ─────────────────────────────────────────────────────────────────────────

    // UFUNCTION(BlueprintOverride)
    // void OnInteractionSphereEnter(
    //     UPrimitiveComponent OverlappedComp,
    //     AActor OtherActor,
    //     UPrimitiveComponent OtherComp,
    //     int32 OtherBodyIndex,
    //     bool bFromSweep,
    //     const FHitResult&in SweepResult)
    // {
    //     if (!IsPlayerPawn(OtherActor)) return;

    //     AFVPlayerCharacter Player = Cast<AFVPlayerCharacter>(OtherActor);
    //     if (Player != nullptr)
    //     {
    //         Player.DialogueComponent.SetFocusedNPC(this);
    //     }
    // }

    // UFUNCTION(BlueprintOverride)
    // void OnInteractionSphereExit(
    //     UPrimitiveComponent OverlappedComp,
    //     AActor OtherActor,
    //     UPrimitiveComponent OtherComp,
    //     int32 OtherBodyIndex)
    // {
    //     if (!IsPlayerPawn(OtherActor)) return;
    //     AFVPlayerCharacter Player = Cast<AFVPlayerCharacter>(OtherActor);
    //     if (Player != nullptr)
    //     {
    //         Player.DialogueComponent.ClearFocusedNPC(this);
    //     }
    // }

    /**
     * Called by APlayerNPC.DialogueComponent when the player presses the
     * interact key and this NPC is the focused interactable.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnPlayerInteract(AActor InstigatorActor)
    {
        // bForceInterrupt = true: pressing E mid-session restarts from Signal.Interact.
        // This lets the player deliberately open a fresh conversation
        // even if a greeting was already running.
        BroadcastSignal(GameplayTags::Dialogue_Signal_Interact, InstigatorActor, true);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Signals from the world (called externally, not by player overlap)
    // ─────────────────────────────────────────────────────────────────────────

    /**
     * Called by your quest system when the player delivers a quest item to
     * this NPC, or by any external system that wants to trigger a specific
     * signal (cutscene director, AI manager, etc.).
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void FireSignal(FGameplayTag SignalTag, AActor InstigatorActor, bool bForceInterrupt = false)
    {
        BroadcastSignal(SignalTag, InstigatorActor, bForceInterrupt);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // GAS event listener — reacts to EventOnPlay / EventOnComplete from lines
    //
    // When FDialoguePlayLineTask fires EventOnPlay with a tag like
    // Event.Dialogue.QuestGiven, the GAS event arrives here via a
    // registered gameplay event listener on the ASC.
    //
    // Wire this up in BeginPlay via:
    //   AbilitySystem.AddGameplayEventTagContainerDelegate(Container, Delegate)
    // or a Gameplay Ability that listens for the tag.
    // ─────────────────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintCallable, Category = "Dialogue|Events")
    void OnDialogueEvent(FGameplayTag EventTag, FGameplayEventData EventData)
    {
        // GameplayTags::Dialogue_Event_Quest
        // if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(n"Event.Dialogue.QuestGiven")))
        // {
        //     // Example: mark this NPC as "has given quest" so follow-up lines
        //     // that check for Personality.QuestGiven pass their condition
        //     AbilitySystem::AddLooseGameplayTags(this, FGameplayTag::RequestGameplayTag(n"NPC.State.QuestGiven"));
        // }
        // else if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(n"Event.Dialogue.ItemRequested")))
        // {
        //     // Enter a waiting state — data table rows gated on NPC.State.WaitingForItem
        //     // will now pass their tag condition
        //     AbilitySystem::AddLooseGameplayTags(this,FGameplayTag::RequestGameplayTag(n"NPC.State.WaitingForItem"));
        //     AbilitySystem::RemoveLooseGameplayTags(this,FGameplayTag::RequestGameplayTag(n"NPC.State.Idle"));
        // }
        // else if (EventTag.MatchesTag(FGameplayTag::RequestGameplayTag(n"Event.Dialogue.DebtRepaid")))
        // {
        //     AbilitySystem::RemoveLooseGameplayTags(this,FGameplayTag::RequestGameplayTag(n"NPC.State.WaitingForItem"));
        //     AbilitySystem::AddLooseGameplayTags(this,FGameplayTag::RequestGameplayTag(n"NPC.State.Satisfied"));
        //     Print("DealeNPC: Debt repaid — NPC state updated to Satisfied.");
        // }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Internal helpers
    // ─────────────────────────────────────────────────────────────────────────

    private void BroadcastSignal(FGameplayTag SignalTag, AActor InstigatorActor, bool bForceInterrupt = false)
    {
        UFVDialogueSubsystem DS = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);
        if (DS == nullptr) return;

        if (!SpeakerID.IsValid())
        {
            PrintWarning(f"DealeNPC: SpeakerID not set on {GetName()} — signal dropped.");
            return;
        }

        FFVDialogueSignal Signal;
        Signal.Signal     = SignalTag;
        Signal.Speaker = InstigatorActor;
        Signal.Listener     = this;

        DS.BroadcastSignal(Signal, bForceInterrupt);
    }

    private bool IsPlayerPawn(AActor Actor)
    {
        APlayerController PC = Cast<APlayerController>(Gameplay::GetPlayerController(0));
        return (PC != nullptr && PC.GetControlledPawn() == Actor);
    }

    private void Print(FString Msg)
    {
        System::PrintString(Msg, true, true, FLinearColor::Green, 3.f);
    }

    private void PrintWarning(FString Msg)
    {
        System::PrintString(Msg, true, true, FLinearColor::Yellow, 5.f);
    }
}
