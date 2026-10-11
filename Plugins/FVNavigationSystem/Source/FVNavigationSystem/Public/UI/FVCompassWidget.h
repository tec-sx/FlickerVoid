#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FVNavigationTypes.h"
#include "UI/FVMarkerWidgetPool.h"
#include "FVCompassWidget.generated.h"

class UCanvasPanel;
class UFVMapMarkerWidget;
class UFVNavigatorComponent;

/** Base of the ring and strip compasses: builds the navigator's compass view and places its markers on MarkerCanvas. */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVCompassWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Turns or scrolls the compass art and calls PlaceMarker for each marker. */
	virtual void UpdateCompass(const FFVCompassView& View, const UFVNavigatorComponent& PlayerNavigator, const FVector2D& Size) {}

	UFUNCTION(BlueprintPure, Category = "FV|Navigation", meta = (ToolTip = "The navigator on the owning player's controller, or on their pawn."))
	UFVNavigatorComponent* GetNavigator() const;

	void PlaceMarker(const FFVMarkerView& Marker, const FVector2D& Position);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compass", meta = (ToolTip = "Icon widget created for each marker."))
	TSubclassOf<UFVMapMarkerWidget> MarkerWidgetClass;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional, ToolTip = "Fills the widget and holds the marker icons."))
	TObjectPtr<UCanvasPanel> MarkerCanvas;

private:
	mutable TWeakObjectPtr<UFVNavigatorComponent> Navigator;
	FFVMarkerWidgetPool MarkerPool;
};
