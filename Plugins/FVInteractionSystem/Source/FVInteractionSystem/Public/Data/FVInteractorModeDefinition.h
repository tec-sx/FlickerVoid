#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/EngineTypes.h"
#include "GameplayTagContainer.h"
#include "FVInteractorModeDefinition.generated.h"

class UUserWidget;

UENUM(BlueprintType)
enum class EFVInteractorTraceOrigin : uint8
{
	Character,
	Camera
};

USTRUCT(BlueprintType)
struct FVINTERACTIONSYSTEM_API FFVInteractorDetectionSettings
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
	EFVInteractorTraceOrigin TraceOrigin = EFVInteractorTraceOrigin::Character;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (Tooltip = "Offset from the view point in view space (X forward, Y right, Z up)."))
	FVector TraceOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (UIMin = 0, ClampMin = 0, Units = "cm"))
	float TraceRadius = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (UIMin = 1, ClampMin = 1, Units = "cm"))
	float TraceRange = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection", meta = (UIMin = 0.01, ClampMin = 0.01, Units = "s"))
	float TickInterval = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
	TEnumAsByte<ECollisionChannel> CollisionChannel = ECC_Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Detection")
	TEnumAsByte<ECollisionChannel> OcclusionChannel = ECC_Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring", meta = (UIMin = 0, ClampMin = 0, Tooltip = "How much being close to the camera's line of sight counts when picking between candidates."))
	float AlignmentWeight = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring", meta = (UIMin = 1, ClampMin = 1, UIMax = 90, ClampMax = 90, Units = "deg", Tooltip = "Angle off the camera's line of sight at which alignment stops counting."))
	float AlignmentAngle = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scoring", meta = (UIMin = 0, ClampMin = 0, Tooltip = "How much being near counts when picking between candidates."))
	float DistanceWeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (DisplayName = "Optional Interaction Mode UI Overlay"))
	TSubclassOf<UUserWidget> OverlayWidgetClass;

	static FFVInteractorDetectionSettings Lerp(const FFVInteractorDetectionSettings& From, const FFVInteractorDetectionSettings& To, const float Alpha)
	{
		FFVInteractorDetectionSettings Result = To;
		Result.TraceOffset = FMath::Lerp(From.TraceOffset, To.TraceOffset, Alpha);
		Result.TraceRadius = FMath::Lerp(From.TraceRadius, To.TraceRadius, Alpha);
		Result.TraceRange = FMath::Lerp(From.TraceRange, To.TraceRange, Alpha);
		Result.AlignmentWeight = FMath::Lerp(From.AlignmentWeight, To.AlignmentWeight, Alpha);
		Result.AlignmentAngle = FMath::Lerp(From.AlignmentAngle, To.AlignmentAngle, Alpha);
		Result.DistanceWeight = FMath::Lerp(From.DistanceWeight, To.DistanceWeight, Alpha);
		return Result;
	}
};

UCLASS(BlueprintType, Const)
class FVINTERACTIONSYSTEM_API UFVInteractorModeDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFVInteractorModeDefinition();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mode", meta = (Categories = "Interactor.Mode"))
	FGameplayTag ModeTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mode")
	FFVInteractorDetectionSettings Detection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mode", meta = (ClampMin = 0, Units = "s", Tooltip = "Time to blend offset/radius/range into this mode. 0 snaps instantly."))
	float BlendTime = 0.f;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(class FDataValidationContext& Context) const override;
#endif
};
