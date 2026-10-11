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

UENUM(BlueprintType)
enum class EFVMapViewMode : uint8
{
	/** Follows the player with the navigator's minimap view. */
	Minimap,
	/** Shows a chosen map and layer that the player pans and zooms, e.g. the full-screen map. */
	WorldMap
};

/**
 * Draws a map through the material on MapImage and places marker icons on MarkerCanvas. A room layer is drawn over
 * its base layer, which dims while the room shows. Minimap mode follows the navigator, including its room zoom;
 * World Map mode pans with a mouse drag and zooms with the wheel. The Widget Blueprint provides the layout, the material
 * and the marker icon class.
 */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVMapViewWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** World map: shows a map and layer; a null Map shows the player's current one. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void ShowMap(UFVMapDefinition* Map, int32 LayerIndex);

	/** World map: moves the view to the player. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void CenterOnPlayer();

	/** World map: moves the view as a drag of DeltaPixels would. */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void Pan(FVector2D DeltaPixels);

	/**
	 * Factor above 1 zooms in. World map: keeps the point under LocalPosition in place.
	 * Minimap: changes the navigator's minimap radius and ignores LocalPosition.
	 */
	UFUNCTION(BlueprintCallable, Category = "FV|Navigation")
	void ZoomAt(FVector2D LocalPosition, float Factor);

	/** World location under a point of this widget, e.g. for a waypoint, at the player's height. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	FVector LocalToWorld(FVector2D LocalPosition) const;

	/** Marker closest to LocalPosition within MaxDistance pixels; the one drawn on top wins a tie. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	bool FindMarkerAt(FVector2D LocalPosition, float MaxDistance, FFVMarkerView& OutMarker) const;

	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	UFVMapDefinition* GetShownMap() const { return ShownMap; }

	/** The room layer while one is drawn, otherwise the base layer. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	int32 GetShownLayer() const { return RoomLayer != INDEX_NONE ? RoomLayer : BaseLayer; }

	/** World map zoom; 1 fits the whole base layer. */
	UFUNCTION(BlueprintPure, Category = "FV|Navigation")
	float GetZoom() const { return Zoom; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View")
	EFVMapViewMode Mode = EFVMapViewMode::Minimap;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View")
	TSubclassOf<UFVMapMarkerWidget> MarkerWidgetClass;

	/** DimAmount of the base layer while a room shows over it: 0 keeps it, 1 is black. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View", meta = (ClampMin = 0, ClampMax = 1))
	float RoomDim = 0.6f;

	/** Speed of the zoom and dim transitions; 0 snaps. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View", meta = (ClampMin = 0))
	float TransitionSpeed = 6.f;

	/** Pixels between the rim and the markers pinned to it. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View|Minimap", meta = (ClampMin = 0))
	float EdgePadding = 12.f;

	/** Turns PlayerIcon to where the character faces on the map; otherwise it always points up. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View|Minimap")
	bool bRotatePlayerIcon = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View|World Map", meta = (ClampMin = 1))
	float MaxZoom = 8.f;

	/** Zoom factor per mouse wheel step. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Map View|World Map", meta = (ClampMin = 1))
	float ZoomStep = 1.15f;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	/** Map is null when the player stands outside every map. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Navigation")
	void OnShownMapChanged(UFVMapDefinition* Map, int32 LayerIndex);

	/** World map: a click that wasn't a drag. */
	UFUNCTION(BlueprintImplementableEvent, Category = "FV|Navigation")
	void OnMapClicked(FVector2D LocalPosition, FVector WorldLocation);

	/** Fills this widget; its brush is the map material. */
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidget))
	TObjectPtr<UImage> MapImage;

	/** Fills this widget and holds the marker icons. */
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> MarkerCanvas;

	/** Laid out at the centre. The world map moves it to the player and hides it while the player is on another layer. */
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> PlayerIcon;

private:
	void UpdateMinimap(float DeltaTime);
	void UpdateWorldMap(float DeltaTime);
	void ShowLayers(UFVMapDefinition* Map, int32 InBaseLayer, int32 InRoomLayer);
	void ClearMap();
	void LoadTexture(const FFVMapLayer* Layer, TSharedPtr<FStreamableHandle>& Handle, TFunction<void(UTexture2D*)>&& OnLoaded);
	void ApplyView(const FVector2D& InCenterUV, const FVector2D& InExtentUV, float InRotation, float DeltaTime);
	void PlaceMarker(const FFVMarkerView& Marker, const FVector2D& Position, float Angle);
	FVector2D GetWorldMapExtent(float InZoom) const;
	FVector2D BaseUVToLocal(const FVector2D& UV) const;
	FVector2D LocalToBaseUV(const FVector2D& Local) const;
	const FFVMapLayer* GetBaseLayer() const;
	UFVNavigatorComponent* GetNavigator() const;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> Material;

	UPROPERTY(Transient)
	TObjectPtr<UFVMapDefinition> ShownMap;

	mutable TWeakObjectPtr<UFVNavigatorComponent> Navigator;
	int32 BaseLayer = INDEX_NONE;
	int32 RoomLayer = INDEX_NONE;
	TSharedPtr<FStreamableHandle> BaseTextureHandle;
	TSharedPtr<FStreamableHandle> RoomTextureHandle;

	FFVMarkerWidgetPool MarkerPool;
	TArray<TPair<FFVMarkerView, FVector2D>> PlacedMarkers;

	FVector2D ViewSize = FVector2D::ZeroVector;
	FVector2D CenterUV = FVector2D(0.5);
	FVector2D ExtentUV = FVector2D(0.5);
	float Rotation = 0.f;
	float Radius = 0.f;
	float Dim = 0.f;
	float TargetDim = 0.f;

	float Zoom = 1.f;
	FVector2D PanUV = FVector2D(0.5);
	FVector2D LastDragPosition = FVector2D::ZeroVector;
	float DragDistance = 0.f;
	bool bPressed = false;
};
