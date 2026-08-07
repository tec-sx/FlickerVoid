event void FDialogueStarted(USUDSDialogue Dialogue); 
event void FDialogueEnded(USUDSDialogue Dialogue); 

class AInteractableAICharacter : AFVAICharacter
{
    UPROPERTY(EditAnywhere, Category = "Character")
    UCharacterDataAsset CharacterData;

    UPROPERTY(DefaultComponent, Category = "Character")
    UFlowComponent FlowComponent;
    default FlowComponent.bAutoStartRootFlow = false;

    UPROPERTY(DefaultComponent)
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
        }
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        UGameplayMessageSubsystem::Get().RegisterListener(
            GameplayTags::Dialogue_CallOut,
            this,
            n"HandleCallOut",
            FFVDialogueCallOutMessage());
    }

    UFUNCTION(BlueprintOverride)
    void EndPlay(EEndPlayReason EndPlayReason)
    {
        AFVAICharacterController AIController = Cast<AFVAICharacterController>(Controller);

        if (IsValid(AIController))
        {
        }
    }

    UFUNCTION()
    void HandleCallOut(FGameplayTag Channel, FFVDialogueCallOutMessage Message)
    {
        if (Message.OwnerActor == this)
        {
            UFloatingTextBar FloatingTextBar = Cast<UFloatingTextBar>(FloatingTextBarComponent.GetUserWidgetObject());
            
            if (IsValid(FloatingTextBar))
            {
                FloatingTextBar.SetText(Message.Text);
                FloatingTextBar.ShowForDuration(Message.DisplayDuration);
            }
        }

    }
}