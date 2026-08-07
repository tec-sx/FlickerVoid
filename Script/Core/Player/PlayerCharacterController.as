class APlayerCharacterController : AFVPlayerController
{
    UPROPERTY()
    FCameraAssetReference PlayerCameraAsset;

    UPROPERTY(DefaultComponent, Category = Camera)
    UGameplayCameraComponent GameplayCamera;
    
    UFUNCTION(BlueprintOverride)
    void ReceivePossess(APawn PossessedPawn)
    {
        SetupCamera(GetControlledCharacter());
    }

    UFUNCTION(BlueprintCallable)
    void SetupCamera(AFVCharacter Character)
    {
        FFVResult Result;

        // Validate GameplayCamera
        Result = FVValidation::ValidateComponent(GameplayCamera, "GameplayCamera");
        if (!Result.bSuccess)
        {
            Warning(Result.ErrorMessage + " on " + GetName());
            return;
        }

        // Validate Character
        Result = FVValidation::ValidateObject(Character, "Character");
        if (!Result.bSuccess)
        {
            Warning(Result.ErrorMessage + " when setting up camera on " + GetName());
            return;
        }

        // Validate Character.Mesh
        Result = FVValidation::ValidateComponent(Character.Mesh, "Character.Mesh");
        if (!Result.bSuccess)
        {
            Warning(Result.ErrorMessage + " when setting up camera on " + GetName());
            return;
        }

        GameplayCamera.AttachToComponent(
            Character.Mesh,
            NAME_None,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::SnapToTarget,
            EAttachmentRule::KeepRelative,
            false);

        GameplayCamera.ActivateCameraForPlayerController(this, true);
    }
}