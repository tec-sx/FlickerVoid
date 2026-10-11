#pragma once

#include "CoreMinimal.h"
#include "UI/FVMapViewWidget.h"
#include "FVMinimapWidget.generated.h"

/**
 * Minimap: follows the player on the navigator's active map, turns with the view, and zooms into rooms with the base
 * layer dimmed around them. Markers out of range that clamp to the edge sit on the rim.
 */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVMinimapWidget : public UFVMapViewWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation", meta = (ToolTip = "Factor above 1 zooms in. Changes the navigator's minimap radius."))
	void ZoomBy(float Factor);

protected:
	virtual void UpdateView(float DeltaTime) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap", meta = (ClampMin = 0, ToolTip = "Pixels between the rim and the markers pinned to it."))
	float EdgePadding = 12.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap", meta = (ToolTip = "Turns PlayerIcon to where the character faces; off keeps it pointing up."))
	bool bRotatePlayerIcon = false;

private:
	float Radius = 0.f;
};
