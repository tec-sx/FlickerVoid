#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"

#include "FVInteractionUISettings.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UUserWidget;

/** Focus indicator brush override. Row name = interactable type tag (e.g. Interactable.Door). */
USTRUCT(BlueprintType)
struct FFVInteractionFocusIndicatorRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FSlateBrush Brush;
};

UCLASS(MinimalAPI, BlueprintType, meta = (DisplayName = "Interaction UI Settings"))
class UFVInteractionUISettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UUserWidget> WidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	FSlateBrush DefaultFocusIndicatorBrush;

	/** Per interactable type focus indicator brushes. Parent tags are used as fallback. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (RequiredAssetDataTags = "RowStructure=/Script/FVInteractionSystem.FVInteractionFocusIndicatorRow"))
	TSoftObjectPtr<UDataTable> FocusIndicatorOverrides;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;
};

#undef UE_API
