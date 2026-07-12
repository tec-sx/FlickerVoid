class AFVComplexNPC : AFVNPCCharacter
{
    UPROPERTY(DefaultComponent)
    UStateTreeComponent StateTree;

    UPROPERTY(DefaultComponent)
    UAIPerceptionComponent Perception;
    default Perception.DominantSense = UAISense_Sight;

    UPROPERTY(EditDefaultsOnly)
    FFVCharacterIdentity Identity;

    UFVDialogueSubsystem DialogueSubsystem;

    UFUNCTION(BlueprintOverride)
    void ConstructionScript()
    {
        Identity.SpeakerId = n"NPC_Dealer";
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        DialogueSubsystem = Subsystem::GetGameInstanceSubsystem(UFVDialogueSubsystem);

        if (DialogueSubsystem == nullptr)
        {
            Warning("FVComplexNPC: UFVDialogueSubsystem not found.");
            return;
        }

        Perception.OnPerceptionUpdated.AddUFunction(this, n"OnPerceptionUpdated");
        Perception.OnTargetPerceptionUpdated.AddUFunction(this, n"OnTargetPerceptionUpdated");
    }

    UFUNCTION(BlueprintOverride)
    void EndPlay(EEndPlayReason EndPlayReason)
    {
        Perception.OnPerceptionUpdated.Unbind(this, n"OnPerceptionUpdated");
        Perception.OnTargetPerceptionUpdated.Unbind(this, n"OnTargetPerceptionUpdated");
    }

    UFUNCTION()
    private void OnPerceptionUpdated(const TArray<AActor>&in UpdatedActors)
    {
    }

    UFUNCTION()
    private void OnTargetPerceptionUpdated(AActor Actor, FAIStimulus Stimulus)
    {
        UAISense Sense = UAIPerceptionSystem::GetSenseClassForStimulus(Stimulus).GetDefaultObject();

        if (Sense.IsA(UAISense_Sight))
        {
            HandleSight(Actor);
        }
    }

    UFUNCTION()
    private void HandleSight(AActor Actor)
    {
        if (Actor == Gameplay::GetPlayerCharacter(0))
        {
            // FFVDialogueContext Context;
            // Context.SpeakerID = Identity.SpeakerId;
            // Context.Instigator = this;
            // Context.Target = Actor;

            // DialogueSubsystem.SendSignal(n"Greeting", Context);
        }
    }
};