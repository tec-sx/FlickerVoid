#pragma once

#include "CoreMinimal.h"
#include "FVNavigationTypes.h"
#include "Templates/SubclassOf.h"
#include "UObject/WeakObjectPtrTemplates.h"

class UCanvasPanel;
class UFVMapMarkerWidget;
class UUserWidget;

/** Marker icons of one canvas, kept per marker handle and reused between updates. Used by the map view and compass widgets. */
struct FFVMarkerWidgetPool
{
	void BeginUpdate();
	void Place(UUserWidget& Owner, UCanvasPanel& Canvas, TSubclassOf<UFVMapMarkerWidget> WidgetClass, const FFVMarkerView& Marker,
		const FVector2D& Position, float Angle);
	/** Removes the icons of markers the update didn't place. */
	void EndUpdate();
	void Clear();

private:
	TMap<FFVMarkerHandle, TWeakObjectPtr<UFVMapMarkerWidget>> Widgets;
	TSet<FFVMarkerHandle> Placed;
};
