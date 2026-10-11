#include "UI/FVMapMarkerWidget.h"

#include "Components/Image.h"
#include "FVMarkerDefinition.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMapMarkerWidget)

void UFVMapMarkerWidget::SetMarker(const FFVMarkerView& InMarker)
{
	Marker = InMarker;

	if (Icon != nullptr && Marker.Definition != nullptr)
	{
		TSoftObjectPtr<UTexture2D> Texture = Marker.Definition->Display.Icon;
		if (!Marker.bDiscovered)
		{
			const FFVMarkerFragment_Discovery* Discovery = Marker.Definition->FindFragment<FFVMarkerFragment_Discovery>();
			if (Discovery && !Discovery->UndiscoveredIcon.IsNull())
			{
				Texture = Discovery->UndiscoveredIcon;
			}
		}

		if (Texture != ShownIcon)
		{
			ShownIcon = Texture;
			if (Texture.IsNull())
			{
				Icon->SetBrushFromTexture(nullptr);
			}
			else
			{
				Icon->SetBrushFromSoftTexture(Texture);
			}
		}
		Icon->SetColorAndOpacity(Marker.Definition->Display.Tint);
	}

	OnMarkerChanged(Marker);
}
