#include "UI/FVRingCompassWidget.h"

#include "FVNavigatorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVRingCompassWidget)

void UFVRingCompassWidget::UpdateCompass(const FFVCompassView& View, const UFVNavigatorComponent& PlayerNavigator, const FVector2D& Size)
{
	// Same turn as the minimap image, so the ring's letters line up with the map.
	const float RingAngle = PlayerNavigator.Minimap.bRotateWithView ? -View.Heading : 0.f;
	if (Ring)
	{
		Ring->SetRenderTransformAngle(RingAngle);
	}

	const double Reach = MarkerRadius > 0.f ? MarkerRadius : 0.5 * FMath::Min(Size.X, Size.Y);
	for (const FFVMarkerView& Marker : View.Markers)
	{
		// Marker.Rotation is the bearing relative to the heading; on screen the bearing turns with the ring.
		double Sin = 0.;
		double Cos = 1.;
		FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(static_cast<double>(Marker.Rotation + View.Heading + RingAngle)));
		PlaceMarker(Marker, Size * 0.5 + FVector2D(Sin, -Cos) * Reach);
	}
}
