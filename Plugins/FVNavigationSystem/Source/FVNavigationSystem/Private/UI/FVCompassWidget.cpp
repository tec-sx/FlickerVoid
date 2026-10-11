#include "UI/FVCompassWidget.h"

#include "Components/CanvasPanel.h"
#include "FVNavigatorComponent.h"
#include "UI/FVMapMarkerWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCompassWidget)

void UFVCompassWidget::NativeDestruct()
{
	MarkerPool.Clear();
	Super::NativeDestruct();
}

void UFVCompassWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (const UFVNavigatorComponent* PlayerNavigator = GetNavigator())
	{
		MarkerPool.BeginUpdate();
		UpdateCompass(PlayerNavigator->BuildCompassView(), *PlayerNavigator, MyGeometry.GetLocalSize());
		MarkerPool.EndUpdate();
	}
}

UFVNavigatorComponent* UFVCompassWidget::GetNavigator() const
{
	if (!Navigator.IsValid())
	{
		Navigator = UFVNavigatorComponent::FindForPlayer(GetOwningPlayer());
	}
	return Navigator.Get();
}

void UFVCompassWidget::PlaceMarker(const FFVMarkerView& Marker, const FVector2D& Position)
{
	if (MarkerCanvas)
	{
		MarkerPool.Place(*this, *MarkerCanvas, MarkerWidgetClass, Marker, Position, 0.f);
	}
}
