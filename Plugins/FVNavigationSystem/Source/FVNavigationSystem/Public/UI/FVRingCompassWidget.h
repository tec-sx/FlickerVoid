#pragma once

#include "CoreMinimal.h"
#include "UI/FVCompassWidget.h"
#include "FVRingCompassWidget.generated.h"

/** Compass ring around the minimap: turns with the map, or stays north-up when the minimap does, with markers around its edge. */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVRingCompassWidget : public UFVCompassWidget
{
	GENERATED_BODY()

protected:
	virtual void UpdateCompass(const FFVCompassView& View, const UFVNavigatorComponent& PlayerNavigator, const FVector2D& Size) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compass", meta = (ClampMin = 0, ToolTip = "Marker distance from the centre in pixels; 0 is half the smaller side."))
	float MarkerRadius = 0.f;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidget, ToolTip = "Ring art with north at the top."))
	TObjectPtr<UWidget> Ring;
};
