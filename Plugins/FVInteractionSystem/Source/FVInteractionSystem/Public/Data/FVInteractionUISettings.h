#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"

#include "FVInteractionUISettings.generated.h"

#define UE_API FVINTERACTIONSYSTEM_API

class UUserWidget;

UCLASS(MinimalAPI, BlueprintType, meta = (DisplayName = "Interaction UI Settings"))
class UFVInteractionUISettings : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftClassPtr<UUserWidget> WidgetClass;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSoftObjectPtr<UTexture2D> DefaultCrosshairIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI", meta = (Categories = "UI.Layer"))
	FGameplayTag LayerTag;
};

#undef UE_API
