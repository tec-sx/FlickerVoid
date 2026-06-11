// // Lockpick mini-game — concrete UFVMiniGameTask subclass.
// //
// // A slider oscillates 0→1→0. The player presses the pick key when inside the
// // sweet spot zone. Each hit advances the pick tension. Three hits open the lock.
// // The slider ticks via ReceiveTick (State Tree drives it, not the widget).
// //
// // Configure in the State Tree editor (inherits MaxAttempts, SuccessTag, FailureTag):
// //   SweetSpotWidth  — normalized width of the hit zone (0–1)
// //   SliderSpeed     — full oscillation cycles per second
// //   HitsRequired    — successful hits needed to open the lock
// //   LockpickItemTag — inventory item consumed on a broken pick (NYI)

// class UFVLockpickTask : UFVMiniGameTask
// {
//     UPROPERTY(Category = "Lockpick")
//     float SweetSpotWidth = 0.15f;

//     UPROPERTY(Category = "Lockpick")
//     float SliderSpeed = 1.2f;

//     UPROPERTY(Category = "Lockpick")
//     int HitsRequired = 3;

//     UPROPERTY(Category = "Lockpick")
//     FGameplayTag LockpickItemTag;

//     // Runtime
//     int SuccessfulHits = 0;
//     float SliderPosition = 0.f;
//     float SliderDirection = 1.f;
//     float SweetSpotStart = 0.f;
//     bool bSliderRunning = false;

//     UFUNCTION(BlueprintOverride)
//     void OnStartMiniGame(AActor OwnerActor)
//     {
//         SuccessfulHits = 0;
//         SliderPosition = 0.f;
//         SliderDirection = 1.f;
//         bSliderRunning = true;
//         SweetSpotStart = Math::RandRange(0.05f, 0.95f - SweetSpotWidth);

//         FGameplayTag EventTag = FGameplayTag::RequestGameplayTag(n"Interaction.Event.LockpickStarted");
//         FGameplayEventData EventData;
//         EventData.Instigator = GetInstigator(OwnerActor);
//         EventData.Target     = OwnerActor;

//         AbilitySystem::SendGameplayEventToActor(GetInstigator(OwnerActor), EventTag, EventData);
//     }

//     UFUNCTION(BlueprintOverride)
//     void OnMiniGameCancelled(AActor OwnerActor)
//     {
//         bSliderRunning = false;
//         SendCloseEvent(OwnerActor);
//     }

//     // Override ReceiveTick to advance the slider each frame.
//     // Super call preserves the completion check from UFVInteractionStateTreeTaskBase.
//     UFUNCTION(BlueprintOverride)
//     EStateTreeRunStatus ReceiveTick(AActor OwnerActor, float DeltaTime)
//     {
//         // Check completion first (set by ReportAttemptResult via CompleteTask)
//         EStateTreeRunStatus BaseStatus = Super::ReceiveTick(OwnerActor, DeltaTime);
//         if (BaseStatus != EStateTreeRunStatus::Running)
//         {
//             bSliderRunning = false;
//             return BaseStatus;
//         }

//         // Advance slider
//         if (bSliderRunning)
//         {
//             SliderPosition += SliderDirection * SliderSpeed * DeltaTime;

//             if (SliderPosition >= 1.f)
//             {
//                 SliderPosition = 1.f;
//                 SliderDirection = -1.f;
//             }
//             else if (SliderPosition <= 0.f)
//             {
//                 SliderPosition = 0.f;
//                 SliderDirection = 1.f;
//             }
//         }

//         return EStateTreeRunStatus::Running;
//     }

//     // Called by the lockpick widget when the player presses the pick key
//     UFUNCTION(BlueprintCallable)
//     void PlayerPressedPick()
//     {
//         if (!bSliderRunning)
//         {
//             return;
//         }

//         bool bInWindow = SliderPosition >= SweetSpotStart &&
//                          SliderPosition <= SweetSpotStart + SweetSpotWidth;

//         if (bInWindow)
//         {
//             SuccessfulHits++;

//             if (SuccessfulHits >= HitsRequired)
//             {
//                 bSliderRunning = false;
//                 ReportAttemptResult(true);
//             }
//             else
//             {
//                 // Good hit — randomize sweet spot for the next pin
//                 SweetSpotStart = Math::RandRange(0.05f, 0.95f - SweetSpotWidth);
//                 OnMiniGameInput(true);
//             }
//         }
//         else
//         {
//             OnMiniGameInput(false);
//             ReportAttemptResult(false);
//         }
//     }

//     // Widget read-only accessors
//     UFUNCTION(BlueprintPure)
//     float GetSliderPosition() const { return SliderPosition; }

//     UFUNCTION(BlueprintPure)
//     float GetSweetSpotStart() const { return SweetSpotStart; }

//     UFUNCTION(BlueprintPure)
//     float GetSweetSpotWidth() const { return SweetSpotWidth; }

//     private void SendCloseEvent(AActor OwnerActor)
//     {
//         AbilitySystem::SendGameplayEventToActor(
//             GetInstigator(OwnerActor),
//             FGameplayTag::RequestGameplayTag(n"Interaction.Event.LockpickEnded"),
//             FGameplayEventData());
//     }
// }
