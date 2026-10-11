#include "UI/FVMapViewWidget.h"

#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"
#include "FVMapDefinition.h"
#include "FVNavigationSystem.h"
#include "FVNavigatorComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UI/FVMapMarkerWidget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVMapViewWidget)

namespace FVMapView
{
	static const FName BaseTextureParameter(TEXT("WorldMapTexture"));
	static const FName RoomTextureParameter(TEXT("IndoorMapTexture"));
	static const FName CenterUVParameter(TEXT("CenterUV"));
	static const FName ExtentUVParameter(TEXT("ExtentUV"));
	static const FName MapRotationParameter(TEXT("MapRotation"));
	static const FName RoomRectParameter(TEXT("RoomRect"));
	static const FName DimAmountParameter(TEXT("DimAmount"));

	/** RoomRect outside the image, so no room shows. */
	static const FLinearColor NoRoom(-2.f, -2.f, -1.f, -1.f);

	static FLinearColor ToColor(const FVector2D& RG, const FVector2D& BA = FVector2D::ZeroVector)
	{
		return FLinearColor(static_cast<float>(RG.X), static_cast<float>(RG.Y), static_cast<float>(BA.X), static_cast<float>(BA.Y));
	}

	static void CancelLoad(TSharedPtr<FStreamableHandle>& Handle)
	{
		if (Handle.IsValid())
		{
			Handle->CancelHandle();
			Handle.Reset();
		}
	}
}

void UFVMapViewWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Material = MapImage ? MapImage->GetDynamicMaterial() : nullptr;
	if (Material == nullptr)
	{
		UE_LOG(LogFVNavigationSystem, Warning, TEXT("%s: MapImage needs a material as its brush."), *GetName());
	}

	ClearMap();
}

void UFVMapViewWidget::NativeDestruct()
{
	FVMapView::CancelLoad(BaseTextureHandle);
	FVMapView::CancelLoad(RoomTextureHandle);
	MarkerPool.Clear();
	PlacedMarkers.Reset();
	Super::NativeDestruct();
}

void UFVMapViewWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	ViewSize = MyGeometry.GetLocalSize();
	if (Material && ViewSize.X > 0. && ViewSize.Y > 0.)
	{
		UpdateView(InDeltaTime);
	}
}

FVector UFVMapViewWidget::LocalToWorld(FVector2D LocalPosition) const
{
	const FFVMapLayer* Base = GetBaseLayer();
	if (Base == nullptr)
	{
		return FVector::ZeroVector;
	}

	const UFVNavigatorComponent* PlayerNavigator = GetNavigator();
	float Z = PlayerNavigator ? static_cast<float>(PlayerNavigator->GetViewLocation().Z) : 0.f;
	if (const FFVMapLayer* Room = ShownMap->GetLayer(RoomLayer))
	{
		Z = FMath::Clamp(Z, Room->MinZ, Room->MaxZ);
	}
	return Base->UVToWorld(LocalToBaseUV(LocalPosition), Z);
}

bool UFVMapViewWidget::FindMarkerAt(FVector2D LocalPosition, float MaxDistance, FFVMarkerView& OutMarker) const
{
	const FFVMarkerView* Best = nullptr;
	double BestDistanceSquared = FMath::Square(MaxDistance);

	// Placed in draw order, so "<=" lets the later, top-most marker win a tie.
	for (const TPair<FFVMarkerView, FVector2D>& Placed : PlacedMarkers)
	{
		const double DistanceSquared = FVector2D::DistSquared(Placed.Value, LocalPosition);
		if (DistanceSquared <= BestDistanceSquared)
		{
			Best = &Placed.Key;
			BestDistanceSquared = DistanceSquared;
		}
	}

	OutMarker = Best ? *Best : FFVMarkerView();
	return Best != nullptr;
}

UFVNavigatorComponent* UFVMapViewWidget::GetNavigator() const
{
	if (!Navigator.IsValid())
	{
		Navigator = UFVNavigatorComponent::FindForPlayer(GetOwningPlayer());
	}
	return Navigator.Get();
}

void UFVMapViewWidget::ShowLayers(UFVMapDefinition* Map, int32 InBaseLayer, int32 InRoomLayer)
{
	if (Material == nullptr || (Map == ShownMap && InBaseLayer == BaseLayer && InRoomLayer == RoomLayer))
	{
		return;
	}

	const bool bBaseChanged = Map != ShownMap || InBaseLayer != BaseLayer;
	ShownMap = Map;
	BaseLayer = InBaseLayer;
	RoomLayer = InRoomLayer;

	const FFVMapLayer* Base = Map ? Map->GetLayer(BaseLayer) : nullptr;
	const FFVMapLayer* Room = Map ? Map->GetLayer(RoomLayer) : nullptr;

	if (bBaseChanged)
	{
		LoadTexture(Base, BaseTextureHandle, [this](UTexture2D* Texture)
		{
			Material->SetTextureParameterValue(FVMapView::BaseTextureParameter, Texture);
		});
	}

	// The room shows once its texture is in, so the previous room's image never flashes in its place.
	Material->SetVectorParameterValue(FVMapView::RoomRectParameter, FVMapView::NoRoom);
	TargetDim = 0.f;
	if (Base && Room)
	{
		FVector2D Min;
		FVector2D Max;
		GetRoomArea(*Base, *Room, Min, Max);
		const FLinearColor Rect = FVMapView::ToColor(Min, Max);
		LoadTexture(Room, RoomTextureHandle, [this, Rect](UTexture2D* Texture)
		{
			Material->SetTextureParameterValue(FVMapView::RoomTextureParameter, Texture);
			Material->SetVectorParameterValue(FVMapView::RoomRectParameter, Rect);
			TargetDim = RoomDim;
		});
	}
	else
	{
		FVMapView::CancelLoad(RoomTextureHandle);
	}

	MapImage->SetVisibility(Base ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	OnShownMapChanged(Map, GetShownLayer());
}

void UFVMapViewWidget::ClearMap()
{
	ShownMap = nullptr;
	BaseLayer = INDEX_NONE;
	RoomLayer = INDEX_NONE;
	FVMapView::CancelLoad(BaseTextureHandle);
	FVMapView::CancelLoad(RoomTextureHandle);
	MarkerPool.Clear();
	PlacedMarkers.Reset();

	if (MapImage)
	{
		MapImage->SetVisibility(ESlateVisibility::Hidden);
	}
	OnShownMapChanged(nullptr, INDEX_NONE);
}

void UFVMapViewWidget::ApplyView(const FVector2D& InCenterUV, const FVector2D& InExtentUV, float InRotation, float DeltaTime)
{
	CenterUV = InCenterUV;
	ExtentUV = InExtentUV;
	Rotation = InRotation;
	Dim = FMath::FInterpTo(Dim, TargetDim, DeltaTime, TransitionSpeed);

	Material->SetVectorParameterValue(FVMapView::CenterUVParameter, FVMapView::ToColor(CenterUV));
	Material->SetVectorParameterValue(FVMapView::ExtentUVParameter, FVMapView::ToColor(ExtentUV));
	Material->SetScalarParameterValue(FVMapView::MapRotationParameter, Rotation);
	Material->SetScalarParameterValue(FVMapView::DimAmountParameter, Dim);
}

void UFVMapViewWidget::BeginMarkers()
{
	MarkerPool.BeginUpdate();
	PlacedMarkers.Reset();
}

void UFVMapViewWidget::PlaceMarker(const FFVMarkerView& Marker, const FVector2D& Position, float Angle)
{
	PlacedMarkers.Emplace(Marker, Position);
	if (MarkerCanvas)
	{
		MarkerPool.Place(*this, *MarkerCanvas, MarkerWidgetClass, Marker, Position, Angle);
	}
}

void UFVMapViewWidget::EndMarkers()
{
	MarkerPool.EndUpdate();
}

const FFVMapLayer* UFVMapViewWidget::GetBaseLayer() const
{
	return ShownMap ? ShownMap->GetLayer(BaseLayer) : nullptr;
}

FVector2D UFVMapViewWidget::BaseUVToLocal(const FVector2D& UV) const
{
	// Inverse of the map material's rotation, see the setup guide.
	const FVector2D Q = (UV - CenterUV) / ExtentUV;
	double Sin = 0.;
	double Cos = 1.;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(static_cast<double>(Rotation)));
	const FVector2D P(Cos * Q.X - Sin * Q.Y, Sin * Q.X + Cos * Q.Y);
	return (P * 0.5 + FVector2D(0.5)) * ViewSize;
}

FVector2D UFVMapViewWidget::LocalToBaseUV(const FVector2D& Local) const
{
	if (ViewSize.X <= 0. || ViewSize.Y <= 0.)
	{
		return CenterUV;
	}

	const FVector2D P = (Local / ViewSize - FVector2D(0.5)) * 2.;
	double Sin = 0.;
	double Cos = 1.;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(static_cast<double>(Rotation)));
	const FVector2D Q(P.X * Cos + P.Y * Sin, -P.X * Sin + P.Y * Cos);
	return CenterUV + Q * ExtentUV;
}

void UFVMapViewWidget::GetRoomArea(const FFVMapLayer& Base, const FFVMapLayer& Room, FVector2D& OutMin, FVector2D& OutMax)
{
	OutMin = Base.WorldToUV(FVector(Room.WorldMax.X, Room.WorldMin.Y, 0.));
	OutMax = Base.WorldToUV(FVector(Room.WorldMin.X, Room.WorldMax.Y, 0.));
}

void UFVMapViewWidget::LoadTexture(const FFVMapLayer* Layer, TSharedPtr<FStreamableHandle>& Handle, TFunction<void(UTexture2D*)>&& OnLoaded)
{
	FVMapView::CancelLoad(Handle);
	if (Material == nullptr || Layer == nullptr || Layer->Texture.IsNull())
	{
		return;
	}

	if (UTexture2D* Loaded = Layer->Texture.Get())
	{
		OnLoaded(Loaded);
		return;
	}

	const TSoftObjectPtr<UTexture2D> Texture = Layer->Texture;
	Handle = UAssetManager::GetStreamableManager().RequestAsyncLoad(Texture.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [Texture, OnLoaded = MoveTemp(OnLoaded)]()
		{
			if (UTexture2D* Loaded = Texture.Get())
			{
				OnLoaded(Loaded);
			}
		}));
}
