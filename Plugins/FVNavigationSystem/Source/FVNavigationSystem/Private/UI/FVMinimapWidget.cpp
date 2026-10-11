#include "UI/FVMinimapWidget.h"

#include "FVNavigatorComponent.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMinimapWidget)

void UFVMinimapWidget::ZoomBy(float Factor)
{
	UFVNavigatorComponent* PlayerNavigator = GetNavigator();
	if (PlayerNavigator && Factor > 0.f)
	{
		PlayerNavigator->ZoomMinimap(1.f / Factor);
	}
}

void UFVMinimapWidget::UpdateView(float DeltaTime)
{
	const UFVNavigatorComponent* PlayerNavigator = GetNavigator();
	const float TargetRadius = PlayerNavigator ? PlayerNavigator->GetMinimapRadius() : 0.f;
	Radius = Radius > 0.f ? FMath::FInterpTo(Radius, TargetRadius, DeltaTime, TransitionSpeed) : TargetRadius;

	const FFVMinimapView View = PlayerNavigator ? PlayerNavigator->BuildMinimapView(Radius) : FFVMinimapView();
	if (View.Map == nullptr)
	{
		if (ShownMap != nullptr)
		{
			ClearMap();
		}
		return;
	}

	ShowLayers(View.Map, View.BaseLayerIndex, View.LayerIndex != View.BaseLayerIndex ? View.LayerIndex : INDEX_NONE);
	ApplyView(View.BaseCenterUV, View.BaseExtentUV, View.MapRotation, DeltaTime);

	if (PlayerIcon && bRotatePlayerIcon)
	{
		PlayerIcon->SetRenderTransformAngle(View.PlayerRotation);
	}

	const FVector2D Half = ViewSize * 0.5;
	BeginMarkers();
	for (const FFVMarkerView& Marker : View.Markers)
	{
		const FVector2D Reach = Marker.bClamped ? Half - FVector2D(EdgePadding) : Half;
		PlaceMarker(Marker, Half + Marker.Position * Reach, Marker.Rotation);
	}
	EndMarkers();
}
