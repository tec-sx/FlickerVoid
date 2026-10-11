#include "UI/FVCompassWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "FVNavigatorComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/FVMapMarkerWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVCompassWidget)

namespace FVCompass
{
	static const FName OffsetParameter(TEXT("CompassOffset"));
	static const FName TilingParameter(TEXT("CompassTiling"));
}

void UFVCompassWidget::NativeConstruct()
{
	Super::NativeConstruct();
	StripMaterial = Style == EFVCompassStyle::Strip && Strip ? Strip->GetDynamicMaterial() : nullptr;
}

void UFVCompassWidget::NativeDestruct()
{
	MarkerPool.Clear();
	Super::NativeDestruct();
}

void UFVCompassWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	const UFVNavigatorComponent* PlayerNavigator = GetNavigator();
	if (PlayerNavigator == nullptr)
	{
		return;
	}

	const FFVCompassView View = PlayerNavigator->BuildCompassView();
	const FVector2D Size = MyGeometry.GetLocalSize();
	MarkerPool.BeginUpdate();

	if (Style == EFVCompassStyle::Ring)
	{
		// Same turn as the minimap image, so the ring's letters line up with the map.
		const float RingAngle = PlayerNavigator->Minimap.bRotateWithView ? -View.Heading : 0.f;
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
			if (MarkerCanvas)
			{
				MarkerPool.Place(*this, *MarkerCanvas, MarkerWidgetClass, Marker, Size * 0.5 + FVector2D(Sin, -Cos) * Reach, 0.f);
			}
		}
	}
	else
	{
		if (StripMaterial)
		{
			const float Tiling = PlayerNavigator->Compass.FieldOfView / 360.f;
			StripMaterial->SetScalarParameterValue(FVCompass::TilingParameter, Tiling);
			StripMaterial->SetScalarParameterValue(FVCompass::OffsetParameter, View.Heading / 360.f - Tiling * 0.5f);
		}

		for (const FFVMarkerView& Marker : View.Markers)
		{
			if (MarkerCanvas)
			{
				const FVector2D Position((Marker.Position.X + 1.) * 0.5 * Size.X, Size.Y * 0.5);
				MarkerPool.Place(*this, *MarkerCanvas, MarkerWidgetClass, Marker, Position, 0.f);
			}
		}
	}

	MarkerPool.EndUpdate();
}

UFVNavigatorComponent* UFVCompassWidget::GetNavigator()
{
	if (!Navigator.IsValid())
	{
		Navigator = UFVNavigatorComponent::FindForPlayer(GetOwningPlayer());
	}
	return Navigator.Get();
}
