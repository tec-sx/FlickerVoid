#include "FVMapDefinition.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMapDefinition)

#define LOCTEXT_NAMESPACE "FVMapDefinition"

bool FFVMapLayer::ContainsXY(const FVector& Location) const
{
	return Location.X >= WorldMin.X && Location.X <= WorldMax.X
		&& Location.Y >= WorldMin.Y && Location.Y <= WorldMax.Y;
}

bool FFVMapLayer::Contains(const FVector& Location) const
{
	return ContainsXY(Location) && (!bLimitHeight || (Location.Z >= MinZ && Location.Z <= MaxZ));
}

FVector2D FFVMapLayer::WorldToUV(const FVector& Location) const
{
	const FVector2D Size = GetWorldSize();
	if (Size.X <= UE_KINDA_SMALL_NUMBER || Size.Y <= UE_KINDA_SMALL_NUMBER)
	{
		return FVector2D(0.5f);
	}

	return FVector2D(
		(Location.Y - WorldMin.Y) / Size.Y,
		(WorldMax.X - Location.X) / Size.X);
}

FVector FFVMapLayer::UVToWorld(const FVector2D& UV, float Z) const
{
	const FVector2D Size = GetWorldSize();
	return FVector(WorldMax.X - UV.Y * Size.X, WorldMin.Y + UV.X * Size.Y, Z);
}

int32 UFVMapDefinition::FindLayerAt(const FVector& Location) const
{
	int32 Fallback = INDEX_NONE;
	for (int32 Index = 0; Index < Layers.Num(); ++Index)
	{
		const FFVMapLayer& Layer = Layers[Index];
		if (!Layer.Contains(Location))
		{
			continue;
		}

		if (Layer.bLimitHeight)
		{
			return Index;
		}

		if (Fallback == INDEX_NONE)
		{
			Fallback = Index;
		}
	}
	return Fallback;
}

int32 UFVMapDefinition::FindLayerByName(FName LayerName) const
{
	return Layers.IndexOfByPredicate([LayerName](const FFVMapLayer& Layer) { return Layer.Name == LayerName; });
}

#if WITH_EDITOR
EDataValidationResult UFVMapDefinition::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	if (Layers.IsEmpty())
	{
		Context.AddError(FText::Format(LOCTEXT("NoLayers", "{0} has no layers."), FText::FromName(GetFName())));
		Result = EDataValidationResult::Invalid;
	}

	for (const FFVMapLayer& Layer : Layers)
	{
		const FVector2D Size = Layer.GetWorldSize();
		if (Size.X <= 0.f || Size.Y <= 0.f)
		{
			Context.AddError(FText::Format(LOCTEXT("EmptyArea", "{0}: layer {1} covers no area."),
				FText::FromName(GetFName()), FText::FromName(Layer.Name)));
			Result = EDataValidationResult::Invalid;
		}

		if (Layer.bLimitHeight && Layer.MinZ >= Layer.MaxZ)
		{
			Context.AddError(FText::Format(LOCTEXT("EmptyHeight", "{0}: layer {1} has MinZ above MaxZ."),
				FText::FromName(GetFName()), FText::FromName(Layer.Name)));
			Result = EDataValidationResult::Invalid;
		}

		if (Layer.Texture.IsNull())
		{
			Context.AddWarning(FText::Format(LOCTEXT("NoTexture", "{0}: layer {1} has no texture yet."),
				FText::FromName(GetFName()), FText::FromName(Layer.Name)));
		}
	}

	return Result;
}
#endif

#undef LOCTEXT_NAMESPACE
