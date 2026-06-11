// // Examine overlay task (Tomb Raider style).
// //
// // Sends Interaction.Event.ExamineStarted to open the UI widget.
// // The widget calls OnExamineRotationUpdated() each frame and CloseExamine() to finish.
// // Secrets are granted as gameplay tags when the item is rotated to the configured angles.
// //
// // Configure in the State Tree editor:
// //   ExamineArmLength — camera distance while examining
// //   Secrets          — array of (TriggerRotation, ToleranceDegrees, SecretTag, DiscoveryMessage)

// struct FFVExamineSecret
// {
//     UPROPERTY()
//     FRotator TriggerRotation;

//     UPROPERTY()
//     float TriggerToleranceDegrees = 15.f;

//     UPROPERTY()
//     FGameplayTag SecretTag;

//     UPROPERTY()
//     FText DiscoveryMessage;
// }

// struct FFVExamineTaskInstanceData
// {
//     UPROPERTY()
//     float ExamineArmLength = 80.f;

//     UPROPERTY()
//     TArray<FFVExamineSecret> Secrets;
// }

// class UFVExamineTask : UFVInteractionStateTaskBase
// {

//     // Runtime
//     AActor CachedOwnerActor;
//     bool bIsExamining = false;
//     TSet<FGameplayTag> TriggeredSecrets;

//     UFUNCTION(BlueprintOverride)
//     EStateTreeStateChangeType ReceiveEnterState(FStateTreeTransitionResult Transition)
//     {
//         CachedOwnerActor = OwnerActor;
//         bIsExamining = true;
//         TriggeredSecrets.Empty();

//         FGameplayEventData EventData;
//         EventData.Instigator = GetInstigator(OwnerActor);
//         EventData.Target     = OwnerActor;

//         AbilitySystem::SendGameplayEventToActor(
//             GetInstigator(OwnerActor),
//             FGameplayTag::RequestGameplayTag(n"Interaction.Event.ExamineStarted"),
//             EventData);

//         return EStateTreeStateChangeType::Changed;
//     }

//     UFUNCTION(BlueprintOverride)
//     void ReceiveExitState(FStateTreeTransitionResult Transition)
//     {
//         bIsExamining = false;
//         CachedOwnerActor = nullptr;
//     }

//     // Called each frame by the examine UI widget while open
//     UFUNCTION(BlueprintCallable)
//     void OnExamineRotationUpdated(FRotator CurrentRotation)
//     {
//         if (!bIsExamining || !IsValid(CachedOwnerActor))
//         {
//             return;
//         }

//         AActor Instigator = GetInstigator(CachedOwnerActor);

//         for (FFVExamineSecret& Secret : Secrets)
//         {
//             if (TriggeredSecrets.Contains(Secret.SecretTag))
//             {
//                 continue;
//             }

//             float Delta = Math::RadiansToDegrees(
//                 FQuat::FindBetweenNormals(
//                     Secret.TriggerRotation.RotateVector(FVector::ForwardVector),
//                     CurrentRotation.RotateVector(FVector::ForwardVector)).GetAngle());

//             if (Delta <= Secret.TriggerToleranceDegrees)
//             {
//                 TriggeredSecrets.Add(Secret.SecretTag);
//                 AbilitySystem::SendGameplayEventToActor(
//                     Instigator,
//                     Secret.SecretTag,
//                     FGameplayEventData());
//             }
//         }
//     }

//     // Called by the examine widget's close button (mapped to Secondary / F)
//     UFUNCTION(BlueprintCallable)
//     void CloseExamine()
//     {
//         if (!bIsExamining)
//         {
//             return;
//         }

//         bIsExamining = false;
//         CompleteTask(CachedOwnerActor, true);
//     }
// }
