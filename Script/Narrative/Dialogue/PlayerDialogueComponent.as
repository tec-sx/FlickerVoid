const FString OnSessionLineStartedFunction = "OnSessionLineStarted";
const FString OnSessionEndedFunction = "OnSessionEnded";

// ═════════════════════════════════════════════════════════════════════════════
// UPlayerDialogueComponent
// Attach to the player character. Contains all dialogue logic so
// APlayerNPC stays focused on movement/combat concerns.
// ═════════════════════════════════════════════════════════════════════════════
class UPlayerDialogueComponent : UActorComponent
{
    // ── State ─────────────────────────────────────────────────────────────────

    // Stack of NPCs in interact range — top of stack is the active focus.
    // Using a stack rather than a single ref handles the edge case where
    // the player is simultaneously in range of two NPCs (e.g. near a doorway):
    // the most recently entered one is preferred, and the previous one
    // becomes the focus again if the player backs away.
    private TArray<AActor> InteractCandidates;

    // The NPC the player is currently focused on (top of candidate stack)
    private AActor FocusedNPC;

    // Whether player movement input should be suppressed during dialogue
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bLockMovementDuringDialogue = true;

    // ── Lifecycle ─────────────────────────────────────────────────────────────

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        BindToSubsystem();
    }

    UFUNCTION(BlueprintOverride)
    void EndPlay(EEndPlayReason Reason)
    {
        UnbindFromSubsystem();
    }

    // ── Focus System ──────────────────────────────────────────────────────────

    /**
     * Called by ADealeNPC (or any NPC) when the player enters their
     * interact sphere.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void SetFocusedNPC(AActor NPC)
    {
        // Don't add duplicates (can happen on sphere re-registration)
        if (InteractCandidates.Contains(NPC)) return;

        InteractCandidates.Add(NPC);
        RefreshFocus();
    }

    /**
     * Called by ADealeNPC when the player leaves their interact sphere.
     * Passing the specific NPC lets us handle multi-NPC radius overlap safely.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void ClearFocusedNPC(AActor NPC)
    {
        InteractCandidates.Remove(NPC);
        RefreshFocus();
    }

    /** Returns the NPC the player is currently looking at / closest to. */
    UFUNCTION(BlueprintPure, Category = "Dialogue")
    AActor GetFocusedNPC() const
    {
        return FocusedNPC;
    }

    UFUNCTION(BlueprintPure, Category = "Dialogue")
    bool HasFocusedNPC() const
    {
        return FocusedNPC != nullptr;
    }

    private void RefreshFocus()
    {
        for (int i = InteractCandidates.Num() - 1; i >= 0; i--)
        {
            if (!IsValid(InteractCandidates[i]))
            {
                InteractCandidates.RemoveAt(i);
            }
        }

        AActor Previous = FocusedNPC;
        FocusedNPC = InteractCandidates.Num() > 0 ? InteractCandidates.Last() : nullptr;

        if (FocusedNPC != Previous)
        {
            OnFocusChanged(Previous, FocusedNPC);
        }
    }

    // Blueprint event so the HUD widget can show/hide the interact prompt
    UFUNCTION(BlueprintEvent, Category = "Dialogue")
    void OnFocusChanged(AActor PreviousFocus, AActor NewFocus) {}

    // ── Input ──────────────────────────────────────────────────────────────

    /**
     * Bind this to your "Interact" input action (E key or gamepad face button).
     * Called from APlayerNPC's input setup.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void OnInteractPressed()
    {
        UFVDialogueSubsystem DS = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);
        if (DS == nullptr) return;

        // If a session is already active, pressing interact dismisses the
        // current line (works like "continue / skip" in dialogue UIs)
        if (DS.IsSessionActive())
        {
            DS.SubmitLineDismissed();
            return;
        }

        // Otherwise, start dialogue with the focused NPC
        if (FocusedNPC == nullptr)
        {
            return; // No NPC in range — silently ignore
        }

        // Route through the NPC's own OnPlayerInteract so the NPC has a
        // chance to handle the signal (validate state, pick signal tag, etc.)
        ADealer DealeActor = Cast<ADealer>(FocusedNPC);
        if (DealeActor != nullptr)
        {
            DealeActor.OnPlayerInteract(GetOwner());
            return;
        }

        // Fallback for any NPC that doesn't have a typed cast —
        // fire Signal.Interact directly. Works for generic NPCs.
        FFVDialogueSignal Signal;
        Signal.SpeakerID  = GameplayTags::Dialogue_Speaker_Player;
        Signal.Signal     = GameplayTags::Dialogue_Signal_Interact;
        Signal.Speaker    = GetOwner();
        Signal.Listener   = FocusedNPC;
        DS.BroadcastSignal(Signal, false);
    }

    // ── Subsystem Delegate Binding ────────────────────────────────────────────

    private void BindToSubsystem()
    {
        UFVDialogueSubsystem DS = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);
        if (DS == nullptr) return;

        DS.OnLineStarted.AddUFunction(this, FName(OnSessionLineStartedFunction));
        DS.OnSessionEnded.AddUFunction(this, FName(OnSessionEndedFunction));
    }

    private void UnbindFromSubsystem()
    {
        UFVDialogueSubsystem DS = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);
        if (DS == nullptr) return;

        DS.OnLineStarted.Unbind(this, FName(OnSessionLineStartedFunction));
        DS.OnSessionEnded.Unbind(this, FName(OnSessionEndedFunction));
    }

    // ── Subsystem Event Handlers ──────────────────────────────────────────────

    UFUNCTION()
    void OnSessionLineStarted(FName LineID, const FFVDialogueLineRow&in LineRow, const FFVDialogueContext&in Context)
    {
        // Only care about sessions where this player is the instigator
        if (Context.Speaker.Get() != GetOwner()) return;

        if (bLockMovementDuringDialogue)
        {
            LockPlayerMovement();
        }

        // Blueprint event — lets the BP child class do camera rig transitions,
        // NPC look-at anim notify, etc.
        BP_OnDialogueStarted(Context.Listener.Get(), Context.SpeakerID);
    }

    UFUNCTION()
    void OnSessionEnded()
    {
        UnlockPlayerMovement();
        BP_OnDialogueEnded();
    }

    // ── Movement Lock ─────────────────────────────────────────────────────────

    private void LockPlayerMovement()
    {
        APawn Pawn = Cast<APawn>(GetOwner());
        if (Pawn == nullptr) return;

        APlayerController PC = Cast<APlayerController>(Pawn.GetController());
        if (PC == nullptr) return;

        // Disable movement input — player can still look around
        // Adjust to your input component / Enhanced Input setup
        PC.SetIgnoreMoveInput(true);

        // UIMode.SetWidgetToFocus(...) — set your dialogue widget here if using mouse
        Widget::SetInputMode_UIOnlyEx(PC);
        PC.bShowMouseCursor = true;
    }

    private void UnlockPlayerMovement()
    {
        APawn Pawn = Cast<APawn>(GetOwner());
        if (Pawn == nullptr) return;

        APlayerController PC = Cast<APlayerController>(Pawn.GetController());
        if (PC == nullptr) return;

        Widget::SetInputMode_GameOnly(PC);
        PC.SetIgnoreMoveInput(false);
        PC.bShowMouseCursor = false;
    }

    // ── Blueprint Events (override in BP child class) ─────────────────────────

    /** Override to start a camera rig blend, show dialogue nameplate, etc. */
    UFUNCTION(BlueprintEvent, Category = "Dialogue")
    void BP_OnDialogueStarted(AActor TargetNPC, FGameplayTag SpeakerID) {}

    /** Override to end camera rig blend, reset NPC look-at, etc. */
    UFUNCTION(BlueprintEvent, Category = "Dialogue")
    void BP_OnDialogueEnded() {}

}