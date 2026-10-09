#include "FVNavigationLibrary.h"

#include "Engine/AssetManager.h"
#include "Engine/Engine.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"
#include "FVMarkerDefinition.h"
#include "LatentActions.h"
#include "Materials/MaterialInstanceDynamic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVNavigationLibrary)

#define LOCTEXT_NAMESPACE "FVNavigationLibrary"

namespace FVNavigationLibrary
{
	class FLoadTextureAction : public FPendingLatentAction
	{
	public:
		FLoadTextureAction(const FLatentActionInfo& LatentInfo, const TSoftObjectPtr<UTexture2D>& InTexture, UTexture2D*& InOutTexture,
			EFVMapLoadResult& InResult)
			: ExecutionFunction(LatentInfo.ExecutionFunction)
			, OutputLink(LatentInfo.Linkage)
			, CallbackTarget(LatentInfo.CallbackTarget)
			, Texture(InTexture)
			, OutTexture(InOutTexture)
			, Result(InResult)
		{
			if (!Texture.IsNull() && Texture.Get() == nullptr)
			{
				Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Texture.ToSoftObjectPath());
			}
		}

		virtual void UpdateOperation(FLatentResponse& Response) override
		{
			if (Handle.IsValid() && Handle->IsLoadingInProgress())
			{
				return;
			}

			OutTexture = Texture.Get();
			Result = OutTexture ? EFVMapLoadResult::Loaded : EFVMapLoadResult::Failed;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}

	private:
		FName ExecutionFunction;
		int32 OutputLink;
		FWeakObjectPtr CallbackTarget;
		TSoftObjectPtr<UTexture2D> Texture;
		UTexture2D*& OutTexture;
		EFVMapLoadResult& Result;
		TSharedPtr<FStreamableHandle> Handle;
	};

	static FVector2D SafeDivide(const FVector2D& A, const FVector2D& B)
	{
		return FVector2D(B.X != 0. ? A.X / B.X : 0., B.Y != 0. ? A.Y / B.Y : 0.);
	}
}

bool UFVNavigationLibrary::GetMapLayer(const UFVMapDefinition* Map, int32 LayerIndex, FFVMapLayer& OutLayer)
{
	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	if (Layer == nullptr)
	{
		OutLayer = FFVMapLayer();
		return false;
	}

	OutLayer = *Layer;
	return true;
}

void UFVNavigationLibrary::AsyncLoadMapLayer(UObject* WorldContextObject, const UFVMapDefinition* Map, int32 LayerIndex,
	FFVMapLayer& OutLayer, UTexture2D*& OutTexture, EFVMapLoadResult& Result, FLatentActionInfo LatentInfo)
{
	OutTexture = nullptr;
	Result = EFVMapLoadResult::Failed;
	GetMapLayer(Map, LayerIndex, OutLayer);

	UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull) : nullptr;
	if (World == nullptr)
	{
		return;
	}

	FLatentActionManager& LatentManager = World->GetLatentActionManager();
	if (LatentManager.FindExistingAction<FVNavigationLibrary::FLoadTextureAction>(LatentInfo.CallbackTarget, LatentInfo.UUID) == nullptr)
	{
		LatentManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID,
			new FVNavigationLibrary::FLoadTextureAction(LatentInfo, OutLayer.Texture, OutTexture, Result));
	}
}

FVector2D UFVNavigationLibrary::WorldToMapUV(const UFVMapDefinition* Map, int32 LayerIndex, FVector Location)
{
	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	return Layer ? Layer->WorldToUV(Location) : FVector2D::ZeroVector;
}

FVector UFVNavigationLibrary::MapUVToWorld(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D UV, float Z)
{
	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	return Layer ? Layer->UVToWorld(UV, Z) : FVector::ZeroVector;
}

bool UFVNavigationLibrary::GetLayerViewArea(const UFVMapDefinition* Map, int32 LayerIndex, FVector Center, float Radius,
	FVector2D& OutCenterUV, FVector2D& OutExtentUV)
{
	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	if (Layer == nullptr)
	{
		OutCenterUV = FVector2D(0.5);
		OutExtentUV = FVector2D(0.5);
		return false;
	}

	OutCenterUV = Layer->WorldToUV(Center);
	OutExtentUV = Layer->RadiusToExtentUV(FMath::Max(Radius, 1.f));
	return true;
}

void UFVNavigationLibrary::SetMaterialVector2D(UMaterialInstanceDynamic* Material, FName ParameterName, FVector2D Value)
{
	if (Material != nullptr)
	{
		Material->SetVectorParameterValue(ParameterName, FLinearColor(Value.X, Value.Y, 0.f, 0.f));
	}
}

void UFVNavigationLibrary::ApplyMapView(UMaterialInstanceDynamic* Material, FVector2D CenterUV, FVector2D ExtentUV, float MapRotation, FName Prefix)
{
	if (Material == nullptr)
	{
		return;
	}

	const FString PrefixString = Prefix.IsNone() ? FString() : Prefix.ToString();
	SetMaterialVector2D(Material, FName(PrefixString + TEXT("CenterUV")), CenterUV);
	SetMaterialVector2D(Material, FName(PrefixString + TEXT("ExtentUV")), ExtentUV);
	Material->SetScalarParameterValue(TEXT("MapRotation"), MapRotation);
}

FVector2D UFVNavigationLibrary::MinimapToWidget(FVector2D MarkerPosition, FVector2D FrameSize, float EdgePadding)
{
	const FVector2D Half = FrameSize * 0.5;
	return Half + MarkerPosition * (Half - FVector2D(EdgePadding));
}

float UFVNavigationLibrary::GetMinimapEdgeAngle(FVector2D MarkerPosition)
{
	return FMath::RadiansToDegrees(FMath::Atan2(MarkerPosition.X, -MarkerPosition.Y));
}

float UFVNavigationLibrary::CompassToWidget(float MarkerPositionX, float StripWidth)
{
	return (MarkerPositionX + 1.f) * 0.5f * StripWidth;
}

bool UFVNavigationLibrary::GetCompassBearingPosition(float Bearing, float Heading, float FieldOfView, float& OutPosition)
{
	const float Half = FMath::Max(FieldOfView * 0.5f, 1.f);
	const float Relative = FRotator::NormalizeAxis(Bearing - Heading);
	OutPosition = Relative / Half;
	return FMath::Abs(Relative) <= Half;
}

void UFVNavigationLibrary::GetCompassStripUV(float Heading, float FieldOfView, float& OutOffset, float& OutTiling)
{
	OutTiling = FieldOfView / 360.f;
	OutOffset = FRotator::ClampAxis(Heading) / 360.f - OutTiling * 0.5f;
}

FVector2D UFVNavigationLibrary::GetWorldMapExtent(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D ViewSize, float Zoom)
{
	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	const FVector2D Size = Layer ? Layer->GetWorldSize() : FVector2D(1.);
	const double ImageAspect = Size.X > 0. ? Size.Y / Size.X : 1.;
	const double ViewAspect = ViewSize.Y > 0. ? ViewSize.X / ViewSize.Y : 1.;
	const double Aspect = FMath::Max(ViewAspect / ImageAspect, 1e-4);
	const double Half = 0.5 / FMath::Max(static_cast<double>(Zoom), 1e-4);
	return FVector2D(Half * FMath::Max(Aspect, 1.), Half * FMath::Max(1. / Aspect, 1.));
}

FVector2D UFVNavigationLibrary::MapUVToScreen(FVector2D UV, FVector2D PanUV, FVector2D ExtentUV, FVector2D ViewSize)
{
	return (FVNavigationLibrary::SafeDivide(UV - PanUV, ExtentUV * 2.) + FVector2D(0.5)) * ViewSize;
}

FVector2D UFVNavigationLibrary::ScreenToMapUV(FVector2D ScreenPosition, FVector2D PanUV, FVector2D ExtentUV, FVector2D ViewSize)
{
	return PanUV + (FVNavigationLibrary::SafeDivide(ScreenPosition, ViewSize) - FVector2D(0.5)) * 2. * ExtentUV;
}

FVector UFVNavigationLibrary::ScreenToWorld(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D ScreenPosition, FVector2D PanUV,
	FVector2D ExtentUV, FVector2D ViewSize, float Z)
{
	return MapUVToWorld(Map, LayerIndex, ScreenToMapUV(ScreenPosition, PanUV, ExtentUV, ViewSize), Z);
}

FVector2D UFVNavigationLibrary::ClampMapPan(FVector2D PanUV, FVector2D ExtentUV)
{
	auto ClampAxis = [](double Pan, double Extent) { return Extent >= 0.5 ? 0.5 : FMath::Clamp(Pan, Extent, 1. - Extent); };
	return FVector2D(ClampAxis(PanUV.X, ExtentUV.X), ClampAxis(PanUV.Y, ExtentUV.Y));
}

FVector2D UFVNavigationLibrary::PanMap(FVector2D PanUV, FVector2D DeltaPixels, FVector2D ExtentUV, FVector2D ViewSize)
{
	return ClampMapPan(PanUV - FVNavigationLibrary::SafeDivide(DeltaPixels, ViewSize) * 2. * ExtentUV, ExtentUV);
}

void UFVNavigationLibrary::ZoomMapAt(const UFVMapDefinition* Map, int32 LayerIndex, FVector2D ViewSize, FVector2D ScreenPosition,
	FVector2D PanUV, FVector2D ExtentUV, float NewZoom, FVector2D& OutPanUV, FVector2D& OutExtentUV)
{
	const FVector2D AnchorUV = ScreenToMapUV(ScreenPosition, PanUV, ExtentUV, ViewSize);
	OutExtentUV = GetWorldMapExtent(Map, LayerIndex, ViewSize, NewZoom);
	OutPanUV = ClampMapPan(AnchorUV - (FVNavigationLibrary::SafeDivide(ScreenPosition, ViewSize) - FVector2D(0.5)) * 2. * OutExtentUV, OutExtentUV);
}

bool UFVNavigationLibrary::FindMarkerAtScreen(const TArray<FFVMarkerView>& Markers, FVector2D ScreenPosition, FVector2D PanUV,
	FVector2D ExtentUV, FVector2D ViewSize, float MaxDistance, FFVMarkerView& OutMarker)
{
	const FFVMarkerView* Best = nullptr;
	double BestDistanceSquared = FMath::Square(MaxDistance);

	// Views are sorted by priority with the top-most last, so "<=" lets later markers win ties.
	for (const FFVMarkerView& Marker : Markers)
	{
		const double DistanceSquared = FVector2D::DistSquared(MapUVToScreen(Marker.Position, PanUV, ExtentUV, ViewSize), ScreenPosition);
		if (DistanceSquared <= BestDistanceSquared)
		{
			Best = &Marker;
			BestDistanceSquared = DistanceSquared;
		}
	}

	OutMarker = Best ? *Best : FFVMarkerView();
	return Best != nullptr;
}

void UFVNavigationLibrary::GetMarkerAppearance(const FFVMarkerView& Marker, TSoftObjectPtr<UTexture2D>& OutIcon, FLinearColor& OutTint)
{
	OutIcon.Reset();
	OutTint = FLinearColor::White;

	const UFVMarkerDefinition* Definition = Marker.Definition;
	if (Definition == nullptr)
	{
		return;
	}

	OutIcon = Definition->Display.Icon;
	OutTint = Definition->Display.Tint;

	if (!Marker.bDiscovered)
	{
		const FFVMarkerFragment_Discovery* Discovery = Definition->FindFragment<FFVMarkerFragment_Discovery>();
		if (Discovery && !Discovery->UndiscoveredIcon.IsNull())
		{
			OutIcon = Discovery->UndiscoveredIcon;
		}
	}
}

FText UFVNavigationLibrary::FormatDistance(float Distance)
{
	const float Metres = FMath::Max(Distance, 0.f) / 100.f;
	if (Metres < 1000.f)
	{
		return FText::Format(LOCTEXT("Metres", "{0} m"), FText::AsNumber(FMath::RoundToInt(Metres)));
	}

	FNumberFormattingOptions Options;
	Options.MinimumFractionalDigits = 1;
	Options.MaximumFractionalDigits = 1;
	return FText::Format(LOCTEXT("Kilometres", "{0} km"), FText::AsNumber(Metres / 1000.f, &Options));
}

#undef LOCTEXT_NAMESPACE
