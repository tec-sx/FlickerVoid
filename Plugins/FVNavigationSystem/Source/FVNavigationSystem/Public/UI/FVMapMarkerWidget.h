#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FVNavigationTypes.h"
#include "FVMapMarkerWidget.generated.h"

class UImage;
class UTexture2D;

/**
 * One marker icon on a map view or compass. The parent widget creates, places and reuses it; the icon and tint come
 * from the marker definition, and OnMarkerChanged adds the looks (elevation arrows, distance, tracked highlight).
 */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVMapMarkerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMarker(const FFVMarkerView& InMarker);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FFVMarkerView GetMarker() const { return Marker; }

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Navigation", meta = (ToolTip = "Called on every view update, so distance and elevation stay current."))
	void OnMarkerChanged(const FFVMarkerView& InMarker);

	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional, ToolTip = "Shows the marker's icon and tint, or its Undiscovered Icon."))
	TObjectPtr<UImage> Icon;

private:
	UPROPERTY(Transient)
	FFVMarkerView Marker;

	TSoftObjectPtr<UTexture2D> ShownIcon;
};
