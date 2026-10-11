#include "UI/FVStripCompassWidget.h"

#include "Components/Image.h"
#include "FVNavigatorComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVStripCompassWidget)

namespace FVStripCompass
{
	static const FName OffsetParameter(TEXT("CompassOffset"));
	static const FName TilingParameter(TEXT("CompassTiling"));
}

void UFVStripCompassWidget::NativeConstruct()
{
	Super::NativeConstruct();
	StripMaterial = Strip ? Strip->GetDynamicMaterial() : nullptr;
}

void UFVStripCompassWidget::UpdateCompass(const FFVCompassView& View, const UFVNavigatorComponent& PlayerNavigator, const FVector2D& Size)
{
	if (StripMaterial)
	{
		const float Tiling = PlayerNavigator.Compass.FieldOfView / 360.f;
		StripMaterial->SetScalarParameterValue(FVStripCompass::TilingParameter, Tiling);
		StripMaterial->SetScalarParameterValue(FVStripCompass::OffsetParameter, View.Heading / 360.f - Tiling * 0.5f);
	}

	for (const FFVMarkerView& Marker : View.Markers)
	{
		PlaceMarker(Marker, FVector2D((Marker.Position.X + 1.) * 0.5 * Size.X, Size.Y * 0.5));
	}
}
