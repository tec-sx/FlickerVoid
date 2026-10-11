#include "UI/FVWorldMapWidget.h"

#include "FVMapDefinition.h"
#include "FVNavigationSubsystem.h"
#include "FVNavigatorComponent.h"
#include "InputCoreTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FVWorldMapWidget)

namespace FVWorldMap
{
	/** Mouse travel in pixels below which a press and release counts as a click. */
	static constexpr float ClickTolerance = 4.f;

	static FVector2D ClampPan(const FVector2D& PanUV, const FVector2D& ExtentUV)
	{
		auto ClampAxis = [](double Pan, double Extent) { return Extent >= 0.5 ? 0.5 : FMath::Clamp(Pan, Extent, 1. - Extent); };
		return FVector2D(ClampAxis(PanUV.X, ExtentUV.X), ClampAxis(PanUV.Y, ExtentUV.Y));
	}
}

void UFVWorldMapWidget::NativeConstruct()
{
	Super::NativeConstruct();
	SetVisibility(ESlateVisibility::Visible);
	ShowMap(nullptr, INDEX_NONE);
}

void UFVWorldMapWidget::UpdateView(float DeltaTime)
{
	const FFVMapLayer* Base = GetBaseLayer();
	if (Base == nullptr)
	{
		if (PlayerIcon)
		{
			PlayerIcon->SetVisibility(ESlateVisibility::Hidden);
		}
		return;
	}

	const FVector2D Extent = GetExtent(Zoom);
	PanUV = FVWorldMap::ClampPan(PanUV, Extent);
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

	BeginMarkers();
	for (const FFVMarkerView& Marker : View.Markers)
	{
		const FVector2D Local = ToLocal(Marker.Position);
		if (Local.X >= 0. && Local.Y >= 0. && Local.X <= ViewSize.X && Local.Y <= ViewSize.Y)
		{
			PlaceMarker(Marker, Local, Marker.Rotation);
		}
	}
	EndMarkers();
}

void UFVWorldMapWidget::ShowMap(UFVMapDefinition* Map, int32 LayerIndex)
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
	if (Layer->bLimitHeight)
	{
		const FVector2D LayerMiddle = (Layer->WorldMin + Layer->WorldMax) * 0.5;
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
		GetRoomArea(*Map->GetLayer(Base), *Layer, Min, Max);
		PanUV = (Min + Max) * 0.5;
		const FVector2D RoomExtent = FVector2D::Max((Max - Min) * 0.6, FVector2D(1e-4));
		const FVector2D Fit = GetExtent(1.f) / RoomExtent;
		Zoom = FMath::Clamp(static_cast<float>(FMath::Min(Fit.X, Fit.Y)), 1.f, MaxZoom);
	}
	else if (bNewBase)
	{
		Zoom = 1.f;
		CenterOnPlayer();
	}
}

void UFVWorldMapWidget::CenterOnPlayer()
{
	const FFVMapLayer* Base = GetBaseLayer();
	if (Base == nullptr)
	{
		return;
	}

	const UFVNavigatorComponent* PlayerNavigator = GetNavigator();
	const FVector Location = PlayerNavigator ? PlayerNavigator->GetViewLocation() : FVector::ZeroVector;
	PanUV = PlayerNavigator && Base->ContainsXY(Location) ? Base->WorldToUV(Location) : FVector2D(0.5);
}

void UFVWorldMapWidget::Pan(FVector2D DeltaPixels)
{
	if (ViewSize.X > 0. && ViewSize.Y > 0.)
	{
		PanUV = FVWorldMap::ClampPan(PanUV - DeltaPixels / ViewSize * 2. * ExtentUV, ExtentUV);
	}
}

void UFVWorldMapWidget::ZoomAt(FVector2D LocalPosition, float Factor)
{
	if (Factor <= 0.f || GetBaseLayer() == nullptr || ViewSize.X <= 0. || ViewSize.Y <= 0.)
	{
		return;
	}

	const FVector2D Anchor = LocalToBaseUV(LocalPosition);
	Zoom = FMath::Clamp(Zoom * Factor, 1.f, MaxZoom);
	ExtentUV = GetExtent(Zoom);
	PanUV = FVWorldMap::ClampPan(Anchor - (LocalPosition / ViewSize - FVector2D(0.5)) * 2. * ExtentUV, ExtentUV);
	CenterUV = PanUV;
}

FReply UFVWorldMapWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	ZoomAt(InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition()), FMath::Pow(ZoomStep, InMouseEvent.GetWheelDelta()));
	return FReply::Handled();
}

FReply UFVWorldMapWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
	}

	bPressed = true;
	DragDistance = 0.f;
	LastDragPosition = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
	return FReply::Handled().CaptureMouse(TakeWidget());
}

FReply UFVWorldMapWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
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

FReply UFVWorldMapWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bPressed || InMouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
	}

	bPressed = false;
	if (DragDistance < FVWorldMap::ClickTolerance)
	{
		const FVector2D Local = InGeometry.AbsoluteToLocal(InMouseEvent.GetScreenSpacePosition());
		OnMapClicked(Local, LocalToWorld(Local));
	}
	return FReply::Handled().ReleaseMouseCapture();
}

FVector2D UFVWorldMapWidget::GetExtent(float InZoom) const
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
