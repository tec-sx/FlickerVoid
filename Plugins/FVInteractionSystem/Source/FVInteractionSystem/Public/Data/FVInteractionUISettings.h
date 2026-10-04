#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

#include "FVInteractionUISettings.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UUserWidget;
class UTexture2D;

/** Crosshair override. Row name = interactable type tag (e.g. Interactable.Door). */
USTRUCT(BlueprintType)
struct FFVInteractionCrosshairRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> Icon;
};

UCLASS(MinimalAPI, BlueprintType, meta = (DisplayName = "Interaction UI Settings"))
class UFVInteractionUISettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UUserWidget> WidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> DefaultCrosshair;

	/** Per interactable type crosshair overrides. Parent tags are used as fallback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (RequiredAssetDataTags = "RowStructure=/Script/FVInteractionSystem.FVInteractionCrosshairRow"))
	TSoftObjectPtr<UDataTable> CrosshairOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;
};

#undef UE_API
