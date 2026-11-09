class APlayerCharacter : AFVPlayerCharacter
{
    UPROPERTY(DefaultComponent, Category = Movement)
    UMotionWarpingComponent MotionWarpingComponent;

    UPROPERTY(DefaultComponent, Category = Animation)
    UContextualAnimSceneActorComponent ContextualAnimation;

    UPROPERTY(DefaultComponent, Category = Camera)
    UGameplayCameraComponent GameplayCamera;

    UPROPERTY(DefaultComponent)
    UFVInteractionUIComponent InteractionUIComponent;
    
    UPROPERTY(DefaultComponent)
    UFVInventoryComponent InventoryComponent;

    UPROPERTY(DefaultComponent)
    UFVItemReceiverComponent ItemReceiverComponent;
    

    UPROPERTY(DefaultComponent, Category = Items)
    UFVHeldItemComponent HeldItems;

    UPROPERTY(DefaultComponent, Category = Items)
    UFVIntoxicationComponent Intoxication;

    // Mutable builds the body into Mesh, with the skin under worn garments clipped away.
    UPROPERTY(DefaultComponent, Attach = Mesh, Category = Appearance)
    UCustomizableSkeletalComponent BodyCustomization;

    // Mutable builds the worn garments into this mesh; it follows the body's pose.
    UPROPERTY(DefaultComponent, Attach = Mesh, Category = Appearance)
    USkeletalMeshComponent Outfit;

    UPROPERTY(DefaultComponent, Attach = Outfit, Category = Appearance)
    UCustomizableSkeletalComponent OutfitCustomization;

    UPROPERTY(EditAnywhere, Category = Appearance, Meta = (Categories = "Appearance.Body"))
    FGameplayTag BodyType;

    UPROPERTY(Transient)
    UCharacterAppearance Appearance;

    UPROPERTY()
    UFVTraversalComponent Traversal;
    
    UPROPERTY()
    FIKGoal RightHandIKGoal;

    bool bIsRagdolling = false;

    default Mesh.SetRelativeLocation(FVector(0, 0, -CapsuleComponent.CapsuleHalfHeight));
    default Mesh.SetRelativeRotation(FRotator(0, -90, 0));

    UFUNCTION(BlueprintOverride)
    void Possessed(AController NewController)
    {
        APlayerCharacterController PlayerController = Cast<APlayerCharacterController>(NewController);

        if (IsValid(PlayerController))
        {
            GameplayCamera.AttachToComponent(
                Mesh,
                NAME_None,
                EAttachmentRule::SnapToTarget,
                EAttachmentRule::SnapToTarget,
                EAttachmentRule::KeepRelative,
                false);

            GameplayCamera.ActivateCameraForPlayerController(PlayerController, true);
        }

        FVAbilitySystemComponent.AddLooseGameplayTag(GameplayTags::Ability_Technical_LockPick);
    }

    APlayerCharacter()
    {
        bUseControllerRotationPitch = false;
        bUseControllerRotationYaw = false;
        bUseControllerRotationRoll = false;
    }

    UFUNCTION(BlueprintOverride)
    void ConstructionScript()
    {
        Outfit.SetLeaderPoseComponent(Mesh);
    }

    UFUNCTION(BlueprintOverride)
    void BeginPlay()
    {
        HeldItems.OnHeldItemsChanged.AddUFunction(this, n"UpdateHoldPoses");
        UpdateHoldPoses();

        UFVEquipmentComponent Equipment = UFVEquipmentComponent::Get(this);
        Appearance = NewObject(this, UCharacterAppearance);
        Appearance.Start(BodyCustomization, OutfitCustomization, Equipment, BodyType);

        Traversal = UFVTraversalComponent::Get(this);

        Locomotion.OnStartedFalling.AddUFunction(this, n"OnStartedFalling");
    }

    UFUNCTION(BlueprintOverride)
    bool RequestTraverse()
    {
        EDrawDebugTrace DrawDebugType = Locomotion.IsOnGround()
            ? EDrawDebugTrace::ForOneFrame
            : EDrawDebugTrace::ForDuration;

        bool bIsTraversing = Traversal.TryTraversalAction(DrawDebugType);

        Locomotion.SetTraversing(bIsTraversing);

        return bIsTraversing;
    }

    UFUNCTION()
    void OnStartedFalling()
    {
        Locomotion.StopCrouching();
    }

    UFUNCTION()
    FFVPlayerCameraSettings GetCameraSettings()
    {
        FFVPlayerCameraSettings Settings;
        Settings.CameraMode = EFVCameraMode::Free;
        Settings.CameraStyle = EFVCameraStyle::Medium;
        Settings.PlayerGait = EFVGait::Walking;
        Settings.PlayerStance = EFVStance::Stand;

        return Settings;
    }

    UFUNCTION()
    bool HasFocusedInteractable()
    {
        return InteractorComponent.HasInteractableTarget();
    }

    // Passes the hold poses of what the hands hold to the anim instance.
    UFUNCTION()
    private void UpdateHoldPoses()
    {
        UFVPlayerAnimInstance AnimInstance = Cast<UFVPlayerAnimInstance>(Mesh.AnimInstance);

        if (AnimInstance != nullptr)
        {
            AnimInstance.RightHandHoldPose = HeldItems.GetHoldPose(true);
            AnimInstance.LeftHandHoldPose = HeldItems.GetHoldPose(false);
        }
    }
}