#pragma once

#include "CoreMinimal.h"
#include "UI/FVCompassWidget.h"
#include "FVStripCompassWidget.generated.h"

class UImage;
class UMaterialInstanceDynamic;

/**
 * Horizontal compass strip: scrolls a 360-degree texture with the view heading and places markers along it. The
 * navigator's Compass > Field Of View sets how many degrees the strip shows.
 */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVStripCompassWidget : public UFVCompassWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void UpdateCompass(const FFVCompassView& View, const UFVNavigatorComponent& PlayerNavigator, const FVector2D& Size) override;

	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation",
		meta = (BindWidget, ToolTip = "Its material samples a 360-degree texture, north at U = 0, at U * CompassTiling + CompassOffset."))
	TObjectPtr<UImage> Strip;

private:
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StripMaterial;
};
