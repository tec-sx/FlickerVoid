#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FVNavigationTypes.h"
#include "UI/FVMarkerWidgetPool.h"
#include "FVMapViewWidget.generated.h"

class UCanvasPanel;
class UFVMapDefinition;
class UFVMapMarkerWidget;
class UFVNavigatorComponent;
class UImage;
class UMaterialInstanceDynamic;
class UTexture2D;
struct FFVMapLayer;
struct FStreamableHandle;

/**
 * Base of the minimap and world map widgets: draws a map layer through the material on MapImage, a room layer over its
 * dimmed base layer, and places marker icons on MarkerCanvas.
 */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVMapViewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "FV|Navigation", meta = (ToolTip = "World location under a point of this widget, at the player's height."))
	FVector LocalToWorld(FVector2D LocalPosition) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation", meta = (ToolTip = "Marker closest to LocalPosition within MaxDistance pixels; the top-most wins a tie."))
	bool FindMarkerAt(FVector2D LocalPosition, float MaxDistance, FFVMarkerView& OutMarker) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	UFVMapDefinition* GetShownMap() const { return ShownMap; }

	UFUNCTION(BlueprintPure, Category = "FV|Navigation", meta = (ToolTip = "The room layer while one is drawn, otherwise the base layer."))
	int32 GetShownLayer() const { return RoomLayer != INDEX_NONE ? RoomLayer : BaseLayer; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Runs every tick once the widget has a size and a map material. */
	virtual void UpdateView(float DeltaTime) {}

	UFUNCTION(BlueprintPure, Category = "FV|Navigation", meta = (ToolTip = "The navigator on the owning player's controller, or on their pawn."))
	UFVNavigatorComponent* GetNavigator() const;

	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Navigation", meta = (ToolTip = "Map is null when nothing is shown, e.g. the player is outside every map."))
	void OnShownMapChanged(UFVMapDefinition* Map, int32 LayerIndex);

	/** Room layer INDEX_NONE shows only the base layer. */
	void ShowLayers(UFVMapDefinition* Map, int32 InBaseLayer, int32 InRoomLayer);
	void ClearMap();
	void ApplyView(const FVector2D& InCenterUV, const FVector2D& InExtentUV, float InRotation, float DeltaTime);

	void BeginMarkers();
	void PlaceMarker(const FFVMarkerView& Marker, const FVector2D& Position, float Angle);
	void EndMarkers();

	const FFVMapLayer* GetBaseLayer() const;
	FVector2D BaseUVToLocal(const FVector2D& UV) const;
	FVector2D LocalToBaseUV(const FVector2D& Local) const;

	/** Room area in base layer UV: north-west and south-east corners. */
	static void GetRoomArea(const FFVMapLayer& Base, const FFVMapLayer& Room, FVector2D& OutMin, FVector2D& OutMax);

private:
	void LoadTexture(const FFVMapLayer* Layer, TSharedPtr<FStreamableHandle>& Handle, TFunction<void(UTexture2D*)>&& OnLoaded);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View", meta = (ToolTip = "Icon widget created for each marker."))
	TSubclassOf<UFVMapMarkerWidget> MarkerWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View", meta = (ClampMin = 0, ClampMax = 1, ToolTip = "How dark the base layer gets while a room shows over it."))
	float RoomDim = 0.6f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View", meta = (ClampMin = 0, ToolTip = "Speed of the zoom and dim transitions; 0 snaps."))
	float TransitionSpeed = 6.f;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidget, ToolTip = "Fills the widget; its brush is the map material."))
	TObjectPtr<UImage> MapImage;

	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional, ToolTip = "Fills the widget and holds the marker icons."))
	TObjectPtr<UCanvasPanel> MarkerCanvas;

	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional, ToolTip = "Player arrow, laid out at the centre."))
	TObjectPtr<UWidget> PlayerIcon;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Material;

	UPROPERTY(Transient)
	TObjectPtr<UFVMapDefinition> ShownMap;

	int32 BaseLayer = INDEX_NONE;
	int32 RoomLayer = INDEX_NONE;
	FVector2D ViewSize = FVector2D::ZeroVector;
	FVector2D CenterUV = FVector2D(0.5);
	FVector2D ExtentUV = FVector2D(0.5);
	float Rotation = 0.f;

private:
	mutable TWeakObjectPtr<UFVNavigatorComponent> Navigator;
	TSharedPtr<FStreamableHandle> BaseTextureHandle;
	TSharedPtr<FStreamableHandle> RoomTextureHandle;
	FFVMarkerWidgetPool MarkerPool;
	TArray<TPair<FFVMarkerView, FVector2D>> PlacedMarkers;
	float Dim = 0.f;
	float TargetDim = 0.f;
};
