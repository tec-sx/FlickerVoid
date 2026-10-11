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
	/** Called on every view update, so distance and elevation stay current. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Navigation")
	void OnMarkerChanged(const FFVMarkerView& InMarker);

	/** Gets the definition's icon, or the Discovery fragment's Undiscovered Icon while undiscovered. */
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional))
	TObjectPtr<UImage> Icon;

private:
	UPROPERTY(Transient)
	FFVMarkerView Marker;

	TSoftObjectPtr<UTexture2D> ShownIcon;
};
