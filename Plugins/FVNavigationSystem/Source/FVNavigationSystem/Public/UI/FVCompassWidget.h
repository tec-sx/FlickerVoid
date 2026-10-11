#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/FVMarkerWidgetPool.h"
#include "FVCompassWidget.generated.h"

class UCanvasPanel;
class UFVMapMarkerWidget;
class UFVNavigatorComponent;
class UImage;
class UMaterialInstanceDynamic;

UENUM(BlueprintType)
enum class EFVCompassStyle : uint8
{
	/** A ring around the minimap that turns with it. */
	Ring,
	/** A horizontal strip that scrolls with the view heading. */
	Strip
};

/**
 * Compass driven by the navigator's compass view: turns Ring or scrolls the material on Strip, and places compass
 * markers on MarkerCanvas. The Widget Blueprint provides the layout, the ring or strip art and the marker icon class.
 */
UCLASS(Abstract)
class FVNAVIGATIONSYSTEM_API UFVCompassWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compass")
	EFVCompassStyle Style = EFVCompassStyle::Ring;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compass")
	TSubclassOf<UFVMapMarkerWidget> MarkerWidgetClass;

	/** Distance of markers from the centre in pixels; 0 is half the widget's smaller side. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Compass", meta = (ClampMin = 0, EditCondition = "Style == EFVCompassStyle::Ring"))
	float MarkerRadius = 0.f;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	/** Ring art with north at the top. Turns with the minimap, or stays north-up when the minimap does. */
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional))
	TObjectPtr<UWidget> Ring;

	/** Strip art: a material sampling a 360-degree texture, north at U = 0, at U * CompassTiling + CompassOffset. */
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional))
	TObjectPtr<UImage> Strip;

	/** Fills this widget and holds the marker icons. */
	UPROPERTY(BlueprintReadOnly, Category = "FV|Navigation", meta = (BindWidgetOptional))
	TObjectPtr<UCanvasPanel> MarkerCanvas;

private:
	UFVNavigatorComponent* GetNavigator();

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> StripMaterial;

	TWeakObjectPtr<UFVNavigatorComponent> Navigator;
	FFVMarkerWidgetPool MarkerPool;
};
