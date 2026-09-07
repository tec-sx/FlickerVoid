class ABeverageItem : APickupItem
{
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUsed;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 UsageReduction = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool InfiniteAmound = false;
    
    UPROPERTY(EditAnywhere)
    UAnimMontage DrinkAnimation;

    default InteractionPointOffset  = FVector(0, 4, -8);

    private FTransform OriginalTransform;
    private APlayerCharacter Interactor;
    private UAnimInstance AnimInstance;

    UFUNCTION(BlueprintOverride)
    void OnInteractionExecuted(const FGameplayTag&in ActionTag, UInteractorComponent InteractorComponent)
    {
        Super::OnInteractionExecuted(ActionTag, InteractorComponent);

        Interactor = Cast<APlayerCharacter>(InteractorComponent.GetOwner());
        
        if (ActionTag == GameplayTags::Interaction_Action_Drink)
        {
            Drink();
        }
    }
    
    UFUNCTION()
    private void Drink()
    {
        if (!IsValid(Interactor))
        {
            return;
        }

        if (bIsUsed)
        {
            Print("Empty");
            return;
        }
        
        AnimInstance = Interactor.Mesh.AnimInstance;

        if (IsValid(AnimInstance))
        {
            Interactor.MotionWarpingComponent.AddOrUpdateWarpTargetFromComponent(n"PickItem", SceneRoot, n"grab_r_Socket", false);
            Interactor.PlayAnimMontage(DrinkAnimation);
            AnimInstance.OnMontageEnded.AddUFunction(this, n"OnMontageEnded");
            AnimInstance.OnMontageSectionChanged.AddUFunction(this, n"OnMontageSectionShanged");

            Interactor.DisableInteractions();
        }

        if (InfiniteAmound == true)
        {
            return;
        }

        Amount -= UsageReduction;
        Print("Amount Left: " + Amount);

        if (Amount <= 0)
        {
            bIsUsed = true;
        }
    }

    UFUNCTION()
    private void OnMontageEnded(UAnimMontage Montage, bool bInterrupted)
    {
        AnimInstance.OnMontageEnded.Clear();
        AnimInstance.OnMontageSectionChanged.Clear();
        Interactor.EnableInteractions();

        AnimInstance = nullptr;
        Interactor = nullptr;
        
        Print("End");
    }

    UFUNCTION()
    private void OnMontageSectionShanged(UAnimMontage Montage, FName SectionName, bool bLooped)
    {
        if (SectionName == n"Grab")
        {
            OriginalTransform = Mesh.WorldTransform;
            Mesh.AttachToComponent(Interactor.Mesh, n"grab_r_Socket", EAttachmentRule::SnapToTarget);
            Mesh.SetRelativeLocation(InteractionPointOffset);
        }

        if (SectionName == n"Release")
        {
            Mesh.DetachFromComponent();
            Mesh.SetWorldTransform(OriginalTransform);
        }
    }
}