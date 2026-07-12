// ═════════════════════════════════════════════════════════════════════════════
// 3. UDialogueHUDManager
//
// Lives on the HUD class (or GameMode, or as a component on a HUD actor).
// Owns the widget lifecycle: creates it once, shows/hides it per session,
// and is the single point that binds to UDialogueSubsystem delegates.
//
// Usage in your HUD BeginPlay:
//   DialogueManager = NewObject<UDialogueHUDManager>(this);
//   DialogueManager.Initialize(DialogueWidgetClass, GetOwningPlayerController());
// ═════════════════════════════════════════════════════════════════════════════
class UDialogueHUDManager : UObject
{
    // ── Config ────────────────────────────────────────────────────────────────

    /**
     * The Blueprint widget class to instantiate.
     * Set this in your HUD Blueprint defaults to WBP_DialogueWidget.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    TSubclassOf<UDialogueWidget> DialogueWidgetClass;

    // ── Internal ──────────────────────────────────────────────────────────────

    private UDialogueWidget DialogueWidget;
    private UFVDialogueSubsystem DialogueSys;
    private APlayerController OwningController;

    // ── Setup ─────────────────────────────────────────────────────────────────

    /**
     * Call this from your HUD's BeginPlay (or equivalent).
     * Creates the widget, adds it to the viewport, and binds all delegates.
     */
    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void Initialize(TSubclassOf<UDialogueWidget> InWidgetClass, APlayerController InController)
    {
        OwningController = InController;

        if (InWidgetClass != nullptr)
        {
            DialogueWidgetClass = InWidgetClass;
        }

        if (DialogueWidgetClass == nullptr)
        {
            System::PrintString(
				"DialogueHUDManager: No DialogueWidgetClass set — dialogue UI will not appear.",
                true,
				true,
				FLinearColor::Red,
				10.f);
            return;
        }

        // Create and add to viewport at a high Z-order so it renders above the HUD
        DialogueWidget = Cast<UDialogueWidget>(WidgetBlueprint::CreateWidget(DialogueWidgetClass, InController));

        if (DialogueWidget == nullptr)
        {
            System::PrintString(
				"DialogueHUDManager: Failed to create DialogueWidget.",
                true,
				true,
				FLinearColor::Red,
				10.f);
            return;
        }

        DialogueWidget.AddToViewport(100); // Z-order 100 — above most HUD elements

        // Wire up the subsystem
        DialogueSys = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);
        if (DialogueSys == nullptr)
        {
            System::PrintString(
				"DialogueHUDManager: UFVDialogueSubsystem not found.",
                true,
				true,
				FLinearColor::Red,
				10.f);
            return;
        }

        DialogueWidget.Initialize(DialogueSys);
        BindDelegates();

        System::PrintString(
			"DialogueHUDManager: Initialized successfully.",
			true,
			true,
			FLinearColor::Green,
			3.f);
    }

    UFUNCTION(BlueprintCallable, Category = "Dialogue")
    void Deinitialize()
    {
        UnbindDelegates();

        if (DialogueWidget != nullptr)
        {
            DialogueWidget.RemoveFromParent();
            DialogueWidget = nullptr;
        }
    }

    // ── Delegate Binding ──────────────────────────────────────────────────────

    private void BindDelegates()
    {
        if (DialogueSys == nullptr) return;

        DialogueSys.OnLineStarted.AddUFunction(this, n"HandleLineStarted");
        DialogueSys.OnResponsesReady.AddUFunction(this, n"HandleResponsesReady");
        DialogueSys.OnHideResponses.AddUFunction(this, n"HandleHideResponses");
        DialogueSys.OnSessionEnded.AddUFunction(this, n"HandleSessionEnded");
    }

    private void UnbindDelegates()
    {
        if (DialogueSys == nullptr) return;

        DialogueSys.OnLineStarted.Unbind(this, n"HandleLineStarted");
        DialogueSys.OnResponsesReady.Unbind(this, n"HandleResponsesReady");
        DialogueSys.OnHideResponses.Unbind(this, n"HandleHideResponses");
        DialogueSys.OnSessionEnded.Unbind(this, n"HandleSessionEnded");
    }

    // ── Delegate Handlers ─────────────────────────────────────────────────────

    /**
     * Fired by UDialogueSubsystem when a new NPC line is ready to display.
     * Forwards the full row and context to the widget.
     */
    UFUNCTION()
    void HandleLineStarted(FName LineID, const FFVDialogueLineRow&in LineRow, const FFVDialogueContext&in Context)
    {
        if (DialogueWidget == nullptr) return;

        DialogueWidget.ShowLine(LineRow, Context.SpeakerID);
        TryPlayVoiceAudio(LineRow, Context);
    }

    UFUNCTION()
    void ShowLine(const FFVDialogueLineRow& LineRow, const FGameplayTag& SpeakerID)
    {
        if (DialogueWidget == nullptr) return;
        DialogueWidget.ShowLine(LineRow, SpeakerID);
    }

    /**
     * Fired when response options are ready for the player to choose.
     * The widget builds and shows a button per entry.
     */
    UFUNCTION()
    void HandleResponsesReady(const TArray<FFVEvaluatedResponse>&in Responses)
    {
        if (DialogueWidget == nullptr) return;
        DialogueWidget.ShowResponses(Responses);
    }

    /**
     * Fired when the State Tree is done with the current response list
     * (player made a choice and the next line is incoming).
     */
    UFUNCTION()
    void HandleHideResponses()
    {
        if (DialogueWidget == nullptr) return;
        DialogueWidget.HideResponses();
    }

    /**
     * Fired when the dialogue session ends entirely (all branches resolved,
     * or the player chose a terminal response).
     */
    UFUNCTION()
    void HandleSessionEnded()
    {
        if (DialogueWidget == nullptr) return;
        DialogueWidget.HideWidget();

        // Notify Blueprint HUD so it can restore the game HUD layout
        BP_OnDialogueSessionEnded();
    }

    // ── Audio ─────────────────────────────────────────────────────────────────

    /**
     * Plays the voice-over for a line if a VoiceAsset is set.
     * Loads async and plays on the NPC's location for 3D spatialization.
     *
     * If your project uses MetaSounds or a dialogue audio manager,
     * replace the body with the appropriate API call.
     */
    private void TryPlayVoiceAudio(const FFVDialogueLineRow& LineRow, const FFVDialogueContext& Context)
    {
        if (LineRow.VoiceAsset.IsNull()) return;
        if (!Context.Listener.IsValid()) return;
        
        UFVAssetManager AssetMgr = Cast<UFVAssetManager>(UAssetManager::Get());
        if (AssetMgr == nullptr) return;
        
        AActor NPC = Context.Listener.Get();

        FOnSoundAssetLoaded Callback;
        Callback.BindUFunction(this, n"OnVoiceAssetLoaded");
        AssetMgr.AsyncLoadSound(LineRow.VoiceAsset, NPC, Callback);
    }

    UFUNCTION()
    private void OnVoiceAssetLoaded(USoundBase LoadedSound, UObject PayloadNPC)
    {
        // Asset is null when the soft ref was invalid or the cook failed —
        // silently skip; the line text is already displaying normally.
        if (LoadedSound == nullptr) return;

        // Payload is the NPC actor we passed in TryPlayVoiceAudio
        AActor NPC = Cast<AActor>(PayloadNPC);
        if (NPC == nullptr) return;

        // Play at NPC location — spatialized 3D audio
        Gameplay::PlaySoundAtLocation(
            LoadedSound,
            NPC.GetActorLocation(),
            NPC.GetActorRotation());

        // TODO: For mid-line skip support, use SpawnSoundAtLocation instead
        // and store the UAudioComponent on the HUDManager so you can call
        // AudioComp.Stop() when SubmitLineDismissed fires.
    }

    // ── Blueprint Events ──────────────────────────────────────────────────────

    /**
     * Override in your HUD Blueprint to restore any HUD elements that were
     * hidden while dialogue was active (minimap, health bar, etc.).
     */
    UFUNCTION(BlueprintEvent, Category = "Dialogue")
    void BP_OnDialogueSessionEnded() {}
}
