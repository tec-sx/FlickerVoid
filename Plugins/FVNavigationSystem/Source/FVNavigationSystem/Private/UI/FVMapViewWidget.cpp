#include "UI/FVMapViewWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"
#include "FVMapDefinition.h"
#include "FVNavigationSubsystem.h"
#include "FVNavigationSystem.h"
#include "FVNavigatorComponent.h"
#include "InputCoreTypes.h"
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

	/** Mouse travel in pixels below which a press and release counts as a click. */
	static constexpr float ClickTolerance = 4.f;

	static FLinearColor ToColor(const FVector2D& RG, const FVector2D& BA = FVector2D::ZeroVector)
	{
		return FLinearColor(static_cast<float>(RG.X), static_cast<float>(RG.Y), static_cast<float>(BA.X), static_cast<float>(BA.Y));
	}

	static FVector2D SafeDivide(const FVector2D& A, const FVector2D& B)
	{
		return FVector2D(B.X != 0. ? A.X / B.X : 0., B.Y != 0. ? A.Y / B.Y : 0.);
	}

	/** Room area in base layer UV: north-west and south-east corners. */
	static void GetRoomArea(const FFVMapLayer& Base, const FFVMapLayer& Room, FVector2D& OutMin, FVector2D& OutMax)
	{
		OutMin = Base.WorldToUV(FVector(Room.WorldMax.X, Room.WorldMin.Y, 0.));
		OutMax = Base.WorldToUV(FVector(Room.WorldMin.X, Room.WorldMax.Y, 0.));
	}

	static void CancelLoad(TSharedPtr<FStreamableHandle>& Handle)
	{
		if (Handle.IsValid())
		{
			Handle->CancelHandle();
			Handle.Reset();
		}
	}

	static FVector2D ClampPan(const FVector2D& PanUV, const FVector2D& ExtentUV)
	{
		auto ClampAxis = [](double Pan, double Extent) { return Extent >= 0.5 ? 0.5 : FMath::Clamp(Pan, Extent, 1. - Extent); };
		return FVector2D(ClampAxis(PanUV.X, ExtentUV.X), ClampAxis(PanUV.Y, ExtentUV.Y));
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
	if (Mode == EFVMapViewMode::WorldMap)
	{
		SetVisibility(ESlateVisibility::Visible);
		ShowMap(nullptr, INDEX_NONE);
	}
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
	if (Material == nullptr || ViewSize.X <= 0. || ViewSize.Y <= 0.)
	{
		return;
	}

	if (Mode == EFVMapViewMode::Minimap)
	{
		UpdateMinimap(InDeltaTime);
	}
	else
	{
		UpdateWorldMap(InDeltaTime);
	}
}

void UFVMapViewWidget::UpdateMinimap(float DeltaTime)
{
	UFVNavigatorComponent* PlayerNavigator = GetNavigator();
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
	MarkerPool.BeginUpdate();
	PlacedMarkers.Reset();
	for (const FFVMarkerView& Marker : View.Markers)
	{
		const FVector2D Reach = Marker.bClamped ? Half - FVector2D(EdgePadding) : Half;
		PlaceMarker(Marker, Half + Marker.Position * Reach, Marker.Rotation);
	}
	MarkerPool.EndUpdate();
}

void UFVMapViewWidget::UpdateWorldMap(float DeltaTime)
{
	const FFVMapLayer* Base = GetBaseLayer();
	if (Base == nullptr)
	{
		return;
	}

	const FVector2D Extent = GetWorldMapExtent(Zoom);
	PanUV = FVMapView::ClampPan(PanUV, Extent);
	ApplyView(PanUV, Extent, 0.f, DeltaTime);

	const UFVNavigatorComponent* PlayerNavigator = GetNavigator();
	const int32 LayerIndex = GetShownLayer();
	const FFVMapLayer* Layer = ShownMap->GetLayer(LayerIndex);
	if (PlayerNavigator == nullptr || Layer == nullptr)
	{
		return;
	}

	const FFVWorldMapView View = PlayerNavigator->BuildWorldMapView(ShownMap, LayerIndex, Zoom);
	auto ToLocal = [this, Base, Layer, LayerIndex](const FVector2D& LayerUV)
	{
		return BaseUVToLocal(LayerIndex == BaseLayer ? LayerUV : Base->WorldToUV(Layer->UVToWorld(LayerUV, 0.f)));
	};

	if (PlayerIcon)
	{
		PlayerIcon->SetVisibility(View.bPlayerOnLayer ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
		PlayerIcon->SetRenderTranslation(ToLocal(View.PlayerUV) - ViewSize * 0.5);
		PlayerIcon->SetRenderTransformAngle(View.PlayerRotation);
	}

	MarkerPool.BeginUpdate();
	PlacedMarkers.Reset();
	for (const FFVMarkerView& Marker : View.Markers)
	{
		const FVector2D Local = ToLocal(Marker.Position);
		if (Local.X >= 0. && Local.Y >= 0. && Local.X <= ViewSize.X && Local.Y <= ViewSize.Y)
		{
			PlaceMarker(Marker, Local, Marker.Rotation);
		}
	}
	MarkerPool.EndUpdate();
}

void UFVMapViewWidget::ShowMap(UFVMapDefinition* Map, int32 LayerIndex)
{
	if (Map == nullptr)
	{
		if (const UFVNavigatorComponent* PlayerNavigator = GetNavigator())
		{
			Map = PlayerNavigator->GetActiveMap();
			LayerIndex = PlayerNavigator->GetActiveLayer();
		}
	}

	if (Map == nullptr)
	{
		const UFVNavigationSubsystem* Subsystem = UFVNavigationSubsystem::Get(this);
		const TArray<UFVMapDefinition*> Maps = Subsystem ? Subsystem->GetMaps() : TArray<UFVMapDefinition*>();
		Map = Maps.IsEmpty() ? nullptr : Maps[0];
		LayerIndex = 0;
	}

	const FFVMapLayer* Layer = Map ? Map->GetLayer(LayerIndex) : nullptr;
	if (Layer == nullptr)
	{
		ClearMap();
		return;
	}

	int32 Base = LayerIndex;
	int32 Room = INDEX_NONE;
	const FVector2D LayerMiddle = (Layer->WorldMin + Layer->WorldMax) * 0.5;
	if (Layer->bLimitHeight)
	{
		const int32 Found = Map->FindBaseLayerAt(FVector(LayerMiddle, 0.));
		if (Found != INDEX_NONE)
		{
			Base = Found;
			Room = LayerIndex;
		}
	}

	const bool bNewBase = Map != ShownMap || Base != BaseLayer;
	ShowLayers(Map, Base, Room);

	if (Room != INDEX_NONE)
	{
		// Centre on the room and zoom until it fills most of the view.
		FVector2D Min;
		FVector2D Max;
		FVMapView::GetRoomArea(*Map->GetLayer(Base), *Layer, Min, Max);
		PanUV = (Min + Max) * 0.5;
		const FVector2D Fit = FVMapView::SafeDivide(GetWorldMapExtent(1.f), (Max - Min) * 0.6);
		Zoom = FMath::Clamp(static_cast<float>(FMath::Min(Fit.X, Fit.Y)), 1.f, MaxZoom);
	}
	else if (bNewBase)
	{
		Zoom = 1.f;
		CenterOnPlayer();
	}
}

void UFVMapViewWidget::CenterOnPlayer()
{
	const FFVMapLayer* Base = GetBaseLayer();
	const UFVNavigatorComponent* PlayerNavigator = GetNavigator();
	if (Mode != EFVMapViewMode::WorldMap || Base == nullptr)
	{
		return;
	}

	const FVector Location = PlayerNavigator ? PlayerNavigator->GetViewLocation() : FVector::ZeroVector;
	PanUV = PlayerNavigator && Base->ContainsXY(Location) ? Base->WorldToUV(Location) : FVector2D(0.5);
}

void UFVMapViewWidget::Pan(FVector2D DeltaPixels)
{
	if (Mode == EFVMapViewMode::WorldMap)
	{
		PanUV = FVMapView::ClampPan(PanUV - FVMapView::SafeDivide(DeltaPixels, ViewSize) * 2. * ExtentUV, ExtentUV);
	}
}

void UFVMapViewWidget::ZoomAt(FVector2D LocalPosition, float Factor)
{
	if (Factor <= 0.f)
	{
		return;
	}

	if (Mode == EFVMapViewMode::Minimap)
	{
		if (UFVNavigatorComponent* PlayerNavigator = GetNavigator())
		{
			PlayerNavigator->ZoomMinimap(1.f / Factor);
		}
		return;
	}

	if (GetBaseLayer() == nullptr || ViewSize.X <= 0. || ViewSize.Y <= 0.)
	{
		return;
	}

	const FVector2D Anchor = LocalToBaseUV(LocalPosition);
	Zoom = FMath::Clamp(Zoom * Factor, 1.f, MaxZoom);
	ExtentUV = GetWorldMapExtent(Zoom);
	PanUV = FVMapView::ClampPan(Anchor - (FVMapView::SafeDivide(LocalPosition, ViewSize) - FVector2D(0.5)) * 2. * ExtentUV, ExtentUV);
	CenterUV = PanUV;
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

FReply UFVMapViewWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (Mode != EFVMapViewMode::WorldMap)
	{
		return Super::NativeOnMouseWheel(InGeometry, InMouseEvent);
	}

	ZoomAt(InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition()), FMath::Pow(ZoomStep, InMouseEvent.GetWheelDelta()));
	return FReply::Handled();
}

FReply UFVMapViewWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (Mode != EFVMapViewMode::WorldMap || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	bPressed = true;
	DragDistance = 0.f;
	LastDragPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UFVMapViewWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bPressed)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	const FVector2D Local = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	const FVector2D Delta = Local - LastDragPosition;
	Pan(Delta);
	DragDistance += static_cast<float>(Delta.Size());
	LastDragPosition = Local;
	return FReply::Handled();
}

FReply UFVMapViewWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bPressed || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	bPressed = false;
	if (DragDistance < FVMapView::ClickTolerance)
	{
		const FVector2D Local = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		OnMapClicked(Local, LocalToWorld(Local));
	}
	return FReply::Handled().ReleaseMouseCapture();
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
		FVMapView::GetRoomArea(*Base, *Room, Min, Max);
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
	if (PlayerIcon && Mode == EFVMapViewMode::WorldMap)
	{
		PlayerIcon->SetVisibility(ESlateVisibility::Hidden);
	}
	OnShownMapChanged(nullptr, INDEX_NONE);
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

void UFVMapViewWidget::PlaceMarker(const FFVMarkerView& Marker, const FVector2D& Position, float Angle)
{
	PlacedMarkers.Emplace(Marker, Position);
	if (MarkerCanvas)
	{
		MarkerPool.Place(*this, *MarkerCanvas, MarkerWidgetClass, Marker, Position, Angle);
	}
}

FVector2D UFVMapViewWidget::GetWorldMapExtent(float InZoom) const
{
	// Keeps the image's proportions: the axis the view is relatively longer on shows more of the image.
	const FFVMapLayer* Base = GetBaseLayer();
	const FVector2D Size = Base ? Base->GetWorldSize() : FVector2D(1.);
	const double ImageAspect = Size.X > 0. ? Size.Y / Size.X : 1.;
	const double ViewAspect = ViewSize.Y > 0. ? ViewSize.X / ViewSize.Y : 1.;
	const double Aspect = FMath::Max(ViewAspect / ImageAspect, 1e-4);
	const double Half = 0.5 / FMath::Max(static_cast<double>(InZoom), 1e-4);
	return FVector2D(Half * FMath::Max(Aspect, 1.), Half * FMath::Max(1. / Aspect, 1.));
}

FVector2D UFVMapViewWidget::BaseUVToLocal(const FVector2D& UV) const
{
	// Inverse of the map material's rotation, see the setup guide.
	const FVector2D Q = FVMapView::SafeDivide(UV - CenterUV, ExtentUV);
	double Sin = 0.;
	double Cos = 1.;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(static_cast<double>(Rotation)));
	const FVector2D P(Cos * Q.X - Sin * Q.Y, Sin * Q.X + Cos * Q.Y);
	return (P * 0.5 + FVector2D(0.5)) * ViewSize;
}

FVector2D UFVMapViewWidget::LocalToBaseUV(const FVector2D& Local) const
{
	const FVector2D P = (FVMapView::SafeDivide(Local, ViewSize) - FVector2D(0.5)) * 2.;
	double Sin = 0.;
	double Cos = 1.;
	FMath::SinCos(&Sin, &Cos, FMath::DegreesToRadians(static_cast<double>(Rotation)));
	const FVector2D Q(P.X * Cos + P.Y * Sin, -P.X * Sin + P.Y * Cos);
	return CenterUV + Q * ExtentUV;
}

const FFVMapLayer* UFVMapViewWidget::GetBaseLayer() const
{
	return ShownMap ? ShownMap->GetLayer(BaseLayer) : nullptr;
}

UFVNavigatorComponent* UFVMapViewWidget::GetNavigator() const
{
	if (!Navigator.IsValid())
	{
		Navigator = UFVNavigatorComponent::FindForPlayer(GetOwningPlayer());
	}
	return Navigator.Get();
}
