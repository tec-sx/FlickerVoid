class AInteractableAICharacter : AFVAICharacter
{
    UPROPERTY(EditAnywhere, Category = "Character")
    UCharacterDataAsset CharacterData;

    UPROPERTY(DefaultComponent, Category = "Flow")
    UFlowComponent FlowComponent;
    default FlowComponent.bAutoStartRootFlow = false;

    UPROPERTY(DefaultComponent, Category = "Identity")
    UFVIdentityComponent IdentityComponent;

    UPROPERTY(DefaultComponent, Category = "Dialogue")
    UFVDialogueParticipantComponent DialogueParticipant;

    UPROPERTY(DefaultComponent, Category = "Interaction")
    UFVInteractableComponent InteractableComponent;
    
    UPROPERTY(DefaultComponent, Category="UI")
    UWidgetComponent FloatingTextBarComponent;
    default FloatingTextBarComponent.RelativeLocation = FVector(0, 0, CapsuleComponent.CapsuleHalfHeight + 20.f);
    default FloatingTextBarComponent.Space = EWidgetSpace::Screen;

    UFUNCTION(BlueprintOverride)
    void ConstructionScript()
    {
        if (IsValid(CharacterData))
        {
            Mesh.SetSkeletalMeshAsset(CharacterData.SkeletalMesh);
            Mesh.SetAnimationMode(CharacterData.AnimationMode);

            switch (CharacterData.AnimationMode)
            {
                case EAnimationMode::AnimationBlueprint:
                    Mesh.SetAnimInstanceClass(CharacterData.AnimInstanceClass);
                    break;
                case EAnimationMode::AnimationSingleNode:
                    Mesh.AnimationData.AnimToPlay = CharacterData.AnimationAsset;
                    Mesh.AnimationData.bSavedLooping = CharacterData.bLoopAnimation;
                    break;
                default:
                    break;
            }

            StateTree = CharacterData.StateTree;
            FlowComponent.RootFlow = CharacterData.FlowAsset;
            FlowComponent.IdentityTags = CharacterData.IdentityTags;
            OwnedTags.AppendTags(CharacterData.IdentityTags);
            DialogueParticipant.SetDialogue(CharacterData.Dialogue);
            IdentityComponent.SetDefinition(CharacterData.Definition);
        }
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        DialogueParticipant.OnBark.AddUFunction(this, n"UpdateFloatingTextBar");
    }

    UFUNCTION()
    private void UpdateFloatingTextBar(FText Text, float Duration)
    {
        UFloatingTextBar FloatingTextBar = Cast<UFloatingTextBar>(FloatingTextBarComponent.GetUserWidgetObject());

        if (IsValid(FloatingTextBar))
        {
            FloatingTextBar.SetText(Text);
            FloatingTextBar.ShowForDuration(Duration);
        }
    }
}
