// Lockpick mini-game — a concrete UFVMiniGameHandler subclass.
//
// Timing model: a slider moves back and forth; the player must press the action
// key when the slider is inside a "sweet spot" zone. Each successful hit advances
// a pick tension bar. Three failed hits break the pick (one pick consumed from inventory).
//
// The actual widget is implemented in Blueprint/UMG; this handler
// drives it via events and listens for ReportAttemptResult.

class UFVLockpickHandlerConfig : UFVMiniGameHandlerConfig
{
    // Width of the sweet spot zone (0–1 normalized slider range)
    UPROPERTY(Category = "Lockpick")
    float SweetSpotWidth = 0.15f;

    // Slider oscillation speed (full traversal per second)
    UPROPERTY(Category = "Lockpick")
    float SliderSpeed = 1.2f;

    // Hits needed to open the lock
    UPROPERTY(Category = "Lockpick")
    int HitsRequired = 3;

    // Tag that identifies the lockpick item in inventory (consumed on each broken pick)
    UPROPERTY(Category = "Lockpick")
    FGameplayTag LockpickItemTag;
}

class UFVLockpickHandler : UFVMiniGameHandler
{
    UFVLockpickHandlerConfig LockpickConfig;
    int SuccessfulHits = 0;
    float SliderPosition = 0.f;  // 0–1
    float SliderDirection = 1.f;
    float SweetSpotStart = 0.f;
    bool bSliderRunning = false;

    UFUNCTION(BlueprintOverride)
    void OnInitialize(UFVInteractionHandlerConfig Config)
    {
        LockpickConfig = Cast<UFVLockpickHandlerConfig>(Config);
        if (LockpickConfig == nullptr)
        {
            Warning(GetName() + ": no UFVLockpickHandlerConfig assigned.");
            LockpickConfig = NewObject(this, UFVLockpickHandlerConfig);
        }

        // Also initialize the parent with the same config
        Super::OnInitialize(Config);
        MiniGameConfig = LockpickConfig;
    }

    UFUNCTION(BlueprintOverride)
    void OnStartMiniGame(FFVInteractionContext Context, UFVMiniGameHandlerConfig Config)
    {
        SuccessfulHits = 0;
        SliderPosition = 0.f;
        SliderDirection = 1.f;
        bSliderRunning = true;

        // Randomize sweet spot position within the safe range
        SweetSpotStart = Math::RandRange(0.05f, 0.95f - LockpickConfig.SweetSpotWidth);

        // Send event to open the lockpick widget
        FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(n"Interaction.Event.LockpickStarted");
        FGameplayEventData EventData;
        EventData.Instigator = Context.Instigator;
        EventData.Target     = Context.TargetActor;

        AbilitySystem::SendGameplayEventToActor(Context.Instigator, EventTag, EventData);
    }

    UFUNCTION(BlueprintOverride)
    void OnMiniGameCancelled(FFVInteractionContext Context)
    {
        bSliderRunning = false;
        SendCloseEvent(Context);
    }

    // Tick forwarded from the UMG widget each frame (widget calls this)
    UFUNCTION(BlueprintCallable)
    void TickSlider(float DeltaTime)
    {
        if (!bSliderRunning)
        {
            return;
        }

        SliderPosition += SliderDirection * LockpickConfig.SliderSpeed * DeltaTime;

        if (SliderPosition >= 1.f)
        {
            SliderPosition = 1.f;
            SliderDirection = -1.f;
        }
        else if (SliderPosition <= 0.f)
        {
            SliderPosition = 0.f;
            SliderDirection = 1.f;
        }
    }

    // The player pressed the pick key — evaluate hit/miss
    UFUNCTION(BlueprintCallable)
    void PlayerPressedPick()
    {
        if (!bSliderRunning)
        {
            return;
        }

        bool bInWindow = SliderPosition >= SweetSpotStart &&
                         SliderPosition <= SweetSpotStart + LockpickConfig.SweetSpotWidth;

        if (bInWindow)
        {
            SuccessfulHits++;

            if (SuccessfulHits >= LockpickConfig.HitsRequired)
            {
                // Lock is open
                bSliderRunning = false;
                ReportAttemptResult(true);
            }
            else
            {
                // Good hit but not done yet — randomize sweet spot for next pin
                SweetSpotStart = Math::RandRange(0.05f, 0.95f - LockpickConfig.SweetSpotWidth);
                OnMiniGameInput(true);
            }
        }
        else
        {
            // Missed — consumed one attempt
            OnMiniGameInput(false);
            ReportAttemptResult(false);
        }
    }

    // Normalized slider position for the widget (0–1)
    UFUNCTION(BlueprintCallable)
    float GetSliderPosition() const { return SliderPosition; }

    UFUNCTION(BlueprintCallable)
    float GetSweetSpotStart() const { return SweetSpotStart; }

    UFUNCTION(BlueprintCallable)
    float GetSweetSpotWidth() const { return LockpickConfig.SweetSpotWidth; }

    void SendCloseEvent(FFVInteractionContext Context)
    {
        AbilitySystem::SendGameplayEventToActor(
            Context.Instigator,
            FGameplayTag::RequestGameplayTag(n"Interaction.Event.LockpickEnded"),
            FGameplayEventData());
    }
}
