#pragma once

#include "CoreMinimal.h"
#include "Data/FVCharacterDefinition.h"
#include "GameplayTagContainer.h"
#include "FVItemAnimationFragment.generated.h"

class UAnimMontage;
class UAnimSequenceBase;

/** A character's montages and poses for item styles (Animation.Item.*). Styles it lacks fall back to the Items settings. */
USTRUCT(BlueprintType, meta = (DisplayName = "Item Animations"))
struct FLICKERVOIDGAMEPLAY_API FFVCharacterFragment_ItemAnimations : public FFVCharacterFragment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Animations", meta = (ForceInlineRow, Categories = "Animation.Item", ToolTip = "Montages by style: use, pickup low/mid/high, give, throw. A style without an entry uses its closest parent's."))
	TMap<FGameplayTag, TObjectPtr<UAnimMontage>> Montages;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Animations", meta = (ForceInlineRow, Categories = "Animation.Item.Hold", ToolTip = "Upper body poses by hold style. A style without an entry uses its closest parent's."))
	TMap<FGameplayTag, TObjectPtr<UAnimSequenceBase>> HoldPoses;
};
