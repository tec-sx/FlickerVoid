#pragma once

#include "CoreMinimal.h"
#include "UI/FVMapViewWidget.h"
#include "FVWorldMapWidget.generated.h"

/**
 * Full map: shows a chosen map and layer, pans with a mouse drag and zooms around the cursor with the wheel. Opens on
 * the player's map and layer; a room layer shows over its dimmed base layer, zoomed to fit.
 */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVWorldMapWidget : public UFVMapViewWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation", meta = (ToolTip = "Shows a map and layer; a null Map shows the player's current one."))
	void ShowMap(UFVMapDefinition* Map, int32 LayerIndex);

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation", meta = (ToolTip = "Moves the view to the player."))
	void CenterOnPlayer();

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation", meta = (ToolTip = "Moves the view as a drag of DeltaPixels would."))
	void Pan(FVector2D DeltaPixels);

	UFUNCTION(BlueprintCallable, Category = "FV|Navigation", meta = (ToolTip = "Factor above 1 zooms in, keeping the point under LocalPosition in place."))
	void ZoomAt(FVector2D LocalPosition, float Factor);

	UFUNCTION(BlueprintPure, Category = "FV|Navigation", meta = (ToolTip = "1 fits the whole base layer."))
	float GetZoom() const { return Zoom; }

protected:
	virtual void NativeConstruct() override;
	virtual void UpdateView(float DeltaTime) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Navigation", meta = (ToolTip = "A click that wasn't a drag, e.g. to set a waypoint."))
	void OnMapClicked(FVector2D LocalPosition, FVector WorldLocation);

private:
	FVector2D GetExtent(float InZoom) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Map", meta = (ClampMin = 1))
	float MaxZoom = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Map", meta = (ClampMin = 1, ToolTip = "Zoom factor per mouse wheel step."))
	float ZoomStep = 1.15f;

private:
	float Zoom = 1.f;
	FVector2D PanUV = FVector2D(0.5);
	FVector2D LastDragPosition = FVector2D::ZeroVector;
	float DragDistance = 0.f;
	bool bPressed = false;
};
