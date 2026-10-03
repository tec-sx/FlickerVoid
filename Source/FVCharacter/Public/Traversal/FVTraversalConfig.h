#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Movement/FVMovementHandlerConfigBase.h"
#include "FVTraversalConfig.generated.h"

class UChooserTable;

UCLASS(BlueprintType)
class FLICKERVOIDCHARACTER_API UFVTraversalConfig : public UFVMovementHandlerConfigBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Ground")
	float GroundTraceForwardDistanceMin = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Ground")
	float GroundTraceForwardDistanceMax = 350.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Ground")
	FVector2D GroundSpeedRange = FVector2D(0.f, 500.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Ground")
	FVector GroundTraceOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Ground")
	FVector GroundTraceEndOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Ground")
	float GroundTraceHalfHeight = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Air")
	float AirTraceForwardDistance = 75.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Air")
	FVector AirTraceOriginOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Air")
	FVector AirTraceEndOffset = FVector(0.f, 0.f, 50.f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Air")
	float AirTraceHalfHeight = 86.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Common")
	float TraceRadius = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Trace|Common")
	TEnumAsByte<ETraceTypeQuery> ObstacleTraceType = TraceTypeQuery1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clearance")
	float CapsuleOffsetDistance = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Clearance")
	float FloorTraceVerticalOffset = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Motion Matching")
	TObjectPtr<UChooserTable> TraversalChooserTable;
};

UCLASS(BlueprintType)
class FLICKERVOIDCHARACTER_API UFVTraversableConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	float MinLedgeWidth = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Ledge")
	float LedgeNormalOffset = 10.f;
};
