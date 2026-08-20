#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UI/FVInteractionInfo.h"
#include "FVInteractionConfig.generated.h"

USTRUCT(BlueprintType)
struct FLICKERVOIDGAMEPLAY_API FFVInteractionConfig
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "Interaction.Action"))
	FGameplayTag AbilityTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> Icon;

	bool IsValid() const { return AbilityTag.IsValid(); }

	FFVInteractionInfo CreateUIInfo(EFVInteractionSlot Slot) const;
};
