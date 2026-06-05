enum EFVCameraMode
{
    Free,
    Aim
}

enum EFVCameraStyle
{
    Close,
    Medium,
    Far
}

struct FFVPlayerCameraSettings
{
    UPROPERTY()
    EFVCameraMode CameraMode;
    
    UPROPERTY()
    EFVCameraStyle CameraStyle;
    
    UPROPERTY()
    EFVGait PlayerGait;
    
    UPROPERTY()
    EFVStance PlayerStance;
}