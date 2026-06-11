// Examine overlay (Tomb Raider style):
//   1. Moves the item to a fixed camera-relative socket and freezes input
//   2. Player rotates the item with the right stick / mouse
//   3. At each configured secret angle the handler fires a tag on the world state
//   4. Pressing Secondary (F) closes the examine overlay

class UFVExamineHandlerConfig : UFVInteractionHandlerConfig
{
    UPROPERTY(Category = "Examine")
    float ExamineArmLength = 80.f;

    // Each entry: rotation threshold (degrees from identity) and the tag to grant when hit
    UPROPERTY(Category = "Examine")
    TArray<FFVExamineSecret> Secrets;
}

struct FFVExamineSecret
{
    UPROPERTY()
    FRotator TriggerRotation;

    UPROPERTY()
    float TriggerToleranceDegrees = 15.f;

    // Applied to instigator ASC when the secret angle is first reached
    UPROPERTY()
    FGameplayTag SecretTag;

    UPROPERTY()
    FText DiscoveryMessage;
}

class UFVExamineHandler : UFVInteractionActionHandler
{
    UFVExamineHandlerConfig ExamineConfig;
    FFVInteractionContext ExamineContext;
    bool bIsExamining = false;
    TSet<FGameplayTag> TriggeredSecrets;

    UFUNCTION(BlueprintOverride)
    void OnInitialize(UFVInteractionHandlerConfig Config)
    {
        ExamineConfig = Cast<UFVExamineHandlerConfig>(Config);
        if (ExamineConfig == nullptr)
        {
            Warning(GetName() + ": no UFVExamineHandlerConfig, using defaults.");
            ExamineConfig = NewObject(this, UFVExamineHandlerConfig);
        }
    }

    UFUNCTION(BlueprintOverride)
    void OnExecute(FFVInteractionContext Context)
    {
        ExamineContext = Context;
        bIsExamining = true;
        TriggeredSecrets.Empty();

        // Show examine overlay via gameplay event — the UI/BP listens for this tag
        FGameplayEventData EventData;
        EventData.Instigator = Context.Instigator;
        EventData.Target     = Context.TargetActor;

        AbilitySystem::SendGameplayEventToActor(
            Context.Instigator,
            FGameplayTag::RequestGameplayTag(n"Interaction.Event.ExamineStarted"),
            EventData);
    }

    // Called each frame by the examine UI widget while open
    UFUNCTION(BlueprintCallable)
    void OnExamineRotationUpdated(FRotator CurrentRotation)
    {
        if (!bIsExamining)
        {
            return;
        }

        for (FFVExamineSecret& Secret : ExamineConfig.Secrets)
        {
            if (TriggeredSecrets.Contains(Secret.SecretTag))
            {
                continue;
            }

            float Delta = Math::RadiansToDegrees(
                FQuat::FindBetweenNormals(
                    Secret.TriggerRotation.RotateVector(FVector::ForwardVector),
                    CurrentRotation.RotateVector(FVector::ForwardVector)).GetAngle());

            if (Delta <= Secret.TriggerToleranceDegrees)
            {
                TriggeredSecrets.Add(Secret.SecretTag);

                AbilitySystem::SendGameplayEventToActor(
                    ExamineContext.Instigator,
                    Secret.SecretTag,
                    FGameplayEventData());
            }
        }
    }

    // Called by the examine UI widget's close button (mapped to Secondary / F)
    UFUNCTION(BlueprintCallable)
    void CloseExamine()
    {
        if (!bIsExamining)
        {
            return;
        }

        bIsExamining = false;
        CompleteExecution(true);
    }

    UFUNCTION(BlueprintOverride)
    void OnCancel(FFVInteractionContext Context)
    {
        bIsExamining = false;
    }
}
