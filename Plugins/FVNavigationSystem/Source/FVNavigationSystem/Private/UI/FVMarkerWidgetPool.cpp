#include "UI/FVMarkerWidgetPool.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "UI/FVMapMarkerWidget.h"

void FFVMarkerWidgetPool::BeginUpdate()
{
	Placed.Reset();
}

void FFVMarkerWidgetPool::Place(UUserWidget& Owner, UCanvasPanel& Canvas, TSubclassOf<UFVMapMarkerWidget> WidgetClass,
	const FFVMarkerView& Marker, const FVector2D& Position, float Angle)
{
	if (WidgetClass == nullptr)
	{
		return;
	}

	TWeakObjectPtr<UFVMapMarkerWidget>& Entry = Widgets.FindOrAdd(Marker.Handle);
	UFVMapMarkerWidget* Widget = Entry.Get();
	if (Widget == nullptr)
	{
		Widget = CreateWidget<UFVMapMarkerWidget>(&Owner, WidgetClass);
		if (Widget == nullptr)
		{
			return;
		}

		UCanvasPanelSlot* CanvasSlot = Canvas.AddChildToCanvas(Widget);
		CanvasSlot->SetAutoSize(true);
		CanvasSlot->SetAlignment(FVector2D(0.5));
		Entry = Widget;
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Widget->Slot))
	{
		CanvasSlot->SetPosition(Position);
		CanvasSlot->SetZOrder(Placed.Num());
	}
	Widget->SetRenderTransformAngle(Angle);
	Widget->SetMarker(Marker);
	Placed.Add(Marker.Handle);
}

void FFVMarkerWidgetPool::EndUpdate()
{
	for (auto It = Widgets.CreateIterator(); It; ++It)
	{
		if (Placed.Contains(It.Key()))
		{
			continue;
		}

		if (UFVMapMarkerWidget* Widget = It.Value().Get())
		{
			Widget->RemoveFromParent();
		}
		It.RemoveCurrent();
	}
}

void FFVMarkerWidgetPool::Clear()
{
	Placed.Reset();
	EndUpdate();
}
