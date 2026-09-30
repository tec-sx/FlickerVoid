class UInteractableResponseComponent_Consume : UFVInteractableResponseComponent
{
    UPROPERTY(EditAnywhere)
    UAnimMontage ConsumeAnimation;

    UPROPERTY(BlueprintReadWrite, Meta=(MakeEditWidget = true))
    FVector InteractionPointOffset = FVector(0, 4, -8);

    private FTransform OriginalTransform;
    private APlayerCharacter InteractorActor;
    private UFVInteractorComponent InteractorComponent;
    private UAnimInstance AnimInstance;

    UFUNCTION(BlueprintPure)
    FVector GetInteractionPoint()
    {
        return GetOwner().GetActorLocation() - InteractionPointOffset;
    }

    
    UFUNCTION(BlueprintOverride)
    void ExecuteAction(UFVInteractorComponent Interactor)
    {
        Print(ActionTag.ToString());

        InteractorComponent = Interactor;
        InteractorActor = Cast<APlayerCharacter>(InteractorComponent.GetOwner());

        if (!IsValid(InteractorActor))
        {
            return;
        }
        
        AnimInstance = InteractorActor.Mesh.AnimInstance;

        if (!IsValid(AnimInstance))
        {
            return;
        }
        
        USceneComponent SceneRoot = GetOwner().GetComponentByClass(USceneComponent);
        InteractorActor.MotionWarpingComponent.AddOrUpdateWarpTargetFromComponent(n"PickItem", SceneRoot, n"grab_r_Socket", false);
        InteractorActor.PlayAnimMontage(ConsumeAnimation);
        AnimInstance.OnMontageEnded.AddUFunction(this, n"OnMontageEnded");
        AnimInstance.OnMontageSectionChanged.AddUFunction(this, n"OnMontageSectionChanged");

        InteractorComponent.AddSuppression(GameplayTags::Interaction_Suppression_Cutscene);
    }

    UFUNCTION()
    private void OnMontageEnded(UAnimMontage Montage, bool bInterrupted)
    {
        AnimInstance.OnMontageEnded.Clear();
        AnimInstance.OnMontageSectionChanged.Clear();
        InteractorComponent.RemoveSuppression(GameplayTags::Interaction_Suppression_Cutscene);

        AnimInstance = nullptr;
        InteractorActor = nullptr;
        InteractorComponent = nullptr;
        
        Print("End");
    }

    UFUNCTION()
    private void OnMontageSectionChanged(UAnimMontage Montage, FName SectionName, bool bLooped)
    {
        UMeshComponent InteractableMesh = GetOwner().GetComponentByClass(UMeshComponent);
        
        if (!IsValid(InteractableMesh))
        {
            return;
        }

        if (SectionName == n"Grab")
        {
            OriginalTransform = InteractableMesh.WorldTransform;
            InteractableMesh.AttachToComponent(InteractorActor.Mesh, n"grab_r_Socket", EAttachmentRule::SnapToTarget);
            InteractableMesh.SetRelativeLocation(InteractionPointOffset);
        }

        if (SectionName == n"Release")
        {
            InteractableMesh.DetachFromComponent();
            InteractableMesh.SetWorldTransform(OriginalTransform);
        }
    }
}