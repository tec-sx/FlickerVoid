#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FVMapCaptureActor.generated.h"

class UBoxComponent;
class UFVMapDefinition;
class USceneCaptureComponent2D;

/** Actors a map capture leaves out. */
USTRUCT(BlueprintType)
struct FVNAVIGATIONSYSTEM_API FFVMapCaptureFilter
{
	GENERATED_BODY()

	/** Also leave out the classes and tags listed under Capture in Navigation settings. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bUseProjectDefaults = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TArray<TSubclassOf<AActor>> IgnoredClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TArray<FName> IgnoredActorTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	TArray<TSoftObjectPtr<AActor>> IgnoredActors;

	/** Leave out actors entirely above the box, such as the roof over an interior floor. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter")
	bool bIgnoreActorsAboveBox = true;
};

/**
 * Top-down orthographic camera that renders a map layer image. The box sets the area and the camera height.
 * Capturing (from the Map Capture panel or the button in Details) saves a texture and writes it, with the
 * captured area, into the chosen layer of the map definition. Editor only; it is stripped from cooked builds.
 */
UCLASS(Blueprintable, HideCategories = (Collision, Physics, Networking, Replication, Input, LOD, Cooking, HLOD))
class FVNAVIGATIONSYSTEM_API AFVMapCaptureActor : public AActor
{
	GENERATED_BODY()

public:
	AFVMapCaptureActor();

	UBoxComponent* GetCaptureBox() const { return CaptureBox; }
	USceneCaptureComponent2D* GetCaptureComponent() const { return CaptureComponent; }

	/** World box the capture covers, axis aligned. */
	FBox GetCaptureBounds() const;

	/** Render target size: Resolution along the longer side of the box, rounded to multiples of 4. */
	FIntPoint GetCaptureSize() const;

	/** World XY the image covers exactly, after rounding the image size. */
	FBox2D GetCapturedArea() const;

	/** Points the camera down from the top of the box and hides the ignored actors. */
	void PrepareCapture();

	void GatherIgnoredActors(TArray<AActor*>& OutActors) const;

	UFUNCTION()
	TArray<FString> GetLayerNames() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	TObjectPtr<UFVMapDefinition> Map;

	/** Layer of Map that receives the image and area; it is added when missing. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture", meta = (GetOptions = "GetLayerNames"))
	FName Layer = TEXT("Ground");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture", meta = (ClampMin = 64, ClampMax = 16384))
	int32 Resolution = 2048;

	/** Texture asset name, saved to the Capture Folder of Navigation settings. Empty uses T_<Map>_<Layer>. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FString TextureName;

	/**
	 * The layer only covers the box's height range and wins over layers without it while the player is inside.
	 * Use it for interiors and floors; leave it off for the outdoor ground layer.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	bool bLimitHeight = false;

	/** Stop rendering at the bottom of the box, so floors below don't show through. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	bool bClipBelowBox = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Capture")
	FFVMapCaptureFilter Filter;

#if WITH_EDITOR
	DECLARE_MULTICAST_DELEGATE_OneParam(FFVOnMapCaptureRequested, AFVMapCaptureActor*);

	/** Bound by the editor module, which renders, saves the texture and updates the map definition. */
	static FFVOnMapCaptureRequested OnCaptureRequested;

	UFUNCTION(CallInEditor, Category = "Map Capture")
	void CaptureMap();
#endif

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Capture")
	TObjectPtr<UBoxComponent> CaptureBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Map Capture")
	TObjectPtr<USceneCaptureComponent2D> CaptureComponent;
};
