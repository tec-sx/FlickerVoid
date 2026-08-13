#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Interactions/FVInteractionTypes.h"
#include "FVInteractionAction.generated.h"

struct FFVInteractionActionInfo;

UCLASS()
class FLICKERVOIDGAMEPLAY_API UFVInteractionAction : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Ability.Interact"))
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EFVInteractionSlot Slot = EFVInteractionSlot::Primary;

	UFUNCTION(BlueprintCallable)
	FFVInteractionActionInfo CreateActionUIInfo() const;
};
